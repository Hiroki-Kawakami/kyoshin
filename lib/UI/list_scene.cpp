/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#include <algorithm>
#include "ui.hpp"

UI_IMPL_BEGIN

void ListScene::eventLoop() {
    if (M5.BtnB.wasPressed()) {
        int prev = selectedRow++;
        if (selectedRow >= numberOfRows()) selectedRow = 0;
        drawItem(prev);
        drawItem(selectedRow);
    }
    if (M5.BtnC.wasPressed()) {
        itemSelected(selectedRow);
    }
    Scene::eventLoop();
}

void ListScene::display() {
    M5.Display.clear(TFT_WHITE);
    for (int i = 0, n = numberOfRows(); i < n; i++) {
        drawItem(i);
    }
    drawButton(0, 1, "戻る");
    drawButton(1, 1, "▼");
}

void ListScene::drawItem(int index) {
    ListItem item = { "" , "" , "選択" } ;
    int itemHeight = 50;
    int displayWidth = M5.Display.width();
    int bgColor = index == selectedRow ? TFT_SKYBLUE : TFT_WHITE;
    int fgColor = index == selectedRow ? TFT_WHITE : TFT_BLACK;
    itemForRow(index, item);
    M5.Display.fillRect(0, index * itemHeight, displayWidth, itemHeight, bgColor);
    M5.Display.setTextColor(fgColor);
    if (item.value.empty()) {
        M5.Display.setFont(&lgfxJapanGothicP_24);
        M5.Display.drawString(item.title.c_str(), 8, index * itemHeight + 13);
    } else {
        M5.Display.setFont(&lgfxJapanGothicP_24);
        M5.Display.drawString(item.title.c_str(), 8, index * itemHeight + 4);
        M5.Display.setFont(&lgfxJapanGothicP_16);
        M5.Display.drawString(item.value.c_str(), 8, index * itemHeight + 28);
    }
    M5.Display.drawRect(0, (index + 1) * itemHeight - 1, displayWidth, 1, TFT_LIGHTGREY);
    if (index == selectedRow) drawButton(2, 1, item.button);
}


UI_IMPL_END
