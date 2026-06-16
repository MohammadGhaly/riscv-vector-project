#pragma once
#include <cstdint>
#include <cmath>    // To be able to use some math funcs like square square root and arctangent for angles
#include <cstdlib>  // for basic math such as (absolte)


// 'inline' added to prevent ODR violations (same reason as gaussian_blur.h)
// The 3x3 sobel kernels for X (horizontal changes) and Y (vertical changes)
inline const int SOBEL_X[3][3] = {
    {-1,  0,  1},
    {-2,  0,  2},
    {-1,  0,  1}
};

inline const int SOBEL_Y[3][3] = {
    { 1,  2,  1},
    { 0,  0,  0},
    {-1, -2, -1}
};

// BUG FIX: Old version clamped magnitude to 255 which destroyed bright edges.
// FIXED: Two-pass approach — Pass 1 finds the true maximum, Pass 2 normalizes
// all values to [0,255] proportionally. This preserves edge contrast correctly.

// 1. Sobel magnitude function to calculate edge strength (Magnitude)
template <typename PixelType, typename AccType>
void sobel_magnitude(const PixelType* input, PixelType* output,
                     int width, int height, bool use_L2_norm) {
    int n = width * height;
    AccType* raw = new AccType[n];
    AccType max_val = 1;

    // compute raw magnitudes and find max
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            AccType gx = 0, gy = 0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int ny = y + ky, nx = x + kx;
                    AccType pv = 0;
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height)
                        pv = static_cast<AccType>(input[ny * width + nx]);
                    gx += pv * SOBEL_X[ky + 1][kx + 1];
                    gy += pv * SOBEL_Y[ky + 1][kx + 1];
                }
            }
            AccType mag = use_L2_norm
                ? static_cast<AccType>(std::sqrt((float)(gx*gx + gy*gy)))
                : std::abs(gx) + std::abs(gy);
            raw[y * width + x] = mag;
            if (mag > max_val) max_val = mag;
        }
    }

    // normalize to [0, 255]
    for (int i = 0; i < n; ++i)
        output[i] = static_cast<PixelType>((raw[i] * 255) / max_val);

    delete[] raw;
}

// 2. Function to calculate Edge Angle which will be the direction
template <typename PixelType, typename AccType>
void sobel_direction(const PixelType* input, PixelType* output, int width, int height) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            
            AccType gx = 0;
            AccType gy = 0;

            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int neighbor_y = y + ky;
                    int neighbor_x = x + kx;
                    AccType pixel_val = 0;
                    
                    if (neighbor_x >= 0 && neighbor_x < width && neighbor_y >= 0 && neighbor_y < height) {
                        pixel_val = static_cast<AccType>(input[neighbor_y * width + neighbor_x]);
                    }
                    gx += pixel_val * SOBEL_X[ky + 1][kx + 1];
                    gy += pixel_val * SOBEL_Y[ky + 1][kx + 1];
                }
            }

            // BUG FIX: Replaced atan2() with integer cross-multiplication.
            
           // Integer-only direction quantization (no atan2, no floats)
            // tan(22.5°) ≈ 2/5,  tan(67.5°) ≈ 12/5
            AccType ax = std::abs(gx);
            AccType ay = std::abs(gy);

            PixelType quantized_dir = 0;
            if (ay * 5 < ax * 2) {
                quantized_dir = 0;    // ~0°  horizontal
            } else if (ay * 5 > ax * 12) {
                quantized_dir = 90;   // ~90° vertical
            } else if ((gx >= 0) == (gy >= 0)) {
                quantized_dir = 45;   // same sign → 45°
            } else {
                 quantized_dir = 135;  // opposite sign → 135°
            }

            output[y * width + x] = quantized_dir;
        }
    }
}
