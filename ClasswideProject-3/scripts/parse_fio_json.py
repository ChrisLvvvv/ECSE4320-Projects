import json
import sys
from pathlib import Path
import re

def ns_to_us(ns: float) -> float:
    return ns / 1000.0

def get_percentile(clat, pct):
    pmap = clat.get("percentile", {})
    key = f"{pct:.6f}"
    if key in pmap:
        return pmap[key]
    for k, v in pmap.items():
        try:
            if abs(float(k) - pct) < 1e-6:
                return v
        except Exception:
            pass
    return None

def infer_iodepth(opts, jobname):
    # 1) try job options
    if "iodepth" in opts:
        try:
            return int(opts["iodepth"])
        except Exception:
            pass
    # 2) infer from jobname like qd_16
    m = re.search(r"qd[_-]?(\d+)", jobname.lower())
    if m:
        return int(m.group(1))
    return None

def main():
    if len(sys.argv) < 3:
        print("usage: parse_fio_json.py <input.json> <output.jsonl> [tag]", file=sys.stderr)
        sys.exit(2)

    inp = Path(sys.argv[1])
    out = Path(sys.argv[2])
    tag = sys.argv[3] if len(sys.argv) >= 4 else ""

    data = json.loads(inp.read_text())
    jobs = data.get("jobs", [])
    rows = []

    for j in jobs:
        name = j.get("jobname", "")
        opts = j.get("job options", {})

        read = j.get("read", {})
        write = j.get("write", {})

        r_clat = read.get("clat_ns") or read.get("lat_ns") or {}
        w_clat = write.get("clat_ns") or write.get("lat_ns") or {}

        r_iops = float(read.get("iops", 0.0))
        w_iops = float(write.get("iops", 0.0))
        r_bw_kib = float(read.get("bw", 0.0))
        w_bw_kib = float(write.get("bw", 0.0))

        r_mean_ns = float(r_clat.get("mean", 0.0)) if r_clat else 0.0
        w_mean_ns = float(w_clat.get("mean", 0.0)) if w_clat else 0.0

        def pct_vals(clat):
            return {
                "p50_us": ns_to_us(get_percentile(clat, 50.0)) if get_percentile(clat, 50.0) else None,
                "p95_us": ns_to_us(get_percentile(clat, 95.0)) if get_percentile(clat, 95.0) else None,
                "p99_us": ns_to_us(get_percentile(clat, 99.0)) if get_percentile(clat, 99.0) else None,
                "p999_us": ns_to_us(get_percentile(clat, 99.9)) if get_percentile(clat, 99.9) else None,
            }

        row = {
            "tag": tag,
            "jobfile": inp.name,
            "jobname": name,
            "rw": opts.get("rw", ""),
            "bs": opts.get("bs", ""),
            "iodepth": infer_iodepth(opts, name),
            "numjobs": opts.get("numjobs", ""),
            "size": opts.get("size", ""),
            "read_iops": r_iops,
            "write_iops": w_iops,
            "total_iops": r_iops + w_iops,
            "read_bw_mib_s": r_bw_kib / 1024.0,
            "write_bw_mib_s": w_bw_kib / 1024.0,
            "total_bw_mib_s": (r_bw_kib + w_bw_kib) / 1024.0,
            "read_mean_us": ns_to_us(r_mean_ns) if r_mean_ns else 0.0,
            "write_mean_us": ns_to_us(w_mean_ns) if w_mean_ns else 0.0,
        }

        row.update({f"read_{k}": v for k, v in pct_vals(r_clat).items()})
        row.update({f"write_{k}": v for k, v in pct_vals(w_clat).items()})

        rows.append(row)

    out.parent.mkdir(parents=True, exist_ok=True)
    with out.open("w") as f:
        for r in rows:
            f.write(json.dumps(r) + "\n")

if __name__ == "__main__":
    main()
