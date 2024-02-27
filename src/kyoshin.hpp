/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include "rtos.hpp"
#include "nvs.hpp"
#include "networking.hpp"
#include "ui.hpp"
#include "config/server_config.hpp"
#include "modules/flash_img_controller.hpp"
#include "modules/settings.hpp"
#include "modules/sound_controller.hpp"

using std::function;
using std::string;
using std::shared_ptr;
using std::weak_ptr;

// Shared Instances
static constexpr int blockHeight = 20;
static constexpr int imgWidth = SERVER_CONFIG.imgWidth, imgHeight = SERVER_CONFIG.imgHeight;
static constexpr int imgBufferSize = imgWidth * imgHeight;
union ImageBuffer {
    uint8_t u8[imgBufferSize];
    uint16_t u16[imgBufferSize / 2];
};
extern ImageBuffer imgBuffer;
extern Networking::HTTPClient httpClient;
extern RTOS::Task<portMAX_DELAY> bgTask1;
extern Settings settings;
extern FlashImageController flashImage;
extern SoundController soundController;

// Common
#define defaultFount lgfxJapanGothicP_24
void init();
