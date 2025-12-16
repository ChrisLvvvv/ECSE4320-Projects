#!/usr/bin/env python3
import sys, csv
import matplotlib.pyplot as plt

if len(sys.argv) != 3:
    print(f"usage: {sys.argv[0]} input.csv output.png", file=sys.stderr)
    sys.exit(1)

inp, out = sys.argv[1], sys.argv[2]

series = {0: {"x": [], "y": [], "e": []}, 1: {"x": [], "y": [], "e": []}}

with open(inp) as f:
    r = csv.DictReader(f)
    for row in r:
        if row["write"] != "0":
            continue
        rnd = int(row["random"])
        series[rnd]["x"].append(int(row["stride"]))
        series[rnd]["y"].append(float(row["mean_ns_per_access"]))
        series[rnd]["e"].append(float(row["std_ns_per_access"]))

plt.figure(figsize=(6,4))
plt.errorbar(series[0]["x"], series[0]["y"], yerr=series[0]["e"], fmt='o-', capsize=3, label="sequential")
plt.errorbar(series[1]["x"], series[1]["y"], yerr=series[1]["e"], fmt='o-', capsize=3, label="random")
plt.xlabel("Stride (bytes)")
plt.ylabel("Latency (ns per access)")
plt.title("Stride Sweep Latency (Read): Sequential vs Random")
plt.grid(True, linestyle="--", linewidth=0.5)
plt.legend()
plt.tight_layout()
plt.savefig(out, dpi=150)
print(f"Wrote {out}")
