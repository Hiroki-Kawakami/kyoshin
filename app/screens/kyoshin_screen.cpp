#include "kyoshin_screen.hpp"
#include "kyoshin_app.hpp"
#include "bilinear.hpp"

void KyoshinScreen::build() {
    if (!kyoshin_monitor) {
        kyoshin_monitor = new KyoshinMonitor();
        kyoshin_monitor->setCallback(this);
    }
    auto label = lv_label_create(root_);
    lv_obj_center(label);
    lv_label_set_text(label, "Kyoshin Monitor");

    image_ = lv_image_create(root_);
    lv_obj_center(image_);
    lv_obj_set_size(image_, 320, 240);
}

void KyoshinScreen::onAppear() {
    kyoshin_monitor->startUpdateTimer();
}
void KyoshinScreen::onDisappear() {
    kyoshin_monitor->stopUpdateTimer();
}

void KyoshinScreen::onData(uint16_t *data) {
    auto input = BilinearInput{ KYOSHIN_SERVER_CONFIG.imgWidth, KYOSHIN_SERVER_CONFIG.imgHeight, data };
    auto output = BilinearOutput { 320, 240, data };
    bilinear_resize(&input, &output);
    lv_lock();
    lv_async_call([this, data](){
        static lv_image_dsc_t img;
        img.header.cf = LV_COLOR_FORMAT_RGB565;
        img.header.magic = LV_IMAGE_HEADER_MAGIC;
        img.header.w = 320;
        img.header.h = 240;
        img.data_size = 320 * 240 * 2;
        if (data) {
            img.data = (const uint8_t*)data;
            update(kyoshin_monitor->getForecast(), &img);
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
