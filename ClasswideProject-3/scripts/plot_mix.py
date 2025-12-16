import json, sys
from pathlib import Path
import matplotlib.pyplot as plt

def load(path):
    rows=[]
    for line in Path(path).read_text().splitlines():
        if line.strip():
            rows.append(json.loads(line))
    return rows

def main():
    if len(sys.argv) < 3:
        print("usage: plot_mix.py <combined.jsonl> <out.png>", file=sys.stderr)
        sys.exit(2)

    rows = load(sys.argv[1])
    mix = [r for r in rows if r.get("jobfile","").startswith("fio_mix_4k_random")]
    if not mix:
        print("no mix rows found", file=sys.stderr)
        sys.exit(1)

    # Stable order
    order = ["mix_100r","mix_100w","mix_70r30w","mix_50r50w"]
    mix_sorted = sorted(mix, key=lambda r: order.index(r.get("jobname","")) if r.get("jobname","") in order else 999)

    labels = [r["jobname"].replace("mix_","") for r in mix_sorted]
    iops = [r.get("total_iops",0.0) for r in mix_sorted]
    lat = []
    for r in mix_sorted:
        # for mixed, use read_mean_us as proxy; could also compute weighted, but keep simple/clear
        lat.append(r.get("read_mean_us",0.0) or r.get("write_mean_us",0.0))

    plt.figure()
    plt.plot(labels, iops, marker="o")
    plt.xlabel("Read/Write mix")
    plt.ylabel("IOPS")
    plt.title("Mix sweep (4KiB Random, QD=32)")
    plt.tight_layout()
    Path(sys.argv[2]).parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(sys.argv[2].replace(".png","_iops.png"), dpi=200)

    plt.figure()
    plt.plot(labels, lat, marker="o")
    plt.xlabel("Read/Write mix")
    plt.ylabel("Mean latency (us)")
    plt.title("Mix sweep (4KiB Random, QD=32)")
    plt.tight_layout()
    plt.savefig(sys.argv[2].replace(".png","_lat.png"), dpi=200)

if __name__=="__main__":
    main()
