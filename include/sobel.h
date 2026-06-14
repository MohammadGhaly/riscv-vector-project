#ifndef SOBEL_H
#define SOBEL_H

#include <vector>
#include <cstdint>

void sobelFilter(
    const std::vector<uint8_t>& input,
    std::vector<int16_t>& gx,
    std::vector<int16_t>& gy,
    int width,
    int height
);

#endif
