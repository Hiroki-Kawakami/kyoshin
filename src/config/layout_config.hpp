/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include "config_internal.hpp"
#include "flash_img_config.hpp"

DEF_CONFIG_ENUM(ViewLayout, ZoomHorizontal, ZoomVertical, HorizontalInfo);
struct ViewLayoutConfig {
    FlashImg flashImg;
    uint16_t imgWidth;
    uint16_t imgHeight;
    uint8_t rotation;
    bool forecast;
};
inline constexpr ViewLayoutConfig VIEW_LAYOUT_CONFIG[ViewLayout::count] = {
    { FlashImg::MapBase16bitSwap320x240 , 320 , 240 , 1 , false },
    { FlashImg::MapBase16bitSwap240x320 , 240 , 320 , 2 , false },
    { FlashImg::MapBase16bitSwap212x240 , 212 , 240 , 1 , true  },
};

DEF_CONFIG_ENUM(ViewLayoutMode, AutoHorizontal, ZoomHorizontal, ZoomVertical, HorizontalInfo);
struct ViewLayoutModeConfig {
    const char *displayString;
    ViewLayout normal;
    ViewLayout alert;
};
inline constexpr ViewLayoutModeConfig VIEW_LAYOUT_MODE_CONFIG[ViewLayoutMode::count] = {
    { "自動(横)", ViewLayout::ZoomHorizontal, ViewLayout::HorizontalInfo },
    { "ズーム(横)", ViewLayout::ZoomHorizontal, ViewLayout::ZoomHorizontal },
    { "ズーム(縦)", ViewLayout::ZoomVertical, ViewLayout::ZoomVertical },
    { "予報(横)", ViewLayout::HorizontalInfo, ViewLayout::HorizontalInfo },
};
inline const char *displayString(ViewLayoutMode value) {
    return VIEW_LAYOUT_MODE_CONFIG[value.value].displayString;
}
