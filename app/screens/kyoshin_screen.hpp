#pragma once
#include "screen_manager.hpp"
#include "kyoshin_monitor.hpp"

class KyoshinScreen: public Screen, KyoshinMonitorCallback {
public:
    virtual void build();
    virtual void onAppear();
    virtual void onDisappear();
    virtual void onData(const uint16_t *data);

private:
    lv_obj_t *image_;
};
