#include <iostream>
#include <vector>
#include <chrono>

#include "image_io.h"
#include "gaussian.h"
#include "sobel.h"
#include "gradient.h"

int main()
{
    const int width = 256;
    const int height = 256;
    const int iterations = 100; // Mandatory project requirement for stability

    // Load input image
    std::vector<uint8_t> inputImage =
        load_image(
            "assets/input.raw",
            width,
            height
        );

    if (inputImage.empty()) {
        std::cerr << "Error: Input image is empty. Pipeline aborted.\n";
        return -1;
    }

    // =========================
    // Gaussian Blur Timing (100+ Iterations Loop)
    // =========================
    std::vector<uint8_t> blurredImage;
    
    auto gaussianStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        blurredImage = gaussian_blur(inputImage, width, height);
    }
    auto gaussianEnd = std::chrono::high_resolution_clock::now();

    auto gaussianTotalTime = std::chrono::duration_cast<
        std::chrono::microseconds
    >(gaussianEnd - gaussianStart);

    // Save blurred image (from the final iteration)
    save_image(
        "assets/blurred.raw",
        blurredImage
    );

    // =========================
    // Sobel Filter Timing (100+ Iterations Loop)
    // =========================
    std::vector<int16_t> gx;
    std::vector<int16_t> gy;

    auto sobelStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        sobelFilter(
            blurredImage,
            gx,
            gy,
            width,
            height
        );
    }
    auto sobelEnd = std::chrono::high_resolution_clock::now();

    auto sobelTotalTime = std::chrono::duration_cast<
        std::chrono::microseconds
    >(sobelEnd - sobelStart);

    // =========================
    // Magnitude Timing (100+ Iterations Loop)
    // =========================
    std::vector<uint8_t> magnitudeL1;
    std::vector<uint8_t> magnitudeL2;

    auto magnitudeStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        computeMagnitudeL1(
            gx,
            gy,
            magnitudeL1,
            width,
            height
        );

        computeMagnitudeL2(
            gx,
            gy,
            magnitudeL2,
            width,
            height
        );
    }
    auto magnitudeEnd = std::chrono::high_resolution_clock::now();

    auto magnitudeTotalTime = std::chrono::duration_cast<
        std::chrono::microseconds
    >(magnitudeEnd - magnitudeStart);

    // =========================
    // Direction Timing (100+ Iterations Loop)
    // =========================
    std::vector<uint8_t> direction;

    auto directionStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        computeDirection(
            gx,
            gy,
            direction,
            width,
            height
        );
    }
    auto directionEnd = std::chrono::high_resolution_clock::now();

    auto directionTotalTime = std::chrono::duration_cast<
        std::chrono::microseconds
    >(directionEnd - directionStart);

    // Save outputs
    save_image(
        "assets/magnitude_l1.raw",
        magnitudeL1
    );

    save_image(
        "assets/magnitude_l2.raw",
        magnitudeL2
    );

    save_image(
        "assets/direction.raw",
        direction
    );

    // =========================
    // Print Average Results per Run
    // =========================
    std::cout << "\nExecution Time (Average over " << iterations << " iterations):\n";

    std::cout
        << "Gaussian Blur: "
        << (static_cast<double>(gaussianTotalTime.count()) / iterations)
        << " us\n";

    std::cout
        << "Sobel Filter: "
        << (static_cast<double>(sobelTotalTime.count()) / iterations)
        << " us\n";

    std::cout
        << "Magnitude: "
        << (static_cast<double>(magnitudeTotalTime.count()) / iterations)
        << " us\n";

    std::cout
        << "Direction: "
        << (static_cast<double>(directionTotalTime.count()) / iterations)
        << " us\n";

    std::cout << "\nCanny stages complete\n";

    return 0;
}