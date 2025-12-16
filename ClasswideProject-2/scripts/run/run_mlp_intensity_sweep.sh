#!/usr/bin/env bash
set -euo pipefail
BIN=bin/p2_bench
OUT=results/raw/mlp_intensity.jsonl
mkdir -p results/raw

bytes=$((256*1024*1024))
stride=64
iters=20000000
reps=3
chains_list=(1 2 4 8 16 32)

echo "Writing $OUT"
for c in "${chains_list[@]}"; do
  for r in $(seq 1 $reps); do
    taskset -c 0 $BIN --mode mlp --bytes $bytes --stride $stride --iters $iters --chains $c --out /tmp/t.jsonl
    cat /tmp/t.jsonl >> $OUT
  done
done
echo "Done."
