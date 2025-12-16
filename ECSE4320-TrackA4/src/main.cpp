#include "config.h"
#include "hashtable.h"
#include "hashtable_coarse.h"
#include "hashtable_striped.h"
#include "timer.h"
#include "workload.h"

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

static a4::Value make_value(std::uint64_t x) {
  a4::Value v{};
  for (std::size_t i = 0; i < v.size(); ++i) {
    v[i] = static_cast<std::uint8_t>((x >> (i * 8)) & 0xFF);
  }
  return v;
}

enum class Impl { kCoarse, kStriped };
enum class Work { kLookup, kInsert, kMixed };

struct Args {
  Impl impl = Impl::kCoarse;
  Work work = Work::kLookup;
  int threads = a4::kDefaultThreads;
  std::size_t ops = a4::kDefaultOps;
  std::uint64_t nkeys = 100000;     // dataset size
  std::uint64_t key_space = 200000; // controls hit rate for lookup
  std::size_t buckets = a4::kDefaultBuckets;
  std::size_t stripes = a4::kDefaultStripes;
  int reps = 3;
};

static void usage(const char* prog) {
  std::cerr
      << "Usage: " << prog << " [--impl=coarse|striped] [--work=lookup|insert|mixed]\n"
      << "       [--threads=N] [--ops=N] [--nkeys=N] [--keyspace=N]\n"
      << "       [--buckets=N] [--stripes=N] [--reps=N]\n";
}

static bool parse_kv(const std::string& s, std::string* k, std::string* v) {
  const auto pos = s.find('=');
  if (pos == std::string::npos) return false;
  *k = s.substr(0, pos);
  *v = s.substr(pos + 1);
  return true;
}

static Args parse_args(int argc, char** argv) {
  Args a;
  for (int i = 1; i < argc; ++i) {
    std::string s(argv[i]);
    if (s == "--help" || s == "-h") {
      usage(argv[0]);
      std::exit(0);
    }
    std::string k, v;
    if (!parse_kv(s, &k, &v)) {
      std::cerr << "Bad arg: " << s << "\n";
      usage(argv[0]);
      std::exit(1);
    }

    if (k == "--impl") {
      if (v == "coarse") a.impl = Impl::kCoarse;
      else if (v == "striped") a.impl = Impl::kStriped;
      else { std::cerr << "Unknown impl: " << v << "\n"; std::exit(1); }
    } else if (k == "--work") {
      if (v == "lookup") a.work = Work::kLookup;
      else if (v == "insert") a.work = Work::kInsert;
      else if (v == "mixed") a.work = Work::kMixed;
      else { std::cerr << "Unknown work: " << v << "\n"; std::exit(1); }
    } else if (k == "--threads") a.threads = std::stoi(v);
    else if (k == "--ops") a.ops = static_cast<std::size_t>(std::stoull(v));
    else if (k == "--nkeys") a.nkeys = std::stoull(v);
    else if (k == "--keyspace") a.key_space = std::stoull(v);
    else if (k == "--buckets") a.buckets = static_cast<std::size_t>(std::stoull(v));
    else if (k == "--stripes") a.stripes = static_cast<std::size_t>(std::stoull(v));
    else if (k == "--reps") a.reps = std::stoi(v);
    else {
      std::cerr << "Unknown flag: " << k << "\n";
      std::exit(1);
    }
  }
  return a;
}

static std::unique_ptr<a4::IHashTable> make_table(const Args& a) {
  if (a.impl == Impl::kCoarse) {
    return std::make_unique<a4::HashTableCoarse>(a.buckets);
  }
  return std::make_unique<a4::HashTableStriped>(a.buckets, a.stripes);
}

struct RunResult {
  double seconds = 0.0;
  double ops_per_sec = 0.0;
};

static RunResult run_once(const Args& a, std::uint64_t seed) {
  auto ht = make_table(a);

  // Preload nkeys so lookup/mixed can hit.
  // Use keys in [0, key_space).
  const auto preload = a4::gen_keys(static_cast<std::size_t>(a.nkeys), seed ^ 0xabcULL, a.key_space);
  for (std::size_t i = 0; i < preload.size(); ++i) {
    ht->insert(preload[i], make_value(preload[i]));
  }

  std::atomic<std::size_t> next{0};
  const auto ops_keys = a4::gen_keys(a.ops, seed ^ 0x1234ULL, a.key_space);

  a4::Timer timer;
  timer.start();

  std::vector<std::thread> ts;
  ts.reserve(static_cast<std::size_t>(a.threads));

  for (int t = 0; t < a.threads; ++t) {
    ts.emplace_back([&, t]() {
      a4::Value out{};
      while (true) {
        const std::size_t i = next.fetch_add(1, std::memory_order_relaxed);
        if (i >= a.ops) break;

        const a4::Key k = ops_keys[i];

        if (a.work == Work::kLookup) {
          (void)ht->find(k, &out);
        } else if (a.work == Work::kInsert) {
          (void)ht->insert(k + static_cast<a4::Key>(t) * 1'000'000'000ULL, make_value(k));
        } else { // mixed 70/30: 70% lookup, 30% insert
          // Deterministic mix based on i.
          if ((i % 10) < 7) (void)ht->find(k, &out);
          else (void)ht->insert(k + static_cast<a4::Key>(t) * 1'000'000'000ULL, make_value(k));
        }
      }
    });
  }
  for (auto& th : ts) th.join();

  const double sec = timer.seconds();
  RunResult r;
  r.seconds = sec;
  r.ops_per_sec = static_cast<double>(a.ops) / sec;
  return r;
}

static const char* impl_name(Impl x) { return x == Impl::kCoarse ? "coarse" : "striped"; }
static const char* work_name(Work x) {
  if (x == Work::kLookup) return "lookup";
  if (x == Work::kInsert) return "insert";
  return "mixed";
}

int main(int argc, char** argv) {
  const Args a = parse_args(argc, argv);

  // CSV header to stdout:
  // impl,work,threads,nkeys,keyspace,ops,rep,seconds,ops_per_sec
  std::cout << "impl,work,threads,nkeys,keyspace,ops,rep,seconds,ops_per_sec\n";

  for (int rep = 0; rep < a.reps; ++rep) {
    const auto r = run_once(a, 0xC0FFEEULL + static_cast<std::uint64_t>(rep));
    std::cout
        << impl_name(a.impl) << ","
        << work_name(a.work) << ","
        << a.threads << ","
        << a.nkeys << ","
        << a.key_space << ","
        << a.ops << ","
        << rep << ","
        << r.seconds << ","
        << r.ops_per_sec << "\n";
  }
  return 0;
}
