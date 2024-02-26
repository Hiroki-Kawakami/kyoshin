/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include "kyoshin.hpp"
#include "date.hpp"
#include "Bilinear.hpp"
#include "GIF.hpp"
#include "config/layout_config.hpp"
#include "modules/forecast.hpp"
#include "settings/settings_scene.hpp"
#include "resources/fonts.hpp"

class MapViewScene : public UI::Scene {
    shared_ptr<FlashImagePartition> flashImagePartition;
    Forecast forecast;
    bool updating = false;
    Date lastUpdated = Date(0);
    int nextUpdateInterval = 0;
    Date showRealtimeImgTypeSwitch = Date(0);
    Date displayOnTime = Date(0);

    const MapRegionConfig &regionConfig() const {
        return SERVER_CONFIG.regions[settings.mapRegion.value];
    }
    const char *realtimeImgUrlFormat() const {
        if (settings.borehole) return regionConfig().boreholeUrlFormats[settings.realtimeImgType.value];
        return regionConfig().surfaceUrlFormats[settings.realtimeImgType.value];
    }
    const ViewLayoutConfig &layoutConfig() const {
        auto &layoutModeConfig = VIEW_LAYOUT_MODE_CONFIG[settings.layoutMode.value];
        auto layout = forecast.empty() ? layoutModeConfig.normal : layoutModeConfig.alert;
        return VIEW_LAYOUT_CONFIG[layout.value];
    }

    void willAppear() override {
        M5.Display.setRotation(layoutConfig().rotation);
        flashImagePartition = flashImage.partition(regionConfig().identifier);
        prepareBaseMap();
        memset(imgBuffer.u8, 255, sizeof(imgBuffer.u8));
        forecast.clear();
        displayOn(Date());
        nextUpdateInterval = 0;
    }

    void prepareBaseMap() {
        if (flashImagePartition->isInitialized()) return;

        M5.Display.setCursor(0, 0);
        M5.Display.clear(TFT_WHITE);
        M5.Display.setFont(&defaultFount);
        M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);
        M5.Display.println("Downloading base map image...");
        flashImagePartition->erase(FlashImg::MapOriginalGif);
        Networking::HTTPClient httpClient([this](uint8_t *buffer, int offset, int length) {
            flashImagePartition->write(FlashImg::MapOriginalGif, offset, buffer, length);
        });
        httpClient.get(regionConfig().baseMapUrl);
        M5.Display.println("Download base map done.");

        // 16bit/8bit Original Size
        flashImagePartition->erase(FlashImg::MapBase16bitOriginal);
        flashImagePartition->erase(FlashImg::MapBase8bitOriginal);
        GIF::Decoder decoder((uint8_t*)flashImagePartition->ptr(FlashImg::MapOriginalGif), httpClient.received);
        int blockSize = sizeof(imgBuffer.u8) / 3;
        uint16_t *imgBuffer16 = imgBuffer.u16;
        uint8_t *imgBuffer8 = &imgBuffer.u8[blockSize * 2];
        int i = 0, offset = 0;
        decoder.pixels([&](uint8_t r, uint8_t g, uint8_t b, bool transparent) {
            imgBuffer16[i] = lgfx::color565(r, g, b);
            imgBuffer8[i] = lgfx::color332(r, g, b);
            if (++i == blockSize) {
                flashImagePartition->write(FlashImg::MapBase16bitOriginal, offset * 2, imgBuffer16, i * 2);
                flashImagePartition->write(FlashImg::MapBase8bitOriginal, offset, imgBuffer8, i);
                offset += i;
                i = 0;
            }
        });
        if (i > 0) {
            flashImagePartition->write(FlashImg::MapBase16bitOriginal, offset * 2, imgBuffer16, i * 2);
            flashImagePartition->write(FlashImg::MapBase8bitOriginal, offset, imgBuffer8, i);
        }
        flashImagePartition->setInitialized(true);

