#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

namespace mem {

// Build a single-cycle pointer ring in a buffer, then chase pointers.
// Output is ns per load (dependent loads), approximating latency.
struct PointerChaseConfig {
  std::size_t bytes = 1 << 20;      // working set size
  std::size_t stride = 64;          // bytes between nodes
  std::uint64_t iters = 100000000;  // pointer dereferences
  std::uint64_t warmup = 1000000;   // warmup dereferences
  bool randomize = false;           // shuffle nodes to defeat prefetch
  std::uint64_t seed = 1;           // RNG seed for shuffle
};

struct PointerChaseResult {
  double ns_per_load = 0.0;
  std::uint64_t iters = 0;
  std::size_t bytes = 0;
  std::size_t stride = 0;
  bool randomize = false;
};

PointerChaseResult run_pointer_chase(const PointerChaseConfig& cfg);

}  // namespace mem
