#pragma once   // to avoid reading the file twice
#include <cstdint>
#include <cstdlib>   // gives us memory management tools

// Represents a grayscale image in memory.
// Each pixel is 1 byte: 0 = black, 255 = white.
struct Image {
    uint8_t* data;  // pointer to the pixel bytes in memory
    int width;      // number of columns
    int height;     // number of rows
};

// Creating a new empty black image of given size
Image image_create(int width, int height);

// Frees the memory used by an image when we are done with it
void image_free(Image& img);

// Loads a raw grayscale file from disk
Image image_load(const char* filename, int width, int height);

// Saves an image to disk as a raw grayscale file
void image_save(const Image& img, const char* filename);
