import csv
import sys
from collections import defaultdict
import matplotlib.pyplot as plt

# Usage:
#   python3 plots/throughput.py results/raw/a4_bench.csv results/plots/throughput_lookup_n1e5.png
#
# It will generate one figure per (work,nkeys), with two lines: coarse vs striped.

def mean(xs):
    return sum(xs) / len(xs) if xs else 0.0

def main():
    if len(sys.argv) != 3:
        print("usage: python3 plots/throughput.py <bench_csv> <out_prefix_png>")
        sys.exit(1)

    path = sys.argv[1]
    out_prefix = sys.argv[2].rsplit(".png", 1)[0]

    # key: (work, nkeys, impl, threads) -> list of ops_per_sec
    data = defaultdict(list)

    with open(path, newline="") as f:
        r = csv.DictReader(f)
        for row in r:
            work = row["work"]
            nkeys = int(row["nkeys"])
            impl = row["impl"]
            th = int(row["threads"])
            ops = float(row["ops_per_sec"])
            data[(work, nkeys, impl, th)].append(ops)

    works = sorted({k[0] for k in data.keys()})
    nkeyss = sorted({k[1] for k in data.keys()})

    for work in works:
        for nkeys in nkeyss:
            # collect threads
            threads = sorted({k[3] for k in data.keys() if k[0] == work and k[1] == nkeys})
            if not threads:
                continue

            for impl in ["coarse", "striped"]:
                ys = [mean(data[(work, nkeys, impl, th)]) for th in threads]
                plt.plot(threads, ys, marker="o", label=impl)

            plt.xlabel("Threads")
            plt.ylabel("Throughput (ops/s)")
            plt.title(f"Throughput vs Threads (work={work}, nkeys={nkeys})")
            plt.legend()
            plt.grid(True)

            out = f"{out_prefix}_{work}_n{nkeys}.png"
            plt.savefig(out, dpi=150, bbox_inches="tight")
            plt.clf()
            print("wrote", out)

if __name__ == "__main__":
    main()
