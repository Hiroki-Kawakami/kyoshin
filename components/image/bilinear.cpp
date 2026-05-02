#include "bilinear.hpp"
#include <cstddef>

// Bilinear interpolation of one RGB565 pixel.
// fx, fy are 8-bit fractions [0,255]; weights sum to 65536 (2^16).
static inline uint16_t lerp_px(
    const uint16_t *r0, const uint16_t *r1,
    int ix, int ix1, uint16_t fx, uint16_t fy)
{
    const uint16_t ifx = 256u - fx, ify = 256u - fy;
    const uint16_t p00 = r0[ix], p10 = r0[ix1], p01 = r1[ix], p11 = r1[ix1];

    const uint32_t r = (((p00 >> 11)           * ifx + (p10 >> 11)           * fx) * ify
                      + ((p01 >> 11)           * ifx + (p11 >> 11)           * fx) * fy) >> 16;
    const uint32_t g = ((((p00 >> 5) & 0x3Fu)  * ifx + ((p10 >> 5) & 0x3Fu)  * fx) * ify
                      + (((p01 >> 5) & 0x3Fu)  * ifx + ((p11 >> 5) & 0x3Fu)  * fx) * fy) >> 16;
    const uint32_t b = (((p00 & 0x1Fu)         * ifx + (p10 & 0x1Fu)         * fx) * ify
                      + ((p01 & 0x1Fu)         * ifx + (p11 & 0x1Fu)         * fx) * fy) >> 16;

    return (uint16_t)((r << 11) | (g << 5) | b);
}

// Endpoint-aligned bilinear downscale: pixel 0 -> pixel 0, pixel (out-1) -> pixel (in-1).
//
// In-place safety (dst == src->data): for any downscale, the flat write index
// (oy*out_w + ox) is always <= the flat read index (iy*in_w + ix), so
// row-major processing never overwrites a pixel before it is read.
void bilinear_resize(BilinearInput *input, BilinearOutput *output)
{
    const int in_w  = input->width,  in_h  = input->height;
    const int out_w = output->width, out_h = output->height;
    const uint16_t *src = input->data;
    uint16_t       *dst = output->data;

    // Step size in 16.16 fixed point.
    const uint32_t x_step = out_w > 1
        ? (uint32_t)(in_w  - 1) * 65536u / (uint32_t)(out_w - 1) : 0u;
    const uint32_t y_step = out_h > 1
        ? (uint32_t)(in_h - 1) * 65536u / (uint32_t)(out_h - 1) : 0u;

    uint32_t y_fp = 0u;
    for (int oy = 0; oy < out_h; oy++, y_fp += y_step) {
        const int iy  = (int)(y_fp >> 16);
        const int iy1 = iy < in_h - 1 ? iy + 1 : iy;
        const uint16_t fy = (uint16_t)((y_fp >> 8) & 0xFFu);

        const uint16_t *r0  = src + (size_t)iy  * in_w;
        const uint16_t *r1  = src + (size_t)iy1 * in_w;
        uint16_t       *row = dst + (size_t)oy  * out_w;

        uint32_t x_fp = 0u;
        for (int ox = 0; ox < out_w; ox++, x_fp += x_step) {
            const int ix  = (int)(x_fp >> 16);
            const int ix1 = ix < in_w - 1 ? ix + 1 : ix;
            const uint16_t fx = (uint16_t)((x_fp >> 8) & 0xFFu);
            row[ox] = lerp_px(r0, r1, ix, ix1, fx, fy);
        }
    }
}
