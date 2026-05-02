#include "flash_image.hpp"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cerrno>

static constexpr size_t IMAGE_WIDTH  = 352;
static constexpr size_t IMAGE_HEIGHT = 400;
static constexpr size_t IMAGE_SIZE   = IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(uint16_t);

static const char *image_path(int id) {
    static char buf[64];
    snprintf(buf, sizeof(buf), "flash_img_%d.bin", id);
    return buf;
}

FlashImage flash_image;

static void unmap(int &fd, void *&ptr) {
    if (ptr) { munmap(ptr, IMAGE_SIZE); ptr = nullptr; }
    if (fd >= 0) { close(fd); fd = -1; }
}

void FlashImage::selectImage(int id) {
    if (id_ == id) return;
    unmap(mmap_fd_, mmap_ptr_);
    data_ = nullptr;
    id_ = id;
}

bool FlashImage::exists() {
    if (id_ < 0 || id_ > 1) return false;
    struct stat st{};
    return stat(image_path(id_), &st) == 0 && (size_t)st.st_size == IMAGE_SIZE;
}

bool FlashImage::writeImage(uint16_t *image) {
    if (id_ < 0 || id_ > 1) return false;

    unmap(mmap_fd_, mmap_ptr_);
    data_ = nullptr;

    FILE *f = fopen(image_path(id_), "wb");
    if (!f) return false;
    bool ok = fwrite(image, 1, IMAGE_SIZE, f) == IMAGE_SIZE;
    fclose(f);
    return ok;
}

uint16_t *FlashImage::getData() {
    if (id_ < 0 || id_ > 1) return nullptr;
    if (data_) return data_;
    if (!exists()) return nullptr;

    int fd = open(image_path(id_), O_RDONLY);
    if (fd < 0) return nullptr;

    void *ptr = mmap(nullptr, IMAGE_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { close(fd); return nullptr; }

    mmap_fd_  = fd;
    mmap_ptr_ = ptr;
    data_     = static_cast<uint16_t *>(ptr);
    return data_;
}

bool FlashImage::deleteImage() {
    if (id_ < 0 || id_ > 1) return false;
    unmap(mmap_fd_, mmap_ptr_);
    data_ = nullptr;
    return remove(image_path(id_)) == 0 || errno == ENOENT;
}

bool FlashImage::deleteImageAll() {
    unmap(mmap_fd_, mmap_ptr_);
    data_ = nullptr;
    bool ok = true;
    for (int i = 0; i < 2; ++i)
        ok &= (remove(image_path(i)) == 0 || errno == ENOENT);
    return ok;
}
