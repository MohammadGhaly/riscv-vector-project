#include <riscv_vector.h>
#include <stdio.h>
#include <stdint.h>

int main() {
    int32_t a[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    int32_t b[16] = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    int32_t c[16] = {0};

    size_t n = 16;
    for (size_t i = 0; i < n; ) {
        size_t vl = __riscv_vsetvl_e32m1(n - i);
        vint32m1_t va = __riscv_vle32_v_i32m1(a + i, vl);
        vint32m1_t vb = __riscv_vle32_v_i32m1(b + i, vl);
        vint32m1_t vc = __riscv_vadd_vv_i32m1(va, vb, vl);
        __riscv_vse32_v_i32m1(c + i, vc, vl);
        i += vl;
    }

    for (int i = 0; i < 16; i++) printf("%d ", c[i]);
    printf("\n");
    return 0;
}

