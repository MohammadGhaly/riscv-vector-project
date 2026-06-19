#include <iostream>
#include <string>
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <cstdlib>

#include "../include/image.h"
#include "../include/gaussian_blur.h"
#include "../include/sobel.h"
#include "canny_rvv.h"

// RISC-V cycle counter
static inline uint64_t read_cycles() {
    uint64_t cycles;
    asm volatile ("rdcycle %0" : "=r"(cycles));
    return cycles;
}

// Verification helper
void verify_buffers(const uint8_t* scalar,
                    const uint8_t* rvv,
                    int total_pixels,
                    const char* stage_name)
{
    for (int i = 0; i < total_pixels; i++) {
        if (std::abs((int)scalar[i] - (int)rvv[i]) > 1) {
            std::cerr << "\n[CRITICAL ERROR] Mismatch in " << stage_name
                      << " at index " << i
                      << " Scalar=" << (int)scalar[i]
                      << " RVV=" << (int)rvv[i] << "\n";
            std::exit(1);
        }
    }
    std::cout << "[SUCCESS] " << stage_name << " matches.\n";
}

int main(int argc, char** argv)
{
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <image.raw> <width> <height>\n";
        return 1;
    }

    int width = std::stoi(argv[2]);
    int height = std::stoi(argv[3]);

    const char* input_file = argv[1];

    FILE* f = std::fopen(input_file, "rb");
    if (!f) {
        std::cerr << "Cannot open file\n";
        return 1;
    }
    std::fclose(f);

    Image img_in = image_load(input_file, width, height);

    Image img_blur       = image_create(width, height);
    Image img_blur_rvv   = image_create(width, height);

    Image img_mag_L1     = image_create(width, height);
    Image img_mag_L1_rvv = image_create(width, height);

    Image img_mag_L2     = image_create(width, height);
    Image img_dir        = image_create(width, height);

    int n = width * height;

    int16_t* gx      = new int16_t[n];
    int16_t* gy      = new int16_t[n];

    int16_t* gx_rvv  = new int16_t[n];
    int16_t* gy_rvv  = new int16_t[n];

    const int RUNS = 100;
    uint64_t c0, c1;

    std::cout << "\n--- SCALAR PIPELINE ---\n";

    c0 = read_cycles();
    for (int i = 0; i < RUNS; i++)
        gaussian_blur_5x5<uint8_t,int>(img_in.data, img_blur.data, width, height);
    c1 = read_cycles();
    uint64_t cycles_gaussian = (c1 - c0) / RUNS;

    c0 = read_cycles();
    for (int i = 0; i < RUNS; i++)
        sobel_gradients<uint8_t,int16_t>(img_blur.data, gx, gy, width, height);
    c1 = read_cycles();
    uint64_t cycles_sobel = (c1 - c0) / RUNS;

    c0 = read_cycles();
    for (int i = 0; i < RUNS; i++)
        sobel_magnitude<int16_t,uint8_t>(gx, gy, img_mag_L1.data, width, height, false);
    c1 = read_cycles();
    uint64_t cycles_mag = (c1 - c0) / RUNS;

    std::cout << "\n--- RVV PIPELINE ---\n";

    c0 = read_cycles();
    for (int i = 0; i < RUNS; i++)
        gaussian_blur_5x5_rvv(img_in.data, img_blur_rvv.data, width, height);
    c1 = read_cycles();
    uint64_t cycles_gaussian_rvv = (c1 - c0) / RUNS;

    c0 = read_cycles();
    for (int i = 0; i < RUNS; i++)
        sobel_gradients_rvv(img_blur.data, gx_rvv, gy_rvv, width, height);
    c1 = read_cycles();
    uint64_t cycles_sobel_rvv = (c1 - c0) / RUNS;

    c0 = read_cycles();
    for (int i = 0; i < RUNS; i++)
        magnitude_l1_rvv(gx_rvv, gy_rvv, img_mag_L1_rvv.data, n);
    c1 = read_cycles();
    uint64_t cycles_mag_rvv = (c1 - c0) / RUNS;

    // ---------------- VALIDATION ----------------

    std::cout << "\n--- VALIDATION ---\n";

   // Gaussian RVV only processes interior pixels (border left as 0)
