#include "resources.h"

// MARK: icons
extern const lv_image_dsc_t settings;

// MARK: fonts
extern const lv_font_t ipa_16;
extern const lv_font_t ipa_24;
extern const lv_font_t ipa_numbers_40;

// MARK: wav
extern const uint8_t alarm1[];
extern const uint8_t alarm2[];
extern const uint8_t alarm3[];

const struct Resources R = {
    .icon = {
        .settings = &settings,
    },
    .font = {
        .ipa_16 = &ipa_16,
        .ipa_24 = &ipa_24,
        .ipa_numbers_40 = &ipa_numbers_40,
    },
    .sound = {
        .alarm1 = &(struct Sound){ alarm1, 3, 1, 10, 50 },
        .alarm2 = &(struct Sound){ alarm2, 1, 2,  8, 70 },
        .alarm3 = &(struct Sound){ alarm3, 2, 1, 20, 60 },
    },
};
