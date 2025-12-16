#!/usr/bin/env python3
import csv, json, sys, math
from collections import defaultdict

# usage:
#   compare_stride_seq_random.py seq_summary.csv random_raw.jsonl out_compare.csv

if len(sys.argv) != 4:
    print(f"usage: {sys.argv[0]} seq_summary.csv random_raw.jsonl out.csv", file=sys.stderr)
    sys.exit(1)

seq_csv, rnd_jsonl, out_csv = sys.argv[1], sys.argv[2], sys.argv[3]

def mean(xs): return sum(xs)/len(xs)
def std(xs, mu):
    if len(xs) <= 1: return 0.0
    return math.sqrt(sum((x-mu)**2 for x in xs)/(len(xs)-1))

# Load sequential summary produced by summarize_stride.py
seq_rows = []
with open(seq_csv, newline="") as f:
    r = csv.DictReader(f)
    for row in r:
        # expected columns:
        # stride,write,mean_bandwidth_GBps,std_bandwidth_GBps,mean_ns_per_access,std_ns_per_access,n
        seq_rows.append({
            "stride": int(row["stride"]),
            "write": int(row["write"]),
            "random": 0,
            "mean_bandwidth_GBps": float(row["mean_bandwidth_GBps"]),
            "std_bandwidth_GBps": float(row["std_bandwidth_GBps"]),
            "mean_ns_per_access": float(row["mean_ns_per_access"]),
            "std_ns_per_access": float(row["std_ns_per_access"]),
            "n": int(row["n"]),
        })

# Parse random raw jsonl and summarize
bw = defaultdict(list)   # key: (stride, write)
lat = defaultdict(list)

skipped = 0
with open(rnd_jsonl) as f:
    for line in f:
        line = line.strip()
        if not line:
            skipped += 1
            continue
        try:
            rec = json.loads(line)
        except json.JSONDecodeError:
            skipped += 1
            continue
        if rec.get("bench") != "stride":
            continue
        stride = int(rec["stride"])
        write = 1 if bool(rec.get("write", False)) else 0
        key = (stride, write)
        bw[key].append(float(rec.get("bandwidth_touch_GBps", rec["bandwidth_GBps"])))
        lat[key].append(float(rec["ns_per_access"]))

rnd_rows = []
for (stride, write), xs in bw.items():
    ys = lat[(stride, write)]
    mbw = mean(xs); sbw = std(xs, mbw)
    mlat = mean(ys); slat = std(ys, mlat)
    rnd_rows.append({
        "stride": stride,
        "write": write,
        "random": 1,
        "mean_bandwidth_GBps": mbw,
        "std_bandwidth_GBps": sbw,
        "mean_ns_per_access": mlat,
        "std_ns_per_access": slat,
        "n": len(xs),
    })

# Combine and write
rows = seq_rows + sorted(rnd_rows, key=lambda d: (d["stride"], d["write"]))
rows = sorted(rows, key=lambda d: (d["stride"], d["write"], d["random"]))

with open(out_csv, "w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["stride","write","random","mean_bandwidth_GBps","std_bandwidth_GBps","mean_ns_per_access","std_ns_per_access","n"])
    for d in rows:
        w.writerow([
            d["stride"], d["write"], d["random"],
            f"{d['mean_bandwidth_GBps']:.3f}",
            f"{d['std_bandwidth_GBps']:.3f}",
            f"{d['mean_ns_per_access']:.3f}",
            f"{d['std_ns_per_access']:.3f}",
            d["n"],
        ])

print(f"Wrote {out_csv} (skipped {skipped} lines)")
