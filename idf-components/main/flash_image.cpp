#include "flash_image.hpp"
#include <esp_log.h>
#include <string.h>

static const char *TAG = "FlashImage";

// Partition layout (1MB total):
//   0x00000 - 0x00FFF : metadata sector (4KB, ImageMeta per slot)
//   0x10000 - 0x54800 : image 0 data  (64KB-aligned for MMU page mapping)
//   0x60000 - 0xA4800 : image 1 data  (384KB-aligned for MMU page mapping)
//
// 64KB alignment satisfies DMA burst requirements on ESP32-S3.

static constexpr uint32_t MAGIC         = 0x494D4748; // "IMGH"
static constexpr size_t   IMAGE_WIDTH   = 352;
static constexpr size_t   IMAGE_HEIGHT  = 400;
static constexpr size_t   IMAGE_SIZE    = IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(uint16_t);
static constexpr size_t   ERASE_BLOCK   = 4096;
static constexpr size_t   IMAGE_ERASE_SIZE = (IMAGE_SIZE + ERASE_BLOCK - 1) & ~(ERASE_BLOCK - 1);
static constexpr size_t   META_OFFSET   = 0x00000;
static constexpr size_t   IMAGE_OFFSETS[2] = {0x10000, 0x60000};

struct ImageMeta {
    uint32_t magic;
};

FlashImage flash_image;

static const esp_partition_t *get_partition() {
    return esp_partition_find_first(
        static_cast<esp_partition_type_t>(0x40), ESP_PARTITION_SUBTYPE_ANY, "img_data");
}

void FlashImage::selectImage(int id) {
    if (id_ == id) return;
    if (mmap_handle_ != 0) {
        esp_partition_munmap(mmap_handle_);
        mmap_handle_ = 0;
        data_ = nullptr;
    }
    id_ = id;
}

bool FlashImage::exists() {
    if (id_ < 0 || id_ > 1) return false;
    const auto *part = get_partition();
    if (!part) return false;

    ImageMeta meta{};
    esp_err_t err = esp_partition_read(part,
        META_OFFSET + id_ * sizeof(ImageMeta), &meta, sizeof(ImageMeta));
    return err == ESP_OK && meta.magic == MAGIC;
}

bool FlashImage::writeImage(uint16_t *image) {
    if (id_ < 0 || id_ > 1) return false;
    const auto *part = get_partition();
    if (!part) return false;

    if (mmap_handle_ != 0) {
        esp_partition_munmap(mmap_handle_);
        mmap_handle_ = 0;
        data_ = nullptr;
    }

    if (esp_partition_erase_range(part, IMAGE_OFFSETS[id_], IMAGE_ERASE_SIZE) != ESP_OK) return false;
    if (esp_partition_write(part, IMAGE_OFFSETS[id_], image, IMAGE_SIZE) != ESP_OK) return false;

    uint8_t meta_buf[ERASE_BLOCK];
    if (esp_partition_read(part, META_OFFSET, meta_buf, ERASE_BLOCK) != ESP_OK) return false;

    reinterpret_cast<ImageMeta *>(meta_buf + id_ * sizeof(ImageMeta))->magic = MAGIC;

    if (esp_partition_erase_range(part, META_OFFSET, ERASE_BLOCK) != ESP_OK) return false;
    if (esp_partition_write(part, META_OFFSET, meta_buf, ERASE_BLOCK) != ESP_OK) return false;

    return true;
}

uint16_t *FlashImage::getData() {
    if (id_ < 0 || id_ > 1) return nullptr;
    if (!exists()) return nullptr;
    if (data_) return data_;

    const auto *part = get_partition();
    if (!part) return nullptr;

    const void *ptr = nullptr;
    esp_err_t err = esp_partition_mmap(part, IMAGE_OFFSETS[id_], IMAGE_SIZE,
                                       ESP_PARTITION_MMAP_DATA, &ptr, &mmap_handle_);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mmap failed: %s", esp_err_to_name(err));
        return nullptr;
    }

    data_ = static_cast<uint16_t *>(const_cast<void *>(ptr));
    return data_;
}

bool FlashImage::deleteImage() {
    if (id_ < 0 || id_ > 1) return false;
    const auto *part = get_partition();
    if (!part) return false;

    if (mmap_handle_ != 0) {
        esp_partition_munmap(mmap_handle_);
        mmap_handle_ = 0;
        data_ = nullptr;
    }

    uint8_t meta_buf[ERASE_BLOCK];
    if (esp_partition_read(part, META_OFFSET, meta_buf, ERASE_BLOCK) != ESP_OK) return false;

    reinterpret_cast<ImageMeta *>(meta_buf + id_ * sizeof(ImageMeta))->magic = 0;

    if (esp_partition_erase_range(part, META_OFFSET, ERASE_BLOCK) != ESP_OK) return false;
    if (esp_partition_write(part, META_OFFSET, meta_buf, ERASE_BLOCK) != ESP_OK) return false;

    return true;
}

bool FlashImage::deleteImageAll() {
    if (mmap_handle_ != 0) {
        esp_partition_munmap(mmap_handle_);
        mmap_handle_ = 0;
        data_ = nullptr;
    }

    const auto *part = get_partition();
    if (!part) return false;

    return esp_partition_erase_range(part, 0, part->size) == ESP_OK;
}
