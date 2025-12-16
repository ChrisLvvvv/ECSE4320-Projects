#!/usr/bin/env python3
import sys, csv
import matplotlib.pyplot as plt

if len(sys.argv) != 3:
    print(f"usage: {sys.argv[0]} input.csv output.png", file=sys.stderr)
    sys.exit(1)

inp, out = sys.argv[1], sys.argv[2]

# group by random flag
series = {0: {"x": [], "y": [], "e": []}, 1: {"x": [], "y": [], "e": []}}

with open(inp) as f:
    r = csv.DictReader(f)
    for row in r:
        if row["write"] != "0":
            continue  # read only for now
        rnd = int(row["random"])
        series[rnd]["x"].append(int(row["stride"]))
        series[rnd]["y"].append(float(row["mean_bandwidth_GBps"]))
        series[rnd]["e"].append(float(row["std_bandwidth_GBps"]))

plt.figure(figsize=(6,4))
plt.errorbar(series[0]["x"], series[0]["y"], yerr=series[0]["e"], fmt='o-', capsize=3, label="sequential")
plt.errorbar(series[1]["x"], series[1]["y"], yerr=series[1]["e"], fmt='o-', capsize=3, label="random")
plt.xlabel("Stride (bytes)")
plt.ylabel("Bandwidth (GB/s)")
plt.title("Stride Sweep Bandwidth (Read): Sequential vs Random")
plt.grid(True, linestyle="--", linewidth=0.5)
plt.legend()
plt.tight_layout()
plt.savefig(out, dpi=150)
print(f"Wrote {out}")
