#include "resources.h"

// MARK: fonts
extern const lv_font_t ipa_16;
extern const lv_font_t ipa_24;
extern const lv_font_t ipa_numbers_40;

const struct Resources R = {
    .font = {
        .ipa_16 = &ipa_16,
        .ipa_24 = &ipa_24,
        .ipa_numbers_40 = &ipa_numbers_40,
    },
};
