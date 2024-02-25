/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include <cstdint>
#include <functional>

namespace Bilinear {

template<class T>
inline float interpolate(T a, T b, float point) {
    return roundf((a * (1 - point)) + (b * point));
}
template<class T>
inline float interpolate2d(T *p, float px, float py) {
    float i0x = interpolate(p[0], p[1], px);
    float i1x = interpolate(p[2], p[3], px);
    return interpolate(i0x, i1x, py);
}

template<class T>
inline void resize(int fw, int fh, int tw, int th, const T *data,
    std::function<void(int, int, T)> dot,
    std::function<void(int x, int y, T, T*, T*, T*)> toRGB, std::function<T(T, T, T)> fromRGB) {
    const float scaleX = (float)fw / tw, scaleY = (float)fh / th;
    for (int y = 0; y < th; y++) {
        for (int x = 0; x < tw; x++) {
            const float sx = x * scaleX, sy = y * scaleY;
            const int isx = (int)sx, isy = (int)sy;

            T pixels[4] = {
                data[(isy    ) * fw + (isx    )],
                data[(isy    ) * fw + (isx + 1)],
                data[(isy + 1) * fw + (isx    )],
                data[(isy + 1) * fw + (isx + 1)],
            };
            if (pixels[0] == pixels[1] && pixels[0] == pixels[2] && pixels[0] == pixels[3]) {
                dot(x, y, pixels[0]);
            } else {
                T rgb[3][4] = {};
                toRGB(isx, isy, pixels[0], &rgb[0][0], &rgb[1][0], &rgb[2][0]);
                toRGB(isx, isy, pixels[1], &rgb[0][1], &rgb[1][1], &rgb[2][1]);
                toRGB(isx, isy, pixels[2], &rgb[0][2], &rgb[1][2], &rgb[2][2]);
                toRGB(isx, isy, pixels[3], &rgb[0][3], &rgb[1][3], &rgb[2][3]);

                T r = interpolate2d((T*)rgb[0], sx - isx, sy - isy);
                T g = interpolate2d((T*)rgb[1], sx - isx, sy - isy);
                T b = interpolate2d((T*)rgb[2], sx - isx, sy - isy);

                dot(x, y, fromRGB(r, g, b));
            }
        }
    }
}

template<class T>
inline void resize(int fw, int fh, int tw, int th, const T *data, std::function<void(int, int, T)> dot, T rm, T gm, T bm) {
    resize<T>(fw, fh, tw, th, data, dot, [=](int x, int y, T value, T *r, T *g, T *b) {
        *r = value & rm;
        *g = value & gm;
        *b = value & bm;
    }, [=](T r, T g, T b) {
        return (r & rm) | (g & gm) | (b & bm);
    });
}

template<class T>
inline void reduce(int fw, int fh, int tw, int th, T *data, T rm, T gm, T bm) {
    resize(fw, fh, tw, th, data, [=](int x, int y, T value) {
        data[y * tw + x] = value;
    }, rm, gm, bm);
}

}
