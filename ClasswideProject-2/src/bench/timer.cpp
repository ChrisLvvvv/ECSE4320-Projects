#include "bench/timer.h"
#include <chrono>

namespace bench {
std::uint64_t Timer::now_ns() {
  using clock = std::chrono::steady_clock;
  return static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          clock::now().time_since_epoch()).count());
}
}  // namespace bench
