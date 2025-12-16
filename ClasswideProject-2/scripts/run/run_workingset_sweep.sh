#!/usr/bin/env bash
set -euo pipefail

BIN="bin/p2_bench"
OUT_DIR="results/raw"
mkdir -p "${OUT_DIR}"

ts="$(date -u +%Y%m%dT%H%M%SZ)"
out="${OUT_DIR}/pchase_workingset_${ts}.jsonl"

# Sweep sizes (bytes): 4 KiB .. 512 MiB (log-like steps)
sizes=(
  4096 8192 16384 32768 65536
  131072 262144 524288 1048576
  2097152 4194304 8388608 16777216
  33554432 67108864 134217728 268435456 536870912
)

stride=64
iters=200000000
warmup=2000000

# Repeat each point N times for error bars.
reps=3

echo "Writing ${out}"
for b in "${sizes[@]}"; do
  for r in $(seq 1 "${reps}"); do
    taskset -c 0 "${BIN}" --mode pchase --bytes "${b}" --stride "${stride}" \
      --iters "${iters}" --warmup "${warmup}" --random --seed "${r}" \
      --out /tmp/pchase_tmp.jsonl
    # Append with extra fields for plotting.
    awk -v bytes="${b}" -v rep="${r}" '{print substr($0,1,length($0)-1) ",\"rep\":" rep ",\"bytes_req\":" bytes "}\n"}' \
      /tmp/pchase_tmp.jsonl >> "${out}"
  done
done

rm -f /tmp/pchase_tmp.jsonl
echo "Done."
