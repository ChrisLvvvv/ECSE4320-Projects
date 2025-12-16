#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p results/raw
make -s

MB=512
ROUNDS=40
RUNS=6

echo "time,mode,run,time_sec,bandwidth" > results/raw/numa.csv

run_case () {
  local mode="$1"
  local cmd="$2"

  for r in $(seq 1 $RUNS); do
    out=$($cmd)
    time=$(echo "$out" | awk -F'time_sec=' '{print $2}' | awk '{print $1}')
    bw=$(echo "$out" | awk -F'bandwidth=' '{print $2}' | awk '{print $1}')
    echo "$(date +%s),$mode,$r,$time,$bw" >> results/raw/numa.csv
    echo "mode=$mode run=$r time=$time bw=$bw"
  done
}

echo
echo "Case A: default allocation"
run_case "default" "./results/raw/numa_bench $MB $ROUNDS"

echo
echo "Case B: numactl cpu=0 mem=0"
run_case "node0" "numactl --cpunodebind=0 --membind=0 ./results/raw/numa_bench $MB $ROUNDS"

echo
echo "Saved: results/raw/numa.csv"
