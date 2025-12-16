#!/usr/bin/env python3
import sys, csv, math
import warnings
warnings.filterwarnings("ignore", message="Unable to import Axes3D.*")
import matplotlib.pyplot as plt

if len(sys.argv) != 3:
    print(f"usage: {sys.argv[0]} input.csv output.png", file=sys.stderr)
    sys.exit(1)

inp, out = sys.argv[1], sys.argv[2]

chains=[]
bw=[]
lat=[]
bw_e=[]
lat_e=[]

with open(inp) as f:
    r = csv.DictReader(f)
    for row in r:
        chains.append(int(row["chains"]))
        bw.append(float(row["mean_bandwidth_GBps"]))
        lat.append(float(row["mean_ns_per_step"]))
        bw_e.append(float(row["std_bandwidth_GBps"]))
        lat_e.append(float(row["std_ns_per_step"]))

# sort by chains
order = sorted(range(len(chains)), key=lambda i: chains[i])
chains = [chains[i] for i in order]
bw = [bw[i] for i in order]
lat = [lat[i] for i in order]
bw_e = [bw_e[i] for i in order]
lat_e = [lat_e[i] for i in order]

# Knee detection via max curvature on (x=bw, y=log(lat)).
# Discrete curvature proxy: second difference magnitude.
knee_idx = None
best = -1.0
for i in range(1, len(bw)-1):
    y0 = math.log(max(lat[i-1], 1e-9))
    y1 = math.log(max(lat[i], 1e-9))
    y2 = math.log(max(lat[i+1], 1e-9))
    x0 = bw[i-1]; x1 = bw[i]; x2 = bw[i+1]
    # normalize by x spacing to reduce sensitivity
    dx1 = max(x1 - x0, 1e-9)
    dx2 = max(x2 - x1, 1e-9)
    d2 = abs((y2 - y1)/dx2 - (y1 - y0)/dx1)
    if d2 > best:
        best = d2
        knee_idx = i

plt.figure(figsize=(6,4))
plt.errorbar(bw, lat, xerr=bw_e, yerr=lat_e, fmt='o-', capsize=3)
plt.xlabel("Throughput (GB/s)")
plt.ylabel("Latency (ns per step)")
plt.title("Throughput–Latency Curve (MLP Sweep, Single-Thread)")
plt.grid(True, linestyle="--", linewidth=0.5)

if knee_idx is not None:
    plt.scatter([bw[knee_idx]], [lat[knee_idx]])
    plt.annotate(f"knee (chains={chains[knee_idx]})",
                 (bw[knee_idx], lat[knee_idx]),
                 textcoords="offset points", xytext=(10, 10))

plt.tight_layout()
plt.savefig(out, dpi=150)
print(f"Wrote {out}")
if knee_idx is not None:
    print(f"KNEE chains={chains[knee_idx]} bw={bw[knee_idx]:.3f} lat={lat[knee_idx]:.3f}")
