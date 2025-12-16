#!/usr/bin/env python3
import sys, csv, warnings
warnings.filterwarnings("ignore", message="Unable to import Axes3D.*")
import matplotlib.pyplot as plt

if len(sys.argv) != 3:
    print(f"usage: {sys.argv[0]} input.csv output.png", file=sys.stderr)
    sys.exit(1)

inp, out = sys.argv[1], sys.argv[2]

xs=[]; lat=[]; lat_e=[]
with open(inp) as f:
    r = csv.DictReader(f)
    for row in r:
        xs.append(float(row["bytes_mb"]))
        lat.append(float(row["mean_ns_per_access"]))
        lat_e.append(float(row["std_ns_per_access"]))

plt.figure(figsize=(6,4))
plt.errorbar(xs, lat, yerr=lat_e, fmt='o-', capsize=3)
plt.xscale("log")
plt.xlabel("Working Set Size (MB)")
plt.ylabel("Latency (ns per access)")
plt.title("TLB Miss Impact (Stride=4KB, Sequential)")
plt.grid(True, which="both", linestyle="--", linewidth=0.5)
plt.tight_layout()
plt.savefig(out, dpi=150)
print(f"Wrote {out}")
