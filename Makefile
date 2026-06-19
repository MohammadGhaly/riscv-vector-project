# ==============================================================================
# Canny Edge Detection - Dual-Target Build System (Host & RISC-V Vector)
# ==============================================================================

# ──────────────────────────────────────────────────────────────────────────────
# Compilers & Toolchains
# ──────────────────────────────────────────────────────────────────────────────
HOST_CXX   = g++
RV_CXX     = riscv64-linux-gnu-g++

# ──────────────────────────────────────────────────────────────────────────────
# QEMU User Space Emulator Configuration
# ──────────────────────────────────────────────────────────────────────────────
QEMU_BIN   = $(HOME)/qemu/build/qemu-riscv64
QEMU_L     = /usr/riscv64-linux-gnu
QEMU       = $(QEMU_BIN) -cpu rv64,v=true,vlen=128 -L $(QEMU_L)

# ──────────────────────────────────────────────────────────────────────────────
# GoogleTest Configurations (Host-Only Baseline Validation)
# ──────────────────────────────────────────────────────────────────────────────
GTEST_INC  = $(HOME)/gtest-install/include
GTEST_LIB  = $(HOME)/gtest-install/lib

# ──────────────────────────────────────────────────────────────────────────────
# Compilation Directories & Flags
# ──────────────────────────────────────────────────────────────────────────────
COMMON_INC = -I./include
BUILD_DIR  = build

HOST_FLAGS = -std=c++17 -O2 -Wall $(COMMON_INC) -I$(GTEST_INC)

# RV64GCV mandates the application of the Vector Extension (v1.0)
RV_FLAGS_BASE = -std=c++17 -Wall $(COMMON_INC) -march=rv64gcv -mabi=lp64d

RV_FLAGS_O0   = $(RV_FLAGS_BASE) -O0
RV_FLAGS_O2   = $(RV_FLAGS_BASE) -O2
RV_FLAGS_O3   = $(RV_FLAGS_BASE) -O3

# ──────────────────────────────────────────────────────────────────────────────
# Source File Discovery
# ──────────────────────────────────────────────────────────────────────────────
HOST_SRCS = src/image.cpp tests/test_pipeline.cpp
RV_SRCS   = src/image.cpp src/canny_rvv.cpp src/main.cpp

# ──────────────────────────────────────────────────────────────────────────────
# Output Binaries
# ──────────────────────────────────────────────────────────────────────────────
TEST_BIN   = $(BUILD_DIR)/test_pipeline

RV_BIN_O0  = $(BUILD_DIR)/canny_rv_o0
RV_BIN_O2  = $(BUILD_DIR)/canny_rv_o2
RV_BIN_O3  = $(BUILD_DIR)/canny_rv_o3
RV_BIN_RVV = ./canny_rv

# ──────────────────────────────────────────────────────────────────────────────
# Test Images & Spatial Constraints
# ──────────────────────────────────────────────────────────────────────────────
TEST_IMG   = $(PWD)/$(BUILD_DIR)/test_256x256.raw
IMG_W      = 256
IMG_H      = 256

# ==============================================================================
# Primary Project Targets
# ==============================================================================
.PHONY: all test sweep build_rv run vlen_sweep size clean help

all: test sweep build_rv

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# ──────────────────────────────────────────────────────────────────────────────
# Phase 3: Host Testing (GoogleTest)
# ──────────────────────────────────────────────────────────────────────────────
test: $(TEST_BIN)
	@echo "=== Running Host-Side GoogleTest Verification ==="
	./$(TEST_BIN)

$(TEST_BIN): $(HOST_SRCS) | $(BUILD_DIR)
	$(HOST_CXX) $(HOST_FLAGS) $^ \
		-L$(GTEST_LIB) -lgtest -lgtest_main -lpthread \
		-o $@

# ──────────────────────────────────────────────────────────────────────────────
# Phase 4: Compiler Optimization Sweep
# ──────────────────────────────────────────────────────────────────────────────
sweep: $(RV_BIN_O0) $(RV_BIN_O2) $(RV_BIN_O3)

$(RV_BIN_O0): $(RV_SRCS) | $(BUILD_DIR)
	$(RV_CXX) $(RV_FLAGS_O0) $^ -o $@

