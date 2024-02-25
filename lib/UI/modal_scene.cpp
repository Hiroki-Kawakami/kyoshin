/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#include <algorithm>
#include "ui.hpp"

UI_IMPL_BEGIN

void ModalScene::display() {
    centerX = M5.Display.width() / 2;
    centerY = 100;
    modalWidth = M5.Display.width() - 40;
    modalHeight = 170;
    uint16_t modalX = centerX - modalWidth / 2;
    uint16_t modalY = centerY - modalHeight / 2;
    M5.Display.fillRect(modalX, modalY, modalWidth, modalHeight, backgroundColor);
    M5.Display.drawRect(modalX + 2, modalY + 2, modalWidth - 4, modalHeight - 4, TFT_WHITE);
}

UI_IMPL_END