        M5.Display.println("Decode base map done.");
        M5.Display.println("Resizing image...");
        uint16_t *ptr = (uint16_t*)flashImagePartition->ptr(FlashImg::MapBase16bitOriginal);
        createDisplayBaseMap(FlashImg::MapBase16bitSwap320x240, 320, 240, ptr);
        createDisplayBaseMap(FlashImg::MapBase16bitSwap240x320, 240, 320, ptr);
        createDisplayBaseMap(FlashImg::MapBase16bitSwap212x240, 212, 240, ptr);
        M5.Display.println("Resize image done.");

        vTaskDelay(pdMS_TO_TICKS(1000));
        displayOn(Date());
    }

    void createDisplayBaseMap(FlashImg type, int width, int height, uint16_t *ptr) {
        M5.Display.println(("Creating " + std::to_string(width) + "x" + std::to_string(height) + " image...").c_str());

        int i = 0, offset = 0;
        flashImagePartition->erase(type);
        Bilinear::resize<uint16_t>(imgWidth, imgHeight, width, height, ptr, [&](int x, int y, uint16_t value) {
            imgBuffer.u16[i++] = (value >> 8) | ((value & 0xff) << 8);
            if (i == (sizeof(imgBuffer.u16) / sizeof(imgBuffer.u16[0]))) {
                flashImagePartition->write(type, offset * 2, imgBuffer.u16, i * 2);
                offset += i;
                i = 0;
            }
        }, 0xf800, 0x07e0, 0x001f);
        if (i > 0) flashImagePartition->write(type, offset * 2, imgBuffer.u16, i * 2);
    }

    void displayOn(Date now) {
        displayOnTime = now;
        if (M5.Display.getBrightness() <= settings.dimBrightness) M5.Display.setBrightness(settings.brightness);
    }

    void eventLoop() override {
        auto now = Date();
        Date target = now + SERVER_CONFIG.timeOffset;
        if (!nextUpdateInterval) nextUpdateInterval = SERVER_CONFIG.updateInterval;
        if (!updating && lastUpdated + nextUpdateInterval < target) {
            int delay = 0;
            if (lastUpdated) delay = (target - lastUpdated) - nextUpdateInterval;
            nextUpdateInterval = SERVER_CONFIG.updateInterval;
            if (0 < delay && delay < 200) nextUpdateInterval -= delay;
            lastUpdated = target;
            updating = true;
            bgTask1.send([this, target]() {
                update(target);
                UI::send([this]() {
                    updating = false;
                    setNeedsDisplay();
                });
            });
        }

        bool shouldRing = false;
        if (!forecast.empty()) {
            displayOn(now);
            shouldRing = !forecast.isFinal;
            if (settings.muteTraining && forecast.isTraining) shouldRing = false;
        }
        if (shouldRing) {
            auto forecastType = forecast.type().value;
            if (settings.soundRepeat[forecastType] == SoundRepeat::On) {
                if (!soundController.isPlaying()) {
                    soundController.playSound(settings.soundType[forecastType], true, settings.soundVolume[forecastType]);
                }
            }
            if (settings.soundRepeat[forecastType] == SoundRepeat::Update) {
                if (forecast.isUpdated()) {
                    printf("playSound %d\n", forecastType);
                    soundController.playSound(settings.soundType[forecastType], false, settings.soundVolume[forecastType]);
                }
            }
            if (settings.soundRepeat[forecastType] == SoundRepeat::None) {
                if (forecast.isStarted()) {
                    soundController.playSound(settings.soundType[forecastType], false, settings.soundVolume[forecastType]);
                }
            }
        } else {
            soundController.stop();
        }
        forecast.updateReportTime();

        if (M5.Display.getBrightness() <= settings.dimBrightness) {
            if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) displayOn(now);
            return;
        }
        if (!realtimeImgTypeSwitchButtonAction(now)) buttonAction(now);
        if (settings.dimBrightness >= 0 && now - displayOnTime > settings.dimDuration * 1000) M5.Display.setBrightness(settings.dimBrightness);
        soundController.eventLoop();
    }

    void update(Date target) {
        printf("update %s\n", target.strftime("%Y-%m-%d %H:%M:%S").c_str());
        checkForecast(target);
        if (!updateRealtimeImg(target)) return;
        if (!lastUpdated) return;
        updatePsWaveImg(target);
        if (!lastUpdated) return;

        // resize
        int i = 0, width = layoutConfig().imgWidth, height = layoutConfig().imgHeight;
        uint8_t *ptr = (uint8_t*)flashImagePartition->ptr(FlashImg::MapBase8bitOriginal);
        Bilinear::resize<uint8_t>(imgWidth, imgHeight, width, height, (uint8_t*)imgBuffer.u8, [&](int x, int y, uint8_t value) {
            imgBuffer.u8[i++] = value;
        }, [=](int x, int y, uint8_t value, uint8_t *r, uint8_t *g, uint8_t *b) {
            if (value == 255) value = ptr[y * imgWidth + x];
            *r = value & 0b11100000;
            *g = value & 0b00011100;
            *b = value & 0b00000011;
        }, [](uint8_t r, uint8_t g, uint8_t b) {
            return (r & 0b11100000) | (g & 0b00011100) | (b & 0b00000011);
        });
    }

    void checkForecast(Date target) {
        auto url = target.strftime(SERVER_CONFIG.forecastUrlFormat);
        if (!httpClient.get(url) || httpClient.statusCode() != 200) return;
        forecast.update((const char*)httpClient.buffer);
    }

    bool updateRealtimeImg(Date target) {
        auto url = target.strftime(realtimeImgUrlFormat());
        if (!httpClient.get(url) || httpClient.statusCode() != 200) return false;

        int i = 0;
        GIF::Decoder decoder(httpClient.buffer, httpClient.received);
        memset(imgBuffer.u8, 255, sizeof(imgBuffer.u8));
        decoder.pixels([&](uint8_t r, uint8_t g, uint8_t b, bool transparent) {
            if (!transparent) imgBuffer.u8[i] = lgfx::color332(r, g, b);
            i++;
        });
        return true;
    }

    bool updatePsWaveImg(Date target) {
        auto url = target.strftime(regionConfig().psWaveUrlFormat);
        if (!httpClient.get(url) || httpClient.statusCode() != 200) return false;

        int i = 0;
        GIF::Decoder decoder(httpClient.buffer, httpClient.received);
        decoder.pixels([&](uint8_t r, uint8_t g, uint8_t b, bool transparent) {
            if (!transparent) imgBuffer.u8[i] = lgfx::color332(r, g, b);
            i++;
        });
        return true;
    }

    void drawRealtimeImgTypeSwitchButtons() {
        drawButton(0, 2, displayString(settings.realtimeImgType));
        drawButton(2, 1, settings.borehole ? "地中" : "地表");
    }

    bool realtimeImgTypeSwitchButtonAction(Date now) {
        if (!showRealtimeImgTypeSwitch) return false;
        if (now - showRealtimeImgTypeSwitch > 5000) {
            showRealtimeImgTypeSwitch = 0;
            return true;
        }
        auto changed = [&]() {
            showRealtimeImgTypeSwitch = now;
            lastUpdated = 0;
            drawRealtimeImgTypeSwitchButtons();
            setNeedsDisplay();
            displayOn(now);
        };
        if (M5.BtnA.wasPressed()) {
            settings.setRealtimeImgType(settings.realtimeImgType.prev());
            changed();
        }
        if (M5.BtnB.wasPressed()) {
            settings.setRealtimeImgType(settings.realtimeImgType.next());
            changed();
        }
        if (M5.BtnC.wasPressed()) {
            settings.setBorehole(!settings.borehole);
            changed();
        }
        return true;
    }

    void buttonAction(Date now) {
        if (M5.BtnA.wasPressed()) {
            lastUpdated = 0;
            while (!bgTask1.isBlocked()) vTaskDelay(pdMS_TO_TICKS(10));
            settings.setMapRegion(settings.mapRegion.next());
            flashImagePartition = flashImage.partition(regionConfig().identifier);
            prepareBaseMap();
            setNeedsDisplay();
        }
        if (M5.BtnB.wasPressed()) {
            showRealtimeImgTypeSwitch = now;
            clearButton();
            drawRealtimeImgTypeSwitchButtons();
            displayOn(now);
        }
        if (M5.BtnC.wasPressed()) {
            presentScene(std::make_shared<SettingsScene>());
        }
    }

    void display() override {
        if (updating && lastUpdated != 0) return;
        if (showRealtimeImgTypeSwitch) M5.Display.setClipRect(0, 0, M5.Display.width(), 200);

        uint16_t *ptr = (uint16_t*)flashImagePartition->ptr(layoutConfig().flashImg);
        int displayHeight = M5.Display.height();
        int imgWidth = layoutConfig().imgWidth;

        M5Canvas drawBuffer[2];
        drawBuffer[0].createSprite(imgWidth, blockHeight);
        drawBuffer[1].createSprite(imgWidth, blockHeight);
        M5.Display.startWrite();
        for (int i = 0, top = 0; top < displayHeight; i++, top += blockHeight) {
            if (showRealtimeImgTypeSwitch && i == 10) break;
            int flip = i % 2, offset = top * imgWidth, height = displayHeight - top < blockHeight ? displayHeight : blockHeight;
            drawBuffer[flip].clear(TFT_WHITE);
            drawBuffer[flip].pushImage(0, 0, imgWidth, height, &ptr[offset]);
            if (lastUpdated > 0 && !updating) drawBuffer[flip].pushImage(0, 0, imgWidth, height, &imgBuffer.u8[offset], (uint8_t)255);
            drawBuffer[flip].pushSprite(&M5.Display, 0, top);
        }
        M5.Display.endWrite();
        drawBuffer[0].deleteSprite();
        drawBuffer[1].deleteSprite();

        drawForecast();
        M5.Display.clearClipRect();
    }

    void drawForecast() {
        if (!layoutConfig().forecast) return;

        int displayWidth = M5.Display.width(), displayHeight = M5.Display.height();
        int x = 212, width = displayWidth - x;
        if (forecast.empty()) {
            M5.Display.fillRect(x, 0, 4, displayHeight, TFT_LIGHTGRAY); x += 4; width -= 4;
            M5.Display.fillRect(x, 0, width, 88, TFT_LIGHTGRAY);
            M5.Display.fillRect(x, 88, width, displayHeight - 88, TFT_WHITE);
            return;
        }

        auto backgroundColor = forecast.color();
        M5.Display.fillRect(x, 0, 4, displayHeight, backgroundColor); x += 4; width -= 4;

        M5Canvas drawBuffer(&M5.Display);
        drawBuffer.createSprite(width, 88);

        drawBuffer.clear(backgroundColor);
        drawBuffer.setTextColor(TFT_WHITE, backgroundColor);
        drawBuffer.setFont(&lgfxJapanGothicP_24);
        drawBuffer.drawString(forecast.alertflg.c_str(), 4, 8);
        drawBuffer.setFont(&lgfxJapanGothicP_16);
        drawBuffer.drawString("最大", 4, 46);
        drawBuffer.drawString("震度", 4, 62);
        drawBuffer.setFont(&lgfxJapanGothicP_40_numbers);
        drawBuffer.drawCenterString(forecast.calcintensity.c_str(), width / 2 + 16, 40);
        drawBuffer.pushSprite(x, 0);

        drawBuffer.clear(TFT_WHITE);
        drawBuffer.setFont(&lgfxJapanGothicP_24);
        drawBuffer.setTextColor(TFT_BLACK, TFT_WHITE);
        drawBuffer.drawCenterString(("M" + forecast.magnitude).c_str(), width / 2, 8);
        drawBuffer.setFont(&lgfxJapanGothicP_16);
        drawBuffer.drawString("深さ", 4, 40);
        drawBuffer.setFont(&lgfxJapanGothicP_24);
        drawBuffer.drawCenterString(forecast.depth.c_str(), width / 2, 56);
        drawBuffer.pushSprite(x, 88);

        drawBuffer.clear(TFT_WHITE);
        drawBuffer.setCursor(4, 0);
        drawBuffer.setClipRect(4, 0, width - 8, 88);
        drawBuffer.setFont(&lgfxJapanGothicP_16);
        drawBuffer.println(forecast.regionName.c_str());
        drawBuffer.clearClipRect();
        drawBuffer.pushSprite(x, 88 * 2);

        drawBuffer.deleteSprite();
    }
};
