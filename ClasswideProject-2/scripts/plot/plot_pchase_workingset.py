#!/usr/bin/env python3
import sys
import csv
import matplotlib.pyplot as plt

if len(sys.argv) != 3:
    print(f"usage: {sys.argv[0]} input.csv output.png", file=sys.stderr)
    sys.exit(1)

inp = sys.argv[1]
out = sys.argv[2]

kb = []
mean = []
std = []

with open(inp) as f:
    r = csv.DictReader(f)
    for row in r:
        kb.append(float(row["bytes_kb"]))
        mean.append(float(row["mean_ns"]))
        std.append(float(row["std_ns"]))

plt.figure(figsize=(7, 4))
plt.errorbar(kb, mean, yerr=std, fmt='o-', capsize=3)
plt.xscale("log")
plt.xlabel("Working Set Size (KB)")
plt.ylabel("Latency (ns per load)")
plt.title("Pointer-Chase Latency vs Working Set Size")
plt.grid(True, which="both", linestyle="--", linewidth=0.5)

# Cache annotations for i7-12700F
plt.axvline(48, linestyle="--")
plt.text(48, max(mean) * 0.9, "L1", rotation=90, va="top")

plt.axvline(1280, linestyle="--")
plt.text(1280, max(mean) * 0.9, "L2", rotation=90, va="top")

plt.axvline(25600, linestyle="--")
plt.text(25600, max(mean) * 0.9, "L3", rotation=90, va="top")

plt.tight_layout()
plt.savefig(out, dpi=150)
print(f"Wrote {out}")
