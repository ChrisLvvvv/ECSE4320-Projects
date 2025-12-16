#include "mem/stride_stream.h"
#include "bench/timer.h"
#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

namespace mem {

static inline std::uint64_t lcg_step(std::uint64_t x) {
  // Deterministic LCG for reproducible read/write decisions.
  return x * 2862933555777941757ULL + 3037000493ULL;
}

StrideResult run_stride_stream(const StrideConfig& cfg) {
  std::vector<std::uint8_t> buf(cfg.bytes, 1);

  const std::size_t step = cfg.stride;
  const std::size_t N = (step == 0) ? 0 : (cfg.bytes / step);
  if (N == 0) {
    StrideResult r;
    r.bytes = cfg.bytes;
    r.stride = cfg.stride;
    r.write = cfg.write;
    return r;
  }

  std::vector<std::size_t> idx(N);
  for (std::size_t i = 0; i < N; ++i) idx[i] = i;

  if (cfg.random) {
    std::mt19937_64 rng(12345);
    std::shuffle(idx.begin(), idx.end(), rng);
  }

  int rw = cfg.rw_pct;
  if (rw < 0) rw = 0;
  if (rw > 100) rw = 100;

  std::uint64_t sink = 0;
  std::uint64_t prng = 0x123456789abcdefULL;

  const std::uint64_t t0 = bench::Timer::now_ns();
  for (std::uint64_t it = 0; it < cfg.iters; ++it) {
    for (std::size_t j = 0; j < N; ++j) {
      const std::size_t off = idx[j] * step;

      prng = lcg_step(prng);
      const int p = static_cast<int>(prng % 100ULL);
      const bool do_read = (p < rw);

      if (do_read) {
        sink += buf[off];
      } else {
        buf[off] = static_cast<std::uint8_t>(buf[off] + 1);
      }
    }
  }
  const std::uint64_t t1 = bench::Timer::now_ns();

  asm volatile("" : : "r"(sink) : "memory");

  const double ns = static_cast<double>(t1 - t0);

  const double accesses = static_cast<double>(cfg.iters) * static_cast<double>(N);
  const double bytes_ws = static_cast<double>(cfg.iters) * static_cast<double>(cfg.bytes);  // legacy
  const double bytes_touch = accesses * 64.0;                                                // 64B per access

  StrideResult r;
  r.bytes = cfg.bytes;
  r.stride = cfg.stride;
  r.write = (rw == 0); // "true" only when all writes; legacy-ish

  r.elapsed_ms = ns / 1e6;
  r.ns_per_access = ns / accesses;

  r.touches = static_cast<std::uint64_t>(accesses);
  r.bandwidth_GBps = (bytes_ws / (ns * 1e-9)) / 1e9;
  r.bandwidth_touch_GBps = (bytes_touch / (ns * 1e-9)) / 1e9;
  return r;
}

} // namespace mem
