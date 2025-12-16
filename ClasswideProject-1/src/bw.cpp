#include "util.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

static void usage(const char* argv0) {
  std::cerr << "Usage: " << argv0
            << " --dtype f32|f64 --n N --reps R --out results/raw/bw.jsonl\n";
}

template <typename T>
static void run_bw(const std::string& build, std::size_t n, int reps, const std::string& out_path) {
  const std::size_t alignment = 64;
  auto a = util::AlignedArray<T>::make(n, alignment, 0);
  auto b = util::AlignedArray<T>::make(n, alignment, 0);

  util::fill_random(a.ptr, n, 111);
  util::fill_random(b.ptr, n, 222);
  util::touch_pages(a.ptr, n * sizeof(T));
  util::touch_pages(b.ptr, n * sizeof(T));

  // warmup
  for (std::size_t i = 0; i < n; ++i) b.ptr[i] = a.ptr[i];

  std::vector<std::uint64_t> samples;
  samples.reserve(static_cast<std::size_t>(reps));
  for (int r = 0; r < reps; ++r) {
    const auto t0 = util::now_ns();
    for (std::size_t i = 0; i < n; ++i) b.ptr[i] = a.ptr[i];
    const auto t1 = util::now_ns();
    samples.push_back(t1 - t0);
  }

  const auto st = util::summarize_ns(samples);
  const double sec = st.median_ns * 1e-9;

  // copy: read + write
  const double bytes = 2.0 * static_cast<double>(n) * static_cast<double>(sizeof(T));
  const double gib_s = (sec > 0.0) ? (bytes / sec / (1024.0 * 1024.0 * 1024.0)) : 0.0;

  std::ofstream out(out_path, std::ios::app);
  out << std::fixed << std::setprecision(6);
  out << "{"
      << "\"build\":\"" << build << "\","
      << "\"dtype\":\"" << (std::is_same_v<T,float> ? "f32" : "f64") << "\","
      << "\"kernel\":\"copy\","
      << "\"n\":" << n << ","
      << "\"reps\":" << reps << ","
      << "\"t_median_ns\":" << st.median_ns << ","
      << "\"t_p10_ns\":" << st.p10_ns << ","
      << "\"t_p90_ns\":" << st.p90_ns << ","
      << "\"gib_s\":" << gib_s
      << "}\n";

  a.destroy();
  b.destroy();
}

int main(int argc, char** argv) {
  util::pin_to_cpu0();

  std::string dtype_s = "f32";
  std::size_t n = 1 << 24;
  int reps = 7;
  std::string out = "results/raw/bw.jsonl";

  for (int i = 1; i < argc; ++i) {
    const std::string a(argv[i]);
    if (a == "--dtype" && i + 1 < argc) dtype_s = argv[++i];
    else if (a == "--n" && i + 1 < argc) n = static_cast<std::size_t>(std::stoull(argv[++i]));
    else if (a == "--reps" && i + 1 < argc) reps = std::stoi(argv[++i]);
    else if (a == "--out" && i + 1 < argc) out = argv[++i];
    else { usage(argv[0]); return 1; }
  }

  const std::string build = util::build_tag_from_argv0(argv[0]);

  if (dtype_s == "f32") run_bw<float>(build, n, reps, out);
  else if (dtype_s == "f64") run_bw<double>(build, n, reps, out);
  else { std::cerr << "Unknown dtype\n"; return 1; }

  return 0;
}
