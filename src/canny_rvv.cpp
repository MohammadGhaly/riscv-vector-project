#include "../include/canny_rvv.h"
#include <riscv_vector.h>
#include <algorithm>

// =========================================================================
// STAGE 1: GAUSSIAN BLUR 5x5 RVV
// =========================================================================
void gaussian_blur_5x5_rvv(const uint8_t* input,
                           uint8_t* output,
                           int width,
                           int height)
{
    const int16_t kernel[5][5] = {
        {1,  4,  7,  4, 1},
        {4, 16, 26, 16, 4},
        {7, 26, 41, 26, 7},
        {4, 16, 26, 16, 4},
        {1,  4,  7,  4, 1}
    };

    std::fill_n(output, width * height, 0);

    for (int r = 2; r < height - 2; ++r)
    {
        int c = 2;
        const int interior_end = width - 2;

        while (c < interior_end)
        {
            size_t vl = __riscv_vsetvl_e32m4(interior_end - c);

            vint32m4_t acc = __riscv_vmv_v_x_i32m4(0, vl);

            for (int kr = -2; kr <= 2; ++kr)
            {
                const uint8_t* row = &input[(r + kr) * width + c];

                for (int kc = -2; kc <= 2; ++kc)
                {
                    int16_t weight = kernel[kr + 2][kc + 2];

                    vuint8m1_t p8 = __riscv_vle8_v_u8m1(row + kc, vl);
                    vuint16m2_t p16 = __riscv_vzext_vf2_u16m2(p8, vl);
                    vint16m2_t pi16 =
                        __riscv_vreinterpret_v_u16m2_i16m2(p16);

                    acc = __riscv_vwmacc_vx_i32m4(acc, weight, pi16, vl);
                }
            }

            vint32m4_t scaled = __riscv_vmul_vx_i32m4(acc, 240, vl);
            vint32m4_t norm   = __riscv_vsra_vx_i32m4(scaled, 16, vl);

            vint32m4_t zero   = __riscv_vmv_v_x_i32m4(0, vl);
            vint32m4_t max255 = __riscv_vmv_v_x_i32m4(255, vl);

            vint32m4_t clamped =
                __riscv_vmax_vv_i32m4(
                    __riscv_vmin_vv_i32m4(norm, max255, vl),
                    zero,
                    vl);

            vuint16m2_t out16 =
                __riscv_vncvt_x_x_w_u16m2(
                    __riscv_vreinterpret_v_i32m4_u32m4(clamped),
                    vl);

            vuint8m1_t out8 =
                __riscv_vncvt_x_x_w_u8m1(out16, vl);

            __riscv_vse8_v_u8m1(
                &output[r * width + c],
                out8,
                vl);

            c += vl;
        }
    }
}

// =========================================================================
// STAGE 2: SOBEL MAGNITUDE L1 RVV
// =========================================================================
void magnitude_l1_rvv(
    const int16_t* Gx,
    const int16_t* Gy,
    uint8_t* magnitude,
    int total_pixels)
{
    uint16_t* raw = new uint16_t[total_pixels];

    int i = 0;
    while (i < total_pixels)
    {
        size_t vl = __riscv_vsetvl_e16m2(total_pixels - i);

        vint16m2_t v_gx = __riscv_vle16_v_i16m2(&Gx[i], vl);
        vint16m2_t v_gy = __riscv_vle16_v_i16m2(&Gy[i], vl);

        vint16m2_t v_neg_gx = __riscv_vrsub_vx_i16m2(v_gx, 0, vl);
        vint16m2_t v_neg_gy = __riscv_vrsub_vx_i16m2(v_gy, 0, vl);

        vint16m2_t v_abs_gx = __riscv_vmax_vv_i16m2(v_gx, v_neg_gx, vl);
        vint16m2_t v_abs_gy = __riscv_vmax_vv_i16m2(v_gy, v_neg_gy, vl);

        vint16m2_t v_sum =
            __riscv_vadd_vv_i16m2(v_abs_gx, v_abs_gy, vl);

        __riscv_vse16_v_u16m2(
            &raw[i],
            __riscv_vreinterpret_v_i16m2_u16m2(v_sum),
            vl);

        i += vl;
    }

    uint16_t max_val = 1;
    for (int j = 0; j < total_pixels; j++)
        if (raw[j] > max_val) max_val = raw[j];

    for (int j = 0; j < total_pixels; j++)
        magnitude[j] =
            (uint8_t)((raw[j] * 255) / max_val);

    delete[] raw;
}

