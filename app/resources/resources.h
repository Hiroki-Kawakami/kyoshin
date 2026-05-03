#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "lvgl.h"

struct Resources {
    struct {
        const lv_font_t *ipa_16;
        const lv_font_t *ipa_24;
        const lv_font_t *ipa_numbers_40;
    } font;
};
extern const struct Resources R;

#ifdef __cplusplus
} /*extern "C"*/
#endif
