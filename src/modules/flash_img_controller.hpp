/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include "types.hpp"
#include "nvs.hpp"
#include "config/flash_img_config.hpp"

class FlashImagePartition : private NoMove {
public:
    const char *name;
    NVS &nvs;
    const esp_partition_t *espPartition;
    const uint8_t *mmapPtr;
    esp_partition_mmap_handle_t mmapHandle;
    FlashImagePartition(const char *name, NVS &nvs) : name(name), nvs(nvs) {
        espPartition = esp_partition_find_first(static_cast<esp_partition_type_t>(0x40), static_cast<esp_partition_subtype_t>(0), name);
        ESP_ERROR_CHECK(esp_partition_mmap(espPartition, 0, espPartition->size, ESP_PARTITION_MMAP_DATA, (const void**)&mmapPtr, &mmapHandle));
    }
    ~FlashImagePartition() {
        esp_partition_munmap(mmapHandle);
    }

    void erase(FlashImg img) const {
        int offset = FLASH_IMG_LOCATION[img].offset, size = FLASH_IMG_LOCATION[img].size;
        ESP_ERROR_CHECK(esp_partition_erase_range(espPartition, offset, size));
    }
    void write(FlashImg img, int offset, void *data, int size) const {
        offset += FLASH_IMG_LOCATION[img].offset;
        ESP_ERROR_CHECK(esp_partition_write(espPartition, offset, data, size));
    }
    bool isInitialized() const {
        uint8_t initialized;
        esp_err_t err = nvs.get(name, &initialized);
        return err == ESP_OK && initialized;
    }
    void setInitialized(bool initialized) const {
        nvs.set(name, (uint8_t)initialized);
    }
    const void *ptr(FlashImg img) const {
        return &mmapPtr[FLASH_IMG_LOCATION[img].offset];
    }
};

class FlashImageController : private NoMove {
public:
    NVS nvs = NVS("img");
    void init() {
        uint8_t version = 0;
        if (nvs.get("version", &version) != ESP_OK || version != FLASH_IMG_VERSION) {
            clear();
        }

        printf("FlashImage Locations\n");
        for (int i = 0; i < FlashImg::count; i++) {
            printf("%d: offset: %8d, size: %8d\n", i, FLASH_IMG_LOCATION[i].offset, FLASH_IMG_LOCATION[i].size);
        }
    }
    void clear() {
        nvs.eraseAll();
        nvs.set("version", (uint8_t)FLASH_IMG_VERSION);
    }
    std::shared_ptr<FlashImagePartition> partition(const char *name) {
        static std::map<std::string, std::weak_ptr<FlashImagePartition>> cache;
        if (cache.contains(name) && !cache[name].expired()) {
            return cache[name].lock();
        }
        auto partition = std::make_shared<FlashImagePartition>(name, nvs);
        cache.insert_or_assign(name, partition);
        return partition;
    }
};
