/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include "config_internal.hpp"
#include "server_config.hpp"

inline constexpr int FLASH_IMG_VERSION = 1;

DEF_CONFIG_ENUM(FlashImg,
    MapOriginalGif,
    MapBase16bitOriginal,
    MapBase8bitOriginal,
    MapBase16bitSwap320x240,
    MapBase16bitSwap240x320,
    MapBase16bitSwap212x240
);
inline constexpr int FLASH_IMG_DATA_SIZE[FlashImg::count] = {
    65536                                                , // MapOriginalGif
    SERVER_CONFIG.imgWidth * SERVER_CONFIG.imgHeight * 2 , // MapBase16bitOriginal
    SERVER_CONFIG.imgWidth * SERVER_CONFIG.imgHeight * 1 , // MapBase8bitOriginal
    320                    * 240                     * 2 , // MapBase16bitSwap320x240
    240                    * 320                     * 2 , // MapBase16bitSwap240x320
    212                    * 240                     * 2 , // MapBase16bitSwap212x240
};

struct FlashImgLocation {
    int offset;
    int size;
};

inline constexpr int FLASH_IMG_DATA_ALIGN = 4096;
struct FlashImgLocationArray {
    FlashImgLocation locations[FlashImg::count];
    constexpr FlashImgLocationArray() : locations() {
        int offset = 0;
        for (int i = 0; i < FlashImg::count; i++) {
            locations[i].offset = offset;
            int size = FLASH_IMG_DATA_SIZE[i];
            size += FLASH_IMG_DATA_ALIGN - (size % FLASH_IMG_DATA_ALIGN);
            locations[i].size = size;
            offset += size;
        }
    }
    const FlashImgLocation &operator[](FlashImg type) const {
        return locations[type.value];
    }
};
inline constexpr FlashImgLocationArray FLASH_IMG_LOCATION;
