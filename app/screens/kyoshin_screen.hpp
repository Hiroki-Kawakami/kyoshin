#pragma once
#include <optional>
#include "screen_manager.hpp"
#include "kyoshin_monitor.hpp"
#include "kyoshin_settings.hpp"

class KyoshinScreen: public Screen, KyoshinMonitorCallback {
public:
    virtual void build();
    virtual void onAppear();
    virtual void onDisappear();
    virtual void onData(time_t time, uint16_t *data);

private:
    std::optional<ScreenLayout> screen_layout_{std::nullopt};
    lv_obj_t *image_;
    lv_img_dsc_t img_dsc_{};

    lv_obj_t *forecast_{nullptr};
    lv_obj_t *forecast_header_{nullptr};
    lv_obj_t *menu_{nullptr};

    ScreenLayout preferredScreenLayout() const;
    void preferredImageSize(ScreenLayout layout, uint16_t *width, uint16_t *height);
    void ring(time_t time, const KyoshinForecast &forecast);
    void buildScreenLayout(ScreenLayout screen_layout);
    void updateForecast(const KyoshinForecast &forecast);
    void update(time_t time, ScreenLayout screen_layout, const lv_image_dsc_t *img);

    void screenClicked();
    void openMenu();
    void closeMenu();
    void refresh();
};
