#include <stdio.h>
#include <stdint.h>
#include <time.h>

static inline uint64_t ns_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

int main(void) {
    const uint64_t iters = 200000000ull; // 2e8
    volatile uint64_t x = 0;

    uint64_t t0 = ns_now();
    for (uint64_t i = 0; i < iters; i++) {
        x += i;
    }
    uint64_t t1 = ns_now();

    double sec = (t1 - t0) / 1e9;
    printf("iters=%llu x=%llu time_sec=%.6f\n",
           (unsigned long long)iters,
           (unsigned long long)x,
           sec);
    return 0;
}
