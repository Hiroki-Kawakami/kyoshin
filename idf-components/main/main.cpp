#include "M5Unified.h"
#include "esp_lvgl_port.h"
#include "kyoshin_port.hpp"
#include "kyoshin_app.hpp"
#include "kyoshin_settings.hpp"

static PowerMode power_mode = PowerMode::Normal;
static uint32_t last_activity_tick;

static void gui_init() {
    M5.begin();
    M5.Display.setSwapBytes(true);
    int fbsize = 320 * 80 * sizeof(uint16_t);

    uint16_t *lvgl_frame_buffer = (uint16_t*)pvPortMalloc(fbsize);
    assert(lvgl_frame_buffer);

    lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,
        .task_stack = 7168,
        .task_affinity = 0,
        .task_max_sleep_ms = 500,
        .task_stack_caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_DEFAULT,
        .timer_period_ms = 5,
    };
    lvgl_port_init(&lvgl_cfg);

    lv_display_t *disp = lv_display_create(M5.Display.width(), M5.Display.height());
    lv_display_set_buffers(disp, lvgl_frame_buffer, NULL, fbsize, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, [](lv_display_t *disp, const lv_area_t *area, uint8_t *px_map){
        M5.Display.pushImage(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (uint16_t*)px_map);
        lv_display_flush_ready(disp);
    });

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, [](lv_indev_t *indev, lv_indev_data_t *data){
        lgfx::v1::touch_point_t tp;
        auto count = M5.Display.getTouchRaw(&tp);
        if (kyoshin_port_get_power_mode() == PowerMode::Standby || kyoshin_port_get_power_mode() == PowerMode::Night) {
            if (count > 0) kyoshin_port_set_power_mode(PowerMode::Interrupt);
            data->state = LV_INDEV_STATE_RELEASED;
            return;
        }
        if (kyoshin_port_get_power_mode() == PowerMode::Interrupt) {
            if (count == 0) kyoshin_port_set_power_mode(PowerMode::Normal);
            data->state = LV_INDEV_STATE_RELEASED;
            return;
        }
        if (count > 0) {
            data->point.x = tp.x;
            data->point.y = tp.y;
            data->state = LV_INDEV_STATE_PRESSED;
            kyoshin_port_feed_last_activity_tick();
        } else {
            data->state = LV_INDEV_STATE_RELEASED;
        }
    });
    kyoshin_port_feed_last_activity_tick();
}

extern "C" void app_main(void) {
    gui_init();
    kyoshin_app();
}

PowerMode kyoshin_port_get_power_mode() {
    return power_mode;
}
void kyoshin_port_update_power_mode(time_t time, const KyoshinForecast &forecast) {
    if (forecast.isAlert() || lv_tick_elaps(last_activity_tick) < kyoshin_settings.getStandbyDuration()) {
        kyoshin_port_set_power_mode(PowerMode::Normal);
        return;
    }
    if (kyoshin_settings.inNightMode(time)) {
        kyoshin_port_set_power_mode(PowerMode::Night);
        return;
    }
    if (forecast.empty()) {
        kyoshin_port_set_power_mode(PowerMode::Standby);
        return;
    }
    kyoshin_port_set_power_mode(PowerMode::Normal);
}
void kyoshin_port_set_power_mode(PowerMode mode) {
    if (power_mode == mode) return;
    switch (mode) {
    case PowerMode::Standby:
        M5.Display.setBrightness(kyoshin_settings.getStandbyBrightness());
        break;
    case PowerMode::Night:
        M5.Display.setBrightness(kyoshin_settings.getNightBrightness());
        break;
    default:
        M5.Display.setBrightness(kyoshin_settings.getBrightness());
        kyoshin_port_feed_last_activity_tick();
        break;
    }
    power_mode = mode;
}
void kyoshin_port_feed_last_activity_tick() {
    last_activity_tick = lv_tick_get();
}
