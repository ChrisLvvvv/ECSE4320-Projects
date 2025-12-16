#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RAW="${ROOT}/results/raw"
FIG="${ROOT}/results/figures"

mkdir -p "${RAW}" "${FIG}"

# bandwidth
: > "${RAW}/bw.jsonl"
make -C "${ROOT}" bw_native

# Use a few large sizes to be DRAM-resident (adjust down if your machine is tight on RAM)
Ns=(8388608 16777216 33554432)
REPS=7

for n in "${Ns[@]}"; do
  "${ROOT}/build/bw_native" --dtype f32 --n "${n}" --reps "${REPS}" --out "${RAW}/bw.jsonl"
done

python3 "${ROOT}/scripts/roofline.py" \
  "${RAW}/simd_project1.csv" \
  "${RAW}/bw.jsonl" \
  "${FIG}/roofline.png"
