#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

static inline uint64_t ns_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static double run_compute(uint64_t iters) {
    volatile uint64_t x = 1;
    uint64_t t0 = ns_now();
    for (uint64_t i = 0; i < iters; i++) {
        x = x * 1664525u + 1013904223u; // cheap LCG, keeps CPU busy
    }
    uint64_t t1 = ns_now();
    (void)x;
    return (t1 - t0) / 1e9;
}

static double run_mem(size_t bytes, uint64_t rounds) {
    size_t n = bytes / sizeof(uint64_t);
    if (n < 1024) n = 1024;

    uint64_t *a = (uint64_t*)aligned_alloc(64, n * sizeof(uint64_t));
    if (!a) {
        perror("aligned_alloc");
        exit(1);
    }
    for (size_t i = 0; i < n; i++) a[i] = (uint64_t)i;

    volatile uint64_t sum = 0;
    uint64_t t0 = ns_now();
    for (uint64_t r = 0; r < rounds; r++) {
        // stride 8 gives less perfect prefetch than pure sequential
        for (size_t i = 0; i < n; i += 8) sum += a[i];
    }
    uint64_t t1 = ns_now();

    free(a);
    (void)sum;
    return (t1 - t0) / 1e9;
}

int main(int argc, char **argv) {
    // defaults
    uint64_t iters = 300000000ull;     // compute loop iterations
    size_t mem_mb = 256;               // memory working set in MB
    uint64_t rounds = 30;              // memory passes

    if (argc >= 2) iters = strtoull(argv[1], NULL, 10);
    if (argc >= 3) mem_mb = (size_t)strtoull(argv[2], NULL, 10);
    if (argc >= 4) rounds = strtoull(argv[3], NULL, 10);

    double t_compute = run_compute(iters);
    double t_mem = run_mem(mem_mb * 1024ull * 1024ull, rounds);

    printf("compute_sec=%.6f mem_sec=%.6f iters=%llu mem_mb=%zu rounds=%llu\n",
           t_compute, t_mem,
           (unsigned long long)iters,
           mem_mb,
           (unsigned long long)rounds);

    return 0;
}
