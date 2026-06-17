#include <iostream>
#include <string>
#include <cstdio>
#include "../include/image.h"
#include "../include/gaussian_blur.h"
#include "../include/sobel.h"

int main(int argc, char** argv) {
    // Stage 1: Check argument count
    if (argc < 4) {
        std::cerr << "Error: Missing arguments!\n";
        std::cerr << "How to use: " << argv[0] << " <image_name.raw> <width> <height>\n";
        return 1;
    }
    if (argc > 4) {
        std::cerr << "Warning: Extra arguments ignored. Expected exactly 3 arguments.\n";
    }

    // Stage 2: Parse width and height
    int width = 0;
    int height = 0;
    try {
        width  = std::stoi(argv[2]);
        height = std::stoi(argv[3]);
    } catch (...) {
        std::cerr << "Error: Width and height must be valid numbers!\n";
        return 1;
    }

    // Stage 3: Check for negative or zero dimensions
    if (width <= 0 || height <= 0) {
        std::cerr << "Error: Width and height must be positive, non-zero numbers!\n";
        return 1;
    }

    // Stage 4: Check if the file exists on disk
    const char* input_file = argv[1];
    FILE* f = std::fopen(input_file, "rb");
    if (!f) {
        std::cerr << "Error: File not found or cannot be opened: " << input_file << "\n";
        return 1;
    }
    std::fclose(f);

    // Pipeline begins
    std::cout << "Loading image: " << input_file << " (" << width << "x" << height << ")...\n";
    Image img_in = image_load(input_file, width, height);

    // Output canvases
    Image img_blur   = image_create(width, height);
    Image img_mag_L1 = image_create(width, height);
    Image img_mag_L2 = image_create(width, height);
    Image img_dir    = image_create(width, height);

    // Allocate separate Gx and Gy gradient buffers (SoA layout for RVV readiness)
    int n = width * height;
    int* gx = new int[n];
    int* gy = new int[n];

    std::cout << "1. Applying Gaussian Blur...\n";
    gaussian_blur_5x5<uint8_t, int>(img_in.data, img_blur.data, width, height);

    std::cout << "2. Computing Sobel Gradients (Gx, Gy)...\n";
    sobel_gradients<uint8_t, int>(img_blur.data, gx, gy, width, height);

    std::cout << "3a. Computing Edge Magnitude via L1 Norm...\n";
    sobel_magnitude<int, uint8_t>(gx, gy, img_mag_L1.data, width, height, false);

    std::cout << "3b. Computing Edge Magnitude via L2 Norm...\n";
    sobel_magnitude<int, uint8_t>(gx, gy, img_mag_L2.data, width, height, true);

    std::cout << "4. Computing Edge Direction...\n";
    sobel_direction<int, uint8_t>(gx, gy, img_dir.data, width, height);

    delete[] gx;
    delete[] gy;

    // Save outputs
    std::cout << "Saving outputs to disk...\n";
    image_save(img_blur,   "output_1_blur.raw");
    image_save(img_mag_L1, "output_2_magnitude_L1.raw");
    image_save(img_mag_L2, "output_2_magnitude_L2.raw");
    image_save(img_dir,    "output_3_direction.raw");

    // Cleanup
    image_free(img_in);
    image_free(img_blur);
    image_free(img_mag_L1);
    image_free(img_mag_L2);
    image_free(img_dir);

    std::cout << "Pipeline completed successfully!\n";
    return 0;
}
