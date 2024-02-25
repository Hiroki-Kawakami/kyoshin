/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include <string>
#include <sys/time.h>

struct Date {
    long long time_ms;
    Date() {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }
    Date(long long time_ms) : time_ms(time_ms) {}
    Date(int year, int month, int day, int hour, int minute, int second) {
        struct tm timeinfo = {};
        timeinfo.tm_year = year - 1900;
        timeinfo.tm_mon = month - 1;
        timeinfo.tm_mday = day;
        timeinfo.tm_hour = hour;
        timeinfo.tm_min = minute;
        timeinfo.tm_sec = second;
        time_ms = mktime(&timeinfo) * 1000;
    }

    Date operator+(long long ms) const { return Date(time_ms + ms); }
    void operator+=(long long ms) { time_ms += ms; }
    Date operator-(long long ms) const { return Date(time_ms - ms); }
    void operator-=(long long ms) { time_ms -= ms; }
    long long operator-(const Date &other) const { return time_ms - other.time_ms; }
    bool operator<(const Date &other) const { return time_ms < other.time_ms; }
    bool operator>(const Date &other) const { return time_ms > other.time_ms; }
    bool operator<=(const Date &other) const { return time_ms <= other.time_ms; }
    bool operator>=(const Date &other) const { return time_ms >= other.time_ms; }
    bool operator==(const Date &other) const { return time_ms == other.time_ms; }
    bool operator!=(const Date &other) const { return time_ms != other.time_ms; }
    explicit operator bool() const { return time_ms > 0; }

    std::string strftime(const char *format) const {
        char buf[128];
        time_t t = time_ms / 1000;
        struct tm timeinfo;
        localtime_r(&t, &timeinfo);
        ::strftime(buf, sizeof(buf), format, &timeinfo);
        return std::string(buf);
    }
};
