#include "kernels.hpp"
#include "kernels_ref.hpp"
#include "util.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

enum class Kernel : int { AXPY=0, DOT=1, STENCIL=2 };

static Kernel parse_kernel(const std::string& s) {
  if (s == "axpy") return Kernel::AXPY;
  if (s == "dot") return Kernel::DOT;
  if (s == "stencil") return Kernel::STENCIL;
  std::cerr << "Unknown kernel: " << s << "\n";
  std::exit(1);
}

template <typename T>
static std::size_t flops_per_elem(Kernel k) {
  switch (k) {
    case Kernel::AXPY:    return 2;
    case Kernel::DOT:     return 2;
    case Kernel::STENCIL: return 5;
  }
  return 0;
}

template <typename T>
static double bytes_per_elem_streaming(Kernel k) {
  const double b = static_cast<double>(sizeof(T));
  if (k == Kernel::AXPY)    return 3.0 * b;
  if (k == Kernel::DOT)     return 2.0 * b;
  if (k == Kernel::STENCIL) return 2.0 * b;
  return 0.0;
}

template <typename T>
static void correctness_check(Kernel k, const T* x, const T* y, std::size_t n, std::size_t stride) {
  const T a = static_cast<T>(0.7);
  const T b = static_cast<T>(1.3);
  const T c = static_cast<T>(-0.2);
  const double tol = std::is_same_v<T,float> ? 1e-5 : 1e-12;

  if (k == Kernel::AXPY) {
    std::vector<T> ref(n), got(n);
    for (std::size_t i = 0; i < n; ++i) { ref[i] = y[i]; got[i] = y[i]; }
    refk::axpy(a, x, ref.data(), n, stride);
    kernels::axpy(a, x, got.data(), n, stride);
    double worst = 0.0;
    for (std::size_t i = 0; i < n; ++i) worst = std::max(worst, refk::rel_err(got[i], ref[i]));
    if (worst > tol) { std::cerr << "Correctness failed (axpy)\n"; std::exit(2); }
  } else if (k == Kernel::DOT) {
    const T s_ref = refk::dot(x, y, n, stride);
    const T s_got = kernels::dot(x, y, n, stride);
    if (refk::rel_err(s_got, s_ref) > tol) { std::cerr << "Correctness failed (dot)\n"; std::exit(2); }
  } else {
    std::vector<T> ref(n), got(n);
    refk::stencil3(a,b,c,x,ref.data(),n,stride);
    kernels::stencil3(a,b,c,x,got.data(),n,stride);
    double worst = 0.0;
    for (std::size_t i = 0; i < n; ++i) worst = std::max(worst, refk::rel_err(got[i], ref[i]));
    if (worst > tol) { std::cerr << "Correctness failed (stencil)\n"; std::exit(2); }
  }
}

