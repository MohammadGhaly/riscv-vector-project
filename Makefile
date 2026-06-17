# ─── Compilers ───────────────────────────────────────────────────────────────
HOST_CXX   = g++
RV_CXX     = /opt/riscv/bin/riscv64-unknown-elf-g++

# ─── GoogleTest (built locally) ──────────────────────────────────────────────
GTEST_INC  = $(HOME)/gtest-install/include
GTEST_LIB  = $(HOME)/gtest-install/lib

# ─── Flags ───────────────────────────────────────────────────────────────────
HOST_FLAGS = -std=c++17 -O2 -Wall -I./include -I$(GTEST_INC)
RV_FLAGS   = -std=c++17 -O2 -Wall -I./include -march=rv64gcv -mabi=lp64d

# ─── Sources ─────────────────────────────────────────────────────────────────
HOST_SRCS  = src/image.cpp tests/test_pipeline.cpp
RV_SRCS    = src/image.cpp src/main.cpp

# ─── Outputs ─────────────────────────────────────────────────────────────────
TEST_BIN   = build/test_pipeline
RV_BIN     = build/canny_rv

# ─── Default ─────────────────────────────────────────────────────────────────
all: test canny_rv

# ─── Host: GoogleTest suite ───────────────────────────────────────────────────
test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(HOST_SRCS) | build
	$(HOST_CXX) $(HOST_FLAGS) $^ \
		-L$(GTEST_LIB) -lgtest -lgtest_main -lpthread \
		-o $@

# ─── RISC-V: cross-compile pipeline ──────────────────────────────────────────
canny_rv: $(RV_BIN)

$(RV_BIN): $(RV_SRCS) | build
	$(RV_CXX) $(RV_FLAGS) $^ -o $@

# ─── Run on QEMU (default VLEN=128) ──────────────────────────────────────────
VLEN ?= 128
run: canny_rv
	qemu-riscv64 -cpu rv64,v=true,vlen=$(VLEN) ./$(RV_BIN)

# ─── Build directory ─────────────────────────────────────────────────────────
build:
	mkdir -p build

# ─── Clean ───────────────────────────────────────────────────────────────────
clean:
	rm -rf build *.raw
