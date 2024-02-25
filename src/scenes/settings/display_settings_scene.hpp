/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include "kyoshin.hpp"

class DisplaySettingsScene : public UI::ListScene {
public:
    int numberOfRows() override {
        return 3;
    }
    void itemForRow(int row, UI::ListItem &item) override {
        switch (row) {
        case 0:
            item.title = "画面の明るさ";
            item.value = std::to_string(settings.brightness) + "%";
            break;
        case 1:
            item.title = "無操作時の動作";
            item.value = []() {
                if (settings.dimBrightness < 0) return "何もしない";
                if (settings.dimBrightness == 0) return "画面を消灯";
                return "画面を暗くする";
            }();
            break;
        case 2:
            item.title = "無操作の時間";
            item.value = []() {
                if (settings.dimDuration < 60) return std::to_string(settings.dimDuration) + "秒";
                return std::to_string(settings.dimDuration / 60) + "分";
            }();
            break;
        }
        item.button = "切替";
    }
    int16_t nextDimBrightness() {
        if (settings.dimBrightness < 0) return 0;
        if (settings.dimBrightness == 0) return 3;
        return -1;
    }
    int16_t nextDimDuration() {
        static constexpr int16_t durations[] = {10, 15, 20, 30, 60, 120, 300, 600, 900, 1800};
        int n = sizeof(durations) / sizeof(durations[0]);
        int current = 0;
        for (int i = 0; i < n; i++) {
            if (settings.dimDuration <= durations[i]) {
                current = i;
                break;
            }
        }
        return durations[(current + 1) % n];
    }
    void itemSelected(int index) override {
        switch (index) {
        case 0:
            settings.setBrightness(settings.brightness % 100 + 10);
            drawItem(index);
            break;
        case 1:
            settings.setDimBrightness(nextDimBrightness());
            drawItem(index);
            break;
        case 2:
            settings.setDimDuration(nextDimDuration());
            drawItem(index);
            break;
        }
    }
};

