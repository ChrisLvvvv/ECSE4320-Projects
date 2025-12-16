#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="${ROOT}/build"
RAW="${ROOT}/results/raw"
FIG="${ROOT}/results/figures"

mkdir -p "${RAW}" "${FIG}"

OUT="${RAW}/simd_project1.jsonl"
: > "${OUT}"

REPS=7

Ns=(1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576 2097152 4194304 8388608)
kernels=(axpy dot stencil)
dtypes=(f32 f64)
builds=(bench_scalar bench_auto bench_native)

run_one() {
  local exe="$1" kernel="$2" dtype="$3" n="$4" stride="$5" misalign="$6" tail="$7"
  "${BIN}/${exe}" --kernel "${kernel}" --dtype "${dtype}" --n "${n}" --stride "${stride}" \
    --misalign "${misalign}" --tail "${tail}" --reps "${REPS}" --out "${OUT}"
}

# 1) Baseline scalar vs SIMD across N (unit-stride, aligned, no tail)
for exe in "${builds[@]}"; do
  for kernel in "${kernels[@]}"; do
    for dtype in "${dtypes[@]}"; do
      for n in "${Ns[@]}"; do
        run_one "${exe}" "${kernel}" "${dtype}" "${n}" 1 0 0
      done
    done
  done
done

# 3) Alignment & tail handling (representative N)
N_ALIGN=1048576
for exe in "${builds[@]}"; do
  for dtype in "${dtypes[@]}"; do
    for kernel in axpy dot; do
      run_one "${exe}" "${kernel}" "${dtype}" "${N_ALIGN}" 1 0 0
      run_one "${exe}" "${kernel}" "${dtype}" "${N_ALIGN}" 1 4 0
      run_one "${exe}" "${kernel}" "${dtype}" "${N_ALIGN}" 1 0 1
      run_one "${exe}" "${kernel}" "${dtype}" "${N_ALIGN}" 1 4 1
    done
  done
done

# 4) Stride effects
N_STRIDE=2097152
strides=(1 2 4 8)
for exe in "${builds[@]}"; do
  for dtype in "${dtypes[@]}"; do
    for kernel in axpy dot; do
      for s in "${strides[@]}"; do
        run_one "${exe}" "${kernel}" "${dtype}" "${N_STRIDE}" "${s}" 0 0
      done
    done
  done
done

python3 "${ROOT}/scripts/parse.py" "${OUT}" "${RAW}/simd_project1.csv"
python3 "${ROOT}/scripts/plot.py" "${RAW}/simd_project1.csv" "${FIG}"
