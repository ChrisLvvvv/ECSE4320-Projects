import json
import csv
import sys
from pathlib import Path
from collections import defaultdict

def load_jsonl(path: Path):
    rows = []
    for line in path.read_text().splitlines():
        if not line.strip():
            continue
        rows.append(json.loads(line))
    return rows

def write_csv(path: Path, rows, fieldnames):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fieldnames)
        w.writeheader()
        for r in rows:
            w.writerow({k: r.get(k, "") for k in fieldnames})

def main():
    if len(sys.argv) < 3:
        print("usage: summarize.py <input.jsonl> <out_dir>", file=sys.stderr)
        sys.exit(2)

    inp = Path(sys.argv[1])
    out_dir = Path(sys.argv[2])

    rows = load_jsonl(inp)

    # 1) Baseline QD=1 table: just dump key fields
    base = [r for r in rows if "qd1" in r.get("jobname","") or "qd1" in r.get("jobfile","")]
    if base:
        fields = [
            "tag","jobfile","jobname","rw","bs","iodepth","numjobs","size",
            "total_iops","total_bw_mib_s",
            "read_mean_us","read_p95_us","read_p99_us",
            "write_mean_us","write_p95_us","write_p99_us",
        ]
        write_csv(out_dir / "baseline_qd1.csv", base, fields)

    # 2) Generic dump (all rows)
    all_fields = sorted({k for r in rows for k in r.keys()})
    write_csv(out_dir / "all_results.csv", rows, all_fields)

if __name__ == "__main__":
    main()
