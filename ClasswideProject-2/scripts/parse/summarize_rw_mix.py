#!/usr/bin/env python3
import json
import sys
from collections import defaultdict
import math

if len(sys.argv) != 3:
    print(f"usage: {sys.argv[0]} input.jsonl output.csv", file=sys.stderr)
    sys.exit(1)

inp, out = sys.argv[1], sys.argv[2]

# key = (stride, random, rw_pct)
bw = defaultdict(list)
lat = defaultdict(list)
skipped = 0

with open(inp) as f:
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
        rnd = int(bool(rec.get("random", False)))
        rw_pct = int(rec.get("rw_pct", 100))

        key = (stride, rnd, rw_pct)
        bw[key].append(float(rec["bandwidth_GBps"]))
        lat[key].append(float(rec["ns_per_access"]))

def mean(xs):
    return sum(xs) / len(xs)

def std(xs, mu):
    if len(xs) <= 1:
        return 0.0
    return math.sqrt(sum((x - mu) ** 2 for x in xs) / (len(xs) - 1))

with open(out, "w") as f:
    f.write("stride,random,rw_pct,mean_bandwidth_GBps,std_bandwidth_GBps,mean_ns_per_access,std_ns_per_access,n\n")
    keys = sorted(bw.keys(), key=lambda k: (k[1], k[0], -k[2]))  # random, stride, rw_pct desc
    for k in keys:
        bws = bw[k]
        lats = lat[k]
        mbw = mean(bws); sbw = std(bws, mbw)
        mlat = mean(lats); slat = std(lats, mlat)
        f.write(f"{k[0]},{k[1]},{k[2]},{mbw:.3f},{sbw:.3f},{mlat:.3f},{slat:.3f},{len(bws)}\n")

print(f"Wrote {out} (skipped {skipped} lines)")
