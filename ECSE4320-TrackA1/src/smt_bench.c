#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static inline uint64_t ns_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static double run_compute(uint64_t iters) {
    volatile uint64_t x = 1;
    uint64_t t0 = ns_now();
    for (uint64_t i = 0; i < iters; i++) {
        x = x * 1664525u + 1013904223u;
    }
    uint64_t t1 = ns_now();
    (void)x;
    return (t1 - t0) / 1e9;
}

static double run_mem(size_t mb, uint64_t rounds) {
    size_t bytes = mb * 1024ull * 1024ull;
    size_t n = bytes / sizeof(uint64_t);
    if (n < 1024) n = 1024;

    uint64_t *a = aligned_alloc(64, n * sizeof(uint64_t));
    if (!a) {
        perror("alloc");
        exit(1);
    }
    for (size_t i = 0; i < n; i++) a[i] = (uint64_t)i;

    volatile uint64_t sum = 0;
    uint64_t t0 = ns_now();
    for (uint64_t r = 0; r < rounds; r++) {
        for (size_t i = 0; i < n; i += 8) sum += a[i];
    }
    uint64_t t1 = ns_now();

    free(a);
    (void)sum;
    return (t1 - t0) / 1e9;
}

int main(int argc, char **argv) {
    // usage:
    // smt_bench compute <iters>
    // smt_bench mem <mb> <rounds>
    if (argc < 3) {
        fprintf(stderr, "usage:\n  %s compute <iters>\n  %s mem <mb> <rounds>\n", argv[0], argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "compute") == 0) {
        uint64_t iters = strtoull(argv[2], NULL, 10);
        double sec = run_compute(iters);
        printf("mode=compute time_sec=%.6f iters=%llu\n", sec, (unsigned long long)iters);
        return 0;
    }

    if (strcmp(argv[1], "mem") == 0) {
        if (argc < 4) return 1;
        size_t mb = (size_t)strtoull(argv[2], NULL, 10);
        uint64_t rounds = strtoull(argv[3], NULL, 10);
        double sec = run_mem(mb, rounds);
        printf("mode=mem time_sec=%.6f mem_mb=%zu rounds=%llu\n", sec, mb, (unsigned long long)rounds);
        return 0;
    }

    fprintf(stderr, "unknown mode\n");
    return 1;
}
