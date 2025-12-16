#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p results/raw

make -s

CPU=$(nproc)
RUNS=6
ITERS=250000000

# thread list: 1..2xCPU, plus 3xCPU
THREADS=""
for t in 1 2 4 8 12 16 24 32; do THREADS="$THREADS $t"; done
# also add CPU, 2CPU, 3CPU (unique)
THREADS="$THREADS $CPU $((2*CPU)) $((3*CPU))"

# remove duplicates by awk
THREADS=$(echo "$THREADS" | tr ' ' '\n' | awk 'NF' | sort -n | uniq | tr '\n' ' ')

echo "time,pin,threads,run,time_sec,gops" > results/raw/oversub.csv

run_one () {
  local pin="$1"   # none or cpu-range like 0-7
  local tag="$2"
  local threads="$3"
  local run="$4"

  local cmd="./results/raw/oversub_bench $threads $ITERS"
  local out

  if [ "$pin" = "none" ]; then
    out=$($cmd)
  else
    out=$(taskset -c "$pin" $cmd)
  fi

  local time_sec
  local gops
  time_sec=$(echo "$out" | awk -F'time_sec=' '{print $2}' | awk '{print $1}')
  gops=$(echo "$out" | awk -F'gops=' '{print $2}' | awk '{print $1}')

  echo "$(date +%s),$tag,$threads,$run,$time_sec,$gops" >> results/raw/oversub.csv
  echo "pin=$tag threads=$threads run=$run time=$time_sec gops=$gops"
}

PIN_RANGE="0-$((CPU-1))"

echo "CPU=$CPU ITERS=$ITERS RUNS=$RUNS"
echo "pin range for pinned case: $PIN_RANGE"
echo

for threads in $THREADS; do
  for r in $(seq 1 $RUNS); do
    run_one "none" "none" "$threads" "$r"
  done
done

echo
for threads in $THREADS; do
  for r in $(seq 1 $RUNS); do
    run_one "$PIN_RANGE" "pinned" "$threads" "$r"
  done
done

echo
echo "Saved: results/raw/oversub.csv"
