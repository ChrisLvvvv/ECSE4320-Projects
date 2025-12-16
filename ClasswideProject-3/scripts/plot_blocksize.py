import json, sys
from pathlib import Path
import matplotlib.pyplot as plt

def load(path):
    rows=[]
    for line in Path(path).read_text().splitlines():
        if line.strip():
            rows.append(json.loads(line))
    return rows

def bs_to_k(bs):
    # "4k","128k" -> 4,128
    bs = (bs or "").lower().strip()
    if bs.endswith("k"):
        return int(bs[:-1])
    if bs.endswith("m"):
        return int(float(bs[:-1]) * 1024)
    return None

def plot(rows, title, outp):
    rows = [r for r in rows if r.get("bs")]
    rows = sorted(rows, key=lambda r: bs_to_k(r.get("bs")) or 0)
    x = [bs_to_k(r["bs"]) for r in rows]

    iops = [r.get("total_iops",0.0) for r in rows]
    bw = [r.get("total_bw_mib_s",0.0) for r in rows]
    lat = [r.get("read_mean_us",0.0) if "read" in (r.get("rw","")) else (r.get("write_mean_us",0.0) or r.get("read_mean_us",0.0)) for r in rows]

    # Two plots: throughput vs bs, latency vs bs
    plt.figure()
    plt.plot(x, iops, marker="o")
    plt.xlabel("Block size (KiB)")
    plt.ylabel("IOPS")
    plt.title(title + " (IOPS)")
    plt.tight_layout()
    Path(outp).parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(outp.replace(".png","_iops.png"), dpi=200)

    plt.figure()
    plt.plot(x, bw, marker="o")
    plt.xlabel("Block size (KiB)")
    plt.ylabel("Throughput (MiB/s)")
    plt.title(title + " (MiB/s)")
    plt.tight_layout()
    plt.savefig(outp.replace(".png","_bw.png"), dpi=200)

    plt.figure()
    plt.plot(x, lat, marker="o")
    plt.xlabel("Block size (KiB)")
    plt.ylabel("Mean latency (us)")
    plt.title(title + " (Latency)")
    plt.tight_layout()
    plt.savefig(outp.replace(".png","_lat.png"), dpi=200)

def main():
    if len(sys.argv) < 3:
        print("usage: plot_blocksize.py <combined.jsonl> <out_prefix.png>", file=sys.stderr)
        sys.exit(2)

    rows = load(sys.argv[1])
    out_prefix = sys.argv[2]

    rnd = [r for r in rows if r.get("jobfile","").startswith("fio_blocksize_random")]
    seq = [r for r in rows if r.get("jobfile","").startswith("fio_blocksize_sequential")]

    if rnd:
        plot(rnd, "Block-size sweep: Random (QD=32, numjobs=1)", str(Path(out_prefix).with_name("blocksize_random.png")))
    if seq:
        plot(seq, "Block-size sweep: Sequential (QD=32, numjobs=1)", str(Path(out_prefix).with_name("blocksize_sequential.png")))

if __name__=="__main__":
    main()
