#!/usr/bin/env python3
import sys, csv
import warnings
warnings.filterwarnings("ignore", message="Unable to import Axes3D.*")
import matplotlib.pyplot as plt

if len(sys.argv) != 4:
    print(f"usage: {sys.argv[0]} input.csv stride_bytes output.png", file=sys.stderr)
    sys.exit(1)

inp = sys.argv[1]
stride_target = int(sys.argv[2])
out = sys.argv[3]

# rw_pct -> (x,y,e) ; x will just be rw_pct (read percentage)
xs = []
ys = []
es = []

with open(inp) as f:
    r = csv.DictReader(f)
    for row in r:
        if int(row["random"]) != 0:
            continue
        if int(row["stride"]) != stride_target:
            continue
        rw = int(row["rw_pct"])
        xs.append(rw)
        ys.append(float(row["mean_bandwidth_GBps"]))
        es.append(float(row["std_bandwidth_GBps"]))

# sort by rw_pct
order = sorted(range(len(xs)), key=lambda i: xs[i])
xs = [xs[i] for i in order]
ys = [ys[i] for i in order]
es = [es[i] for i in order]

plt.figure(figsize=(6,4))
plt.errorbar(xs, ys, yerr=es, fmt='o-', capsize=3)
plt.xlabel("Read Percentage (%)")
plt.ylabel("Bandwidth (GB/s)")
plt.title(f"R/W Mix Bandwidth (Stride={stride_target}B, Sequential)")
plt.grid(True, linestyle="--", linewidth=0.5)
plt.tight_layout()
plt.savefig(out, dpi=150)
print(f"Wrote {out}")
