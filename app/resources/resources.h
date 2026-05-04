#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "lvgl.h"

struct Sound {
    const uint8_t *data;
    uint8_t repeat;
    uint8_t short_repeat;
    uint8_t short_interval;
    uint8_t long_interval;
};

struct Resources {
    struct {
        const lv_image_dsc_t *settings;
        const lv_image_dsc_t *monitor;
        const lv_image_dsc_t *bell_ring;
        const lv_image_dsc_t *moon;
        const lv_image_dsc_t *ellipsis;
    } icon;
    struct {
        const lv_font_t *ipa_16;
        const lv_font_t *ipa_24;
        const lv_font_t *ipa_numbers_40;
    } font;
    struct {
        const struct Sound *alarm1;
        const struct Sound *alarm2;
        const struct Sound *alarm3;
    } sound;
};
extern const struct Resources R;

#ifdef __cplusplus
} /*extern "C"*/
#endif
