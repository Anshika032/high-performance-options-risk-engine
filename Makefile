CXX := g++
CXXFLAGS := -std=c++20 -O3 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Iinclude
LDFLAGS := -pthread

BUILD := build
TARGET := $(BUILD)/options_engine
TEST := $(BUILD)/test_pricing
BENCH := $(BUILD)/benchmark

.PHONY: all test benchmark debug clean

all: $(TARGET)

$(BUILD):
	mkdir -p $(BUILD)

$(TARGET): $(BUILD) src/main.cpp src/pricing_engine.cpp include/option.hpp include/pricing_engine.hpp
	$(CXX) $(CXXFLAGS) src/main.cpp src/pricing_engine.cpp -o $@ $(LDFLAGS)

$(TEST): $(BUILD) tests/test_pricing.cpp src/pricing_engine.cpp include/option.hpp include/pricing_engine.hpp
	$(CXX) $(CXXFLAGS) tests/test_pricing.cpp src/pricing_engine.cpp -o $@ $(LDFLAGS)

$(BENCH): $(BUILD) benchmarks/benchmark.cpp src/pricing_engine.cpp include/option.hpp include/pricing_engine.hpp
	$(CXX) $(CXXFLAGS) benchmarks/benchmark.cpp src/pricing_engine.cpp -o $@ $(LDFLAGS)

test: $(TEST)
	./$(TEST)

benchmark: $(BENCH)
	./$(BENCH)

debug: CXXFLAGS := -std=c++20 -O0 -g3 -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -Wpedantic -Iinclude
debug: $(BUILD) src/main.cpp src/pricing_engine.cpp include/option.hpp include/pricing_engine.hpp
	$(CXX) $(CXXFLAGS) src/main.cpp src/pricing_engine.cpp -o $(BUILD)/options_engine_debug $(LDFLAGS)

clean:
	rm -rf $(BUILD)
