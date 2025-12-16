#include <cstdint>

namespace a4 {

// SplitMix64: fast, decent diffusion for integer keys.
static inline std::uint64_t splitmix64(std::uint64_t x) {
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}

std::uint64_t hash_key(std::uint64_t k) {
  return splitmix64(k);
}

} // namespace a4
