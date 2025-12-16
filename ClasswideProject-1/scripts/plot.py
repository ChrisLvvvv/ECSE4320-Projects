import warnings
warnings.filterwarnings("ignore", message="Unable to import Axes3D")

import sys
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

csv_path = sys.argv[1]
fig_dir = sys.argv[2]
df = pd.read_csv(csv_path)

# normalize string keys
for c in ["build","dtype","kernel"]:
    df[c] = df[c].astype(str).str.strip()

# numeric coercion
num_cols = ["n","stride","misalign","tail","reps","t_median_ns","t_p10_ns","t_p90_ns","gflops","ai"]
for c in num_cols:
    df[c] = pd.to_numeric(df[c], errors="coerce")

# robust discrete columns: treat NaN as -1, then cast
for c in ["stride","misalign","tail"]:
    df[c] = df[c].fillna(-1).astype(int)

df = df.dropna(subset=["n","t_median_ns","gflops"])
print(f"rows={len(df)} builds={sorted(df.build.unique())} dtypes={sorted(df.dtype.unique())} kernels={sorted(df.kernel.unique())}")

def savefig(name):
    plt.savefig(f"{fig_dir}/{name}.png", dpi=200, bbox_inches="tight")
    plt.close()

def base_filter(d):
    # avoid brittle == comparisons; keep the intended meaning
    return d[(d["stride"].isin([1])) & (d["misalign"].isin([0])) & (d["tail"].isin([0]))]

def speedup_plot(kernel, dtype):
    d = base_filter(df[(df.kernel==kernel)&(df.dtype==dtype)])
    piv = d.pivot_table(index="n", columns="build", values="t_median_ns", aggfunc="median").reset_index()
    if not {"scalar","auto","native"}.issubset(set(piv.columns)):
        return
    n = piv["n"].to_numpy()
    plt.figure()
    plt.plot(n, piv["scalar"]/piv["auto"], label="scalar/auto")
    plt.plot(n, piv["scalar"]/piv["native"], label="scalar/native")
    plt.xscale("log", base=2)
    plt.xlabel("N (elements)")
    plt.ylabel("Speedup")
    plt.title(f"Speedup vs N ({kernel}, {dtype})")
    plt.legend()
    savefig(f"speedup_{kernel}_{dtype}")

def gflops_plot(kernel, dtype):
    d = base_filter(df[(df.kernel==kernel)&(df.dtype==dtype)])
    plt.figure()
    drawn = 0
    for b in ["scalar","auto","native"]:
        dd = d[d.build==b].sort_values("n")
        if len(dd) == 0:
            continue
        plt.plot(dd["n"], dd["gflops"], label=b)
        drawn += 1
    plt.xscale("log", base=2)
    plt.xlabel("N (elements)")
    plt.ylabel("GFLOP/s")
    plt.title(f"GFLOP/s vs N ({kernel}, {dtype})")
    if drawn:
        plt.legend()
    savefig(f"gflops_{kernel}_{dtype}")

def alignment_tail_plot(kernel, dtype):
    d = df[(df.kernel==kernel)&(df.dtype==dtype)&(df.n==1048576)&(df.stride==1)&(df.build=="native")]
    if len(d)==0:
        return
    plt.figure()
    labels, vals = [], []
    for mis in [0,4]:
        for tail in [0,1]:
            dd = d[(d.misalign==mis)&(d.tail==tail)]
            if len(dd)==0:
                continue
            labels.append(f"mis{mis}_tail{tail}")
            vals.append(dd["gflops"].median())
    if len(vals)==0:
        plt.close()
        return
    x = np.arange(len(vals))
    plt.bar(x, vals)
    plt.xticks(x, labels, rotation=20)
    plt.ylabel("GFLOP/s (native)")
    plt.title(f"Alignment & tail ({kernel}, {dtype}, N=1,048,576)")
    savefig(f"align_tail_{kernel}_{dtype}")

def stride_plot(kernel, dtype):
    d = df[(df.kernel==kernel)&(df.dtype==dtype)&(df.n==2097152)&(df.tail==0)&(df.misalign==0)&(df.build=="native")].sort_values("stride")
    if len(d)==0:
        return
    plt.figure()
    plt.plot(d["stride"], d["gflops"], marker="o")
    plt.xlabel("Stride")
    plt.ylabel("GFLOP/s (native)")
    plt.title(f"Stride impact ({kernel}, {dtype}, N=2,097,152)")
    savefig(f"stride_{kernel}_{dtype}")

for k in ["axpy","dot","stencil"]:
    for t in ["f32","f64"]:
        speedup_plot(k, t)
        gflops_plot(k, t)

for k in ["axpy","dot"]:
    for t in ["f32","f64"]:
        alignment_tail_plot(k, t)
        stride_plot(k, t)
