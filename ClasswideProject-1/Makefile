CXX := g++
CXXFLAGS := -O3 -std=c++20 -Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion

BIN_DIR := build
SRC_DIR := src

TARGETS := bench_scalar bench_auto bench_native

all: $(TARGETS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

bench_scalar: $(SRC_DIR)/bench.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -fno-tree-vectorize $< -o $(BIN_DIR)/$@

bench_auto: $(SRC_DIR)/bench.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $(BIN_DIR)/$@

bench_native: $(SRC_DIR)/bench.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -march=native $< -o $(BIN_DIR)/$@

clean:
	rm -rf $(BIN_DIR)

# --- Vectorization evidence (GCC reports + disassembly snippets) ---
EVID_DIR := docs
$(EVID_DIR):
	mkdir -p $(EVID_DIR)

vec_report: | $(EVID_DIR)
	$(CXX) $(CXXFLAGS) -march=native -fopt-info-vec-optimized -fopt-info-vec-missed -c src/bench.cpp -o /tmp/bench_vec.o 2> $(EVID_DIR)/vectorization_report.full.txt
	grep -E "vectorized|not vectorized|missed|optimized|loop" $(EVID_DIR)/vectorization_report.full.txt > $(EVID_DIR)/vectorization_report.txt || true

disasm: all | $(EVID_DIR)
	objdump -d -Mintel $(BIN_DIR)/bench_native > $(EVID_DIR)/disasm_native.full.txt
	grep -nE "\b(vfmadd|vfma|vmul|vadd|vsub|vxor|vbroadcast|vmovups|vmovaps)\b" $(EVID_DIR)/disasm_native.full.txt | head -n 120 > $(EVID_DIR)/disasm_native_snippet.txt || true

evidence: vec_report disasm

# --- Bandwidth microbenchmark for roofline ---
bw_scalar: src/bw.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -fno-tree-vectorize $< -o $(BIN_DIR)/$@

bw_native: src/bw.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -march=native $< -o $(BIN_DIR)/$@
