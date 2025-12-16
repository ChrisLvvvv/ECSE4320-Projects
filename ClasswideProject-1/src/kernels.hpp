#pragma once
#include <cstddef>

namespace kernels {

template <typename T>
inline void axpy(T a, const T* x, T* y, std::size_t n, std::size_t stride) {
  const std::size_t s = stride;
  for (std::size_t i = 0; i < n; ++i) y[i] = a * x[i * s] + y[i];
}

template <typename T>
inline T dot(const T* x, const T* y, std::size_t n, std::size_t stride) {
  const std::size_t s = stride;
  T acc = static_cast<T>(0);
  for (std::size_t i = 0; i < n; ++i) acc += x[i * s] * y[i * s];
  return acc;
}

template <typename T>
inline void stencil3(T a, T b, T c, const T* x, T* y, std::size_t n, std::size_t stride) {
  const std::size_t s = stride;
  if (n < 3) return;
  y[0] = static_cast<T>(0);
  for (std::size_t i = 1; i + 1 < n; ++i) {
    const T xm = x[(i - 1) * s];
    const T x0 = x[i * s];
    const T xp = x[(i + 1) * s];
    y[i] = a * xm + b * x0 + c * xp;
  }
  y[n - 1] = static_cast<T>(0);
}

} // namespace kernels
