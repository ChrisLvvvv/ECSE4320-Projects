#pragma once
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <random>
#include <string>
#include <vector>

#if defined(__linux__)
  #include <sched.h>
#endif

namespace util {

inline void pin_to_cpu0() {
#if defined(__linux__)
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(0, &set);
  (void)sched_setaffinity(0, sizeof(set), &set);
#endif
}

inline std::uint64_t now_ns() {
  using clock = std::chrono::steady_clock;
  return static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now().time_since_epoch()).count());
}

inline void touch_pages(void* p, std::size_t bytes) {
  auto* b = static_cast<std::uint8_t*>(p);
  constexpr std::size_t step = 4096;
  for (std::size_t i = 0; i < bytes; i += step) b[i] = static_cast<std::uint8_t>(b[i] + 1);
}

inline void* aligned_alloc_bytes(std::size_t alignment, std::size_t bytes) {
  void* p = nullptr;
  if (posix_memalign(&p, alignment, bytes) != 0) return nullptr;
  std::memset(p, 0, bytes);
  return p;
}

inline void aligned_free(void* p) { std::free(p); }

template <typename T>
struct AlignedArray {
  T* base{nullptr};
  T* ptr{nullptr};
  std::size_t n{0};

  static AlignedArray make(std::size_t n_elems, std::size_t alignment, std::size_t misalign_bytes) {
    AlignedArray a;
    a.n = n_elems;
    const std::size_t bytes = n_elems * sizeof(T) + alignment + misalign_bytes;
    a.base = static_cast<T*>(aligned_alloc_bytes(alignment, bytes));
    if (!a.base) throw std::bad_alloc{};
    auto* raw = reinterpret_cast<std::uint8_t*>(a.base);
    raw += misalign_bytes;
    std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(raw);
    const std::uintptr_t aligned = (addr + (alignment - 1)) & ~(alignment - 1);
    a.ptr = reinterpret_cast<T*>(aligned);
    return a;
  }

  void destroy() {
    aligned_free(base);
    base = nullptr;
    ptr = nullptr;
    n = 0;
  }
};

template <typename T>
inline void fill_random(T* x, std::size_t n, std::uint64_t seed) {
  std::mt19937_64 rng(seed);
  std::uniform_real_distribution<double> dist(0.1, 1.0);
  for (std::size_t i = 0; i < n; ++i) x[i] = static_cast<T>(dist(rng));
}

template <typename T>
inline T median(std::vector<T> v) {
  std::sort(v.begin(), v.end());
  return v[v.size() / 2];
}

template <typename T>
inline T percentile(std::vector<T> v, double p) {
  std::sort(v.begin(), v.end());
  const double idx = p * static_cast<double>(v.size() - 1);
  const auto lo = static_cast<std::size_t>(idx);
  const auto hi = std::min(lo + 1, v.size() - 1);
  const double frac = idx - static_cast<double>(lo);
  return static_cast<T>((1.0 - frac) * v[lo] + frac * v[hi]);
}

struct RunStats {
  double median_ns{0.0};
  double p10_ns{0.0};
  double p90_ns{0.0};
};

inline RunStats summarize_ns(const std::vector<std::uint64_t>& samples) {
  std::vector<double> v;
  v.reserve(samples.size());
  for (auto s : samples) v.push_back(static_cast<double>(s));
  RunStats r;
  r.median_ns = median(v);
  r.p10_ns = percentile(v, 0.10);
  r.p90_ns = percentile(v, 0.90);
  return r;
}

inline std::string build_tag_from_argv0(const std::string& argv0) {
  if (argv0.find("bench_scalar") != std::string::npos) return "scalar";
  if (argv0.find("bench_auto")   != std::string::npos) return "auto";
  if (argv0.find("bench_native") != std::string::npos) return "native";
  if (argv0.find("bw_scalar") != std::string::npos) return "scalar";
  if (argv0.find("bw_native") != std::string::npos) return "native";
  return "unknown";
}

} // namespace util
