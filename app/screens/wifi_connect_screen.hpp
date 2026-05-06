#pragma once
#include "screen_manager.hpp"

class WiFiConnectScreen: public Screen {
public:
    virtual void build();
    virtual void onAppear();
    void check();

private:
    lv_timer_t *timer_;
};