template <typename T>
static void run_one(const std::string& build, Kernel k, std::size_t n, std::size_t stride,
                    std::size_t misalign_bytes, bool tail, int reps, const std::string& out_path) {
  const std::size_t alignment = 64;
  const std::size_t n_eff = tail ? (n + 1) : n;
  const std::size_t alloc_n = n_eff * stride + 16;

  auto ax = util::AlignedArray<T>::make(alloc_n, alignment, misalign_bytes);
  auto ay = util::AlignedArray<T>::make(alloc_n, alignment, misalign_bytes);
  auto az = util::AlignedArray<T>::make(alloc_n, alignment, misalign_bytes);

  T* x = ax.ptr;
  T* y = ay.ptr;
  T* z = az.ptr;

  util::fill_random(x, alloc_n, 123);
  util::fill_random(y, alloc_n, 456);
  util::fill_random(z, alloc_n, 789);

  util::touch_pages(x, alloc_n * sizeof(T));
  util::touch_pages(y, alloc_n * sizeof(T));
  util::touch_pages(z, alloc_n * sizeof(T));

  correctness_check<T>(k, x, y, n_eff, stride);

  const T a = static_cast<T>(0.7);
  const T b = static_cast<T>(1.3);
  const T c = static_cast<T>(-0.2);

  // warmup
  if (k == Kernel::AXPY) kernels::axpy(a, x, y, n_eff, stride);
  if (k == Kernel::DOT) (void)kernels::dot(x, y, n_eff, stride);
  if (k == Kernel::STENCIL) kernels::stencil3(a,b,c,x,z,n_eff,stride);

  std::vector<std::uint64_t> samples;
  samples.reserve(static_cast<std::size_t>(reps));
  for (int r = 0; r < reps; ++r) {
    const auto t0 = util::now_ns();
    if (k == Kernel::AXPY) kernels::axpy(a, x, y, n_eff, stride);
    if (k == Kernel::DOT) (void)kernels::dot(x, y, n_eff, stride);
    if (k == Kernel::STENCIL) kernels::stencil3(a,b,c,x,z,n_eff,stride);
    const auto t1 = util::now_ns();
    samples.push_back(t1 - t0);
  }

  const auto st = util::summarize_ns(samples);
  const double flops = static_cast<double>(flops_per_elem<T>(k)) * static_cast<double>(n_eff);
  const double sec = st.median_ns * 1e-9;
  const double gflops = (sec > 0.0) ? (flops / sec / 1e9) : 0.0;
  const double ai = static_cast<double>(flops_per_elem<T>(k)) / bytes_per_elem_streaming<T>(k);

  std::ofstream out(out_path, std::ios::app);
  out << std::fixed << std::setprecision(6);
  out << "{"
      << "\"build\":\"" << build << "\","
      << "\"dtype\":\"" << (std::is_same_v<T,float> ? "f32" : "f64") << "\","
      << "\"kernel\":\"" << (k==Kernel::AXPY ? "axpy" : (k==Kernel::DOT ? "dot" : "stencil")) << "\","
      << "\"n\":" << n_eff << ","
      << "\"stride\":" << stride << ","
      << "\"misalign\":" << misalign_bytes << ","
      << "\"tail\":" << (tail ? 1 : 0) << ","
      << "\"reps\":" << reps << ","
      << "\"t_median_ns\":" << st.median_ns << ","
      << "\"t_p10_ns\":" << st.p10_ns << ","
      << "\"t_p90_ns\":" << st.p90_ns << ","
      << "\"gflops\":" << gflops << ","
      << "\"ai\":" << ai
      << "}\n";

  ax.destroy(); ay.destroy(); az.destroy();
}

static void usage(const char* argv0) {
  std::cerr
    << "Usage: " << argv0
    << " --kernel axpy|dot|stencil --dtype f32|f64 --n N --stride S "
    << "--misalign BYTES --tail 0|1 --reps R --out results/raw/out.jsonl\n";
}

int main(int argc, char** argv) {
  util::pin_to_cpu0();

  std::string kernel_s = "axpy";
  std::string dtype_s = "f32";
  std::size_t n = 1 << 20;
  std::size_t stride = 1;
  std::size_t misalign = 0;
  bool tail = false;
  int reps = 7;
  std::string out = "results/raw/out.jsonl";

  for (int i = 1; i < argc; ++i) {
    const std::string a(argv[i]);
    if (a == "--kernel" && i + 1 < argc) kernel_s = argv[++i];
    else if (a == "--dtype" && i + 1 < argc) dtype_s = argv[++i];
    else if (a == "--n" && i + 1 < argc) n = static_cast<std::size_t>(std::stoull(argv[++i]));
    else if (a == "--stride" && i + 1 < argc) stride = static_cast<std::size_t>(std::stoull(argv[++i]));
    else if (a == "--misalign" && i + 1 < argc) misalign = static_cast<std::size_t>(std::stoull(argv[++i]));
    else if (a == "--tail" && i + 1 < argc) tail = (std::stoi(argv[++i]) != 0);
    else if (a == "--reps" && i + 1 < argc) reps = std::stoi(argv[++i]);
    else if (a == "--out" && i + 1 < argc) out = argv[++i];
    else { usage(argv[0]); return 1; }
  }

  const std::string build = util::build_tag_from_argv0(argv[0]);
  const Kernel k = parse_kernel(kernel_s);

  if (dtype_s == "f32") run_one<float>(build, k, n, stride, misalign, tail, reps, out);
  else if (dtype_s == "f64") run_one<double>(build, k, n, stride, misalign, tail, reps, out);
  else { std::cerr << "Unknown dtype\n"; return 1; }

  (void)argc; (void)argv;
  return 0;
}
