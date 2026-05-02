#pragma once
#include "screen_manager.hpp"
#include "kyoshin_monitor.hpp"

class MapLoadScreen: public Screen, KyoshinMonitorCallback {
public:
    MapLoadScreen() {}
    virtual void build();
    virtual void onAppear();
    virtual void onDisappear();
    virtual void onBaseMapReady(bool result);

private:
    lv_timer_t *timer_;
};
