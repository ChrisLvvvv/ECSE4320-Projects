#!/usr/bin/env python3
import json, sys, math
from collections import defaultdict

if len(sys.argv) != 3:
    print(f"usage: {sys.argv[0]} input.jsonl output.csv", file=sys.stderr)
    sys.exit(1)

inp, out = sys.argv[1], sys.argv[2]

# key = chains
bw = defaultdict(list)
lat = defaultdict(list)  # ns_per_step
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
        if rec.get("bench") != "mlp":
            continue
        c = int(rec["chains"])
        bw[c].append(float(rec["bandwidth_GBps"]))
        lat[c].append(float(rec["ns_per_step"]))

def mean(xs):
    return sum(xs)/len(xs)

def std(xs, mu):
    if len(xs) <= 1:
        return 0.0
    return math.sqrt(sum((x-mu)**2 for x in xs)/(len(xs)-1))

with open(out, "w") as f:
    f.write("chains,mean_bandwidth_GBps,std_bandwidth_GBps,mean_ns_per_step,std_ns_per_step,n\n")
    for c in sorted(bw.keys()):
        mbw = mean(bw[c]); sbw = std(bw[c], mbw)
        mlat = mean(lat[c]); slat = std(lat[c], mlat)
        f.write(f"{c},{mbw:.3f},{sbw:.3f},{mlat:.3f},{slat:.3f},{len(bw[c])}\n")

print(f"Wrote {out} (skipped {skipped} lines)")
