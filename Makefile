# ==================================================
# Canny Edge Detection - Build System
# ==================================================

# ───────────────────────────────────────────────────
# Compilers
# ───────────────────────────────────────────────────
HOST_CXX   = g++
RV_CXX     = riscv64-linux-gnu-g++

# ───────────────────────────────────────────────────
# QEMU
# FIX A: Point to your locally-built QEMU binary.
# FIX B: Add -L so QEMU can find the RISC-V runtime
#         loader at /usr/riscv64-linux-gnu/lib/ld-linux-riscv64-lp64d.so.1
# ───────────────────────────────────────────────────
QEMU_BIN = /usr/bin/qemu-riscv64
QEMU_L   = /usr/riscv64-linux-gnu
QEMU     = $(QEMU_BIN) -L $(QEMU_L)

# ───────────────────────────────────────────────────
# GoogleTest (Host-only)
# ───────────────────────────────────────────────────
GTEST_INC  = $(HOME)/gtest-install/include
GTEST_LIB  = $(HOME)/gtest-install/lib

# ───────────────────────────────────────────────────
# Flags
# ───────────────────────────────────────────────────
COMMON_INC = -I./include

HOST_FLAGS = -std=c++17 -O2 -Wall $(COMMON_INC) -I$(GTEST_INC)

RV_FLAGS_O0 = -std=c++17 -O0 -Wall $(COMMON_INC) -march=rv64gcv
RV_FLAGS_O2 = -std=c++17 -O2 -Wall $(COMMON_INC) -march=rv64gcv
RV_FLAGS_O3 = -std=c++17 -O3 -ftree-vectorize -Wall $(COMMON_INC) \
              -march=rv64gcv -fopt-info-vec-all

# ───────────────────────────────────────────────────
# Sources
# ───────────────────────────────────────────────────
HOST_SRCS = src/image.cpp tests/test_pipeline.cpp
RV_SRCS   = src/image.cpp src/main.cpp

# ───────────────────────────────────────────────────
# Outputs
# ───────────────────────────────────────────────────
BUILD_DIR  = build

TEST_BIN   = $(BUILD_DIR)/test_pipeline

RV_BIN_O0  = $(BUILD_DIR)/canny_rv_o0
RV_BIN_O2  = $(BUILD_DIR)/canny_rv_o2
RV_BIN_O3  = $(BUILD_DIR)/canny_rv_o3

# FIX C: Use the TEST_IMG variable everywhere in sweep_run
#         instead of the hardcoded string "build/test_256x256.raw".
#         $(PWD) makes it an absolute path — safe regardless of
#         what directory make is invoked from.
TEST_IMG     = $(PWD)/$(BUILD_DIR)/test_256x256.raw
IMG_W        = 256
IMG_H        = 256

# ==================================================
# Default target
# ==================================================
all: test sweep

# ==================================================
# Build directory
# ==================================================
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# ==================================================
# HOST TESTING (Phase 3)
# ==================================================

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(HOST_SRCS) | $(BUILD_DIR)
	$(HOST_CXX) $(HOST_FLAGS) $^ \
		-L$(GTEST_LIB) -lgtest -lgtest_main -lpthread \
		-o $@

# ==================================================
# OPTIMIZATION SWEEP (Phase 4)
# ==================================================

sweep: $(RV_BIN_O0) $(RV_BIN_O2) $(RV_BIN_O3)

$(RV_BIN_O0): $(RV_SRCS) | $(BUILD_DIR)
	$(RV_CXX) $(RV_FLAGS_O0) $^ -o $@

$(RV_BIN_O2): $(RV_SRCS) | $(BUILD_DIR)
	$(RV_CXX) $(RV_FLAGS_O2) $^ -o $@

$(RV_BIN_O3): $(RV_SRCS) | $(BUILD_DIR)
	$(RV_CXX) $(RV_FLAGS_O3) $^ -o $@

sweep_run: sweep
	@echo "=== O0 ==="
	$(QEMU) -cpu rv64,v=true,vlen=128 $(RV_BIN_O0) $(TEST_IMG) $(IMG_W) $(IMG_H)
	@echo "=== O2 ==="
	$(QEMU) -cpu rv64,v=true,vlen=128 $(RV_BIN_O2) $(TEST_IMG) $(IMG_W) $(IMG_H)
	@echo "=== O3 ==="
	$(QEMU) -cpu rv64,v=true,vlen=128 $(RV_BIN_O3) $(TEST_IMG) $(IMG_W) $(IMG_H)

# ==================================================
# BINARY SIZE (Phase 4)
# ==================================================

size: sweep
	@echo "=== Binary Size Analysis ==="
	@ls -lh $(RV_BIN_O0) $(RV_BIN_O2) $(RV_BIN_O3)

# ==================================================
# CLEAN
# ==================================================

clean:
	rm -rf $(BUILD_DIR) *.o

# ==================================================
# HELP
# ==================================================

help:
	@echo "Available targets:"
	@echo "  make test       -> Run GoogleTest suite"
	@echo "  make sweep      -> Build O0/O2/O3 binaries"
	@echo "  make sweep_run  -> Run all 3 and print timing"
	@echo "  make size       -> Compare binary sizes"
	@echo "  make clean      -> Remove build artifacts"
	@echo ""
	@echo "Variables (override on command line):"
	@echo "  TEST_IMG=$(TEST_IMG)"
	@echo "  IMG_W=$(IMG_W)  IMG_H=$(IMG_H)"
