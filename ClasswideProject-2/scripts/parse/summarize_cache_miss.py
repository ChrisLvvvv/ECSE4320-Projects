#!/usr/bin/env python3
import json, sys, math
from collections import defaultdict

if len(sys.argv) != 3:
    print(f"usage: {sys.argv[0]} input.jsonl output.csv", file=sys.stderr)
    sys.exit(1)

inp, out = sys.argv[1], sys.argv[2]

bw = defaultdict(list)
lat = defaultdict(list)

with open(inp) as f:
    for line in f:
        line = line.strip()
        if not line:
            continue
        rec = json.loads(line)
        if rec.get("bench") != "stride":
            continue
        b = int(rec["bytes"])
        bw[b].append(float(rec.get("bandwidth_touch_GBps", rec["bandwidth_GBps"])))

        lat[b].append(float(rec["ns_per_access"]))

def mean(xs): return sum(xs)/len(xs)
def std(xs, m): return math.sqrt(sum((x-m)**2 for x in xs)/(len(xs)-1)) if len(xs)>1 else 0.0

with open(out, "w") as f:
    f.write("bytes,bytes_kb,mean_bandwidth_GBps,std_bandwidth_GBps,mean_ns_per_access,std_ns_per_access,n\n")
    for b in sorted(bw.keys()):
        mb = mean(bw[b]); sb = std(bw[b], mb)
        ml = mean(lat[b]); sl = std(lat[b], ml)
        f.write(f"{b},{b/1024:.1f},{mb:.3f},{sb:.3f},{ml:.3f},{sl:.3f},{len(bw[b])}\n")

print(f"Wrote {out}")
