#pragma once
#include "screen_manager.hpp"

class WiFiConnectScreen: public Screen {
public:
    virtual void build();
    virtual void onAppear();
    void check();
    void animateDots();

private:
    lv_timer_t *timer_;
    lv_timer_t *dot_timer_;
    lv_obj_t *status_label_;
    int dot_count_ = 0;
};