// so we only compare interior pixels
bool gaussian_ok = true;
for (int y = 2; y < height - 2; y++) {
    for (int x = 2; x < width - 2; x++) {
        int i = y * width + x;
        if (std::abs((int)img_blur.data[i] - (int)img_blur_rvv.data[i]) > 1) {
            std::cerr << "[ERROR] Gaussian mismatch at (" << x << "," << y << ")"
                      << " Scalar=" << (int)img_blur.data[i]
                      << " RVV=" << (int)img_blur_rvv.data[i] << "\n";
            gaussian_ok = false;
            break;
        }
    }
    if (!gaussian_ok) break;
}
if (gaussian_ok)
    std::cout << "[SUCCESS] Gaussian Blur interior pixels match.\n";

   bool ok = true;
for (int y = 1; y < height - 1; y++) {
    for (int x = 1; x < width - 1; x++) {
        int i = y * width + x;
        if (gx[i] != gx_rvv[i] || gy[i] != gy_rvv[i]) {
            ok = false;
            std::cerr << "[ERROR] Sobel mismatch at (" << x << "," << y << ")"
                      << " Gx_scalar=" << gx[i] << " Gx_rvv=" << gx_rvv[i]
                      << " Gy_scalar=" << gy[i] << " Gy_rvv=" << gy_rvv[i] << "\n";
            break;
        }
    }
    if (!ok) break;
}

    if (ok)
        std::cout << "[SUCCESS] Sobel Gx/Gy RVV matches scalar baseline.\n";
    else {
        std::cerr << "[ERROR] Sobel mismatch.\n";
        return 1;
    }

    
// Magnitude depends on Sobel which skips the 1-pixel border
bool mag_ok = true;
for (int y = 1; y < height - 1; y++) {
    for (int x = 1; x < width - 1; x++) {
        int i = y * width + x;
        if (std::abs((int)img_mag_L1.data[i] - (int)img_mag_L1_rvv.data[i]) > 1) {
            std::cerr << "[ERROR] Magnitude mismatch at (" << x << "," << y << ")"
                      << " Scalar=" << (int)img_mag_L1.data[i]
                      << " RVV=" << (int)img_mag_L1_rvv.data[i] << "\n";
            mag_ok = false;
            break;
        }
    }
    if (!mag_ok) break;
}
if (mag_ok)
    std::cout << "[SUCCESS] Magnitude L1 interior pixels match.\n";



    // ---------------- PERFORMANCE ----------------

    std::cout << "\n--- PERFORMANCE ---\n";

    std::printf("Gaussian Blur | %lu | %lu | %.2fx\n",
        cycles_gaussian,
        cycles_gaussian_rvv,
        (double)cycles_gaussian / cycles_gaussian_rvv);

    std::printf("Sobel         | %lu | %lu | %.2fx\n",
        cycles_sobel,
        cycles_sobel_rvv,
        (double)cycles_sobel / cycles_sobel_rvv);

    std::printf("Magnitude     | %lu | %lu | %.2fx\n",
        cycles_mag,
        cycles_mag_rvv,
        (double)cycles_mag / cycles_mag_rvv);

    // ---------------- SAVE ----------------

    image_save(img_blur, "blur.raw");
    image_save(img_mag_L1, "mag.raw");
    
    
    image_save(img_blur_rvv, "blur_rvv.raw");
    image_save(img_mag_L1_rvv, "mag_rvv.raw");
    // ---------------- CLEANUP ----------------

    delete[] gx;
    delete[] gy;
    delete[] gx_rvv;
    delete[] gy_rvv;

    image_free(img_in);
    image_free(img_blur);
    image_free(img_blur_rvv);
    image_free(img_mag_L1);
    image_free(img_mag_L1_rvv);
    image_free(img_mag_L2);
    image_free(img_dir);

    std::cout << "\nDone.\n";
    return 0;
}