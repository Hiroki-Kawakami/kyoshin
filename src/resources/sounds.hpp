/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#include <cstdint>

struct SoundData {
    inline static constexpr uint8_t alarm1[] = {
        #include "alarm1.data"
    };
    inline static constexpr uint8_t alarm2[] = {
        #include "alarm2.data"
    };
    inline static constexpr uint8_t alarm3[] = {
        #include "alarm3.data"
    };
};
