/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "M5Unified.h"
#include "esp_heap_caps.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target_obj(e);
    if(code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

/**
 * Create a button with a label and react on click event.
 */
void lv_example_get_started_2(void)
{
    lv_obj_t * btn = lv_button_create(lv_screen_active());     /*Add a button the current screen*/
    lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

    lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");                     /*Set the labels text*/
    lv_obj_center(label);
}


static uint16_t *lvgl_frame_buffer;

extern "C" void app_main(void) {
    M5.begin();
    M5.Display.setSwapBytes(true);
    int width = M5.Display.width(), height = M5.Display.height();

    lvgl_frame_buffer = (uint16_t*)pvPortMalloc(width * height * sizeof(uint16_t));
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
    lv_display_set_buffers(disp, lvgl_frame_buffer, NULL, width * height * sizeof(uint16_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
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

    lv_example_get_started_2();
    while (true) {
        vTaskDelay(1000);
    }
}
