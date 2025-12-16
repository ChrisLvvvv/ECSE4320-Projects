#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "bench/timer.h"

#include "mem/pointer_chase.h"
#include "mem/stride_stream.h"
#include "mem/mlp_chase.h"

static void die(const std::string& msg) {
  std::cerr << "error: " << msg << "\n";
  std::exit(1);
}

static std::string get_arg(int argc, char** argv, const std::string& key,
                           const std::string& def = "") {
  for (int i = 1; i + 1 < argc; ++i) {
    if (std::string(argv[i]) == key) return std::string(argv[i + 1]);
  }
  return def;
}

static bool has_flag(int argc, char** argv, const std::string& key) {
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == key) return true;
  }
  return false;
}

static std::uint64_t parse_u64(const std::string& s, const std::string& name) {
  char* end = nullptr;
  unsigned long long v = std::strtoull(s.c_str(), &end, 10);
  if (!end || *end != '\0') die("invalid " + name + ": " + s);
  return static_cast<std::uint64_t>(v);
}

static std::size_t parse_size(const std::string& s, const std::string& name) {
  char* end = nullptr;
  unsigned long long v = std::strtoull(s.c_str(), &end, 10);
  if (!end || *end != '\0') die("invalid " + name + ": " + s);
  return static_cast<std::size_t>(v);
}

static void run_smoke(std::ofstream& out, std::uint64_t reps) {
  volatile std::uint64_t x = 1;
  for (std::uint64_t r = 0; r < reps; ++r) {
    const std::uint64_t t0 = bench::Timer::now_ns();
    for (std::uint64_t i = 0; i < 50'000'000ULL; ++i) {
      x = x * 1664525ULL + 1013904223ULL;
    }
    const std::uint64_t t1 = bench::Timer::now_ns();
    const double ms = static_cast<double>(t1 - t0) / 1e6;

    out << "{"
        << "\"bench\":\"smoke\","
        << "\"rep\":" << r << ","
        << "\"elapsed_ms\":" << ms << ","
        << "\"sink\":" << static_cast<std::uint64_t>(x)
        << "}\n";
  }
}

static void run_pchase(std::ofstream& out, int argc, char** argv) {
  mem::PointerChaseConfig cfg;
  cfg.bytes = parse_size(get_arg(argc, argv, "--bytes", "1048576"), "bytes");
  cfg.stride = parse_size(get_arg(argc, argv, "--stride", "64"), "stride");
  cfg.iters = parse_u64(get_arg(argc, argv, "--iters", "200000000"), "iters");
  cfg.warmup = parse_u64(get_arg(argc, argv, "--warmup", "2000000"), "warmup");
  cfg.randomize = has_flag(argc, argv, "--random");
  cfg.seed = parse_u64(get_arg(argc, argv, "--seed", "1"), "seed");

  const auto res = mem::run_pointer_chase(cfg);

  out << "{" 
      << "\"bench\":\"pointer_chase\"," 
      << "\"bytes\":" << res.bytes << "," 
      << "\"stride\":" << res.stride << "," 
      << "\"iters\":" << res.iters << "," 
      << "\"random\":" << (res.randomize ? "true" : "false") << "," 
      << "\"ns_per_load\":" << res.ns_per_load 
      << "}\n";
}




int main(int argc, char** argv) {
  const std::string mode = get_arg(argc, argv, "--mode", "smoke");
  const std::string out_path = get_arg(argc, argv, "--out", "");
  const std::uint64_t reps = parse_u64(get_arg(argc, argv, "--reps", "3"), "reps");

  if (out_path.empty()) die("--out is required");

  std::ofstream out(out_path, std::ios::out | std::ios::trunc);
  if (!out) die("cannot open output: " + out_path);

  if (mode == "smoke") {
    run_smoke(out, reps);

  } else if (mode == "pchase") {
    // pchase controls its own repetitions.
    (void)reps;
    run_pchase(out, argc, argv);

  } else if (mode == "stride") {
    mem::StrideConfig cfg;
    cfg.bytes  = parse_size(get_arg(argc, argv, "--bytes", "67108864"), "bytes");
    cfg.stride = parse_size(get_arg(argc, argv, "--stride", "64"), "stride");
    cfg.iters  = parse_u64(get_arg(argc, argv, "--iters", "20"), "iters");
    cfg.write  = has_flag(argc, argv, "--write");    // legacy
    cfg.random = has_flag(argc, argv, "--random");
    cfg.rw_pct = static_cast<int>(parse_u64(get_arg(argc, argv, "--rw_pct", "100"), "rw_pct"));

    const auto r = mem::run_stride_stream(cfg);
    out << "{"
        << "\"bench\":\"stride\","
        << "\"bytes\":" << r.bytes << ","
        << "\"stride\":" << r.stride << ","
        << "\"write\":" << (r.write ? "true" : "false") << ","
        << "\"random\":" << (cfg.random ? "true" : "false") << ","
        << "\"rw_pct\":" << cfg.rw_pct << ","
        << "\"elapsed_ms\":" << r.elapsed_ms << ","
        << "\"bandwidth_GBps\":" << r.bandwidth_GBps << ","
        << "\"touches\":" << r.touches << ","
        << "\"bandwidth_touch_GBps\":" << r.bandwidth_touch_GBps << ","
        << "\"ns_per_access\":" << r.ns_per_access
        << "}\n";

  } else if (mode == "mlp") {
    mem::MlpConfig cfg;
    cfg.bytes  = parse_size(get_arg(argc, argv, "--bytes", "268435456"), "bytes");
    cfg.stride = parse_size(get_arg(argc, argv, "--stride", "64"), "stride");
    cfg.iters  = parse_u64(get_arg(argc, argv, "--iters", "20000000"), "iters");
    cfg.chains = static_cast<int>(parse_u64(get_arg(argc, argv, "--chains", "1"), "chains"));

    const auto r = mem::run_mlp_chase(cfg);
    out << "{"
        << "\"bench\":\"mlp\","
        << "\"bytes\":" << r.bytes << ","
        << "\"stride\":" << r.stride << ","
        << "\"iters\":" << r.iters << ","
        << "\"chains\":" << r.chains << ","
        << "\"elapsed_ms\":" << r.elapsed_ms << ","
        << "\"bandwidth_GBps\":" << r.bandwidth_GBps << ","
        << "\"ns_per_step\":" << r.ns_per_step
        << "}\n";

  } else {
    die("unknown --mode: " + mode);
  }

  return 0;
}