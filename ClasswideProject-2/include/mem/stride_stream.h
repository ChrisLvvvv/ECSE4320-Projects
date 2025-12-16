#pragma once

#include <cstddef>
#include <cstdint>

namespace mem {

struct StrideConfig {
  std::size_t bytes = 64ull << 20;
  std::size_t stride = 64;
  std::uint64_t iters = 20;
  bool write = false;     // legacy flag (kept for compatibility)
  bool random = false;
  int rw_pct = 100;       // percent reads (0..100)
};

struct StrideResult {
  std::size_t bytes = 0;
  std::size_t stride = 0;
  bool write = false;

  double elapsed_ms = 0.0;

  // Legacy: workspace-normalized bandwidth (iters * bytes).
  double bandwidth_GBps = 0.0;

  // New: touched-bytes bandwidth (assume 64B touched per access).
  double bandwidth_touch_GBps = 0.0;

  double ns_per_access = 0.0;

  // New: number of accesses performed (iters * (bytes/stride)).
  std::uint64_t touches = 0;
};

StrideResult run_stride_stream(const StrideConfig& cfg);

} // namespace mem
