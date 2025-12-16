import json, sys
from pathlib import Path
import matplotlib.pyplot as plt
import csv

def load(path):
    rows=[]
    for line in Path(path).read_text().splitlines():
        if line.strip():
            rows.append(json.loads(line))
    return rows

def main():
    if len(sys.argv) < 4:
        print("usage: plot_tail.py <combined.jsonl> <out_csv> <out_png>", file=sys.stderr)
        sys.exit(2)

    rows = load(sys.argv[1])
    tail = [r for r in rows if r.get("jobfile","").startswith("fio_tail_latency")]
    if not tail:
        print("no tail rows found", file=sys.stderr)
        sys.exit(1)

    order = ["tail_mid_qd16","tail_near_knee_qd64"]
    tail_sorted = sorted(tail, key=lambda r: order.index(r.get("jobname","")) if r.get("jobname","") in order else 999)

    fields = ["jobname","iodepth","read_p50_us","read_p95_us","read_p99_us","read_p999_us","read_mean_us","total_iops"]
    out_csv = Path(sys.argv[2])
    out_csv.parent.mkdir(parents=True, exist_ok=True)
    with out_csv.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        for r in tail_sorted:
            w.writerow({k: r.get(k,"") for k in fields})

    labels = [r["jobname"].replace("tail_","") for r in tail_sorted]
    p50 = [r.get("read_p50_us") for r in tail_sorted]
    p95 = [r.get("read_p95_us") for r in tail_sorted]
    p99 = [r.get("read_p99_us") for r in tail_sorted]
    p999 = [r.get("read_p999_us") for r in tail_sorted]

    plt.figure()
    plt.plot(labels, p50, marker="o", label="p50")
    plt.plot(labels, p95, marker="o", label="p95")
    plt.plot(labels, p99, marker="o", label="p99")
    plt.plot(labels, p999, marker="o", label="p99.9")
    plt.xlabel("Condition")
    plt.ylabel("Completion latency (us)")
    plt.title("Tail latency vs queueing (4KiB Random Read)")
    plt.legend()
    plt.tight_layout()
    out_png = Path(sys.argv[3])
    out_png.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(out_png, dpi=200)

if __name__=="__main__":
    main()
