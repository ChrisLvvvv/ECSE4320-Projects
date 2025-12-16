#!/usr/bin/env bash
set -euo pipefail
BIN=bin/p2_bench
OUT=results/raw/stride_sweep_$(date -u +%Y%m%dT%H%M%SZ).jsonl
mkdir -p results/raw

bytes=$((64*1024*1024))
iters=20
reps=3
strides=(64 256 1024)

echo "Writing $OUT"
for s in "${strides[@]}"; do
  for r in $(seq 1 $reps); do
    taskset -c 0 $BIN --mode stride --bytes $bytes --stride $s --iters $iters --out /tmp/t.jsonl
    cat /tmp/t.jsonl >> $OUT
  done
done
echo "Done."