// =========================================================================
// STAGE 3: SOBEL GRADIENTS RVV (FIXED - NOW OUTSIDE FUNCTION)
// =========================================================================
void sobel_gradients_rvv(
    const uint8_t* input,
    int16_t* gx_out,
    int16_t* gy_out,
    int width,
    int height)
{
    std::fill_n(gx_out, width * height, 0);
    std::fill_n(gy_out, width * height, 0);

    for (int r = 1; r < height - 1; r++)
    {
        int c = 1;

        while (c < width - 1)
        {
            size_t vl = __riscv_vsetvl_e16m2(width - 1 - c);

            const uint8_t* top = &input[(r - 1) * width + c];
            const uint8_t* mid = &input[(r    ) * width + c];
            const uint8_t* bot = &input[(r + 1) * width + c];

            vuint8m1_t tl8 = __riscv_vle8_v_u8m1(top - 1, vl);
            vuint8m1_t tc8 = __riscv_vle8_v_u8m1(top, vl);
            vuint8m1_t tr8 = __riscv_vle8_v_u8m1(top + 1, vl);

            vuint8m1_t ml8 = __riscv_vle8_v_u8m1(mid - 1, vl);
            vuint8m1_t mr8 = __riscv_vle8_v_u8m1(mid + 1, vl);

            vuint8m1_t bl8 = __riscv_vle8_v_u8m1(bot - 1, vl);
            vuint8m1_t bc8 = __riscv_vle8_v_u8m1(bot, vl);
            vuint8m1_t br8 = __riscv_vle8_v_u8m1(bot + 1, vl);

            vint16m2_t tl = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(tl8, vl));
            vint16m2_t tc = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(tc8, vl));
            vint16m2_t tr = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(tr8, vl));

            vint16m2_t ml = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(ml8, vl));
            vint16m2_t mr = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(mr8, vl));

            vint16m2_t bl = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(bl8, vl));
            vint16m2_t bc = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(bc8, vl));
            vint16m2_t br = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(br8, vl));

            vint16m2_t gx = __riscv_vsub_vv_i16m2(tr, tl, vl);

            gx = __riscv_vadd_vv_i16m2(
                    gx,
                    __riscv_vsll_vx_i16m2(__riscv_vsub_vv_i16m2(mr, ml, vl), 1, vl),
                    vl);

            gx = __riscv_vadd_vv_i16m2(gx, __riscv_vsub_vv_i16m2(br, bl, vl), vl);

            vint16m2_t gy = __riscv_vadd_vv_i16m2(tl, tr, vl);

            gy = __riscv_vadd_vv_i16m2(
                    gy,
                    __riscv_vsll_vx_i16m2(tc, 1, vl),
                    vl);

            vint16m2_t bottom = __riscv_vadd_vv_i16m2(bl, br, vl);
            bottom = __riscv_vadd_vv_i16m2(bottom, __riscv_vsll_vx_i16m2(bc, 1, vl), vl);

            gy = __riscv_vsub_vv_i16m2(gy, bottom, vl);

            __riscv_vse16_v_i16m2(&gx_out[r * width + c], gx, vl);
            __riscv_vse16_v_i16m2(&gy_out[r * width + c], gy, vl);

            c += vl;
        }
    }
}