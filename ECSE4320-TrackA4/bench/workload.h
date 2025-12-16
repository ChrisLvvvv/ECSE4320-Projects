#pragma once
#include <cstdint>
#include <vector>

namespace a4 {

// Simple xorshift RNG (fast, deterministic).
struct XorShift64 {
  std::uint64_t s;
  explicit XorShift64(std::uint64_t seed) : s(seed ? seed : 0xdeadbeefULL) {}
  std::uint64_t next() {
    std::uint64_t x = s;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    s = x;
    return x;
  }
};

// Generate keys in [0, key_space).
inline std::vector<std::uint64_t> gen_keys(std::size_t n, std::uint64_t seed, std::uint64_t key_space) {
  std::vector<std::uint64_t> keys;
  keys.reserve(n);
  XorShift64 rng(seed);
  for (std::size_t i = 0; i < n; ++i) {
    keys.push_back(rng.next() % key_space);
  }
  return keys;
}

} // namespace a4
