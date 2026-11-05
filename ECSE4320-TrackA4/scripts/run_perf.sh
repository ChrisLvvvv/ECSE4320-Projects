#!/usr/bin/env bash
set -euo pipefail

BIN=./bin/a4
OUT=results/raw/a4_perf.csv

mkdir -p results/raw

# perf events (LLC events may vary by CPU; these are common on Intel)
EVENTS="cycles,instructions,LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses"

# CSV header
echo "impl,work,threads,nkeys,keyspace,ops,rep,seconds,ops_per_sec,cycles,instructions,LLC_loads,LLC_load_misses,LLC_stores,LLC_store_misses" > "$OUT"

THREADS_LIST=(1 2 4 8 16)
NKEYS_LIST=(10000 100000 1000000)
WORK_LIST=(lookup insert mixed)

OPS=2000000
KEYSPACE_FACTOR=2
REPS=3

for nkeys in "${NKEYS_LIST[@]}"; do
  keyspace=$((KEYSPACE_FACTOR * nkeys))
  for work in "${WORK_LIST[@]}"; do
    for th in "${THREADS_LIST[@]}"; do
      for impl in coarse striped; do
        for rep in $(seq 0 $((REPS-1))); do
          echo "perf impl=$impl work=$work th=$th nkeys=$nkeys rep=$rep"

          # Run the program once and capture its CSV (one data row)
          PROG_OUT=$($BIN --impl=$impl --work=$work --threads=$th --ops=$OPS --nkeys=$nkeys --keyspace=$keyspace --reps=1 | tail -n 1)

          # Run perf around the program (same args), parse perf CSV output.
          # Note: perf prints to stderr by default.
          PERF_OUT=$(perf stat -x, -e "$EVENTS" -- \
            $BIN --impl=$impl --work=$work --threads=$th --ops=$OPS --nkeys=$nkeys --keyspace=$keyspace --reps=1 \
            2>&1 >/dev/null)

          # Extract numeric values from perf CSV lines: value,event,...
          # Remove commas in big numbers.
          get_val() {
            local name="$1"
            echo "$PERF_OUT" | awk -F, -v ev="$name" '$2==ev {gsub(/,/, "", $1); print $1; exit}'
          }

          cycles=$(get_val "cycles")
          inst=$(get_val "instructions")
          llc_loads=$(get_val "LLC-loads")
          llc_load_misses=$(get_val "LLC-load-misses")
          llc_stores=$(get_val "LLC-stores")
          llc_store_misses=$(get_val "LLC-store-misses")

          echo "${PROG_OUT},${cycles},${inst},${llc_loads},${llc_load_misses},${llc_stores},${llc_store_misses}" >> "$OUT"
        done
      done
    done
  done
done

echo "Wrote $OUT"
