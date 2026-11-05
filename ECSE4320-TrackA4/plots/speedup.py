#!/usr/bin/env python3
import csv
import sys
from collections import defaultdict

def mean(xs):
    return sum(xs) / len(xs) if xs else 0.0

def main():
    # Usage:
    #   python3 plots/speedup.py results/raw/a4_bench.csv
    if len(sys.argv) != 2:
        print("usage: python3 plots/speedup.py <bench_csv>")
        sys.exit(1)

    bench_csv = sys.argv[1]
    bench_mean_out = "results/raw/a4_bench_mean.csv"
    speedup_out = "results/raw/a4_speedup_mean.csv"

    # group key: (impl, work, threads, nkeys) -> list[ops_per_sec]
    groups = defaultdict(list)

    with open(bench_csv, newline="") as f:
        r = csv.DictReader(f)
        for row in r:
            impl = row["impl"]
            work = row["work"]
            th = int(row["threads"])
            nkeys = int(row["nkeys"])
            ops = float(row["ops_per_sec"])
            groups[(impl, work, th, nkeys)].append(ops)

    # mean throughput table
    mean_thr = {}
    for k, xs in groups.items():
        mean_thr[k] = mean(xs)

    # write bench mean
    with open(bench_mean_out, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["impl", "work", "threads", "nkeys", "mean_ops_per_sec"])
        for (impl, work, th, nkeys) in sorted(mean_thr.keys(), key=lambda x: (x[1], x[3], x[2], x[0])):
            w.writerow([impl, work, th, nkeys, "{:.10g}".format(mean_thr[(impl, work, th, nkeys)])])

    # build base (1-thread) per (impl, work, nkeys)
    base = {}
    for (impl, work, th, nkeys), thr in mean_thr.items():
        if th == 1:
            base[(impl, work, nkeys)] = thr

    # write speedup
    with open(speedup_out, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["impl", "work", "threads", "nkeys", "speedup"])
        for (impl, work, th, nkeys) in sorted(mean_thr.keys(), key=lambda x: (x[1], x[3], x[2], x[0])):
            b = base.get((impl, work, nkeys), None)
            if b is None or b <= 0:
                continue
            sp = mean_thr[(impl, work, th, nkeys)] / b
            w.writerow([impl, work, th, nkeys, "{:.10g}".format(sp)])

    print("Wrote", bench_mean_out)
    print("Wrote", speedup_out)

if __name__ == "__main__":
    main()
