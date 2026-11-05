#!/usr/bin/env bash
set -euo pipefail

BIN=./bin/a4
OUT=results/raw/a4_bench.csv

mkdir -p results/raw

# Write header
echo "impl,work,threads,nkeys,keyspace,ops,rep,seconds,ops_per_sec" > "$OUT"

# Experiment grid
THREADS_LIST=(1 2 4 8 16)
NKEYS_LIST=(10000 100000 1000000)
WORK_LIST=(lookup insert mixed)

OPS=2000000
KEYSPACE_FACTOR=2
REPS=3

# Sanity check: binary exists
if [[ ! -x "$BIN" ]]; then
  echo "ERROR: $BIN not found. Run: make -j" >&2
  exit 1
fi

for nkeys in "${NKEYS_LIST[@]}"; do
  keyspace=$((KEYSPACE_FACTOR * nkeys))
  for work in "${WORK_LIST[@]}"; do
    for th in "${THREADS_LIST[@]}"; do
      for impl in coarse striped; do
        echo "Running impl=$impl work=$work threads=$th nkeys=$nkeys keyspace=$keyspace"
        $BIN --impl=$impl --work=$work --threads=$th --ops=$OPS --nkeys=$nkeys --keyspace=$keyspace --reps=$REPS \
          | tail -n +2 >> "$OUT"
      done
    done
  done
done

echo "Wrote $OUT"
