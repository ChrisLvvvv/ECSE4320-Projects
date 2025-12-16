#!/usr/bin/env bash
set -euo pipefail

: "${FIO_TARGET:?FIO_TARGET is not set. Refusing to run.}"

mkdir -p results/raw

QDS=(1 2 4 8 16 32 64 128 256)

for qd in "${QDS[@]}"; do
  out_json="results/raw/fio_qd_4k_rr_qd${qd}.json"
  out_jsonl="results/raw/fio_qd_4k_rr_qd${qd}.jsonl"
  echo "[RUN] QD=${qd} -> ${out_json}"

  fio --output-format=json --output="${out_json}" \
    --name="qd_${qd}" \
    --filename="${FIO_TARGET}" \
    --ioengine=libaio --direct=1 --invalidate=1 --group_reporting=1 \
    --time_based=1 --runtime=30 --ramp_time=5 \
    --rw=randread --bs=4k --size=16g \
    --iodepth="${qd}" --numjobs=1 \
    --randrepeat=0 --norandommap=1 \
    --percentile_list=50:95:99:99.9 \
    --refill_buffers=1 --buffer_pattern=0xdeadbeef

  python3 scripts/parse_fio_json.py "${out_json}" "${out_jsonl}" "fio_qd_4k_rr"
done
