#include <iostream>
#include <string>
#include <cstdio>
#include "../include/image.h"
#include "../include/gaussian_blur.h"
#include "../include/sobel.h"

// argc is an integer variable that holds the number of argument words passed in the terminal
// argv is a C-style array of text pointers storing the actual argument strings in order
int main(int argc, char** argv) {
    // Stage 1: checking the inputs are more than 3 words
    if (argc < 4) {
        std::cerr << "Error: Missing arguments!\n";
        std::cerr << "How to use: " << argv[0] << " <image_name.raw> <width> <height>\n";
        return 1;
    }
    if (argc > 4) {
        std::cerr << "Warning: Extra arguments ignored. Expected exactly 3 arguments.\n";
    }

    // Stage 2: checking that the input W and H are actual numbers and not text using (try catch) func
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

    // Stage 4: Check if the file actually exists on disk
    const char* input_file = argv[1];
    FILE* f = std::fopen(input_file, "rb");
    if (!f) {
        std::cerr << "Error: File not found or cannot be opened: " << input_file << "\n";
        return 1;
    }
    std::fclose(f); // Closing it again after we checked it exists

    // Processing Pipeline Begins here
    std::cout << "Loading verified image: " << input_file << " (" << width << "x" << height << ")...\n";
    Image img_in = image_load(input_file, width, height);

    // Create our blank output canvases
    Image img_blur   = image_create(width, height);
    Image img_mag_L1 = image_create(width, height); // Canvas for fast L1 math
    Image img_mag_L2 = image_create(width, height); // Canvas for accurate L2 math
    Image img_dir    = image_create(width, height);

    std::cout << "1. Applying Gaussian Blur...\n";
    gaussian_blur_5x5<uint8_t, int>(img_in.data, img_blur.data, width, height);

    std::cout << "2a. Computing Edge Strength via L1 Norm (Fast approximation)...\n";
    sobel_magnitude<uint8_t, int>(img_blur.data, img_mag_L1.data, width, height, false);

    std::cout << "2b. Computing Edge Strength via L2 Norm (Precise Pythagorean)...\n";
    sobel_magnitude<uint8_t, int>(img_blur.data, img_mag_L2.data, width, height, true);

    std::cout << "3. Finding Edge Angles (Sobel Direction)...\n";
    sobel_direction<uint8_t, int>(img_blur.data, img_dir.data, width, height);

    std::cout << "Saving all processed variants to disk...\n";
    image_save(img_blur,   "output_1_blur.raw");
    image_save(img_mag_L1, "output_2_magnitude_L1.raw");
    image_save(img_mag_L2, "output_2_magnitude_L2.raw");
    image_save(img_dir,    "output_3_direction.raw");

    // Clean up all memory blocks
    image_free(img_in);
    image_free(img_blur);
    image_free(img_mag_L1);
    image_free(img_mag_L2);
    image_free(img_dir);

    std::cout << "Pipeline completed successfully with full input protection!\n";
    return 0;
}
