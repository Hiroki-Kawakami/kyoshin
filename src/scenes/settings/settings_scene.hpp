/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include "kyoshin.hpp"
#include "config/layout_config.hpp"
#include "display_settings_scene.hpp"
#include "sound_settings_scene.hpp"
#include "reset_scene.hpp"

class SettingsScene : public UI::ListScene {
public:
    int numberOfRows() override {
        return 4;
    }
    void itemForRow(int row, UI::ListItem &item) override {
        switch (row) {
        case 0:
            item.title = "表示レイアウト";
            item.value = displayString(settings.layoutMode);
            item.button = "切替";
            break;
        case 1:
            item.title = "画面設定";
            break;
        case 2:
            item.title = "警報音設定";
            break;
        case 3:
            item.title = "リセット";
            break;
        }
    }
    void itemSelected(int index) override {
        switch (index) {
        case 0:
            settings.setLayoutMode(settings.layoutMode.next());
            drawItem(index);
            break;
        case 1:
            presentScene(std::make_shared<DisplaySettingsScene>());
            break;
        case 2:
            presentScene(std::make_shared<SoundSettingsScene>());
            break;
        case 3:
            presentScene(std::make_shared<ResetScene>());
            break;
        }
    }
};

