#pragma once
#include <cstddef>
#include <cstdint>

class GifDecoder {
public:
    enum class Error {
        None,
        InvalidSignature,
        SizeMismatch,
        InvalidData,
        NoImageData,
    };

    GifDecoder(int width, int height);
    Error decode(const uint8_t* data, size_t size, uint16_t* output);

private:
    static constexpr int MAX_CODES = 4096;

    int _width;
    int _height;

    // Pre-allocated LZW dictionary, reused across decode calls (16 KB total)
    uint16_t _prefix[MAX_CODES];  // prefix code index; chain terminates at root (< clearCode+2)
    uint8_t  _suffix[MAX_CODES];  // suffix color index
    uint8_t  _stack[MAX_CODES];   // decode output stack
};
