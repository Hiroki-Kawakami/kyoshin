#pragma once
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <optional>
#include <functional>
#include <cjson/cJSON.h>
#include <SDL2/SDL.h>
#include "kyoshin_forecast.hpp"

// FreeRTOS Compat Definitions
using TickType_t = uint32_t;
using EventBits_t = uint32_t;
static constexpr TickType_t portMAX_DELAY = UINT32_MAX;
static constexpr TickType_t pdMS_TO_TICKS(uint32_t ms) { return ms; }

// MARK: Task
inline void kyoshin_port_task_create(const char * /*name*/, std::function<void()> fn, int /*priority*/, size_t /*stack_size*/, int /*core*/) {
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, &attr, [](void *arg) -> void* {
        auto func = static_cast<std::function<void()>*>(arg);
        (*func)();
        delete func;
        return nullptr;
    }, new std::function<void()>(std::move(fn)));
    pthread_attr_destroy(&attr);
}

// MARK: Timer
struct kyoshin_port_timer_t {
    std::function<void()> fn;
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool running = false;
    bool restarted = false;
    int period_ms = 0;
};
static void *kyoshin_port_timer_thread(void *arg) {
    auto timer = static_cast<kyoshin_port_timer_t*>(arg);
    pthread_mutex_lock(&timer->mutex);
    while (timer->running) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec  += timer->period_ms / 1000;
        ts.tv_nsec += static_cast<long>(timer->period_ms % 1000) * 1'000'000L;
        if (ts.tv_nsec >= 1'000'000'000L) {
            ts.tv_sec++;
            ts.tv_nsec -= 1'000'000'000L;
        }
        timer->restarted = false;
        pthread_cond_timedwait(&timer->cond, &timer->mutex, &ts);
        if (timer->running && !timer->restarted) {
            timer->fn();
        }
    }
    pthread_mutex_unlock(&timer->mutex);
    return nullptr;
}
inline kyoshin_port_timer_t *kyoshin_port_timer_create(const char * /*name*/, std::function<void()> fn) {
    auto *timer = new kyoshin_port_timer_t();
    timer->fn = fn;
    pthread_mutex_init(&timer->mutex, nullptr);
    pthread_cond_init(&timer->cond, nullptr);
    return timer;
}
inline void kyoshin_port_timer_start_periodic(kyoshin_port_timer_t *timer, int period_ms) {
    timer->period_ms = period_ms;
    timer->running = true;
    pthread_create(&timer->thread, nullptr, kyoshin_port_timer_thread, timer);
}
inline void kyoshin_port_timer_restart(kyoshin_port_timer_t *timer, int timeout_ms) {
    pthread_mutex_lock(&timer->mutex);
    timer->period_ms = timeout_ms;
    timer->restarted = true;
    pthread_cond_signal(&timer->cond);
    pthread_mutex_unlock(&timer->mutex);
}
inline void kyoshin_port_timer_stop(kyoshin_port_timer_t *timer) {
    pthread_mutex_lock(&timer->mutex);
    timer->running = false;
    pthread_cond_signal(&timer->cond);
    pthread_mutex_unlock(&timer->mutex);
    pthread_join(timer->thread, nullptr);
}
inline void kyoshin_port_timer_delete(kyoshin_port_timer_t *timer) {
    pthread_mutex_destroy(&timer->mutex);
    pthread_cond_destroy(&timer->cond);
    delete timer;
}

// MARK: EventGroup
class EventGroupBase {
protected:
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    EventBits_t     bits = 0;

    struct WaitOptions {
        EventBits_t bits_to_wait;
        bool        wait_all;
        bool        clear_on_exit;
        TickType_t  ticks_to_wait;
    };

