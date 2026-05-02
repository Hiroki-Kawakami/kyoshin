#include "kyoshin_screen.hpp"
#include "kyoshin_app.hpp"
#include "bilinear.hpp"
#include "map_load_screen.hpp"

void KyoshinScreen::build() {
    if (!kyoshin_monitor) {
        kyoshin_monitor = new KyoshinMonitor();
    }
    auto label = lv_label_create(root_);
    lv_obj_center(label);
    lv_label_set_text(label, "Kyoshin Monitor");

    image_ = lv_image_create(root_);
    lv_obj_center(image_);
    lv_obj_set_size(image_, 320, 240);
}

void KyoshinScreen::onAppear() {
    kyoshin_monitor->setCallback(this);
    if (!kyoshin_monitor->loadBaseMapImage()) {
        screen_manager.push(std::make_unique<MapLoadScreen>());
        return;
    }

    auto data = kyoshin_monitor->copyBaseMapImage();
    auto input = BilinearInput{ KYOSHIN_SERVER_CONFIG.imgWidth, KYOSHIN_SERVER_CONFIG.imgHeight, data };
    auto output = BilinearOutput{ 320, 240, data };
    bilinear_resize(&input, &output);

    img_dsc_.header.cf = LV_COLOR_FORMAT_RGB565;
    img_dsc_.header.magic = LV_IMAGE_HEADER_MAGIC;
    img_dsc_.header.w = 320;
    img_dsc_.header.h = 240;
    img_dsc_.data_size = 320 * 240 * 2;
    img_dsc_.data = (const uint8_t*)data;
    lv_image_set_src(image_, &img_dsc_);

    kyoshin_monitor->startUpdateTimer();
}
void KyoshinScreen::onDisappear() {
    kyoshin_monitor->setCallback(nullptr);
    kyoshin_monitor->stopUpdateTimer();
}

void KyoshinScreen::onData(uint16_t *data) {
    auto input = BilinearInput{ KYOSHIN_SERVER_CONFIG.imgWidth, KYOSHIN_SERVER_CONFIG.imgHeight, data };
    auto output = BilinearOutput{ 320, 240, data };
    bilinear_resize(&input, &output);
    lv_lock();
    lv_async_call([this, data](){
        if (data) {
            img_dsc_.data = (const uint8_t*)data;
            update(kyoshin_monitor->getForecast(), &img_dsc_);
        } else {
            update(kyoshin_monitor->getForecast(), nullptr);
        }
    });
    lv_unlock();
}

void KyoshinScreen::update(const KyoshinForecast &forecast, const lv_image_dsc_t *img) {
    if (img) {
        lv_image_set_src(image_, img);
    }
    if (!forecast.empty()) {
        printf("forecast: %s\n", forecast.reportNumString().c_str());
    }
}
