#include "mem/mlp_chase.h"
#include "bench/timer.h"
#include <vector>
#include <cstdint>
#include <algorithm>

namespace mem {

MlpResult run_mlp_chase(const MlpConfig& cfg) {
  const std::size_t step = cfg.stride;
  const std::size_t N = (cfg.bytes / step);
  const int K = std::max(1, cfg.chains);

  // Build K disjoint pointer-chase rings in one array of indices.
  std::vector<std::uint32_t> next(N);
  for (std::size_t i = 0; i < N; ++i) next[i] = static_cast<std::uint32_t>(i);

  // Partition indices into K groups by i % K.
  for (int k = 0; k < K; ++k) {
    std::size_t first = static_cast<std::size_t>(k);
    std::size_t prev = first;
    for (std::size_t i = first + K; i < N; i += static_cast<std::size_t>(K)) {
      next[prev] = static_cast<std::uint32_t>(i);
      prev = i;
    }
    next[prev] = static_cast<std::uint32_t>(first);
  }

  // Start pointers at each chain head.
  std::vector<std::uint32_t> p(K);
  for (int k = 0; k < K; ++k) p[k] = static_cast<std::uint32_t>(k);

  const std::uint64_t t0 = bench::Timer::now_ns();
  for (std::uint64_t it = 0; it < cfg.iters; ++it) {
    // Round-robin through chains: increases outstanding misses with K.
    for (int k = 0; k < K; ++k) {
      p[k] = next[p[k]];
    }
  }
  const std::uint64_t t1 = bench::Timer::now_ns();

  // Prevent optimization.
  std::uint64_t sink = 0;
  for (int k = 0; k < K; ++k) sink += p[k];
  asm volatile("" : : "r"(sink) : "memory");

  const double ns = static_cast<double>(t1 - t0);
  const double total_steps = static_cast<double>(cfg.iters) * static_cast<double>(K);

  // Each step touches one "stride-sized" location; treat as bytes touched per step.
  const double total_bytes = total_steps * static_cast<double>(step);

  MlpResult r;
  r.elapsed_ms = ns / 1e6;
  r.ns_per_step = ns / total_steps;
  r.bandwidth_GBps = (total_bytes / (ns * 1e-9)) / 1e9;
  r.bytes = cfg.bytes;
  r.stride = cfg.stride;
  r.iters = cfg.iters;
  r.chains = K;
  return r;
}

} // namespace mem
