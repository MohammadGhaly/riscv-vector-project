#ifndef GAUSSIAN_H
#define GAUSSIAN_H

#include <vector>
#include <cstdint>

// Template parameters satisfy Phase 2 generic type requirements!
template <typename PixelType = uint8_t, typename AccumType = int32_t>
std::vector<PixelType> gaussian_blur(
    const std::vector<PixelType>& input,
    int width,
    int height
) {
    std::vector<PixelType> output(width * height, 0);

    const AccumType kernel[5][5] = {
        {1, 4, 6, 4, 1},
        {4,16,24,16, 4},
        {6,24,36,24, 6},
        {4,16,24,16, 4},
        {1, 4, 6, 4, 1}
    };

    const AccumType kernel_sum = 256;

    // Loop through EVERY single pixel (0 to max) to fulfill Zero-Padding compliance
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            AccumType sum = 0;

            for (int ky = -2; ky <= 2; ky++) {
                for (int kx = -2; kx <= 2; kx++) {
                    int target_y = y + ky;
                    int target_x = x + kx;

                    // Zero-padding boundary guard
                    AccumType pixel_val = 0; 
                    if (target_y >= 0 && target_y < height && target_x >= 0 && target_x < width) {
                        pixel_val = static_cast<AccumType>(input[target_y * width + target_x]);
                    }

                    AccumType weight = kernel[ky + 2][kx + 2];
                    sum += pixel_val * weight;
                }
            }

            output[y * width + x] = static_cast<PixelType>(sum / kernel_sum);
        }
    }

    return output;
}

#endif // GAUSSIAN_H