#include "kyoshin_port.hpp"
#include "M5Unified.h"

// MARK: Task
void kyoshin_port_task_create(const char *name, std::function<void()> fn, int priority, size_t stack_size, int core) {
    xTaskCreatePinnedToCore(
        [](void *arg){
            auto func = static_cast<std::function<void()>*>(arg);
            (*func)();
            delete func;
        },
        name,
        stack_size / sizeof(size_t),
        new std::function<void()>(std::move(fn)),
        priority,
        NULL,
        core);
}

// MARK: Timer
kyoshin_port_timer_t *kyoshin_port_timer_create(const char *name, std::function<void()> fn) {
    kyoshin_port_timer_t *timer = new kyoshin_port_timer_t();
    esp_timer_create_args_t args = {};
    args.name = name;
    args.callback = [](void *arg){
        auto timer = static_cast<kyoshin_port_timer_t*>(arg);
        timer->fn();
    };
    args.arg = timer;

    timer->fn = std::move(fn);
    esp_timer_create(&args, &timer->handle);
    return timer;
}
void kyoshin_port_timer_start_periodic(kyoshin_port_timer_t *timer, int period_ms) {
    esp_timer_start_periodic(timer->handle, period_ms * 1000);
}
void kyoshin_port_timer_stop(kyoshin_port_timer_t *timer) {
    esp_timer_stop(timer->handle);
}
void kyoshin_port_timer_delete(kyoshin_port_timer_t *timer) {
    esp_timer_delete(timer->handle);
    delete timer;
}

void kyoshin_port_wav_play(const uint8_t *data, int volume, int repeat) {
    M5.Speaker.setVolume(volume);
    M5.Speaker.playWav(data, ~0, repeat, -1, true);
}
void kyoshin_port_wav_stop() {
    M5.Speaker.stop();
}
