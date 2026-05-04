#pragma once
#include <functional>
#include <cstdint>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_timer.h"
#include "cJSON.h"
#include "kyoshin_forecast.hpp"

// MARK: Task
void kyoshin_port_task_create(const char *name, std::function<void()> fn, int priority, size_t stack_size, int core);

// MARK: Timer
struct kyoshin_port_timer_t {
    esp_timer_handle_t handle;
    std::function<void()> fn;
};
kyoshin_port_timer_t *kyoshin_port_timer_create(const char *name, std::function<void()> fn);
void kyoshin_port_timer_start_periodic(kyoshin_port_timer_t *timer, int period_ms);
void kyoshin_port_timer_stop(kyoshin_port_timer_t *timer);
void kyoshin_port_timer_restart(kyoshin_port_timer_t *timer, int timeout_ms);
void kyoshin_port_timer_delete(kyoshin_port_timer_t *timer);

// MARK: EventGroup
template<class T>
class EventGroup {
private:
    EventGroupHandle_t handle;
public:
    inline EventGroup() {
        this->handle = xEventGroupCreate();
    }
    EventGroup(const EventGroup &) = delete;
    EventGroup& operator=(const EventGroup &) = delete;
    inline EventGroup(EventGroup &&rval) {
        this->handle = rval.handle;
        rval.handle = nullptr;
    }
    inline EventGroup& operator=(EventGroup &&rval) {
        this->handle = rval.handle;
        rval.handle = nullptr;
        return *this;
    }
    inline ~EventGroup() {
        if (this->handle) vEventGroupDelete(this->handle);
    }

    inline bool contains(T bits) {
        return xEventGroupGetBits(this->handle) & static_cast<EventBits_t>(bits);
    }
    inline void setBits(T bits) {
        xEventGroupSetBits(this->handle, static_cast<EventBits_t>(bits));
    }
    inline void clearBits(T bits) {
        xEventGroupClearBits(this->handle, static_cast<EventBits_t>(bits));
    }
    inline T waitBits(T bits, TickType_t ticks_to_wait = portMAX_DELAY) {
        return static_cast<T>(xEventGroupWaitBits(this->handle, static_cast<EventBits_t>(bits), false, false, ticks_to_wait));
    }
    inline T waitAllBits(T bits, TickType_t ticks_to_wait = portMAX_DELAY) {
        return static_cast<T>(xEventGroupWaitBits(this->handle, static_cast<EventBits_t>(bits), false, true, ticks_to_wait));
    }
    inline T waitBitsAndClear(T bits, TickType_t ticks_to_wait = portMAX_DELAY) {
        return static_cast<T>(xEventGroupWaitBits(this->handle, static_cast<EventBits_t>(bits), true, false, ticks_to_wait));
    }
    inline T waitAllBitsAndClear(T bits, TickType_t ticks_to_wait = portMAX_DELAY) {
        return static_cast<T>(xEventGroupWaitBits(this->handle, static_cast<EventBits_t>(bits), true, true, ticks_to_wait));
    }
};

// MARK: Audio
void kyoshin_port_wav_play(const uint8_t *data, int volume, int repeat);
void kyoshin_port_wav_stop();

// MARK: Power Mode
enum class PowerMode {
    Normal,
    Standby,
    Night,
    Interrupt,
};
PowerMode kyoshin_port_get_power_mode();
void kyoshin_port_update_power_mode(time_t time, const KyoshinForecast &forecast);
void kyoshin_port_set_power_mode(PowerMode mode);
void kyoshin_port_feed_last_activity_tick();
void kyoshin_port_set_brightness(int brightness);
