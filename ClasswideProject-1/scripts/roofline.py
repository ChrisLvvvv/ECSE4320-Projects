import warnings
warnings.filterwarnings("ignore", message="Unable to import Axes3D")

import sys, json, math
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

csv_path = sys.argv[1]
bw_jsonl = sys.argv[2]
out_png  = sys.argv[3]

df = pd.read_csv(csv_path)
for c in ["build","dtype","kernel"]:
    df[c] = df[c].astype(str).str.strip()

for c in ["n","stride","misalign","tail","t_median_ns","gflops","ai"]:
    df[c] = pd.to_numeric(df[c], errors="coerce")

df = df.dropna(subset=["gflops","ai","build","dtype","kernel"])

# Focus on native; remove obvious bad rows (negative/NaN)
native = df[df["build"]=="native"].copy()
native = native[native["gflops"] > 0.0]

# Robust peak: use 99th percentile to avoid single-row corruption/outliers
if len(native) == 0:
    peak = 1.0
else:
    peak = float(np.nanpercentile(native["gflops"].to_numpy(), 99))
    peak = max(peak, 1.0)

# Points: best per (kernel,dtype), but also cap at robust peak*1.2 to avoid plotting nonsense
pts = []
for k in ["axpy","dot","stencil"]:
    for t in ["f32","f64"]:
        sub = native[(native.kernel==k) & (native.dtype==t)]
        if len(sub) == 0:
            continue
        g_best = float(sub["gflops"].max())
        g_best = min(g_best, peak * 1.2)
        ai = float(sub["ai"].median())
        n_at = int(sub.loc[sub["gflops"].idxmax(), "n"])
        pts.append((k, t, ai, g_best, n_at))

# Bandwidth: max GiB/s from bw.jsonl
bw_gib = 0.0
with open(bw_jsonl, "r") as f:
    for line in f:
        line = line.strip()
        if not line:
            continue
        try:
            bw_gib = max(bw_gib, float(json.loads(line).get("gib_s", 0.0)))
        except Exception:
            pass

bw_bytes_s = bw_gib * (1024.0**3)
bw_slope = bw_bytes_s / 1e9  # GFLOP/s per (FLOP/byte)

# Plot
plt.figure()
ai_vals = [p[2] for p in pts] if pts else [1e-3, 1.0]
xmin = max(min(ai_vals) * 0.5, 1e-4)
xmax = max(ai_vals) * 2.0

x = np.logspace(math.log10(xmin), math.log10(xmax), 200)
y_bw = bw_slope * x

plt.plot(x, y_bw, label=f"Bandwidth bound (BW≈{bw_gib:.1f} GiB/s)")
plt.axhline(peak, linestyle="--", label=f"Robust peak (p99 native) ≈ {peak:.1f} GFLOP/s")

for k, t, ai, g, n_at in pts:
    plt.scatter([ai], [g], label=f"{k}-{t} (best @ N={n_at})")

plt.xscale("log")
plt.yscale("log")
plt.xlabel("Arithmetic Intensity (FLOP/byte)")
plt.ylabel("Performance (GFLOP/s)")
plt.title("Roofline (measured BW + robust peak)")
plt.legend(fontsize=8)
plt.savefig(out_png, dpi=220, bbox_inches="tight")
plt.close()

print(f"bw_gib_s={bw_gib:.3f} peak_gflops_p99={peak:.3f} points={len(pts)} -> {out_png}")
