#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace refk {

template <typename T>
inline void axpy(T a, const T* x, T* y, std::size_t n, std::size_t stride) {
  for (std::size_t i = 0; i < n; ++i) y[i] = a * x[i * stride] + y[i];
}

template <typename T>
inline T dot(const T* x, const T* y, std::size_t n, std::size_t stride) {
  T s = static_cast<T>(0);
  for (std::size_t i = 0; i < n; ++i) s += x[i * stride] * y[i * stride];
  return s;
}

template <typename T>
inline void stencil3(T a, T b, T c, const T* x, T* y, std::size_t n, std::size_t stride) {
  if (n < 3) return;
  y[0] = static_cast<T>(0);
  for (std::size_t i = 1; i + 1 < n; ++i) {
    y[i] = a * x[(i - 1) * stride] + b * x[i * stride] + c * x[(i + 1) * stride];
  }
  y[n - 1] = static_cast<T>(0);
}

template <typename T>
inline double rel_err(T got, T ref) {
  const double g = static_cast<double>(got);
  const double r = static_cast<double>(ref);
  const double denom = std::max(1.0, std::fabs(r));
  return std::fabs(g - r) / denom;
}

} // namespace refk