    EventBits_t waitImpl(const WaitOptions &opt) {
        auto satisfied = [&]() -> bool {
            return opt.wait_all
                ? (bits & opt.bits_to_wait) == opt.bits_to_wait
                : (bits & opt.bits_to_wait) != 0;
        };

        if (opt.ticks_to_wait == portMAX_DELAY) {
            while (!satisfied())
                pthread_cond_wait(&cond, &mutex);
        } else {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec  += opt.ticks_to_wait / 1000;
            ts.tv_nsec += static_cast<long>(opt.ticks_to_wait % 1000) * 1'000'000L;
            if (ts.tv_nsec >= 1'000'000'000L) {
                ts.tv_sec++;
                ts.tv_nsec -= 1'000'000'000L;
            }

            while (!satisfied()) {
                if (pthread_cond_timedwait(&cond, &mutex, &ts) != 0)
                    break; // ETIMEDOUT or error
            }
        }

        EventBits_t result = bits;
        if (satisfied() && opt.clear_on_exit)
            bits &= ~opt.bits_to_wait;

        return result;
    }

public:
    EventGroupBase() {
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);
    }
    EventGroupBase(const EventGroupBase &) = delete;
    EventGroupBase &operator=(const EventGroupBase &) = delete;

    EventGroupBase(EventGroupBase &&rval) noexcept {
        // mutex/condはムーブ不可なので再初期化し、bitsだけ引き継ぐ
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);
        bits = rval.bits;
        rval.bits = 0;
    }

    EventGroupBase &operator=(EventGroupBase &&rval) noexcept {
        if (this != &rval) {
            bits = rval.bits;
            rval.bits = 0;
        }
        return *this;
    }

    ~EventGroupBase() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }
};

template <class T>
class EventGroup : private EventGroupBase {
    static_assert(sizeof(T) <= sizeof(EventBits_t),
                  "T must fit within EventBits_t (uint32_t)");

    static EventBits_t to_bits(T v) { return static_cast<EventBits_t>(v); }
    static T           from_bits(EventBits_t v) { return static_cast<T>(v); }

public:
    EventGroup() = default;
    EventGroup(const EventGroup &) = delete;
    EventGroup &operator=(const EventGroup &) = delete;
    EventGroup(EventGroup &&) = default;
    EventGroup &operator=(EventGroup &&) = default;
    ~EventGroup() = default;

    inline bool contains(T b) {
        pthread_mutex_lock(&mutex);
        bool result = (bits & to_bits(b)) != 0;
        pthread_mutex_unlock(&mutex);
        return result;
    }
    inline void setBits(T b) {
        pthread_mutex_lock(&mutex);
        bits |= to_bits(b);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
    }
    inline void clearBits(T b) {
        pthread_mutex_lock(&mutex);
        bits &= ~to_bits(b);
        pthread_mutex_unlock(&mutex);
    }
    inline T waitBits(T b, TickType_t ticks_to_wait = portMAX_DELAY) {
        pthread_mutex_lock(&mutex);
        EventBits_t result = waitImpl({to_bits(b), false, false, ticks_to_wait});
        pthread_mutex_unlock(&mutex);
        return from_bits(result);
    }
    inline T waitAllBits(T b, TickType_t ticks_to_wait = portMAX_DELAY) {
        pthread_mutex_lock(&mutex);
        EventBits_t result = waitImpl({to_bits(b), true, false, ticks_to_wait});
        pthread_mutex_unlock(&mutex);
        return from_bits(result);
    }
    inline T waitBitsAndClear(T b, TickType_t ticks_to_wait = portMAX_DELAY) {
        pthread_mutex_lock(&mutex);
        EventBits_t result = waitImpl({to_bits(b), false, true, ticks_to_wait});
        pthread_mutex_unlock(&mutex);
        return from_bits(result);
    }
    inline T waitAllBitsAndClear(T b, TickType_t ticks_to_wait = portMAX_DELAY) {
        pthread_mutex_lock(&mutex);
        EventBits_t result = waitImpl({to_bits(b), true, true, ticks_to_wait});
        pthread_mutex_unlock(&mutex);
        return from_bits(result);
    }
};

