#include "gaussian.h"

#include <vector>
#include <cstdint>

std::vector<uint8_t> gaussian_blur(
    const std::vector<uint8_t>& input,
    int width,
    int height
) {
    std::vector<uint8_t> output(width * height);

    const int kernel[5][5] = {
        {1, 4, 6, 4, 1},
        {4,16,24,16, 4},
        {6,24,36,24, 6},
        {4,16,24,16, 4},
        {1, 4, 6, 4, 1}
    };

    const int kernel_sum = 256;

    for (int y = 2; y < height - 2; y++) {

        for (int x = 2; x < width - 2; x++) {

            int sum = 0;

            for (int ky = -2; ky <= 2; ky++) {

                for (int kx = -2; kx <= 2; kx++) {

                    int pixel =
                        input[(y + ky) * width + (x + kx)];

                    int weight =
                        kernel[ky + 2][kx + 2];

                    sum += pixel * weight;
                }
            }

            output[y * width + x] =
                static_cast<uint8_t>(sum / kernel_sum);
        }
    }

    return output;
}
