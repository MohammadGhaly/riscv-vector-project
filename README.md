Phase 1: Environment Setup focuses on establishing the core tools needed for development and emulation. It requires configuring a WSL2/Linux environment, compiling both the RISC-V GNU Toolchain and QEMU from source to guarantee support for RVV 1.0 vector instructions, and configuring a dual-target Makefile alongside GoogleTest for automated host-side and RISC-V testing.

Phase 2: Scalar Baseline Pipeline involves implementing the foundational Canny edge detection algorithms in standard C++ without external libraries. It includes handling raw image I/O with aligned memory , creating a 5x5 Gaussian blur using integer arithmetic , and applying 3x3 Sobel filters stored in a Structure of Arrays (SoA) layout. Finally, it requires calculating gradient magnitudes using L1 or L2 norms and quantizing gradient directions via integer cross-multiplication to avoid floating-point overhead.

Phase 3: Testing involves developing test suites to verify algorithmic accuracy and the correctness of the RVV vectorization. You must write host-side GoogleTests to catch common bugs by evaluating how the pipeline handles known patterns, such as uniform images, impulse pixels, and sharp edges. Furthermore, you need QEMU-side assert-based equivalence tests to ensure the scalar and RVV functions produce matching outputs. These tests must be run across multiple vector lengths (VLEN=128, 256, 512) and utilize non-power-of-two image sizes to validate your vector-length-agnostic code and strip-mining tail logic.

### Phase 3: Testing and Verification

| Test Category | Environment | Description / Goal |
| :--- | :--- | :--- |
| **Gaussian Blur** | Host-Side (GoogleTest) | [cite_start]Verify uniform images remain uniform, black images remain black, and a single bright pixel spreads symmetrically. |
| **Sobel Gradient** | Host-Side (GoogleTest) | [cite_start]Verify uniform images produce zero gradient, vertical edges produce large Gx, and horizontal edges produce large Gy. |
| **Gradient Direction** | Host-Side (GoogleTest) | [cite_start]Ensure vertical edges return direction 0, horizontal return 2, and diagonal return 1 or 3. |
| **Gradient Magnitude** | Host-Side (GoogleTest) | [cite_start]Confirm both L1 and L2 norms calculate correctly on random images without crashing or outputting all-zeros. |
| **Scalar vs. RVV Equivalence** | QEMU-Side (Asserts) | Compare the output of the C++ scalar code against the RVV code. [cite_start]Outputs must match (with a ±1 rounding tolerance) across `VLEN=128`, `256`, and `512`. |
| **Strip-Mining Tail Logic** | QEMU-Side (Asserts) | [cite_start]Use non-power-of-two image sizes (e.g., 48x48 or 100x75) to force the vector loop's "tail case" to execute, ensuring the code is fully vector-length-agnostic (VLA). |

Phase 4: Compiler Optimization Sweep involves establishing a performance baseline by compiling your scalar code at multiple optimization levels (such as -O0, -O2, and -O3) and measuring both the execution time and binary size. Because QEMU is not cycle-accurate, you must use wall-clock timing (clock_gettime) over multiple iterations to get stable measurements. Additionally, this phase requires you to analyze the compiler's auto-vectorization capabilities by using specific GCC flags to see which loops the compiler managed to vectorize, and counting the vector instructions in the generated disassembly.

### Complete Phase 4 Results (256x256 image, VLEN=128)

| Stage | -O0 | -O2 | -O3 |
| :--- | :--- | :--- | :--- |
| Gaussian Blur | 62.92 ms | 23.76 ms | 14.11 ms |
| Sobel Gx/Gy | 24.68 ms | 11.03 ms | 3.43 ms |
| Magnitude L1 | 3.48 ms | 2.49 ms | 2.28 ms |
| Magnitude L2 | 32.78 ms | 16.69 ms | 16.62 ms |
| Direction | 2.02 ms | 0.93 ms | 0.87 ms |
| **Total** | **125.87 ms** | **54.90 ms** | **37.31 ms** |
| **Binary Size** | **27K** | **20K** | **24K** |

Phase 5: Profiling involves wrapping each individual stage of your pipeline in timing calls to generate a percentage breakdown of the total execution time. This profiling data identifies the major performance bottlenecks (the "hot" stages), such as the Gaussian and Sobel filters. Guided by Amdahl's law, this phase teaches you to focus your hardware optimization efforts strictly on the stages where they will have the most significant impact, rather than wasting time optimizing fast stages like direction computation.

### Profiling Percentage Breakdown

| Stage | -O0 | -O2 | -O3 |
| :--- | :--- | :--- | :--- |
| Gaussian | 49.9% | 43.3% | 37.8% |
| Sobel | 19.6% | 20.1% | 9.2% |
| Magnitude L2 | 26.0% | 30.4% | 44.6% |
| Magnitude L1 | 2.8% | 4.5% | 6.1% |
| Direction | 1.6% | 1.7% | 2.3% |

Phase 6: RVV Intrinsic Optimization focuses on replacing your pipeline's performance bottlenecks by manually rewriting them with RISC-V Vector (RVV) intrinsics. It requires you to implement vector-length-agnostic (VLA) loops using strip-mining (riscv_vsetvl), allowing the same binary to scale dynamically across varying hardware register sizes. You must experiment with different Length Multiplier (LMUL) configurations to balance data throughput against register pressure, manage data widening instructions (e.g., riscv_vwmul) to safely prevent arithmetic overflows when multiplying 8-bit pixels with 16-bit coefficients, and utilize vector reduction intrinsics (riscv_vredmax) to calculate global values for image normalization.

### Final Gaussian LMUL Results

| LMUL | Scalar Cycles | RVV Cycles | Speedup |
| :--- | :--- | :--- | :--- |
| m1 | ~11.97 M | 69.62 M | 0.17× |
| m2 | ~12.64 M | 58.22 M | 0.22× |
| m4 | ~14.24 M | 58.60 M | 0.24× |

### Performance Comparison (Scalar vs RVV)

| Pipeline Stage | Scalar Cycles | RVV Cycles | Speedup |
| :--- | :--- | :--- | :--- |
| Gaussian Blur | 12,192,612 | 58,047,330 | 0.21× |
| Sobel Gx/Gy | 5,435,263 | 14,247,091 | 0.38× |
| Magnitude L1 | 1,775,811 | 5,534,641 | 0.32× |

### The Final Optimization Table

| Stage | -O0 | -O2 | -O3 | RVV 128 (Cycles) |
| :--- | :--- | :--- | :--- | :--- |
| **Gaussian 5x5** | 62.92 ms | 23.76 ms | 14.11 ms | 580.5 ms |
| **Sobel Gx/Gy** | 24.68 ms | 11.03 ms | 3.43 ms | 116.1 ms |
| **Magnitude (L1)** | 3.48 ms | 2.49 ms | 2.28 ms | 58.05 ms |
| **Direction** | 2.02 ms | 0.93 ms | 0.87 ms | scalar |
| **Binary Size** | **27 KB** | **20 KB** | **24 KB** | **26 KB** |

### Performance Across VLEN Configurations

| Stage | VLEN=128 | VLEN=256 | VLEN=512 |
| :--- | :--- | :--- | :--- |
| **Gaussian** | 0.55x | 0.57x | 0.62x |
| **Sobel** | 0.89x | 0.95x | 1.03x |
| **Magnitude** | 0.37x | 0.42x | 0.42x |
