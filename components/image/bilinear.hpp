#pragma once
#include <cstdint>

struct BilinearInput {
    uint16_t width, height;
    const uint16_t *data;
};
struct BilinearOutput {
    uint16_t width, height;
    uint16_t *data;
};

void bilinear_resize(BilinearInput *input, BilinearOutput *output);
