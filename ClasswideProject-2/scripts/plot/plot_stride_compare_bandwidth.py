#!/usr/bin/env python3
import sys, csv, warnings
warnings.filterwarnings("ignore", message="Unable to import Axes3D.*")
import matplotlib.pyplot as plt

# usage: plot_stride_compare_bandwidth.py in.csv out.png
if len(sys.argv) != 3:
    print(f"usage: {sys.argv[0]} in.csv out.png", file=sys.stderr)
    sys.exit(1)

inp, out = sys.argv[1], sys.argv[2]

# Only read (write=0) for this figure by default
seq_x=[]; seq_y=[]; seq_e=[]
rnd_x=[]; rnd_y=[]; rnd_e=[]

with open(inp) as f:
    r=csv.DictReader(f)
    for row in r:
        stride=int(row["stride"])
        write=int(row["write"])
        rnd=int(row["random"])
        if write != 0:
            continue
        y=float(row["mean_bandwidth_GBps"])
        e=float(row["std_bandwidth_GBps"])
        if rnd == 0:
            seq_x.append(stride); seq_y.append(y); seq_e.append(e)
        else:
            rnd_x.append(stride); rnd_y.append(y); rnd_e.append(e)

plt.figure(figsize=(6,4))
plt.errorbar(seq_x, seq_y, yerr=seq_e, fmt='o-', capsize=3, label="sequential")
plt.errorbar(rnd_x, rnd_y, yerr=rnd_e, fmt='o-', capsize=3, label="random")
plt.xscale("log")
plt.xlabel("Stride (bytes)")
plt.ylabel("Bandwidth (GB/s)")
plt.title("Stride Sweep: Bandwidth (Read) — Sequential vs Random")
plt.grid(True, which="both", linestyle="--", linewidth=0.5)
plt.legend()
plt.tight_layout()
plt.savefig(out, dpi=150)
print(f"Wrote {out}")
