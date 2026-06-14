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

    // Load input image
    std::vector<uint8_t> inputImage =
        load_image(
            "assets/input.raw",
            width,
            height
        );

    // =========================
    // Gaussian Blur Timing
    // =========================

    auto gaussianStart =
        std::chrono::high_resolution_clock::now();

    std::vector<uint8_t> blurredImage =
        gaussian_blur(
            inputImage,
            width,
            height
        );

    auto gaussianEnd =
        std::chrono::high_resolution_clock::now();

    auto gaussianTime =
        std::chrono::duration_cast<
            std::chrono::microseconds
        >(gaussianEnd - gaussianStart);

    // Save blurred image
    save_image(
        "assets/blurred.raw",
        blurredImage
    );

    // =========================
    // Sobel Filter Timing
    // =========================

    std::vector<int16_t> gx;
    std::vector<int16_t> gy;

    auto sobelStart =
        std::chrono::high_resolution_clock::now();

    sobelFilter(
        blurredImage,
        gx,
        gy,
        width,
        height
    );

    auto sobelEnd =
        std::chrono::high_resolution_clock::now();

    auto sobelTime =
        std::chrono::duration_cast<
            std::chrono::microseconds
        >(sobelEnd - sobelStart);

    // =========================
    // Magnitude Timing
    // =========================

    std::vector<uint8_t> magnitudeL1;
    std::vector<uint8_t> magnitudeL2;

    auto magnitudeStart =
        std::chrono::high_resolution_clock::now();

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

    auto magnitudeEnd =
        std::chrono::high_resolution_clock::now();

    auto magnitudeTime =
        std::chrono::duration_cast<
            std::chrono::microseconds
        >(magnitudeEnd - magnitudeStart);

    // =========================
    // Direction Timing
    // =========================

    std::vector<uint8_t> direction;

    auto directionStart =
        std::chrono::high_resolution_clock::now();

    computeDirection(
        gx,
        gy,
        direction,
        width,
        height
    );

    auto directionEnd =
        std::chrono::high_resolution_clock::now();

    auto directionTime =
        std::chrono::duration_cast<
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
    // Print Results
    // =========================

    std::cout << "\nExecution Time:\n";

    std::cout
        << "Gaussian Blur: "
        << gaussianTime.count()
        << " us\n";

    std::cout
        << "Sobel Filter: "
        << sobelTime.count()
        << " us\n";

    std::cout
        << "Magnitude: "
        << magnitudeTime.count()
        << " us\n";

    std::cout
        << "Direction: "
        << directionTime.count()
        << " us\n";

    std::cout << "\nCanny stages complete\n";

    return 0;
}
