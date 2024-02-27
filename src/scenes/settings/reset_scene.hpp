/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include "kyoshin.hpp"

class ConfirmResetScene : public UI::ModalScene {
public:
    void display() override {
        ModalScene::display();
        drawButton(0, 2, "いいえ");
        drawButton(2, 1, "はい");
        M5.Display.setFont(&defaultFount);
        M5.Display.drawCenterString("初期化しますか？", centerX, centerY - 12);
    }
    void eventLoop() override {
        soundController.eventLoop();
        if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) {
            dismissScene();
        }
        if (M5.BtnC.wasPressed()) {
            NVS::flashErase();
            esp_restart();
        }
    }
};

class ResetScene : public UI::ListScene {
public:
    bool flashImageCleared = false;
    int numberOfRows() override {
        return 3;
    }
    void itemForRow(int row, UI::ListItem &item) override {
        switch (row) {
        case 0:
            item.title = "画像キャッシュを削除";
            item.value = flashImageCleared ? "削除されました" : "";
            break;
        case 1:
            item.title = "WiFiを再設定";
            break;
        case 2:
            item.title = "初期化";
            break;
        }
    }
    void itemSelected(int index) override {
        switch (index) {
        case 0:
            flashImage.clear();
            flashImageCleared = true;
            reloadData();
            break;
        case 1:
            settings.setWifiSetup(true);
            esp_restart();
            break;
        case 2:
            presentScene(std::make_shared<ConfirmResetScene>());
            break;
        }
    }
};

