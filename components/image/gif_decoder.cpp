#include "gif_decoder.hpp"

static inline uint16_t readU16LE(const uint8_t* p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static inline uint16_t toRGB565(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint16_t)(r & 0xF8) << 8) | ((uint16_t)(g & 0xFC) << 3) | (b >> 3);
}

GifDecoder::GifDecoder(int width, int height)
    : _width(width), _height(height) {}

GifDecoder::Error GifDecoder::decode(const uint8_t* data, size_t size, uint16_t* output) {
    if (size < 13) return Error::InvalidData;

    if (data[0] != 'G' || data[1] != 'I' || data[2] != 'F' ||
        data[3] != '8' || (data[4] != '7' && data[4] != '9') || data[5] != 'a')
        return Error::InvalidSignature;

    if (readU16LE(data + 6) != (uint16_t)_width ||
        readU16LE(data + 8) != (uint16_t)_height)
        return Error::SizeMismatch;

    uint8_t lsdPacked = data[10];
    bool    hasGct    = (lsdPacked >> 7) & 1;
    int     gctBytes  = hasGct ? (3 << ((lsdPacked & 7) + 1)) : 0;
    const uint8_t* gct = hasGct ? (data + 13) : nullptr;
    size_t pos = 13 + (size_t)gctBytes;

    bool    hasTransp = false;
    uint8_t transpIdx = 0;

    while (pos < size) {
        uint8_t blockId = data[pos++];

        if (blockId == 0x3B) break;  // GIF trailer

        if (blockId == 0x21) {  // extension introducer
            if (pos >= size) return Error::InvalidData;
            uint8_t label = data[pos++];

            if (label == 0xF9) {  // Graphic Control Extension
                if (pos + 6 > size) return Error::InvalidData;
                if (data[pos++] != 4) return Error::InvalidData;
                uint8_t packed = data[pos++];
                hasTransp = (packed & 1) != 0;
                pos += 2;  // delay time
                transpIdx = data[pos++];
                pos++;     // block terminator
            } else {
                while (pos < size) {
                    uint8_t len = data[pos++];
                    if (!len) break;
                    if (pos + len > size) return Error::InvalidData;
                    pos += len;
                }
            }
            continue;
        }

        if (blockId != 0x2C) return Error::InvalidData;  // expected image descriptor

        if (pos + 9 > size) return Error::InvalidData;
        pos += 8;  // left, top, width, height — checked via LSD
        uint8_t imgPacked  = data[pos++];
        bool    hasLct     = (imgPacked >> 7) & 1;
        bool    interlaced = (imgPacked >> 6) & 1;
        int     lctBytes   = hasLct ? (3 << ((imgPacked & 7) + 1)) : 0;

        // Reference color table directly in data buffer — no copy
        const uint8_t* ct = hasLct ? (data + pos) : gct;
        if (!ct) return Error::InvalidData;
        if (hasLct && pos + (size_t)lctBytes > size) return Error::InvalidData;
        pos += (size_t)lctBytes;

        if (pos >= size) return Error::InvalidData;
        int lzwMin = data[pos++];
        if (lzwMin < 2 || lzwMin > 8) return Error::InvalidData;

        // ---- LZW decode ----
        const int clearCode = 1 << lzwMin;
        const int eofCode   = clearCode + 1;
        int nextCode = clearCode + 2;
        int codeSize = lzwMin + 1;
        int maxCode  = 1 << codeSize;
        int prevCode = -1;
        uint8_t firstChar = 0;

        // Sub-block-aware bit reader state
        uint32_t bitBuf   = 0;
        int      bitCount = 0;
        int      blockRem = 0;

        auto fillBits = [&]() -> bool {
            while (bitCount < 16) {
                if (blockRem == 0) {
                    if (pos >= size) return bitCount > 0;
                    blockRem = data[pos++];
                    if (!blockRem) return false;
                }
                if (pos >= size) return false;
                bitBuf |= (uint32_t)data[pos++] << bitCount;
                bitCount += 8;
                --blockRem;
            }
            return true;
        };

        auto readCode = [&]() -> int {
            if (bitCount < codeSize && !fillBits()) return -1;
            if (bitCount < codeSize) return -1;
            int v = (int)(bitBuf & ((1u << codeSize) - 1));
            bitBuf >>= codeSize;
            bitCount -= codeSize;
            return v;
        };

        const int totalPixels = _width * _height;
        int pixelOut = 0;

        static constexpr int iStart[] = {0, 4, 2, 1};
        static constexpr int iStep[]  = {8, 8, 4, 2};
        int iPass = 0, iRow = 0, iCol = 0;

        auto putPixel = [&](uint8_t idx) {
            if (pixelOut >= totalPixels) return;
            int out;
            if (!interlaced) {
                out = pixelOut;
            } else {
                if (iRow >= _height) { ++pixelOut; return; }
                out = iRow * _width + iCol;
                if (++iCol == _width) {
                    iCol = 0;
                    iRow += iStep[iPass];
                    while (iPass < 3 && iRow >= _height)
                        iRow = iStart[++iPass];
                }
            }
            ++pixelOut;
            if (hasTransp && idx == transpIdx) return;  // leave output[out] unchanged
            const uint8_t* rgb = ct + (int)idx * 3;
            output[out] = toRGB565(rgb[0], rgb[1], rgb[2]);
        };

        for (;;) {
            int code = readCode();
            if (code < 0) break;

            if (code == clearCode) {
                nextCode = clearCode + 2;
                codeSize = lzwMin + 1;
                maxCode  = 1 << codeSize;
                prevCode = -1;
                continue;
            }
            if (code == eofCode) break;

            if (prevCode < 0) {
                // First symbol after reset — must be a root entry
                firstChar = (uint8_t)code;
                putPixel(firstChar);
                prevCode = code;
                continue;
            }

            // Special case: code not yet added to dictionary
            // String = string(prevCode) + firstChar(prevCode)
            bool isNew = (code == nextCode);
            if (code > nextCode || (isNew && nextCode >= MAX_CODES)) break;

            if (isNew) {
                _prefix[nextCode] = (uint16_t)prevCode;
                _suffix[nextCode] = firstChar;  // firstChar of prevCode's string
            }

            // Walk prefix chain and push to stack
            int top = 0;
            int e   = code;
            while (e >= clearCode + 2) {
                if (top >= MAX_CODES) { e = -1; break; }
                _stack[top++] = _suffix[e];
                e = (int)_prefix[e];
            }
            if (e < 0 || e >= clearCode) break;  // corrupted stream
            _stack[top++] = (uint8_t)e;
            firstChar = _stack[top - 1];  // first char of decoded string (root)

            // Output pixels in forward order
            for (int i = top - 1; i >= 0; --i)
                putPixel(_stack[i]);

            // Add new dictionary entry
            if (!isNew && nextCode < MAX_CODES) {
                _prefix[nextCode] = (uint16_t)prevCode;
                _suffix[nextCode] = firstChar;
            }
            if (nextCode < MAX_CODES) {
                ++nextCode;
                if (nextCode == maxCode && codeSize < 12) {
                    ++codeSize;
                    maxCode <<= 1;
                }
            }

            prevCode = code;
        }

        return Error::None;
    }

    return Error::NoImageData;
}
