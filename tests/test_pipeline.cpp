#include <gtest/gtest.h>
#include <cstdint>
#include <cmath>
#include "image.h"
#include "gaussian_blur.h"
#include "sobel.h"
#include "test_image_gen.h"

// ─── Helpers ─────────────────────────────────────────────────────────────────

// Returns true if all interior pixels (ignoring 2px border) are within
// tolerance of expected value. Border pixels are skipped because of zero-padding
// causes edge darkening, which is expected and correct behavior.
static bool interior_pixels_near(const uint8_t* data, int width, int height,
                                  uint8_t expected, int tolerance = 1) {
    for (int y = 2; y < height - 2; ++y)
        for (int x = 2; x < width - 2; ++x)
            if (std::abs((int)data[y * width + x] - (int)expected) > tolerance)
                return false;
    return true;
}

// ─── Gaussian Tests ───────────────────────────────────────────────────────────

TEST(GaussianBlur, UniformImageUnchanged) {
    // Blurring a uniform image should return the same uniform value.
    // Interior pixels only — border darkens due to zero-padding, which is correct.
    Image img = gen_uniform(64, 64, 128);
    Image out = image_create(64, 64);
    gaussian_blur_5x5<uint8_t, int>(img.data, out.data, 64, 64);
    EXPECT_TRUE(interior_pixels_near(out.data, 64, 64, 128, 1));
    image_free(img);
    image_free(out);
}

TEST(GaussianBlur, AllBlackStaysBlack) {
    // Blurring an all-zero image must produce all zeros everywhere including borders.
    Image img = gen_uniform(64, 64, 0);
    Image out = image_create(64, 64);
    gaussian_blur_5x5<uint8_t, int>(img.data, out.data, 64, 64);
    for (int i = 0; i < 64 * 64; ++i)
        EXPECT_EQ(out.data[i], 0);
    image_free(img);
    image_free(out);
}

TEST(GaussianBlur, ImpulseSpreadsSymmetrically) {
    // A single bright pixel at center should spread to neighbors.
    // The center pixel value must decrease (energy spreads out).
    // Immediate neighbors must be nonzero (energy received).
    int W = 64, H = 64;
    Image img = gen_impulse(W, H);
    Image out = image_create(W, H);
    gaussian_blur_5x5<uint8_t, int>(img.data, out.data, W, H);

    int cx = W / 2, cy = H / 2;
    // Center must be less than 255 (energy spread out)
    EXPECT_LT(out.data[cy * W + cx], 255);
    // Center must be nonzero (it still has the most energy)
    EXPECT_GT(out.data[cy * W + cx], 0);
    // Immediate neighbors must be nonzero
    EXPECT_GT(out.data[(cy - 1) * W + cx], 0);
    EXPECT_GT(out.data[(cy + 1) * W + cx], 0);
    EXPECT_GT(out.data[cy * W + (cx - 1)], 0);
    EXPECT_GT(out.data[cy * W + (cx + 1)], 0);
    // Symmetry: top neighbor == bottom neighbor, left == right
    EXPECT_EQ(out.data[(cy - 1) * W + cx], out.data[(cy + 1) * W + cx]);
    EXPECT_EQ(out.data[cy * W + (cx - 1)], out.data[cy * W + (cx + 1)]);

    image_free(img);
    image_free(out);
}

TEST(GaussianBlur, NonPowerOfTwoSize) {
    // Tests strip-mining tail case — image width not a multiple of any vector length.
    Image img = gen_uniform(100, 75, 200);
    Image out = image_create(100, 75);
    gaussian_blur_5x5<uint8_t, int>(img.data, out.data, 100, 75);
    EXPECT_TRUE(interior_pixels_near(out.data, 100, 75, 200, 1));
    image_free(img);
    image_free(out);
}

// ─── Sobel Gradient Tests ─────────────────────────────────────────────────────

TEST(SobelGradients, UniformImageZeroGradient) {
    // A uniform image has no edges — Gx and Gy must both be zero everywhere.
    int W = 64, H = 64;
    Image img = gen_uniform(W, H, 128);
    int* gx = new int[W * H];
    int* gy = new int[W * H];
    sobel_gradients<uint8_t, int>(img.data, gx, gy, W, H);
    for (int y = 1; y < H - 1; ++y)
        for (int x = 1; x < W - 1; ++x) {
            EXPECT_EQ(gx[y * W + x], 0);
            EXPECT_EQ(gy[y * W + x], 0);
        }
    delete[] gx;
    delete[] gy;
    image_free(img);
}

TEST(SobelGradients, VerticalEdgeLargeGxNearZeroGy) {
    // Left=black, right=white: strong horizontal gradient (large Gx),
    // no vertical gradient (Gy near zero) along the edge column.
    int W = 64, H = 64;
    Image img = gen_vertical_edge(W, H);
    int* gx = new int[W * H];
    int* gy = new int[W * H];
    sobel_gradients<uint8_t, int>(img.data, gx, gy, W, H);

    // Check a pixel right on the edge (center column, center row)
    int ex = W / 2, ey = H / 2;
    EXPECT_GT(std::abs(gx[ey * W + ex]), 100);  // strong Gx
    EXPECT_LT(std::abs(gy[ey * W + ex]), 10);   // near-zero Gy

    delete[] gx;
    delete[] gy;
    image_free(img);
}

TEST(SobelGradients, HorizontalEdgeLargeGyNearZeroGx) {
    // Top=black, bottom=white: strong vertical gradient (large Gy),
    // no horizontal gradient (Gx near zero) along the edge row.
    int W = 64, H = 64;
    Image img = gen_horizontal_edge(W, H);
    int* gx = new int[W * H];
    int* gy = new int[W * H];
    sobel_gradients<uint8_t, int>(img.data, gx, gy, W, H);

    int ex = W / 2, ey = H / 2;
    EXPECT_GT(std::abs(gy[ey * W + ex]), 100);  // strong Gy
    EXPECT_LT(std::abs(gx[ey * W + ex]), 10);   // near-zero Gx

    delete[] gx;
    delete[] gy;
    image_free(img);
}

