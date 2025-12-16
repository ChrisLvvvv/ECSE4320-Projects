#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p results/raw
make -s

RUNS=8
ITERS=350000000
MEM_MB=512
ROUNDS=35

# pick first sibling pair from lscpu -e
PAIR=$(lscpu -e=CPU,CORE | awk 'NR>1 {print $1","$2}' | sort -t, -k2,2n | head -n 2 | awk -F, '{print $1}' | tr '\n' ' ')
CPU_A=$(echo "$PAIR" | awk '{print $1}')
CPU_B=$(echo "$PAIR" | awk '{print $2}')

if [ -z "${CPU_A}" ] || [ -z "${CPU_B}" ]; then
  echo "Could not find CPU pair from lscpu output."
  exit 1
fi

echo "Using sibling CPUs: $CPU_A and $CPU_B (same core if SMT is enabled)."

echo "time,case,run,mode,time_sec" > results/raw/smt.csv

run_alone () {
  local mode="$1"
  local run="$2"
  local cmd

  if [ "$mode" = "compute" ]; then
    cmd="./results/raw/smt_bench compute $ITERS"
  else
    cmd="./results/raw/smt_bench mem $MEM_MB $ROUNDS"
  fi

  out=$(taskset -c "$CPU_A" $cmd)
  t=$(echo "$out" | awk -F'time_sec=' '{print $2}' | awk '{print $1}')
  echo "$(date +%s),alone,$run,$mode,$t" >> results/raw/smt.csv
  echo "alone mode=$mode run=$run time=$t"
}

run_with_interference () {
  local mode="$1"
  local run="$2"
  local cmdA cmdB

  if [ "$mode" = "compute" ]; then
    cmdA="./results/raw/smt_bench compute $ITERS"
    cmdB="./results/raw/smt_bench compute $ITERS"
  else
    cmdA="./results/raw/smt_bench mem $MEM_MB $ROUNDS"
    cmdB="./results/raw/smt_bench mem $MEM_MB $ROUNDS"
  fi

  # Start interferer on sibling CPU_B
  taskset -c "$CPU_B" $cmdB > /dev/null 2>&1 &
  pid=$!

  out=$(taskset -c "$CPU_A" $cmdA)

  wait $pid || true

  t=$(echo "$out" | awk -F'time_sec=' '{print $2}' | awk '{print $1}')
  echo "$(date +%s),interf,$run,$mode,$t" >> results/raw/smt.csv
  echo "interf mode=$mode run=$run time=$t"
}

echo
for mode in compute mem; do
  echo "Mode: $mode"
  for r in $(seq 1 $RUNS); do
    run_alone "$mode" "$r"
  done
  echo
  for r in $(seq 1 $RUNS); do
    run_with_interference "$mode" "$r"
  done
  echo
done

echo "Saved: results/raw/smt.csv"
