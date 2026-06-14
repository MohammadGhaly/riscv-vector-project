#include <vector>
#include <fstream>
#include <cstdint>

int main() {
    int width = 256, height = 256;
    std::vector<uint8_t> img(width * height);

    // simple gradient image
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            img[y * width + x] = x;
        }
    }

    std::ofstream file("assets/input.raw", std::ios::binary);
    file.write(reinterpret_cast<char*>(img.data()), img.size());

    return 0;
}

