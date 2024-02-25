/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include <cstdint>
#include <cstring>
#include <memory>
#include <functional>

namespace GIF {
class _BinaryDecoder {
public:
    uint8_t *data;
    int dataLength;
    _BinaryDecoder(uint8_t *data, int dataLength) : data(data), dataLength(dataLength) {}
    inline uint8_t bit(int offset, int bit, int size, uint8_t def = 0) {
        if (offset < 0 || offset >= dataLength) return def;
        uint8_t mask = (1 << size) - 1;
        return (data[offset] >> bit) & mask;
    }
    inline uint8_t u8(int offset, uint8_t def = 0) {
        if (offset < 0 || offset >= dataLength) return def;
        return data[offset];
    }
    inline uint16_t u16(int offset, uint16_t def = 0) {
        if (offset < 0 || offset >= dataLength) return def;
        return (((uint16_t)data[offset + 1]) << 8) + data[offset];
    }
};

class _SubBlockConsumer {
private:
    uint8_t *data;
    int maxLength;
    int currentBlock = 0;
    int currentOffset = 0;
    bool end = false;
public:
    _SubBlockConsumer(uint8_t *data, int offset, int dataLength) {
        this->data = data + offset;
        this->maxLength = dataLength - offset;
        if (this->maxLength <= 0) end = true;
    }
    bool consume(uint8_t *value) {
        if (end) return false;
        if (currentBlock + currentOffset + 1 >= maxLength) return false;
        *value = data[currentBlock + currentOffset + 1];
        currentOffset++;
        if (currentOffset == data[currentBlock]) {
            currentBlock += data[currentBlock] + 1;
            currentOffset = 0;
            if (data[currentBlock] == 0) end = true;
        }
        return true;
    }
};

class _SubBlockBitConsumer : private _SubBlockConsumer {
private:
    int remainBits = 0;
    uint8_t currentByte = 0;
public:
    _SubBlockBitConsumer(uint8_t *data, int offset, int maxLength) : _SubBlockConsumer(data, offset, maxLength) {}
    bool consume(int16_t *value, int size) {
        if (remainBits == 0) {
            if (!_SubBlockConsumer::consume(&currentByte)) return false;
            remainBits = 8;
        }
        if (remainBits >= size) {
            *value = currentByte & ((1 << size) - 1);
            currentByte >>= size;
            remainBits -= size;
            return true;
        }
        int16_t currentByteValue, nextByteValue, shift = remainBits;
        if (!consume(&currentByteValue, remainBits)) return false;
        if (!consume(&nextByteValue, size - shift)) return false;
        *value = currentByteValue | (nextByteValue << shift);
        return true;
    }
};

struct _LZWDictionaryItem {
    uint8_t dataCode;
    int16_t prevLzwCode;
};

class _LZWDictionary {
public:
    int primaryCodeCount;
    int8_t blockCount = 0;
    _LZWDictionaryItem *blocks[16] = {};
    int16_t codeCount = 0;
    int16_t primaryCodeSize = 0;
    int16_t codeSize = 0;
    inline int blockIndex(int16_t lzwCode) { return (lzwCode - primaryCodeCount) / 256; }
    inline int blockItemIndex(int16_t lzwCode) { return (lzwCode - primaryCodeCount) % 256; }
    _LZWDictionary(int primaryCodeCount, int16_t primaryCodeSize) : primaryCodeCount(primaryCodeCount), primaryCodeSize(primaryCodeSize) {
        reset();
    }
    ~_LZWDictionary() {
        reset();
    }
    int16_t addTemorary(int16_t prevLzwCode) {
        if (codeCount >= 4096) return -1;
        int16_t lzwCode = codeCount;
        int index = blockIndex(codeCount), itemIndex = blockItemIndex(codeCount);
        if (index >= blockCount) {
            blocks[index] = new _LZWDictionaryItem[256]();
            blockCount++;
        }
        blocks[index][itemIndex].prevLzwCode = prevLzwCode;
        codeCount++;
        while ((1 << codeSize) < codeCount) codeSize++;
        return lzwCode;
    }
    void addCompletely(int16_t lzwCode, uint8_t dataCode) {
        int index = blockIndex(lzwCode), itemIndex = blockItemIndex(lzwCode);
        blocks[index][itemIndex].dataCode = dataCode;
    }
    void output(int16_t lzwCode, std::function<void(uint8_t)> emit) {
        if (lzwCode < primaryCodeCount) {
            emit(lzwCode);
            return;
        }
        std::vector<uint8_t> v = {};
        if (lzwCode == codeCount - 1) v.push_back(firstDataCode(lzwCode));
        else v.push_back(blocks[blockIndex(lzwCode)][blockItemIndex(lzwCode)].dataCode);
        while (lzwCode >= primaryCodeCount) {
            lzwCode = blocks[blockIndex(lzwCode)][blockItemIndex(lzwCode)].prevLzwCode;
            if (lzwCode < primaryCodeCount) v.push_back(lzwCode);
            else v.push_back(blocks[blockIndex(lzwCode)][blockItemIndex(lzwCode)].dataCode);
        }
        for (auto it = v.rbegin(); it != v.rend(); ++it) emit(*it);
    }
    uint8_t firstDataCode(int16_t lzwCode) {
        while (lzwCode >= primaryCodeCount) {
            int index = blockIndex(lzwCode), itemIndex = blockItemIndex(lzwCode);
            lzwCode = blocks[index][itemIndex].prevLzwCode;
        }
        return lzwCode;
    }
    void reset() {
        for (int i = 0; i < blockCount; i++) {
            delete[] blocks[i];
        }
        blockCount = 0;
        codeCount = primaryCodeCount;
        codeSize = primaryCodeSize + 1;
    }
};

class LZWDecoder {
public:
    _SubBlockBitConsumer consumer;
    int primaryCodeCount, minLzwCodeSize;
    int16_t clearCode, endCode;
    LZWDecoder(_SubBlockBitConsumer consumer, int dataCodeCount, int minLzwCodeSize) :
        consumer(consumer), minLzwCodeSize(minLzwCodeSize) {
        clearCode = dataCodeCount;
        endCode = clearCode + 1;
        primaryCodeCount = endCode + 1;
    }
    void decode(std::function<void(uint8_t)> emit) {
        _LZWDictionary dict(primaryCodeCount, minLzwCodeSize);
        int16_t lzwCode, prevLzwCode = -1;
        while (true) {
            if (!consumer.consume(&lzwCode, dict.codeSize)) break;
            if (lzwCode == clearCode) {
                dict.reset();
                prevLzwCode = -1;
                continue;
            }
            if (lzwCode == endCode) break;
            if (prevLzwCode >= 0) dict.addCompletely(prevLzwCode, dict.firstDataCode(lzwCode));
            dict.output(lzwCode, emit);
            prevLzwCode = dict.addTemorary(lzwCode);
        }
    }
};

class Decoder : private _BinaryDecoder {
    using _BinaryDecoder::_BinaryDecoder;
public:
    inline bool checkSignature() { return memcmp("GIF89a", data, 6) == 0; }
    inline uint16_t width() { return u16(6); }
    inline uint16_t height() { return u16(8); }
    inline int globalColorTableSize() { return hasGlobalColorTable() ? 1 << (int)(bit(10, 0, 3) + 1) : 0; }
    inline bool globalColorTableSorted() { return bit(10, 3, 1); }
    inline uint8_t colorResolution() { return bit(10, 4, 3) + 1; }
    inline bool hasGlobalColorTable() { return bit(10, 7, 1); }
    inline uint8_t backgroundColorIndex() { return u8(11); }
    inline uint8_t pixelAspectRatio() { return u8(12); }
    inline void globalColor(int index, uint8_t *r, uint8_t *g, uint8_t *b) {
        int offset = 13 + (index * 3);
        *r = u8(offset);
        *g = u8(offset + 1);
        *b = u8(offset + 2);
    };
    inline int headerSize() { return 13 + (globalColorTableSize() * 3); }

