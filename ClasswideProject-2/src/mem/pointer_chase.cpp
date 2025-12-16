#include "mem/pointer_chase.h"

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

#include "bench/timer.h"

namespace mem {

static std::size_t clamp_nodes(std::size_t bytes, std::size_t stride) {
  if (stride < sizeof(std::uintptr_t)) stride = sizeof(std::uintptr_t);
  if (bytes < stride * 2) bytes = stride * 2;
  return bytes / stride;
}

PointerChaseResult run_pointer_chase(const PointerChaseConfig& cfg_in) {
  PointerChaseConfig cfg = cfg_in;
  if (cfg.stride < sizeof(std::uintptr_t)) cfg.stride = sizeof(std::uintptr_t);
  const std::size_t nodes = clamp_nodes(cfg.bytes, cfg.stride);
  const std::size_t bytes = nodes * cfg.stride;

  // Allocate as raw bytes; each node stores a pointer to the next node.
  std::vector<std::uint8_t> buf(bytes, 0);

  // Create an index order [0..nodes-1], optionally shuffled.
  std::vector<std::size_t> order(nodes);
  for (std::size_t i = 0; i < nodes; ++i) order[i] = i;
  if (cfg.randomize) {
    std::mt19937_64 rng(cfg.seed);
    std::shuffle(order.begin(), order.end(), rng);
  }

  // Link nodes into a ring in the chosen order.
  for (std::size_t i = 0; i < nodes; ++i) {
    const std::size_t cur = order[i];
    const std::size_t nxt = order[(i + 1) % nodes];
    auto* cur_ptr = reinterpret_cast<std::uintptr_t*>(&buf[cur * cfg.stride]);
    *cur_ptr = reinterpret_cast<std::uintptr_t>(&buf[nxt * cfg.stride]);
  }

  // Start pointer at the first node in the order.
  volatile std::uintptr_t p =
      reinterpret_cast<std::uintptr_t>(&buf[order[0] * cfg.stride]);

  // Warmup (reduce cold-start effects).
  for (std::uint64_t i = 0; i < cfg.warmup; ++i) {
    p = *reinterpret_cast<std::uintptr_t*>(p);
  }

  const std::uint64_t t0 = bench::Timer::now_ns();
  for (std::uint64_t i = 0; i < cfg.iters; ++i) {
    p = *reinterpret_cast<std::uintptr_t*>(p);
  }
  const std::uint64_t t1 = bench::Timer::now_ns();

  const double ns = static_cast<double>(t1 - t0);
  PointerChaseResult res;
  res.ns_per_load = ns / static_cast<double>(cfg.iters);
  res.iters = cfg.iters;
  res.bytes = bytes;
  res.stride = cfg.stride;
  res.randomize = cfg.randomize;

  // Prevent optimizing away.
  if (p == 0xdeadbeefULL) std::abort();

  return res;
}

}  // namespace mem