$(RV_BIN_O2): $(RV_SRCS) | $(BUILD_DIR)
	$(RV_CXX) $(RV_FLAGS_O2) $^ -o $@

$(RV_BIN_O3): $(RV_SRCS) | $(BUILD_DIR)
	$(RV_CXX) $(RV_FLAGS_O3) $^ -o $@

sweep_run: sweep
	@echo "=== Executing Optimization Sweep Performance Profile (VLEN=128) ==="
	@echo "--- Optimization Level: O0 ---"
	$(QEMU) $(RV_BIN_O0) $(TEST_IMG) $(IMG_W) $(IMG_H)
	@echo "--- Optimization Level: O2 ---"
	$(QEMU) $(RV_BIN_O2) $(TEST_IMG) $(IMG_W) $(IMG_H)
	@echo "--- Optimization Level: O3 ---"
	$(QEMU) $(RV_BIN_O3) $(TEST_IMG) $(IMG_W) $(IMG_H)

size: sweep
	@echo "=== Binary Size Footprint Analysis ==="
	@ls -lh $(RV_BIN_O0) $(RV_BIN_O2) $(RV_BIN_O3)
	@if [ -f $(RV_BIN_RVV) ]; then ls -lh $(RV_BIN_RVV); fi

# ──────────────────────────────────────────────────────────────────────────────
# Phase 1.7 & Phase 6: Core RISC-V / RVV Targets
# ──────────────────────────────────────────────────────────────────────────────
build_rv: $(RV_BIN_RVV)

$(RV_BIN_RVV): $(RV_SRCS) | $(BUILD_DIR)
	@echo "=== Cross-Compiling Phase 6 RVV Optimized Pipeline ==="
	$(RV_CXX) $(RV_FLAGS_O3) $^ -o $@

run: $(RV_BIN_RVV)
	@echo "=== Simulating Production RVV Execution (VLEN=128 Baseline) ==="
	$(QEMU) $(RV_BIN_RVV) $(TEST_IMG) $(IMG_W) $(IMG_H)

vlen_sweep: $(RV_BIN_RVV)
	@echo "=== Executing Hardware VLEN Agnosticism Sweep ==="
	@echo "--- Simulating VLEN = 128 Bits ---"
	$(QEMU_BIN) -cpu rv64,v=true,vlen=128 -L $(QEMU_L) $(RV_BIN_RVV) $(TEST_IMG) $(IMG_W) $(IMG_H)
	@echo "--- Simulating VLEN = 256 Bits ---"
	$(QEMU_BIN) -cpu rv64,v=true,vlen=256 -L $(QEMU_L) $(RV_BIN_RVV) $(TEST_IMG) $(IMG_W) $(IMG_H)
	@echo "--- Simulating VLEN = 512 Bits ---"
	$(QEMU_BIN) -cpu rv64,v=true,vlen=512 -L $(QEMU_L) $(RV_BIN_RVV) $(TEST_IMG) $(IMG_W) $(IMG_H)

# ──────────────────────────────────────────────────────────────────────────────
# Clean & Help Utility Rules
# ──────────────────────────────────────────────────────────────────────────────
clean:
	@echo "Cleaning compiled workspace..."
	rm -rf $(BUILD_DIR) *.o $(RV_BIN_RVV)

help:
	@echo "========================================================================="
	@echo " Canny Edge Detection RISC-V Build System Menu"
	@echo "========================================================================="
	@echo "  make test        -> Compile and execute host-side GoogleTest suite"
	@echo "  make sweep       -> Build compiler optimization binaries (O0/O2/O3)"
	@echo "  make sweep_run   -> Run compiler optimization variants on QEMU"
	@echo "  make build_rv    -> Cross-compile the production RVV intrinsic binary"
	@echo "  make run         -> Run production RVV binary on QEMU at standard 128-bit VLEN"
	@echo "  make vlen_sweep  -> Execute verification validation across 128/256/512-bit VLEN configurations"
	@echo "  make size        -> Evaluate file capacity differences across binaries"
	@echo "  make clean       -> Wipe build directories and artifact files"