    int gceOffset = 0;
    int gce(int offset) {
        if (!gceOffset) {
            gceOffset = headerSize();
            if (u16(gceOffset) != 0xf921) gceOffset = -1;
        }
        return gceOffset >= 0 ? gceOffset + offset : -1;
    }

    int ibOffset = 0;
    inline int ib(int offset) {
        if (!ibOffset) {
            ibOffset = gce(8);
            if (ibOffset < 0) ibOffset = headerSize();
            if (u8(ibOffset) != 0x2c) ibOffset = -1;
        }
        int ibOffset = gce(8);
        if (ibOffset < 0) ibOffset = headerSize();
        if (u8(ibOffset) != 0x2c) return -1;
        return ibOffset >= 0 ? ibOffset + offset : -1;
    }
public:
    inline bool hasTransparentColor() { return bit(gce(3), 0, 1); }
    inline uint8_t transparentColorIndex() { return u8(gce(6)); }

    inline bool hasLocalColorTable() { return bit(ib(9), 7, 1); }
    inline void localColor(int index, uint8_t *r, uint8_t *g, uint8_t *b) {
        int offset = ib(10 + (index * 3));
        *r = u8(offset);
        *g = u8(offset + 1);
        *b = u8(offset + 2);
    }

    void decode(std::function<void(uint8_t)> byte) {
        int minLzwCodeSize = u8(ib(10));
        _SubBlockBitConsumer consumer(data, ib(11), dataLength);
        auto decoder = std::make_unique<LZWDecoder>(consumer, 1 << minLzwCodeSize, minLzwCodeSize);
        decoder->decode(byte);
    }

    void pixels(std::function<void(uint8_t, uint8_t, uint8_t, bool)> dot) {
        int startOffset = 10;
        if (hasLocalColorTable()) startOffset += 3 * 256;
        int minLzwCodeSize = u8(ib(startOffset));
        _SubBlockBitConsumer consumer(data, ib(startOffset + 1), dataLength);
        auto decoder = std::make_unique<LZWDecoder>(consumer, 1 << minLzwCodeSize, minLzwCodeSize);
        uint8_t transparent = transparentColorIndex();
        decoder->decode([&](uint8_t data){
            if (data == transparent) {
                dot(0, 0, 0, true);
            } else {
                uint8_t r, g, b;
                if (hasLocalColorTable()) localColor(data, &r, &g, &b);
                else globalColor(data, &r, &g, &b);
                dot(r, g, b, false);
            }
        });
    }
};

}
