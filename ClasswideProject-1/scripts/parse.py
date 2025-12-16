import sys, json, csv

inp = sys.argv[1]
out = sys.argv[2]

rows = []
with open(inp, "r") as f:
    for line in f:
        line = line.strip()
        if not line:
            continue
        rows.append(json.loads(line))

fields = [
    "build","dtype","kernel","n","stride","misalign","tail","reps",
    "t_median_ns","t_p10_ns","t_p90_ns","gflops","ai"
]

with open(out, "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=fields)
    w.writeheader()
    for r in rows:
        w.writerow({k: r.get(k) for k in fields})
