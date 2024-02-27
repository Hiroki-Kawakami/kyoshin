/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include "kyoshin.hpp"

class SoundTestScene : public UI::ModalScene {
public:
    ForecastType forecastType;
    SoundTestScene(ForecastType forecastType) : forecastType(forecastType) {}
    void didAppear() override {
        auto soundType = settings.soundType[forecastType.value];
        bool repeat = settings.soundRepeat[forecastType.value] == SoundRepeat::On;
        auto volume = settings.soundVolume[forecastType.value];
        soundController.playSound(soundType, repeat, volume);
    }
    void didDisappear() override {
        soundController.stop();
    }
    void display() override {
        ModalScene::display();
        drawButton(0, 3, "完了");
        M5.Display.setFont(&defaultFount);
        M5.Display.drawCenterString("サウンドテスト", centerX, centerY - 12);
    }
    void eventLoop() override {
        soundController.eventLoop();
        if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) {
            dismissScene();
        }
    }
};

class AlarmSettingsScene : public UI::ListScene {
public:
    ForecastType forecastType;
    string name;
    AlarmSettingsScene(ForecastType forecastType, string name) : forecastType(forecastType), name(name) {}
    int numberOfRows() override {
        return 4;
    }
    int16_t volumeValue(int index) {
        constexpr int16_t volumes[] = { 0, 26, 37, 50, 64, 80, 98, 119, 142, 166, 192 };
        return volumes[index];
    }
    int volumeIndex(int16_t value) {
        for (int i = 0; i <= 10; i++) {
            if (volumeValue(i) >= value) return i;
        }
        return 5;
    }
    void itemForRow(int row, UI::ListItem &item) override {
        switch (row) {
        case 0:
            item.title = name + "音の音量";
            item.value = std::to_string(volumeIndex(settings.soundVolume[forecastType.value]) * 10) + "%";
            item.button = "切替";
            break;
        case 1:
            item.title = "通知音";
            item.value = displayString(settings.soundType[forecastType.value]);
            item.button = "切替";
            break;
        case 2:
            item.title = "繰り返し";
            item.value = displayString(settings.soundRepeat[forecastType.value]);
            item.button = "切替";
            break;
        case 3:
            item.title = "サウンドテスト";
            break;
        }
    }
    void itemSelected(int index) override {
        switch (index) {
        case 0:
            settings.setSoundVolume(forecastType, volumeValue((volumeIndex(settings.soundVolume[forecastType.value]) + 1) % 11));
            reloadData();
            break;
        case 1:
            settings.setSoundType(forecastType, settings.soundType[forecastType.value].next());
            reloadData();
            break;
        case 2:
            settings.setSoundRepeat(forecastType, settings.soundRepeat[forecastType.value].next());
            reloadData();
            break;
        case 3:
            presentScene(std::make_shared<SoundTestScene>(forecastType));
            break;
        }
    }
};
