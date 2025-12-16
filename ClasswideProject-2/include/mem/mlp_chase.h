#pragma once
#include <cstddef>
#include <cstdint>

namespace mem {

struct MlpConfig {
  std::size_t bytes = 256ull << 20;   // big enough to hit DRAM
  std::size_t stride = 64;            // link spacing
  std::uint64_t iters = 200000000;    // total steps per chain
  int chains = 1;                     // intensity knob (MLP)
};

struct MlpResult {
  double elapsed_ms = 0.0;
  double ns_per_step = 0.0;
  double bandwidth_GBps = 0.0;
  std::size_t bytes = 0;
  std::size_t stride = 0;
  std::uint64_t iters = 0;
  int chains = 0;
};

MlpResult run_mlp_chase(const MlpConfig& cfg);

} // namespace mem
