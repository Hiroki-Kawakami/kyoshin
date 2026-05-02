#pragma once
#include <cstdint>
#include <esp_partition.h>

class FlashImage {
public:
    void selectImage(int id);
    bool exists();
    bool deleteImage();
    bool deleteImageAll();
    bool writeImage(uint16_t *image);
    uint16_t *getData();

private:
    int id_{-1};
    esp_partition_mmap_handle_t mmap_handle_{0};
    uint16_t *data_{nullptr};
};

extern FlashImage flash_image;
