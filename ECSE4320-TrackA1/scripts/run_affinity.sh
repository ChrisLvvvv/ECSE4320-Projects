#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

mkdir -p results/raw

make -s

RUNS=8
CMD="./results/raw/affinity_bench 300000000 256 30"

echo "time,pin,run,compute_sec,mem_sec" > results/raw/affinity.csv

run_case () {
  local pin="$1"  # "none" or cpu id like "0"
  local tag="$2"

  for i in $(seq 1 $RUNS); do
    if [ "$pin" = "none" ]; then
      out=$($CMD)
    else
      out=$(taskset -c "$pin" $CMD)
    fi

    # out: compute_sec=... mem_sec=...
    compute=$(echo "$out" | awk -F'compute_sec=' '{print $2}' | awk '{print $1}')
    mem=$(echo "$out" | awk -F'mem_sec=' '{print $2}' | awk '{print $1}')
    echo "$(date +%s),$tag,$i,$compute,$mem" >> results/raw/affinity.csv
    echo "pin=$tag run=$i compute=$compute mem=$mem"
  done
}

echo
echo "Case A: no pin"
run_case "none" "none"

echo
echo "Case B: pin cpu0"
run_case "0" "cpu0"

echo
echo "Saved: results/raw/affinity.csv"
