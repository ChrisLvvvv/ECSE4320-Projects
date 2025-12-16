#pragma once
#include <cstdint>

namespace bench {
struct Timer {
  static std::uint64_t now_ns();
};
}  // namespace bench
