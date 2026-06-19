#ifndef CANNY_RVV_H
#define CANNY_RVV_H

#include <cstdint>
#include <cstddef>

// Update this line to use const int16_t* for Gx and Gy:
void magnitude_l1_rvv(const int16_t* Gx, const int16_t* Gy, uint8_t* magnitude, int total_pixels);

// Your Gaussian Blur function prototype:
void gaussian_blur_5x5_rvv(const uint8_t* input, uint8_t* output, int width, int height);

void sobel_gradients_rvv(const uint8_t* input,

                         int16_t* gx_out,
                         int16_t* gy_out,
                         int width,
                         int height);
#endif // CANNY_RVV_H