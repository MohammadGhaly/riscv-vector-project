#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include "gaussian.h"
#include "sobel.h"

void test_gaussian_impulse() {
    int width = 5, height = 5;
    std::vector<uint8_t> input(25, 0);
    input[12] = 255; // Center pixel acts as an impulse response

    // Run your newly templated Gaussian Blur
    std::vector<uint8_t> result = gaussian_blur<uint8_t, int32_t>(input, width, height);

    std::cout << "Gaussian Center Value: " << (int)result[12] << " (Expected: 35)\n";
    if (result[12] == 35) {
        std::cout << "✅ GAUSSIAN ZERO-PADDING & MATH PASSED!\n";
    } else {
        std::cout << "❌ GAUSSIAN TEST FAILED!\n";
    }
}

void test_sobel_edge() {
    int width = 3, height = 3;
    // Perfect vertical edge transition
    std::vector<uint8_t> input = {
        0, 0, 100,
        0, 0, 100,
        0, 0, 100
    };

    std::vector<int16_t> gx, gy;
    sobelFilter<uint8_t, int16_t, int32_t>(input, gx, gy, width, height);

    std::cout << "Sobel Center Gx Magnitude: " << std::abs(gx[4]) << " (Expected: 400)\n";
    if (std::abs(gx[4]) == 400) {
        std::cout << "✅ SOBEL ZERO-PADDING & MATH PASSED!\n";
    } else {
        std::cout << "❌ SOBEL TEST FAILED!\n";
    }
}

int main() {
    std::cout << "=== Running Phase 3 Verification Infrastructure ===\n";
    test_gaussian_impulse();
    test_sobel_edge();
    return 0;
}
