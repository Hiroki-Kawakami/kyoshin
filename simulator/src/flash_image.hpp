#pragma once
#include <cstdint>

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
    int mmap_fd_{-1};
    void *mmap_ptr_{nullptr};
    uint16_t *data_{nullptr};
};

extern FlashImage flash_image;
