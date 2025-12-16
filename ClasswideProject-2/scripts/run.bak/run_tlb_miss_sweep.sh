#!/usr/bin/env bash
set -euo pipefail
BIN=bin/p2_bench
OUT=results/raw/tlb_miss_sweep_$(date -u +%Y%m%dT%H%M%SZ).jsonl
mkdir -p results/raw

stride=4096
iters=50
reps=3
sizes=(4194304 8388608 16777216 33554432 67108864 134217728)

echo "Writing $OUT"
for sz in "${sizes[@]}"; do
  for r in $(seq 1 $reps); do
    taskset -c 0 $BIN \
      --mode stride \
      --bytes $sz \
      --stride $stride \
      --rw_pct 100 \
      --out /tmp/t.jsonl
    cat /tmp/t.jsonl >> $OUT
  done
done
echo "Done."
