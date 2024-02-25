/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include <string>
#include "esp_err.h"
#include "nvs_flash.h"

class NVS {
public:
    static esp_err_t init() {
        esp_err_t ret;
        ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        return ret;
    }
    static esp_err_t flashErase() {
        return nvs_flash_erase();
    }

    nvs_handle_t handle = 0;
    const char *name;
    nvs_open_mode_t open_mode;
    NVS(const char *name, nvs_open_mode_t open_mode = NVS_READWRITE) : name(name), open_mode(open_mode) {}
    ~NVS() {
        nvs_close(handle);
    }
    nvs_handle_t open() {
        if (!handle) ESP_ERROR_CHECK(nvs_open(name, open_mode, &handle));
        return handle;
    }

    esp_err_t erase(std::string key) { return nvs_erase_key(open(), key.c_str()); }
    esp_err_t eraseAll() { return nvs_erase_all(open()); }
    esp_err_t commit() { return nvs_commit(open()); }
    esp_err_t getUsedEntryCount(size_t *used_entries) { return nvs_get_used_entry_count(open(), used_entries); }

    // set functions
    esp_err_t set(std::string key, int8_t value) { return nvs_set_i8(open(), key.c_str(), value); }
    esp_err_t set(std::string key, uint8_t value) { return nvs_set_u8(open(), key.c_str(), value); }
    esp_err_t set(std::string key, int16_t value) { return nvs_set_i16(open(), key.c_str(), value); }
    esp_err_t set(std::string key, uint16_t value) { return nvs_set_u16(open(), key.c_str(), value); }
    esp_err_t set(std::string key, int32_t value) { return nvs_set_i32(open(), key.c_str(), value); }
    esp_err_t set(std::string key, uint32_t value) { return nvs_set_u32(open(), key.c_str(), value); }
    esp_err_t set(std::string key, int64_t value) { return nvs_set_i64(open(), key.c_str(), value); }
    esp_err_t set(std::string key, uint64_t value) { return nvs_set_u64(open(), key.c_str(), value); }
    esp_err_t set(std::string key, const char *value) { return nvs_set_str(open(), key.c_str(), value); }
    esp_err_t set(std::string key, const void *value, size_t length) { return nvs_set_blob(open(), key.c_str(), value, length); }

    // get functions
    esp_err_t get(std::string key, int8_t *out_value) { return nvs_get_i8(open(), key.c_str(), out_value); }
    esp_err_t get(std::string key, uint8_t *out_value) { return nvs_get_u8(open(), key.c_str(), out_value); }
    esp_err_t get(std::string key, int16_t *out_value) { return nvs_get_i16(open(), key.c_str(), out_value); }
    esp_err_t get(std::string key, uint16_t *out_value) { return nvs_get_u16(open(), key.c_str(), out_value); }
    esp_err_t get(std::string key, int32_t *out_value) { return nvs_get_i32(open(), key.c_str(), out_value); }
    esp_err_t get(std::string key, uint32_t *out_value) { return nvs_get_u32(open(), key.c_str(), out_value); }
    esp_err_t get(std::string key, int64_t *out_value) { return nvs_get_i64(open(), key.c_str(), out_value); }
    esp_err_t get(std::string key, uint64_t *out_value) { return nvs_get_u64(open(), key.c_str(), out_value); }
    esp_err_t get(std::string key, char *out_value, size_t *length) { return nvs_get_str(open(), key.c_str(), out_value, length); }
    esp_err_t get(std::string key, void *out_value, size_t *length) { return nvs_get_blob(open(), key.c_str(), out_value, length); }
};
