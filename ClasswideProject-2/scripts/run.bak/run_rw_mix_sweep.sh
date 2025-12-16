#!/usr/bin/env bash
set -euo pipefail
BIN=bin/p2_bench
OUT=results/raw/rw_mix_sweep_$(date -u +%Y%m%dT%H%M%SZ).jsonl
mkdir -p results/raw

bytes=$((64*1024*1024))
iters=20
reps=3
strides=(64 256 1024)
mixes=(100 0 70 50) # read percentage

echo "Writing $OUT"
for s in "${strides[@]}"; do
  for m in "${mixes[@]}"; do
    for r in $(seq 1 $reps); do
      taskset -c 0 $BIN --mode stride --bytes $bytes --stride $s --iters $iters --rw_pct $m --out /tmp/t.jsonl
      cat /tmp/t.jsonl >> $OUT
    done
  done
done
echo "Done."
