#!/usr/bin/env bash
set -euo pipefail
mkdir -p results/raw

echo "[1/3] Dump env"
./env/dump_env.sh

echo "[2/3] Build bench"
gcc -O2 -march=native -o results/raw/hello_bench src/hello_bench.c

echo "[3/3] Run bench + perf stat"
./results/raw/hello_bench | tee results/raw/hello_bench.txt
perf stat -o results/raw/hello_bench.perf.txt ./results/raw/hello_bench || true

echo "Done. See results/raw/* and results/env.txt"
