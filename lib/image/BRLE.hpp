/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include <cstdint>
#include <functional>
#include <vector>

namespace BRLE {

inline std::vector<uint8_t> encode(uint8_t *pixels, int length, uint8_t bg) {
    std::vector<uint8_t> encoded;
    int i = 0;
    while (i < length) {
        int runLength = 0;
        while (i < length && pixels[i] == bg && runLength < 255) {
            runLength++;
            i++;
        }
        encoded.push_back(runLength);

        auto index = encoded.size();
        encoded.push_back(0);
        runLength = 0;
        while (i < length && pixels[i] != bg && runLength < 255) {
            runLength++;
            encoded.push_back(pixels[i++]);
        }
        encoded[index] = runLength;
    }
    return encoded;
}

inline void decode(std::vector<uint8_t> &encoded, uint8_t bg, std::function<void(uint8_t)> byte) {
    size_t i = 0, n = encoded.size(), runLength;
    while (i < n) {
        runLength = encoded[i++];
        for (int j = 0; j < runLength; j++) byte(bg);
        runLength = encoded[i++];
        for (int j = 0; j < runLength; j++) byte(encoded[i++]);
    }
}

}
