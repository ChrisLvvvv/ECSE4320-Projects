import json
import sys
from pathlib import Path
import matplotlib.pyplot as plt

def load(path):
    rows=[]
    for line in Path(path).read_text().splitlines():
        if line.strip():
            rows.append(json.loads(line))
    return rows

def to_int(x):
    try:
        return int(str(x))
    except Exception:
        return 0

def main():
    if len(sys.argv) < 3:
        print("usage: plot_tradeoff.py <combined.jsonl> <out.png>", file=sys.stderr)
        sys.exit(2)

    inp, outp = sys.argv[1], sys.argv[2]
    rows = load(inp)

    # Prefer selecting by tag produced in run_all.sh
    qd = [r for r in rows if r.get("tag") == "fio_qd_4k_rr"]
    if not qd:
        # fallback: select by jobfile substring
        qd = [r for r in rows if "sweep_qd_4k_random_read" in r.get("jobfile","")]

    if not qd:
        print("no qd sweep rows found", file=sys.stderr)
        sys.exit(1)

    qd_sorted = sorted(qd, key=lambda r: to_int(r.get("iodepth")))

    # latency: for randread use read_mean_us; fallback to write_mean_us
    x_lat = []
    y_iops = []
    labels = []
    for r in qd_sorted:
        lat = r.get("read_mean_us", 0.0) or r.get("write_mean_us", 0.0) or 0.0
        iops = r.get("total_iops", 0.0) or 0.0
        q = r.get("iodepth", "")
        x_lat.append(lat)
        y_iops.append(iops)
        labels.append(str(q))

    plt.figure()
    plt.plot(x_lat, y_iops, marker="o")
    for x, y, l in zip(x_lat, y_iops, labels):
        plt.annotate(f"QD{l}", (x, y), textcoords="offset points", xytext=(5, 5))
    plt.xlabel("Mean latency (us)")
    plt.ylabel("IOPS")
    plt.title("Throughput–Latency Trade-off (4KiB Random Read)")
    plt.tight_layout()
    Path(outp).parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(outp, dpi=200)

if __name__ == "__main__":
    main()
