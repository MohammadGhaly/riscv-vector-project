#pragma once
#include <cstdint>
#include <cmath>    // To be able to use some math funcs like square square root and arctangent for angles
#include <cstdlib>  // for basic math such as (absolte)

// The 3x3 sobel kernels for X (horizontal changes) and Y (vertical changes)
const int SOBEL_X[3][3] = {
    {-1,  0,  1},
    {-2,  0,  2},
    {-1,  0,  1}
};

const int SOBEL_Y[3][3] = {
    { 1,  2,  1},
    { 0,  0,  0},
    {-1, -2, -1}
};

// 1. Sobel magnitude function to calculate edge strength (Magnitude)
template <typename PixelType, typename AccType>
void sobel_magnitude(const PixelType* input, PixelType* output, int width, int height, bool use_L2_norm) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            
            AccType gx = 0;
            AccType gy = 0;

            // Slide our 3x3 kernel over the image
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int neighbor_y = y + ky;
                    int neighbor_x = x + kx;

                    AccType pixel_val = 0;
                    
                    // Zero-padding boundary check
                    if (neighbor_x >= 0 && neighbor_x < width && neighbor_y >= 0 && neighbor_y < height) {
                        pixel_val = static_cast<AccType>(input[neighbor_y * width + neighbor_x]);
                    }

                    // Applying the sobel math for X and Y
                    gx += pixel_val * SOBEL_X[ky + 1][kx + 1];
                    gy += pixel_val * SOBEL_Y[ky + 1][kx + 1];
                }
            }

            // Calculate final edge strength using L1 or L2 norm methods
            AccType magnitude = 0;
            if (use_L2_norm) {
                magnitude = static_cast<AccType>(std::sqrt(gx * gx + gy * gy));
            } else {
                magnitude = std::abs(gx) + std::abs(gy);
            }

            // Keep the value between 0 (black) and 255 (white)
            if (magnitude > 255) magnitude = 255;
            
            output[y * width + x] = static_cast<PixelType>(magnitude);
        }
    }
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

            // Find the angle using arctangent
            // multiplying by (180/pi) because atan2() function produce answer in radians
            float angle = std::atan2(gy, gx) * (180.0 / M_PI);
            
            // Fix negative angles
            if (angle < 0) angle += 180.0;

            // Round the angle to 0, 45, 90, or 135 degrees
            PixelType quantized_dir = 0;
            if (angle < 22.5 || angle >= 157.5) {
                quantized_dir = 0;
            } else if (angle >= 22.5 && angle < 67.5) {
                quantized_dir = 45;
            } else if (angle >= 67.5 && angle < 112.5) {
                quantized_dir = 90;
            } else {
                quantized_dir = 135;
            }

            output[y * width + x] = quantized_dir;
        }
    }
}
