#pragma once
#include "screen_manager.hpp"
#include "kyoshin_monitor.hpp"

class KyoshinScreen: public Screen, KyoshinMonitorCallback {
public:
    virtual void build();
    virtual void onAppear();
    virtual void onDisappear();
    virtual void onData(uint16_t *data);

private:
    lv_obj_t *image_;
    lv_img_dsc_t img_dsc_{};

    void update(const KyoshinForecast &forecast, const lv_image_dsc_t *img);
};
