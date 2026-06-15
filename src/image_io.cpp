#include "image_io.h"

#include <fstream>
#include <iostream>
#include <vector>

std::vector<uint8_t> load_image(
    const std::string& path,
    int width,
    int height
) {
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        std::cerr << "Failed to open input file: "
                  << path << std::endl;
        return {};
    }

    std::vector<uint8_t> data(width * height);

    file.read(
        reinterpret_cast<char*>(data.data()),
        data.size()
    );

    return data;
}

void save_image(
    const std::string& path,
    const std::vector<uint8_t>& data
) {
    std::ofstream file(path, std::ios::binary);

    if (!file) {
        std::cerr << "Failed to open output file: "
                  << path << std::endl;
        return;
    }

    file.write(
        reinterpret_cast<const char*>(data.data()),
        data.size()
    );
}