TEST(SobelGradients, DiagonalEdgeBothNonZero) {
    // A diagonal edge should produce significant values in both Gx and Gy.
    int W = 64, H = 64;
    Image img = gen_diagonal(W, H);
    int* gx = new int[W * H];
    int* gy = new int[W * H];
    sobel_gradients<uint8_t, int>(img.data, gx, gy, W, H);

    // Find at least one pixel with both Gx and Gy nonzero
    bool found = false;
    for (int i = 0; i < W * H; ++i)
        if (std::abs(gx[i]) > 10 && std::abs(gy[i]) > 10) { found = true; break; }
    EXPECT_TRUE(found);

    delete[] gx;
    delete[] gy;
    image_free(img);
}

// ─── Direction Tests ──────────────────────────────────────────────────────────

TEST(SobelDirection, VerticalEdgeIsHorizontalGradient) {
    // Vertical edge → gradient points horizontally → direction = 0
    int W = 64, H = 64;
    Image img = gen_vertical_edge(W, H);
    int* gx = new int[W * H];
    int* gy = new int[W * H];
    uint8_t* dir = new uint8_t[W * H]();
    sobel_gradients<uint8_t, int>(img.data, gx, gy, W, H);
    sobel_direction<int, uint8_t>(gx, gy, dir, W, H);

    int ex = W / 2, ey = H / 2;
    EXPECT_EQ(dir[ey * W + ex], 0);

    delete[] gx; delete[] gy; delete[] dir;
    image_free(img);
}

TEST(SobelDirection, HorizontalEdgeIsVerticalGradient) {
    // Horizontal edge → gradient points vertically → direction = 2
    int W = 64, H = 64;
    Image img = gen_horizontal_edge(W, H);
    int* gx = new int[W * H];
    int* gy = new int[W * H];
    uint8_t* dir = new uint8_t[W * H]();
    sobel_gradients<uint8_t, int>(img.data, gx, gy, W, H);
    sobel_direction<int, uint8_t>(gx, gy, dir, W, H);

    int ex = W / 2, ey = H / 2;
    EXPECT_EQ(dir[ey * W + ex], 2);

    delete[] gx; delete[] gy; delete[] dir;
    image_free(img);
}

TEST(SobelDirection, DiagonalEdgeIs45or135) {
    // Diagonal edge → direction should be 1 or 3
    int W = 64, H = 64;
    Image img = gen_diagonal(W, H);
    int* gx = new int[W * H];
    int* gy = new int[W * H];
    uint8_t* dir = new uint8_t[W * H]();
    sobel_gradients<uint8_t, int>(img.data, gx, gy, W, H);
    sobel_direction<int, uint8_t>(gx, gy, dir, W, H);

    // Find a pixel near the diagonal with strong gradient
    bool found = false;
    for (int i = 0; i < W * H; ++i)
        if (std::abs(gx[i]) > 10 && (dir[i] == 1 || dir[i] == 3)) { found = true; break; }
    EXPECT_TRUE(found);

    delete[] gx; delete[] gy; delete[] dir;
    image_free(img);
}

// ─── Magnitude Tests ──────────────────────────────────────────────────────────

TEST(SobelMagnitude, L1NonzeroOnEdgeImage) {
    int W = 64, H = 64;
    Image img = gen_vertical_edge(W, H);
    int* gx = new int[W * H];
    int* gy = new int[W * H];
    uint8_t* mag = new uint8_t[W * H]();
    sobel_gradients<uint8_t, int>(img.data, gx, gy, W, H);
    sobel_magnitude<int, uint8_t>(gx, gy, mag, W, H, false);

    bool has_nonzero = false;
    for (int i = 0; i < W * H; ++i)
        if (mag[i] > 0) { has_nonzero = true; break; }
    EXPECT_TRUE(has_nonzero);

    delete[] gx; delete[] gy; delete[] mag;
    image_free(img);
}

TEST(SobelMagnitude, L2NonzeroOnEdgeImage) {
    int W = 64, H = 64;
    Image img = gen_vertical_edge(W, H);
    int* gx = new int[W * H];
    int* gy = new int[W * H];
    uint8_t* mag = new uint8_t[W * H]();
    sobel_gradients<uint8_t, int>(img.data, gx, gy, W, H);
    sobel_magnitude<int, uint8_t>(gx, gy, mag, W, H, true);

    bool has_nonzero = false;
    for (int i = 0; i < W * H; ++i)
        if (mag[i] > 0) { has_nonzero = true; break; }
    EXPECT_TRUE(has_nonzero);

    delete[] gx; delete[] gy; delete[] mag;
    image_free(img);
}

TEST(SobelMagnitude, UniformImageZeroMagnitude) {
    // Uniform image has no edges — magnitude must be zero everywhere
    // (max_val stays 1 so normalization produces 0 for all zero raw values)
    int W = 64, H = 64;
    Image img = gen_uniform(W, H, 128);
    int* gx = new int[W * H];
    int* gy = new int[W * H];
    uint8_t* mag = new uint8_t[W * H]();
    sobel_gradients<uint8_t, int>(img.data, gx, gy, W, H);
    sobel_magnitude<int, uint8_t>(gx, gy, mag, W, H, false);

    for (int y = 1; y < H - 1; ++y)
        for (int x = 1; x < W - 1; ++x)
            EXPECT_EQ(mag[y * W + x], 0);

    delete[] gx; delete[] gy; delete[] mag;
    image_free(img);
}
