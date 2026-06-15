CXX = g++
CXXFLAGS = -O3 -Iinclude

SRCS = src/main.cpp src/gradient.cpp src/image_io.cpp
TEST_SRCS = tests/test_pipeline.cpp src/gradient.cpp src/image_io.cpp

# Target to compile your main application code
all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o app

# Target to compile and run your verification tests automatically!
test:
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) -o run_tests
	./run_tests

# Clean up binaries
clean:
	rm -f app run_tests
