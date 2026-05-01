#include "kyoshin_screen.hpp"
#include "kyoshin_app.hpp"

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
    lv_image_set_scale_x(image_, 232);
    lv_image_set_scale_y(image_, 153);
}

void KyoshinScreen::onAppear() {
    kyoshin_monitor->startUpdateTimer();
}
void KyoshinScreen::onDisappear() {
    kyoshin_monitor->stopUpdateTimer();
}

void KyoshinScreen::onData(const uint16_t *data) {
    printf("onData: %p\n", data);
    lv_lock();
    lv_async_call([this, data](){
        static lv_image_dsc_t img;
        img.header.cf = LV_COLOR_FORMAT_RGB565;
        img.header.magic = LV_IMAGE_HEADER_MAGIC;
        img.header.w = KYOSHIN_SERVER_CONFIG.imgWidth;
        img.header.h = KYOSHIN_SERVER_CONFIG.imgHeight;
        img.data_size = KYOSHIN_SERVER_CONFIG.imgWidth * KYOSHIN_SERVER_CONFIG.imgHeight * 2;
        img.data = (const uint8_t*)data;
        lv_image_set_src(image_, &img);
    });
    lv_unlock();
}
