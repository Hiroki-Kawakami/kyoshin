#include "kyoshin_screen.hpp"

void KyoshinScreen::build() {
    auto label = lv_label_create(root_);
    lv_obj_center(label);
    lv_label_set_text(label, "Kyoshin Monitor");
}
