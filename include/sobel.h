#ifndef SOBEL_H
#define SOBEL_H

#include <vector>
#include <cstdint>
#include <algorithm>

// Templates allow generic pixel types (e.g., uint8_t) and intermediate math types (e.g., int16_t)
template <typename PixelType = uint8_t, typename OutType = int16_t, typename AccumType = int32_t>
void sobelFilter(
    const std::vector<PixelType>& input,
    std::vector<OutType>& gx,
    std::vector<OutType>& gy,
    int width,
    int height
) {
    gx.resize(width * height, 0); // Initialize with 0s for zero-padding compliance
    gy.resize(width * height, 0);

    const AccumType sobelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    const AccumType sobelY[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    // Notice we start at 0 instead of 1 to process ALL boundaries (Zero-Padding Requirement!)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            AccumType gxSum = 0;
            AccumType gySum = 0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int target_y = y + ky;
                    int target_x = x + kx;

                    // Zero-padding boundary guard
                    AccumType pixel_val = 0;
                    if (target_y >= 0 && target_y < height && target_x >= 0 && target_x < width) {
                        pixel_val = static_cast<AccumType>(input[target_y * width + target_x]);
                    }

                    gxSum += pixel_val * sobelX[ky + 1][kx + 1];
                    gySum += pixel_val * sobelY[ky + 1][kx + 1];
                }
            }

            gx[y * width + x] = static_cast<OutType>(gxSum);
            gy[y * width + x] = static_cast<OutType>(gySum);
        }
    }
}

#endif // SOBEL_H
