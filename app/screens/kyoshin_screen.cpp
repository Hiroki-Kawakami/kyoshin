#include "kyoshin_screen.hpp"
#include "kyoshin_app.hpp"
#include "bilinear.hpp"
#include "map_load_screen.hpp"
#include "settings/settings_screen.hpp"
#include "sound_controller.hpp"

void KyoshinScreen::build() {
    if (!kyoshin_monitor) {
        kyoshin_monitor = new KyoshinMonitor();
    }
    lv_obj_add_flag(root_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_fn(root_, LV_EVENT_PRESSED, [this](lv_event_t*){ screenClicked(); });
    image_ = lv_image_create(root_);
}

void KyoshinScreen::onAppear() {
    kyoshin_monitor->setCallback(this);
    setImageSource(false);
    if (!kyoshin_monitor->loadBaseMapImage()) {
        screen_manager.push(std::make_unique<MapLoadScreen>());
        return;
    }

    uint16_t width, height;
    auto screen_layout = preferredScreenLayout();
    buildScreenLayout(screen_layout);
    preferredImageSize(screen_layout, &width, &height);

    auto data = kyoshin_monitor->copyBaseMapImage();
    auto input = BilinearInput{ KYOSHIN_SERVER_CONFIG.imgWidth, KYOSHIN_SERVER_CONFIG.imgHeight, data };
    auto output = BilinearOutput{ width, height, data };
    bilinear_resize(&input, &output);

    img_dsc_.header.cf = LV_COLOR_FORMAT_RGB565;
    img_dsc_.header.magic = LV_IMAGE_HEADER_MAGIC;
    img_dsc_.header.w = width;
    img_dsc_.header.h = height;
    img_dsc_.data_size = width * height * 2;
    img_dsc_.data = (const uint8_t*)data;
    lv_image_set_src(image_, &img_dsc_);

    kyoshin_monitor->startUpdateTimer();
    kyoshin_port_feed_last_activity_tick();
}
void KyoshinScreen::onDisappear() {
    kyoshin_monitor->setCallback(nullptr);
    kyoshin_monitor->stopUpdateTimer();
    sound_controller.stop();
}

void KyoshinScreen::onData(time_t time, uint16_t *data) {
    uint16_t width, height;
    ScreenLayout screen_layout = preferredScreenLayout();
    preferredImageSize(screen_layout, &width, &height);

    if (data) {
        auto input = BilinearInput{ KYOSHIN_SERVER_CONFIG.imgWidth, KYOSHIN_SERVER_CONFIG.imgHeight, data };
        auto output = BilinearOutput{ width, height, data };
        bilinear_resize(&input, &output);
    }
    ring(time, kyoshin_monitor->getForecast());

    lv_lock();
    lv_async_call([this, time, screen_layout, width, height, data](){
        if (data) {
            img_dsc_.header.w = width;
            img_dsc_.header.h = height;
            img_dsc_.data_size = width * height * 2;
            img_dsc_.data = (const uint8_t*)data;
            update(time, screen_layout, &img_dsc_);
        } else {
            update(time, screen_layout, nullptr);
        }
    });
    lv_unlock();
}

void KyoshinScreen::setImageSource(bool reload) {
    kyoshin_monitor->setImageSource(
        kyoshin_settings.getMapRegion(),
        kyoshin_settings.getBorehole(),
        kyoshin_settings.getRealtimeImageType(),
        reload);
}

ScreenLayout KyoshinScreen::preferredScreenLayout() const {
    auto screen_layout = kyoshin_settings.getScreenLayout();
    if (screen_layout == ScreenLayout::AutoHorizontal) {
        if (kyoshin_monitor->getForecast().empty()) {
            return ScreenLayout::ZoomHorizontal;
        } else {
            return ScreenLayout::HorizontalInfo;
        }
    } else {
        return screen_layout;
    }
}

void KyoshinScreen::preferredImageSize(ScreenLayout layout, uint16_t *width, uint16_t *height) {
    switch (layout) {
    case ScreenLayout::ZoomHorizontal:
        *width = 320;
        *height = 240;
        break;
    case ScreenLayout::ZoomVertical:
        *width = 240;
        *height = 320;
        break;
    default:
        *width = 212;
        *height = 240;
        break;
    }
}

void KyoshinScreen::ring(time_t time, const KyoshinForecast &forecast) {
    if (forecast.empty() ||
        forecast.isFinal ||
        (forecast.isTraining && kyoshin_settings.getMuteTraining()) ||
        kyoshin_port_get_power_mode() == PowerMode::Night ||
        (kyoshin_settings.inNightMode(time) && kyoshin_settings.getNightBehavior(forecast.isAlert()) != NightBehavior::Normal)) {
        sound_controller.stop();
        return;
    }

    if (forecast.isUpdated()) {
        sound_controller.play(
            forecast.isAlert() ? kyoshin_settings.getAlertSound() : kyoshin_settings.getNormalSound());
    }
}

void KyoshinScreen::buildScreenLayout(ScreenLayout screen_layout) {
    if (screen_layout_ == screen_layout) return;

    if (screen_layout == ScreenLayout::ZoomHorizontal ||
        screen_layout == ScreenLayout::ZoomVertical) {
        lv_obj_set_size(image_, 320, 240);
        if (screen_layout == ScreenLayout::ZoomVertical) {
            lv_img_set_angle(image_, 900);
        } else {
            lv_img_set_angle(image_, 0);
        }

        if (forecast_) {
            lv_obj_delete(forecast_);
            forecast_ = nullptr;
        }
        if (forecast_header_) {
            lv_obj_delete(forecast_header_);
            forecast_header_ = nullptr;
        }
    } else {
        lv_obj_set_size(image_, 212, 240);
        lv_img_set_angle(image_, 0);

        if (!forecast_) {
            forecast_ = lv_obj_create(root_);
            lv_obj_remove_style_all(forecast_);
            lv_obj_remove_flag(forecast_, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_size(forecast_, 108, 240);
            lv_obj_align(forecast_, LV_ALIGN_TOP_RIGHT, 0, 0);
            lv_obj_set_style_bg_color(forecast_, lv_color_white(), 0);
            lv_obj_set_style_bg_opa(forecast_, LV_OPA_COVER, 0);
            lv_obj_set_style_border_side(forecast_, LV_BORDER_SIDE_LEFT, 0);
            lv_obj_set_style_border_width(forecast_, 4, 0);
            lv_obj_set_style_border_color(forecast_, lv_color_hex(0xd3d3d3), 0);
            lv_obj_set_style_border_opa(forecast_, LV_OPA_COVER, 0);
        }
        if (!forecast_header_) {
            forecast_header_ = lv_obj_create(root_);
            lv_obj_remove_style_all(forecast_header_);
            lv_obj_remove_flag(forecast_header_, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_size(forecast_header_, 104, 88);
            lv_obj_align(forecast_header_, LV_ALIGN_TOP_RIGHT, 0, 0);
            lv_obj_set_style_bg_color(forecast_header_, lv_color_hex(0xd3d3d3), 0);
            lv_obj_set_style_bg_opa(forecast_header_, LV_OPA_COVER, 0);
        }
        if (menu_) lv_obj_move_foreground(menu_);
    }

    screen_layout_ = screen_layout;
}

void KyoshinScreen::updateForecast(const KyoshinForecast &forecast) {
    if (forecast_) {
        lv_obj_clean(forecast_);
        lv_obj_set_style_border_color(forecast_, lv_color_hex(forecast.color()), 0);

        if (!forecast.empty()) {
            auto magnitude = lv_label_create(forecast_);
            lv_obj_align(magnitude, LV_ALIGN_TOP_MID, 0, 96);
            lv_label_set_text(magnitude, ("M" + forecast.magnitude).c_str());
            lv_obj_set_style_text_font(magnitude, R.font.ipa_24, 0);
            lv_obj_set_style_text_color(magnitude, lv_color_black(), 0);

            auto depth_title = lv_label_create(forecast_);
            lv_obj_align(depth_title, LV_ALIGN_TOP_LEFT, 4, 128);
            lv_label_set_text(depth_title, "深さ");
            lv_obj_set_style_text_font(depth_title, R.font.ipa_16, 0);
            lv_obj_set_style_text_color(depth_title, lv_color_black(), 0);

            auto depth = lv_label_create(forecast_);
            lv_obj_align(depth, LV_ALIGN_TOP_MID, 0, 144);
            lv_label_set_text(depth, forecast.depth.c_str());
            lv_obj_set_style_text_font(depth, R.font.ipa_24, 0);
            lv_obj_set_style_text_color(depth, lv_color_black(), 0);

            auto region = lv_label_create(forecast_);
            lv_obj_align(region, LV_ALIGN_TOP_LEFT, 4, 176);
            lv_obj_set_width(region, 96);
            lv_label_set_text(region, forecast.regionName.c_str());
            lv_obj_set_style_text_font(region, R.font.ipa_16, 0);
            lv_obj_set_style_text_color(region, lv_color_black(), 0);
            lv_label_set_long_mode(region, LV_LABEL_LONG_WRAP);

            auto report_num = lv_label_create(forecast_);
            lv_obj_align(report_num, LV_ALIGN_BOTTOM_RIGHT, -4, -2);
            lv_label_set_text(report_num, forecast.reportNumString().c_str());
            lv_obj_set_style_text_font(report_num, R.font.ipa_16, 0);
            lv_obj_set_style_text_color(report_num, lv_color_black(), 0);
        }
    }
    if (forecast_header_) {
        lv_obj_clean(forecast_header_);
        lv_obj_set_style_bg_color(forecast_header_, lv_color_hex(forecast.color()), 0);

        if (!forecast.empty()) {
            auto alert_label = lv_label_create(forecast_header_);
            lv_obj_align(alert_label, LV_ALIGN_TOP_LEFT, 4, 8);
            lv_label_set_text(alert_label, forecast.alertflg.c_str());
            lv_obj_set_style_text_font(alert_label, R.font.ipa_24, 0);
            lv_obj_set_style_text_color(alert_label, lv_color_white(), 0);

            auto shindo_title = lv_label_create(forecast_header_);
            lv_obj_align(shindo_title, LV_ALIGN_BOTTOM_LEFT, 2, -6);
            lv_label_set_text(shindo_title, "最大\n震度");
            lv_obj_set_style_text_font(shindo_title, R.font.ipa_16, 0);
            lv_obj_set_style_text_color(shindo_title, lv_color_white(), 0);

            auto shindo_label = lv_label_create(forecast_header_);
            lv_obj_set_size(shindo_label, 68, LV_SIZE_CONTENT);
            lv_obj_align(shindo_label, LV_ALIGN_BOTTOM_RIGHT, -2, -8);
            lv_obj_set_style_text_align(shindo_label, LV_TEXT_ALIGN_CENTER, 0);
            lv_label_set_text(shindo_label, forecast.calcintensity.c_str());
            lv_obj_set_style_text_font(shindo_label, R.font.ipa_numbers_40, 0);
            lv_obj_set_style_text_color(shindo_label, lv_color_white(), 0);
        }
    }
}

void KyoshinScreen::update(time_t time, ScreenLayout screen_layout, const lv_image_dsc_t *img) {
    buildScreenLayout(screen_layout);
    if (img) lv_image_set_src(image_, img);

    if (screen_layout == ScreenLayout::HorizontalInfo) {
        updateForecast(kyoshin_monitor->getForecast());
    }
}

void KyoshinScreen::screenClicked() {
    if (!menu_) openMenu();
    else closeMenu();
}

void KyoshinScreen::openMenu() {
    closeMenu();
    menu_ = lv_obj_create(root_);
    lv_obj_remove_style_all(menu_);
    lv_obj_set_size(menu_, LV_PCT(100), 94);
    lv_obj_align(menu_, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(menu_, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(menu_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(menu_, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_width(menu_, 2, 0);
    lv_obj_set_style_border_color(menu_, lv_color_hex(0xdddddd), 0);
    lv_obj_set_style_border_opa(menu_, LV_OPA_COVER, 0);

    auto create_dropdown = [](lv_obj_t *parent){
        auto dropdown = lv_dropdown_create(parent);
        lv_obj_set_height(dropdown, 40);
        lv_obj_set_style_text_font(dropdown, R.font.ipa_16, 0);
        auto list = lv_dropdown_get_list(dropdown);
        lv_obj_set_style_text_font(list, R.font.ipa_16, 0);
        return dropdown;
    };

    auto region_dd = create_dropdown(menu_);
    lv_obj_set_width(region_dd, 125);
    lv_obj_align(region_dd, LV_ALIGN_TOP_LEFT, 4, 4);
    lv_dropdown_set_options(region_dd, "全国\n能登半島");
    lv_dropdown_set_selected(region_dd, kyoshin_monitor->getMapRegion().value);
    lv_obj_add_event_fn(region_dd, LV_EVENT_VALUE_CHANGED, [this, region_dd](lv_event_t*){
        auto i = lv_dropdown_get_selected(region_dd);
        kyoshin_settings.setMapRegion(i);
        refresh();
    });

    auto borehole_dd = create_dropdown(menu_);
    lv_obj_set_width(borehole_dd, 95);
    lv_obj_align(borehole_dd, LV_ALIGN_TOP_LEFT, 133, 4);
    lv_dropdown_set_options(borehole_dd, "地表\n地中");
    lv_dropdown_set_selected(borehole_dd, kyoshin_monitor->getBorehole());
    lv_obj_add_event_fn(borehole_dd, LV_EVENT_VALUE_CHANGED, [this, borehole_dd](lv_event_t*){
        auto i = lv_dropdown_get_selected(borehole_dd);
        kyoshin_settings.setBorehole(i);
        refresh();
    });

    auto rimg_dd = create_dropdown(menu_);
    lv_obj_set_width(rimg_dd, 224);
    lv_obj_align(rimg_dd, LV_ALIGN_BOTTOM_LEFT, 4, -4);
    lv_dropdown_set_options(rimg_dd,
        "リアルタイム震度\n"
        "最大加速度\n"
        "最大速度\n"
        "最大変位\n"
        "0.125Hz速度応答\n"
        "0.25Hz速度応答\n"
        "0.5Hz速度応答\n"
        "1.0Hz速度応答\n"
        "2.0Hz速度応答\n"
        "4.0Hz速度応答"
    );
    lv_dropdown_set_selected(rimg_dd, kyoshin_monitor->getRealtimeImageType().value);
    lv_obj_add_event_fn(rimg_dd, LV_EVENT_VALUE_CHANGED, [this, rimg_dd](lv_event_t*){
        auto i = lv_dropdown_get_selected(rimg_dd);
        kyoshin_settings.setRealtimeImageType(i);
        refresh();
    });

    auto settings_button = lv_button_create(menu_);
    lv_obj_set_size(settings_button, 84, 84);
    lv_obj_align(settings_button, LV_ALIGN_RIGHT_MID, -4, 0);
    lv_obj_set_flex_flow(settings_button, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(settings_button, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_event_fn(settings_button, LV_EVENT_CLICKED, [](lv_event_t*){
        screen_manager.push(std::make_unique<SettingsScreen>());
    });
    auto settings_image = lv_image_create(settings_button);
    lv_image_set_src(settings_image, R.icon.settings);
    auto settings_label = lv_label_create(settings_button);
    lv_label_set_text(settings_label, "設定");
    lv_obj_set_style_text_font(settings_label, R.font.ipa_16, 0);
}

void KyoshinScreen::closeMenu() {
    if (menu_) {
        lv_obj_delete(menu_);
        menu_ = nullptr;
    }
}

void KyoshinScreen::refresh() {
    setImageSource(true);
    if (!kyoshin_monitor->loadBaseMapImage()) {
        screen_manager.push(std::make_unique<MapLoadScreen>());
        return;
    }
    kyoshin_monitor->startUpdateTimer();
    kyoshin_monitor->updateImage();
}
