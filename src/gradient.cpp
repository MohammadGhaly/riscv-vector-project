#include "gradient.h"

#include <cmath>
#include <algorithm>

void computeMagnitudeL1(
    const std::vector<int16_t>& gx,
    const std::vector<int16_t>& gy,
    std::vector<uint8_t>& magnitude,
    int width,
    int height
)
{
    magnitude.resize(width * height);

    for (int i = 0; i < width * height; i++)
    {
        int mag = std::abs(gx[i]) + std::abs(gy[i]);

        mag = std::min(mag, 255);

        magnitude[i] = static_cast<uint8_t>(mag);
    }
}

void computeMagnitudeL2(
    const std::vector<int16_t>& gx,
    const std::vector<int16_t>& gy,
    std::vector<uint8_t>& magnitude,
    int width,
    int height
)
{
    magnitude.resize(width * height);

    for (int i = 0; i < width * height; i++)
    {
        int mag = static_cast<int>(
            std::sqrt(gx[i] * gx[i] + gy[i] * gy[i])
        );

        mag = std::min(mag, 255);

        magnitude[i] = static_cast<uint8_t>(mag);
    }
}

void computeDirection(
    const std::vector<int16_t>& gx,
    const std::vector<int16_t>& gy,
    std::vector<uint8_t>& direction,
    int width,
    int height
)
{
    direction.resize(width * height);

    for (int i = 0; i < width * height; i++)
    {
        float angle = std::atan2(
            static_cast<float>(gy[i]),
            static_cast<float>(gx[i])
        ) * 180.0f / M_PI;

        if (angle < 0)
        {
            angle += 180.0f;
        }

        if ((angle >= 0 && angle < 22.5) ||
            (angle >= 157.5 && angle <= 180))
        {
            direction[i] = 0;
        }
        else if (angle >= 22.5 && angle < 67.5)
        {
            direction[i] = 45;
        }
        else if (angle >= 67.5 && angle < 112.5)
        {
            direction[i] = 90;
        }
        else
        {
            direction[i] = 135;
        }
    }
}
