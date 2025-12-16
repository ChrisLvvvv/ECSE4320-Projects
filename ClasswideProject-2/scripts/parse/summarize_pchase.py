#!/usr/bin/env python3
import json
import sys
from collections import defaultdict
import math

if len(sys.argv) != 3:
    print(f"usage: {sys.argv[0]} input.jsonl output.csv", file=sys.stderr)
    sys.exit(1)

inp = sys.argv[1]
out = sys.argv[2]

data = defaultdict(list)
skipped = 0

with open(inp) as f:
    for lineno, line in enumerate(f, 1):
        line = line.strip()
        if not line:
            skipped += 1
            continue
        try:
            rec = json.loads(line)
        except json.JSONDecodeError:
            skipped += 1
            continue

        if rec.get("bench") != "pointer_chase":
            continue

        data[rec["bytes"]].append(rec["ns_per_load"])

def mean(xs):
    return sum(xs) / len(xs)

def std(xs, mu):
    if len(xs) <= 1:
        return 0.0
    return math.sqrt(sum((x - mu) ** 2 for x in xs) / (len(xs) - 1))

with open(out, "w") as f:
    f.write("bytes,bytes_kb,mean_ns,std_ns,n\n")
    for b in sorted(data.keys()):
        xs = data[b]
        mu = mean(xs)
        sd = std(xs, mu)
        f.write(f"{b},{b/1024.0:.1f},{mu:.3f},{sd:.3f},{len(xs)}\n")

print(f"Wrote {out}")
print(f"Parsed {sum(len(v) for v in data.values())} records, skipped {skipped} lines")
