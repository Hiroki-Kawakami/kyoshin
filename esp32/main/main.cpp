#include "M5Unified.h"
#include "esp_lvgl_port.h"

static void gui_init() {
    M5.begin();
    M5.Display.setSwapBytes(true);
    int fbsize = 240 * 80 * sizeof(uint16_t);

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
        if (count > 0) {
            data->point.x = tp.x;
            data->point.y = tp.y;
            data->state = LV_INDEV_STATE_PRESSED;
        } else {
            data->state = LV_INDEV_STATE_RELEASED;
        }
    });
}

extern "C" void app_main(void) {
    gui_init();

    void kyoshin_app(void);
    kyoshin_app();
}
