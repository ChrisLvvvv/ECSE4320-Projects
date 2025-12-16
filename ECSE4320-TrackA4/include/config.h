#pragma once
#include <cstddef>

namespace a4 {

// Fixed-size value to keep comparisons fair across designs.
constexpr std::size_t kValueBytes = 8;

// Default table capacity (number of buckets). Power of two recommended.
constexpr std::size_t kDefaultBuckets = 1u << 20; // ~1M buckets

// Lock striping: number of stripes (used by striped version).
constexpr std::size_t kDefaultStripes = 256;

// Simple knobs for benchmark defaults.
constexpr int kDefaultThreads = 1;
constexpr std::size_t kDefaultOps = 1'000'000;
constexpr int kDefaultWorkload = 0; // 0=lookup, 1=insert, 2=mixed(70/30)

} // namespace a4
