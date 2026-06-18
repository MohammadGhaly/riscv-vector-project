#pragma once
#include <algorithm>
#include "image.h"

// Generates a white filled rectangle on a black background.
// Used to test Sobel: edges should appear only on rectangle borders.
inline Image gen_rectangle(int width, int height,
                            int rx, int ry, int rw, int rh) {
    Image img = image_create(width, height);
    for (int y = ry; y < ry + rh && y < height; ++y)
        for (int x = rx; x < rx + rw && x < width; ++x)
            img.data[y * width + x] = 255;
    return img;
}

// Generates a diagonal white line from top-left to bottom-right on black.
// Used to test direction quantization: should produce 45° output.
inline Image gen_diagonal(int width, int height) {
    Image img = image_create(width, height);
    int len = std::min(width, height);
    for (int i = 0; i < len; ++i)
        img.data[i * width + i] = 255;
    return img;
}

// Generates a vertical sharp edge: left half black, right half white.
// Used to test Sobel: large Gx, near-zero Gy, direction = 0 (horizontal gradient).
inline Image gen_vertical_edge(int width, int height) {
    Image img = image_create(width, height);
    for (int y = 0; y < height; ++y)
        for (int x = width / 2; x < width; ++x)
            img.data[y * width + x] = 255;
    return img;
}

// Generates a horizontal sharp edge: top half black, bottom half white.
// Used to test Sobel: large Gy, near-zero Gx, direction = 2 (vertical gradient).
inline Image gen_horizontal_edge(int width, int height) {
    Image img = image_create(width, height);
    for (int y = height / 2; y < height; ++y)
        for (int x = 0; x < width; ++x)
            img.data[y * width + x] = 255;
    return img;
}

// Generates a uniform image filled with a constant pixel value.
// Used to test Gaussian: blurring uniform input should return same uniform output.
inline Image gen_uniform(int width, int height, uint8_t value) {
    Image img = image_create(width, height);
    for (int i = 0; i < width * height; ++i)
        img.data[i] = value;
    return img;
}

// Generates a single bright pixel (impulse) at the center on black background.
// Used to test Gaussian: the impulse should spread symmetrically to neighbors.
inline Image gen_impulse(int width, int height) {
    Image img = image_create(width, height);
    img.data[(height / 2) * width + (width / 2)] = 255;
    return img;
}
