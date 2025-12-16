#pragma once
#include <chrono>

namespace a4 {

class Timer {
 public:
  using clock = std::chrono::steady_clock;

  void start() { t0_ = clock::now(); }
  double seconds() const {
    const auto t1 = clock::now();
    return std::chrono::duration<double>(t1 - t0_).count();
  }

 private:
  clock::time_point t0_;
};

} // namespace a4
