#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

static inline uint64_t ns_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

int main(int argc, char **argv) {
    size_t mb = 512;
    size_t rounds = 50;

    if (argc >= 2) mb = strtoull(argv[1], NULL, 10);
    if (argc >= 3) rounds = strtoull(argv[2], NULL, 10);

    size_t bytes = mb * 1024ull * 1024ull;
    size_t n = bytes / sizeof(uint64_t);

    uint64_t *a = aligned_alloc(64, n * sizeof(uint64_t));
    if (!a) {
        perror("alloc");
        return 1;
    }

    for (size_t i = 0; i < n; i++) a[i] = i;

    volatile uint64_t sum = 0;
    uint64_t t0 = ns_now();
    for (size_t r = 0; r < rounds; r++) {
        for (size_t i = 0; i < n; i += 8) {
            sum += a[i];
        }
    }
    uint64_t t1 = ns_now();

    double sec = (t1 - t0) / 1e9;
    double gb = (double)bytes * rounds / (1024.0 * 1024.0 * 1024.0);

    printf("mem_mb=%zu rounds=%zu time_sec=%.6f bandwidth=%.3f GB/s sum=%llu\n",
           mb, rounds, sec, gb / sec, (unsigned long long)sum);

    free(a);
    return 0;
}
