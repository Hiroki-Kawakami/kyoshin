#include "map_load_screen.hpp"
#include "kyoshin_app.hpp"

void MapLoadScreen::build() {
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    auto spinner = lv_spinner_create(root_);
    lv_obj_set_size(spinner, 60, 60);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_INDICATOR);

    auto label = lv_label_create(root_);
    lv_obj_set_style_pad_top(label, 16, 0);
    lv_label_set_text(label, "Downloading map image...");
}

void MapLoadScreen::onAppear() {
    printf("MapLoadScreen::onAppear\n");
    kyoshin_monitor->setCallback(this);
    kyoshin_monitor->loadBaseMapImage(true);
}

void MapLoadScreen::onDisappear() {
    kyoshin_monitor->setCallback(nullptr);
}

void MapLoadScreen::onBaseMapReady(bool result) {
    printf("MapLoadScreen::onBaseMapReady: %d\n", result);
    lv_lock();
    lv_async_call([this, result](){
        if (result) screen_manager.pop();
        else buildErrorScreen();
    });
    lv_unlock();
}

void MapLoadScreen::buildErrorScreen() {
    lv_obj_clean(root_);
    auto label = lv_label_create(root_);
    lv_label_set_text(label, "Failed to download Base Map Image");

    auto button = lv_button_create(root_);
    auto button_label = lv_label_create(button);
    lv_label_set_text(button_label, "Retry");
    lv_obj_center(button_label);
    lv_obj_add_event_fn(button, LV_EVENT_CLICKED, [](lv_event_t*){ screen_manager.pop(); });
}
