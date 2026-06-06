#pragma once
#include <cstdint>

// The 5x5 Gaussian Kernel Weights
const int GAUSSIAN_KERNEL_5X5[5][5] = {
    {1,  4,  7,  4, 1},
    {4, 16, 26, 16, 4},
    {7, 26, 41, 26, 7},
    {4, 16, 26, 16, 4},
    {1,  4,  7,  4, 1}
};
const int GAUSSIAN_SUM = 273;

// Generic gaussian blur function
template <typename PixelType, typename AccType>
void gaussian_blur_5x5(const PixelType* input, PixelType* output, int width, int height) {

    // Scanning through every pixel in the image (row by row, column by column)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            AccType sum = 0;

            // using the 5x5 kernel over the current pixel
            for (int ky = -2; ky <= 2; ++ky) {
                for (int kx = -2; kx <= 2; ++kx) {

                    int neighbor_y = y + ky;
                    int neighbor_x = x + kx;
                    AccType pixel_val = 0;

                    // Zero-padding: Check if the neighbor is actually inside the image and if not assume its value equal 0 (black)
                    if (neighbor_x >= 0 && neighbor_x < width && neighbor_y >= 0 && neighbor_y < height) {
                        int index = neighbor_y * width + neighbor_x;
                        pixel_val = static_cast<AccType>(input[index]);
                    } else {
                        pixel_val = 0;
                    }

                    // Get the weight from our grid and multiply
                    int weight = GAUSSIAN_KERNEL_5X5[ky + 2][kx + 2];
                    sum += pixel_val * static_cast<AccType>(weight);
                }
            }

            // Calculating the total average and saving it to the output image
            int out_index = y * width + x;
            output[out_index] = static_cast<PixelType>(sum / static_cast<AccType>(GAUSSIAN_SUM));
        }
    }
