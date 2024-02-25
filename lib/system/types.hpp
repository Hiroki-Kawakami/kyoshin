/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once

class NoCopy {
public:
    NoCopy() = default;
    ~NoCopy() = default;
    NoCopy(const NoCopy &) = delete;
    NoCopy &operator=(const NoCopy &) = delete;
};

class NoMove {
public:
    NoMove() = default;
    ~NoMove() = default;
    NoMove(const NoMove &) = delete;
    NoMove &operator=(const NoMove &) = delete;
    NoMove(NoMove &&) = delete;
    NoMove &operator=(NoMove &&) = delete;
};
