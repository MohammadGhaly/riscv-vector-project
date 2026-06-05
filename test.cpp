#include <iostream>
#include <riscv_vector.h>

int main() {
    // Use the specific intrinsic instruction requested in Step E
    size_t vl = __riscv_vsetvl_e8m1(10);

    std::cout << "Success! Vector length configured to: " << vl << std::endl;
    return 0;
}