// MARK: Audio
struct kyoshin_port_wav_state_t {
    SDL_AudioDeviceID dev = 0;
    pthread_mutex_t mtx;
    kyoshin_port_wav_state_t() { pthread_mutex_init(&mtx, nullptr); }
};

inline kyoshin_port_wav_state_t& kyoshin_port_wav_state() {
    static kyoshin_port_wav_state_t s;
    return s;
}

inline void kyoshin_port_wav_stop() {
    auto &s = kyoshin_port_wav_state();
    pthread_mutex_lock(&s.mtx);
    SDL_AudioDeviceID dev = s.dev;
    s.dev = 0;
    pthread_mutex_unlock(&s.mtx);
    if (dev) {
        SDL_ClearQueuedAudio(dev);
        SDL_CloseAudioDevice(dev);
    }
}

inline void kyoshin_port_wav_play(const uint8_t *data, int volume, int repeat) {
    kyoshin_port_wav_stop();
    if (!data) return;
    if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO)) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) return;
    }

    // Derive file size from RIFF header: total = chunk_size field + 8
    if (data[0]!='R'||data[1]!='I'||data[2]!='F'||data[3]!='F') return;
    uint32_t chunk_size;
    memcpy(&chunk_size, data + 4, 4);
    int file_size = (int)(chunk_size + 8);

    SDL_RWops *rw = SDL_RWFromConstMem(data, file_size);
    SDL_AudioSpec wav_spec;
    uint8_t *wav_buf;
    uint32_t wav_len;
    if (!SDL_LoadWAV_RW(rw, 1, &wav_spec, &wav_buf, &wav_len)) return;

    // Scale volume from 0-255 to SDL's 0-128
    uint8_t sdl_vol = (uint8_t)((volume * SDL_MIX_MAXVOLUME) / 255);
    uint8_t *mix_buf = new uint8_t[wav_len]();
    SDL_MixAudioFormat(mix_buf, wav_buf, wav_spec.format, wav_len, sdl_vol);
    SDL_FreeWAV(wav_buf);

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(nullptr, 0, &wav_spec, nullptr, 0);
    if (!dev) { delete[] mix_buf; return; }

    auto &s = kyoshin_port_wav_state();
    pthread_mutex_lock(&s.mtx);
    s.dev = dev;
    pthread_mutex_unlock(&s.mtx);

    int loops = (repeat <= 0) ? 1 : repeat;
    for (int i = 0; i < loops; i++)
        SDL_QueueAudio(dev, mix_buf, wav_len);
    delete[] mix_buf;

    SDL_PauseAudioDevice(dev, 0);

    // Close device after playback in a detached thread
    struct Args { SDL_AudioDeviceID dev; };
    pthread_t t;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&t, &attr, [](void *p) -> void* {
        auto *a = static_cast<Args*>(p);
        while (SDL_GetQueuedAudioSize(a->dev) > 0)
            usleep(50 * 1000);
        auto &s = kyoshin_port_wav_state();
        pthread_mutex_lock(&s.mtx);
        if (s.dev == a->dev) {
            s.dev = 0;
            pthread_mutex_unlock(&s.mtx);
            SDL_CloseAudioDevice(a->dev);
        } else {
            pthread_mutex_unlock(&s.mtx);
        }
        delete a;
        return nullptr;
    }, new Args{dev});
    pthread_attr_destroy(&attr);
}

// MARK: Power Mode
enum class PowerMode {
    Normal,
    Standby,
    Night,
    Interrupt,
};
inline PowerMode kyoshin_port_get_power_mode() { return PowerMode::Normal; }
inline void kyoshin_port_update_power_mode(time_t time, const KyoshinForecast &forecast) {}
inline void kyoshin_port_set_power_mode(PowerMode mode) {}
inline void kyoshin_port_feed_last_activity_tick() {}
