#define _GNU_SOURCE
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    uint64_t iters;
    volatile uint64_t sink;
} worker_arg_t;

static inline uint64_t ns_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static void *worker(void *p) {
    worker_arg_t *a = (worker_arg_t *)p;
    volatile uint64_t x = 1;
    for (uint64_t i = 0; i < a->iters; i++) {
        x = x * 1664525u + 1013904223u;
    }
    a->sink = x;
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <threads> <iters_per_thread>\n", argv[0]);
        return 1;
    }

    int threads = atoi(argv[1]);
    uint64_t iters = strtoull(argv[2], NULL, 10);
    if (threads <= 0) return 1;

    pthread_t *t = (pthread_t *)malloc(sizeof(pthread_t) * (size_t)threads);
    worker_arg_t *args = (worker_arg_t *)calloc((size_t)threads, sizeof(worker_arg_t));
    if (!t || !args) {
        perror("alloc");
        return 1;
    }

    for (int i = 0; i < threads; i++) args[i].iters = iters;

    uint64_t t0 = ns_now();
    for (int i = 0; i < threads; i++) {
        if (pthread_create(&t[i], NULL, worker, &args[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    for (int i = 0; i < threads; i++) pthread_join(t[i], NULL);
    uint64_t t1 = ns_now();

    volatile uint64_t guard = 0;
    for (int i = 0; i < threads; i++) guard ^= args[i].sink;

    double sec = (t1 - t0) / 1e9;
    double total_ops = (double)threads * (double)iters;
    double gops = total_ops / sec / 1e9;

    printf("threads=%d iters=%llu time_sec=%.6f gops=%.3f guard=%llu\n",
           threads, (unsigned long long)iters, sec, gops, (unsigned long long)guard);

    free(t);
    free(args);
    return 0;
}
