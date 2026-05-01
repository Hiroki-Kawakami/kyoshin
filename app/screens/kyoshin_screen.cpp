#include "kyoshin_screen.hpp"
#include "kyoshin_app.hpp"

void KyoshinScreen::build() {
    if (!kyoshin_monitor) {
        kyoshin_monitor = new KyoshinMonitor();
    }
    auto label = lv_label_create(root_);
    lv_obj_center(label);
    lv_label_set_text(label, "Kyoshin Monitor");
}

void KyoshinScreen::onAppear() {
    kyoshin_monitor->startUpdateTimer();
}
void KyoshinScreen::onDisappear() {
    kyoshin_monitor->stopUpdateTimer();
}
