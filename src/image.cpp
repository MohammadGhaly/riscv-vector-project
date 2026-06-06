#include "../include/image.h"
#include <cstdio>   // gives us fast C-style file functions (fopen, fread, fwrite)
#include <iostream>

Image image_create(int width, int height) {
    Image img;
    img.width = width;
    img.height = height;

    // Memory Flattening: To optimize data locality, a 2D pixel array is stored as a 1D linear buffer

    img.data = (uint8_t*)std::calloc(width * height, sizeof(uint8_t));

    // Fault Tolerance: Always validate heap allocations.
    // If the virtual address space is exhausted, calloc returns a null pointer (nullptr).
    if (img.data == nullptr) {
        std::cerr << "Fatal Error: Heap allocation failed for image buffer.\n";
        std::exit(1);
    }
    return img;
}

void image_free(Image& img) {
    if (img.data != nullptr) {
        std::free(img.data); // Deallocate the heap block returning ownership to the system allocator.
        img.data = nullptr;
    }
    img.width = 0;
    img.height = 0;
}

Image image_load(const char* filename, int width, int height) {
    // Instantiate our structural container and allocate the necessary heap footprint
    Image img = image_create(width, height);

    std::FILE* file = std::fopen(filename, "rb");
    if (!file) {
        std::cerr << "IO Error: Failed to open file path '" << filename << "' for reading.\n";
        image_free(img); // this is to clean up heap allocations to avoid memory leaks before termination.
        std::exit(1);
    }

    // Direct Block Transfer: fread fetches (width * height) elements of 1-byte size and copies
    //them straight from the kernel cache into our userspace heap buffer pointer (img.data)
    std::fread(img.data, sizeof(uint8_t), width * height, file);

    std::fclose(file); // Resource cleanup: release the file descriptor back to the operating system kernel.
    return img;
}

void image_save(const Image& img, const char* filename) {
    std::FILE* file = std::fopen(filename, "wb");
    if (!file) {
        std::cerr << "IO Error: Failed to open file path '" << filename << "' for writing.\n";
        std::exit(1);
    }

    // stream the raw sequence from RAM directly to the underlying filesystem storage layout.
    std::fwrite(img.data, sizeof(uint8_t), img.width * img.height, file);
    std::fclose(file);
}
