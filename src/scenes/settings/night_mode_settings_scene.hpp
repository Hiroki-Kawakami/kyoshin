/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include "kyoshin.hpp"
#include "resources/fonts.hpp"

static string timeString(int16_t value) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d", value / 60, value % 60);
    return buf;
}

class NightTimePickerScene : public UI::ModalScene {
public:
    string title;
    int16_t value;
    function<void(int16_t)> valueChanged;
    NightTimePickerScene(string title, int16_t value, function<void(int16_t)> valueChanged) : title(title), value(value), valueChanged(valueChanged) {}

    void display() override {
        int width = modalWidth - 8, height = 40;
        M5Canvas drawBuffer(&M5.Display);
        drawBuffer.setColorDepth(2);
        drawBuffer.createSprite(width, height);
        drawBuffer.clear(TFT_BLACK);

        string valueStr = timeString(value);
        drawBuffer.setFont(&lgfxJapanGothicP_40_numbers);
        drawBuffer.drawCenterString(valueStr.c_str(), width / 2, 0);
        drawBuffer.pushSprite(centerX - width / 2, centerY - height / 2);

        drawBuffer.deleteSprite();
    }

    void willAppear() override {
        ModalScene::display();
        M5.Display.setFont(&lgfxJapanGothicP_16);
        M5.Display.drawString(title.c_str(), centerX - modalWidth / 2 + 6, centerY - modalHeight / 2 + 6);
        drawButton(0, 1, "-");
        drawButton(1, 1, "+");
        drawButton(2, 1, "完了");
    }

    void eventLoop() override {
        if (M5.BtnA.wasPressed()) {
            value -= 30;
            if (value < 0) value += 24 * 60;
            setNeedsDisplay();
        }
        if (M5.BtnB.wasPressed()) {
            value += 30;
            if (value >= 24 * 60) value -= 24 * 60;
            setNeedsDisplay();
        }
        if (M5.BtnC.wasPressed()) {
            valueChanged(value);
            dismissScene();
        }
    }
};

class NightModeSettingsScene : public UI::ListScene {
    int numberOfRows() override {
        return 3;
    }
    void itemForRow(int row, UI::ListItem &item) override {
        switch (row) {
        case 0:
            item.title = "夜間モード";
            item.value = settings.useNightMode ? "使用する" : "使用しない";
            item.button = "切替";
            break;
        case 1:
            item.title = "開始時刻";
            item.value = timeString(settings.nightStart);
            item.button = "変更";
            break;
        case 2:
            item.title = "終了時刻";
            item.value = timeString(settings.nightEnd);
            item.button = "変更";
            break;
        }
    }
    void itemSelected(int index) override {
        switch (index) {
        case 0:
            settings.setUseNightMode(!settings.useNightMode);
            reloadData();
            break;
        case 1:
            presentScene(std::make_shared<NightTimePickerScene>("開始時刻", settings.nightStart, [](int value) {
                settings.setNightStart(value);
            }));
            break;
        case 2:
            presentScene(std::make_shared<NightTimePickerScene>("終了時刻", settings.nightEnd, [](int value) {
                settings.setNightEnd(value);
            }));
            break;
        }
    }
};
