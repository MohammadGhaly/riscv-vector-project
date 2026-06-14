#ifndef GAUSSIAN_H
#define GAUSSIAN_H

#include <vector>
#include <cstdint>

std::vector<uint8_t> gaussian_blur(
    const std::vector<uint8_t>& input,
    int width,
    int height
);

#endif
