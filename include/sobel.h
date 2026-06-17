#pragma once
#include <cstdint>
#include <cmath>
#include <cstdlib>

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

// Computes separate Gx and Gy gradient buffers (Structure of Arrays layout).
// SoA is required for efficient RVV vectorization in Phase 6 — consecutive Gx
// values can be loaded with a single vector load, unlike interleaved AoS layout.
template <typename PixelType, typename AccType>
void sobel_gradients(const PixelType* input,
                     AccType* gx_out, AccType* gy_out,
                     int width, int height) {
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
            gx_out[y * width + x] = gx;
            gy_out[y * width + x] = gy;
        }
    }
}

// Computes edge magnitude from precomputed Gx/Gy buffers.
// Two-pass: Pass 1 finds true max, Pass 2 normalizes to [0,255] proportionally.
// use_L2_norm=false → L1: |Gx|+|Gy| (fast, slight overestimate on diagonals)
// use_L2_norm=true  → L2: sqrt(Gx²+Gy²) (mathematically correct)
template <typename AccType, typename PixelType>
void sobel_magnitude(const AccType* gx, const AccType* gy,
                     PixelType* output, int width, int height,
                     bool use_L2_norm) {
    int n = width * height;
    AccType* raw = new AccType[n];
    AccType max_val = 1;

    for (int i = 0; i < n; ++i) {
        AccType mag = use_L2_norm
            ? static_cast<AccType>(std::sqrt((float)(gx[i]*gx[i] + gy[i]*gy[i])))
            : std::abs(gx[i]) + std::abs(gy[i]);
        raw[i] = mag;
        if (mag > max_val) max_val = mag;
    }

    for (int i = 0; i < n; ++i)
        output[i] = static_cast<PixelType>((raw[i] * 255) / max_val);

    delete[] raw;
}

// Computes quantized edge direction from precomputed Gx/Gy buffers.
// Output values: 0=horizontal(~0°), 1=diagonal(~45°), 2=vertical(~90°), 3=diagonal(~135°)
// Integer-only: uses cross-multiplication instead of atan2().
// tan(22.5°) ≈ 2/5,  tan(67.5°) ≈ 12/5
template <typename AccType, typename PixelType>
void sobel_direction(const AccType* gx, const AccType* gy,
                     PixelType* output, int width, int height) {
    for (int i = 0; i < width * height; ++i) {
        AccType ax = std::abs(gx[i]);
        AccType ay = std::abs(gy[i]);

        if (ay * 5 < ax * 2)
            output[i] = 0;   // ~0°  horizontal
        else if (ay * 5 > ax * 12)
            output[i] = 2;   // ~90° vertical
        else if ((gx[i] >= 0) == (gy[i] >= 0))
            output[i] = 1;   // ~45°
        else
            output[i] = 3;   // ~135°
    }
}
