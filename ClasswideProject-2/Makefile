CXX := g++
CXXFLAGS := -O3 -march=native -std=c++20 -Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion -Iinclude -MMD -MP
LDFLAGS :=
LDLIBS :=

BUILD_DIR := build
BIN_DIR := bin
TARGET := $(BIN_DIR)/p2_bench

SRCS := \
  src/main.cpp \
  src/bench/timer.cpp \
  src/mem/pointer_chase.cpp \
  src/mem/stride_stream.cpp \
  src/mem/mlp_chase.cpp

OBJS := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all build clean run-smoke help

all: build

build: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run-smoke: build
	@mkdir -p results/raw
	@taskset -c 0 $(TARGET) --mode smoke --reps 5 --out results/raw/smoke.jsonl
	@tail -n 3 results/raw/smoke.jsonl || true

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

help:
	@echo "Targets: build | run-smoke | clean | help"

-include $(DEPS)

.PHONY: run-cache-miss run-tlb-miss run-mlp-sweep

run-cache-miss: build
	./scripts/run/run_cache_miss_sweep.sh

run-tlb-miss: build
	./scripts/run/run_tlb_miss_sweep.sh

run-mlp-sweep: build
	./scripts/run/run_mlp_intensity_sweep.sh

.PHONY: run-all
run-all: build
	./scripts/run/run_all.sh
