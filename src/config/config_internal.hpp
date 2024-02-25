/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include <cstdint>
#include <string>

#define DEF_CONFIG_ENUM(name, ...)                                                    \
    struct name {                                                                     \
        enum Value { __VA_ARGS__, count };                                            \
        Value value;                                                                  \
        constexpr name() : value(static_cast<Value>(0)) {}                            \
        constexpr name(Value value) : value(value) {}                                 \
        constexpr name(int value) : value(static_cast<Value>(value)) {}               \
        name next() { return name(static_cast<Value>((value + 1) % count)); }         \
        name prev() { return name(static_cast<Value>((value + count - 1) % count)); } \
        bool operator==(const name &other) const { return value == other.value; }     \
        bool operator!=(const name &other) const { return value != other.value; }     \
    }
