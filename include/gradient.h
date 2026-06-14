#ifndef GRADIENT_H
#define GRADIENT_H

#include <vector>
#include <cstdint>

void computeMagnitudeL1(
    const std::vector<int16_t>& gx,
    const std::vector<int16_t>& gy,
    std::vector<uint8_t>& magnitude,
    int width,
    int height
);

void computeMagnitudeL2(
    const std::vector<int16_t>& gx,
    const std::vector<int16_t>& gy,
    std::vector<uint8_t>& magnitude,
    int width,
    int height
);

void computeDirection(
    const std::vector<int16_t>& gx,
    const std::vector<int16_t>& gy,
    std::vector<uint8_t>& direction,
    int width,
    int height
);

#endif
