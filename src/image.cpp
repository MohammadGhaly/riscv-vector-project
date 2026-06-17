#include "../include/image.h"
#include <cstdio>
#include <cstring>  // for memset
#include <iostream>

Image image_create(int width, int height) {
    Image img;
    img.width = width;
    img.height = height;

    // aligned_alloc(64, ...) ensures 64-byte alignment required for RVV vector loads later.
    // Unlike calloc, aligned_alloc does NOT zero memory, so we memset manually.
    img.data = static_cast<uint8_t*>(aligned_alloc(64, width * height * sizeof(uint8_t)));

    if (img.data == nullptr) {
        std::cerr << "Fatal Error: Heap allocation failed for image buffer.\n";
        std::exit(1);
    }

    memset(img.data, 0, width * height * sizeof(uint8_t));
    return img;
}

void image_free(Image& img) {
    if (img.data != nullptr) {
        std::free(img.data);
        img.data = nullptr;
    }
    img.width = 0;
    img.height = 0;
}

Image image_load(const char* filename, int width, int height) {
    Image img = image_create(width, height);

    std::FILE* file = std::fopen(filename, "rb");
    if (!file) {
        std::cerr << "IO Error: Failed to open file path '" << filename << "' for reading.\n";
        image_free(img);
        std::exit(1);
    }

    (void)std::fread(img.data, sizeof(uint8_t), width * height, file);
    std::fclose(file);
    return img;
}

void image_save(const Image& img, const char* filename) {
    std::FILE* file = std::fopen(filename, "wb");
    if (!file) {
        std::cerr << "IO Error: Failed to open file path '" << filename << "' for writing.\n";
        std::exit(1);
    }

    std::fwrite(img.data, sizeof(uint8_t), img.width * img.height, file);
    std::fclose(file);
}
