#include "sobel.h"

void sobelFilter(
    const std::vector<uint8_t>& input,
    std::vector<int16_t>& gx,
    std::vector<int16_t>& gy,
    int width,
    int height
)
{
    gx.resize(width * height);
    gy.resize(width * height);

    int sobelX[3][3] =
    {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    int sobelY[3][3] =
    {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            int gxSum = 0;
            int gySum = 0;

            for (int ky = -1; ky <= 1; ky++)
            {
                for (int kx = -1; kx <= 1; kx++)
                {
                    int pixel =
                        input[(y + ky) * width + (x + kx)];

                    gxSum += pixel *
                             sobelX[ky + 1][kx + 1];

                    gySum += pixel *
                             sobelY[ky + 1][kx + 1];
                }
            }

            gx[y * width + x] =
                static_cast<int16_t>(gxSum);

            gy[y * width + x] =
                static_cast<int16_t>(gySum);
        }
    }
}    
