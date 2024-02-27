/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#include <algorithm>
#include "ui.hpp"

UI_IMPL_BEGIN

void ListScene::eventLoop() {
    if (M5.BtnB.wasPressed()) {
        selectedRow++;
        int n = numberOfRows();
        if (selectedRow >= n) selectedRow = 0;
        displayOffset = selectedRow > 2 ? std::min(selectedRow - 2, n - 4) : 0;
        reloadData();
    }
    if (M5.BtnC.wasPressed()) {
        itemSelected(selectedRow);
    }
    Scene::eventLoop();
}

void ListScene::display() {
    clearButton(0, 3, TFT_WHITE);
    reloadData();
    drawButton(0, 1, "戻る");
    drawButton(1, 1, "▼");
}

void ListScene::reloadData() {
    M5Canvas drawBuffer(&M5.Display);
    drawBuffer.setColorDepth(2);
    int width = M5.Display.width(), height = 50;
    drawBuffer.createSprite(width, height);

    constexpr uint16_t colors[4] = { TFT_BLACK, TFT_LIGHTGRAY, TFT_WHITE, TFT_SKYBLUE };
    drawBuffer.createPalette(colors, 4);
    for (int i = 0, n = numberOfRows(); i < 4; i++) {
        int index = i + displayOffset;
        if (index >= n) index = -1;
        drawItem(index, drawBuffer, width, height);
        drawBuffer.pushSprite(0, i * height);
    }
}

void ListScene::drawItem(int index, M5Canvas &drawBuffer, int width, int height) {
    bool isSelected = index == selectedRow;
    drawBuffer.clear(isSelected ? 3 : 2);
    drawBuffer.drawRect(0, height - 1, width, 1, 1);
    if (index < 0) return;

    ListItem item = { "" , "" , "選択" } ;
    itemForRow(index, item);
    drawBuffer.setTextColor(isSelected ? 2 : 0);
    if (item.value.empty()) {
        drawBuffer.setFont(&lgfxJapanGothicP_24);
        drawBuffer.drawString(item.title.c_str(), 8, 13);
    } else {
        drawBuffer.setFont(&lgfxJapanGothicP_24);
        drawBuffer.drawString(item.title.c_str(), 8, 4);
        drawBuffer.setFont(&lgfxJapanGothicP_16);
        drawBuffer.drawString(item.value.c_str(), 8, 28);
    }
    if (index == selectedRow) drawButton(2, 1, item.button);
}


UI_IMPL_END
