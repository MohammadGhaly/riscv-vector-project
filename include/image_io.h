#ifndef IMAGE_IO_H
#define IMAGE_IO_H

#include <vector>
#include <string>
#include <cstdint>

std::vector<uint8_t> load_image(
    const std::string& path,
    int width,
    int height
);

void save_image(
    const std::string& path,
    const std::vector<uint8_t>& data
);

#endif