/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include "kyoshin.hpp"
#include "alarm_settings_scene.hpp"

class SoundSettingsScene : public UI::ListScene {
public:
    int numberOfRows() override {
        return 3;
    }
    void itemForRow(int row, UI::ListItem &item) override {
        switch (row) {
        case 0:
            item.title = "予報受信時";
            break;
        case 1:
            item.title = "警報受信時";
            break;
        case 2:
            item.title = "訓練報をミュート";
            item.value = settings.muteTraining ? "有効" : "無効";
            item.button = "切替";
            break;
        }
    }
    void itemSelected(int index) override {
        switch (index) {
        case 0:
            presentScene(std::make_shared<AlarmSettingsScene>(ForecastType::Normal, "予報"));
            break;
        case 1:
            presentScene(std::make_shared<AlarmSettingsScene>(ForecastType::Alert, "警報"));
            break;
        case 2:
            settings.setMuteTraining(!settings.muteTraining);
            reloadData();
            break;
        }
    }
};
