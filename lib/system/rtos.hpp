/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include <functional>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "types.hpp"

namespace RTOS {
    using std::function;

    enum class TaskPriority : UBaseType_t {
        Idle     = tskIDLE_PRIORITY,
        Low      = 1,
        Normal   = 2,
        High     = 3,
        Realtime = configMAX_PRIORITIES - 1,
    };

    template <TickType_t Cycle>
    class Task : private NoMove {
    private:
        const char *name;
        bool needTaskDelete = false;
        TaskHandle_t handle = nullptr;
        QueueHandle_t queue = nullptr;
    public:
        Task() {
            handle = xTaskGetCurrentTaskHandle();
            name = pcTaskGetName(handle);
        }
        Task(const char *name) : name(name) {}
        ~Task() {
            if (needTaskDelete) vTaskDelete(handle);
            if (queue) vQueueDelete(queue);
        }

        void createQueue(UBaseType_t queueLength = 32) {
            if (queue) return;
            queue = xQueueCreate(queueLength, sizeof(void *));
        }
        void start(TaskPriority priority = TaskPriority::Normal, UBaseType_t stackDepth = 2048, BaseType_t core = 0) {
            needTaskDelete = true;
            xTaskCreatePinnedToCore([](void *arg) {
                auto task = static_cast<Task *>(arg);
                task->run();
            }, name, stackDepth, this, static_cast<UBaseType_t>(priority), &handle, core);
        }
        void run() {
            init();
            while (true) {
                void *msg;
                if (queue) {
                    if (xQueueReceive(queue, &msg, Cycle)) {
                        auto func = static_cast<function<void()>*>(msg);
                        (*func)();
                        delete func;
                    }
                } else {
                    vTaskDelay(Cycle);
                }
                cycle();
            }
        }

        virtual void init() {}
        virtual void cycle() {}
        void send(function<void()> func, TickType_t wait = portMAX_DELAY) {
            if (!queue) {
                ESP_LOGE("RTOS::Task", "Task queue is not created.");
                assert(0);
            }
            void *msg = new function<void()>(func);
            xQueueSend(queue, &msg, wait);
        }
        bool isBlocked() {
            return handle && eTaskGetState(handle) == eBlocked;
        }
        bool isCurrentTask() {
            return handle && xTaskGetCurrentTaskHandle() == handle;
        }
    };

    inline void timeout(int ms, function<void()> func) {
        TimerHandle_t timer = xTimerCreate("timeout", pdMS_TO_TICKS(ms), pdFALSE, new function<void()>(func), [](TimerHandle_t timer) {
            auto func = static_cast<function<void()>*>(pvTimerGetTimerID(timer));
            (*func)();
            delete func;
            xTimerDelete(timer, 0);
        });
        xTimerStart(timer, 0);
    }

    class Semaphore {
    private:
        SemaphoreHandle_t handle;
        Semaphore(SemaphoreHandle_t handle) : handle(handle) {}
    public:
        inline static Semaphore binary() {
            return Semaphore(xSemaphoreCreateBinary());
        }
        inline static Semaphore counting(UBaseType_t max, UBaseType_t initial) {
            return Semaphore(xSemaphoreCreateCounting(max, initial));
        }
        inline static Semaphore mutex() {
            return Semaphore(xSemaphoreCreateMutex());
        }

        Semaphore(const Semaphore &) = delete;
        Semaphore& operator=(const Semaphore &) = delete;
        inline Semaphore(Semaphore &&rval) {
            this->handle = rval.handle;
            rval.handle = nullptr;
        }
        inline Semaphore& operator=(Semaphore &&rval) {
            this->handle = rval.handle;
            rval.handle = nullptr;
            return *this;
        }
        inline ~Semaphore() {
            if (this->handle) vSemaphoreDelete(this->handle);
        }
        inline void give() {
            xSemaphoreGive(this->handle);
        }
        inline void take(TickType_t ticks = portMAX_DELAY) {
            xSemaphoreTake(this->handle, ticks);
        }
    };

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
}
