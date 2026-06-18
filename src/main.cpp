#include <iostream>
#include <string>
#include <cstdio>
#include <cstdint>
#include "../include/image.h"
#include "../include/gaussian_blur.h"
#include "../include/sobel.h"

// RISC-V cycle counter — works on QEMU bare-metal
// QEMU simulates cycle counter at 1 GHz so: ms = cycles / 1,000,000
static inline uint64_t read_cycles() {
    uint64_t cycles;
    asm volatile ("rdcycle %0" : "=r"(cycles));
    return cycles;
}

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
    int width = 0, height = 0;
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
    std::cout << "Loading image: " << input_file
              << " (" << width << "x" << height << ")...\n";
    Image img_in = image_load(input_file, width, height);

    // Output canvases
    Image img_blur   = image_create(width, height);
    Image img_mag_L1 = image_create(width, height);
    Image img_mag_L2 = image_create(width, height);
    Image img_dir    = image_create(width, height);

    // Allocate separate Gx/Gy buffers (SoA layout for RVV readiness)
    int n = width * height;
    int* gx = new int[n];
    int* gy = new int[n];

    const int RUNS = 100;
    uint64_t c0, c1;

    // ── Stage 1: Gaussian Blur ─────────────────────────────────────────────
    c0 = read_cycles();
    for (int i = 0; i < RUNS; ++i)
        gaussian_blur_5x5<uint8_t, int>(img_in.data, img_blur.data, width, height);
    c1 = read_cycles();
    uint64_t cycles_gaussian = (c1 - c0) / RUNS;

    // ── Stage 2: Sobel Gradients ───────────────────────────────────────────
    c0 = read_cycles();
    for (int i = 0; i < RUNS; ++i)
        sobel_gradients<uint8_t, int>(img_blur.data, gx, gy, width, height);
    c1 = read_cycles();
    uint64_t cycles_sobel = (c1 - c0) / RUNS;

    // ── Stage 3a: Magnitude L1 ─────────────────────────────────────────────
    c0 = read_cycles();
    for (int i = 0; i < RUNS; ++i)
        sobel_magnitude<int, uint8_t>(gx, gy, img_mag_L1.data, width, height, false);
    c1 = read_cycles();
    uint64_t cycles_mag_l1 = (c1 - c0) / RUNS;

    // ── Stage 3b: Magnitude L2 ─────────────────────────────────────────────
    c0 = read_cycles();
    for (int i = 0; i < RUNS; ++i)
        sobel_magnitude<int, uint8_t>(gx, gy, img_mag_L2.data, width, height, true);
    c1 = read_cycles();
    uint64_t cycles_mag_l2 = (c1 - c0) / RUNS;

    // ── Stage 4: Direction ─────────────────────────────────────────────────
    c0 = read_cycles();
    for (int i = 0; i < RUNS; ++i)
        sobel_direction<int, uint8_t>(gx, gy, img_dir.data, width, height);
    c1 = read_cycles();
    uint64_t cycles_dir = (c1 - c0) / RUNS;

    // ── Profiling Summary ──────────────────────────────────────────────────
    // QEMU simulates cycle counter at 1 GHz so: ms = cycles / 1,000,000
    double ms_gaussian = cycles_gaussian / 1e6;
    double ms_sobel    = cycles_sobel    / 1e6;
    double ms_mag_l1   = cycles_mag_l1   / 1e6;
    double ms_mag_l2   = cycles_mag_l2   / 1e6;
    double ms_dir      = cycles_dir      / 1e6;
    double ms_total    = ms_gaussian + ms_sobel + ms_mag_l1 + ms_mag_l2 + ms_dir;

    std::cout << "\n=== Profiling Results (avg over " << RUNS << " runs) ===\n";
    std::cout << "Gaussian Blur   : " << ms_gaussian
              << " ms (" << 100.0 * ms_gaussian / ms_total << "%)\n";
    std::cout << "Sobel Gx/Gy     : " << ms_sobel
              << " ms (" << 100.0 * ms_sobel / ms_total << "%)\n";
    std::cout << "Magnitude L1    : " << ms_mag_l1
              << " ms (" << 100.0 * ms_mag_l1 / ms_total << "%)\n";
    std::cout << "Magnitude L2    : " << ms_mag_l2
              << " ms (" << 100.0 * ms_mag_l2 / ms_total << "%)\n";
    std::cout << "Direction       : " << ms_dir
              << " ms (" << 100.0 * ms_dir / ms_total << "%)\n";
    std::cout << "Total           : " << ms_total << " ms\n";

    delete[] gx;
    delete[] gy;

    // Save outputs
    std::cout << "\nSaving outputs to disk...\n";
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