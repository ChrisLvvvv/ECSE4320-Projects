#!/usr/bin/env bash
set -euo pipefail

: "${FIO_TARGET:?FIO_TARGET is not set. Refusing to run.}"

mkdir -p results/raw results/tables results/figures

# Metadata
scripts/env_report.sh results/raw/env.meta.txt

run_and_parse() {
  local jobfile="$1"
  local tag="$2"
  local base
  base="$(basename "$jobfile" .fio)"
  local out_json="results/raw/${tag}_${base}.json"
  local out_jsonl="results/raw/${tag}_${base}.jsonl"

  echo "[RUN] $jobfile -> $out_json"
  scripts/run_one.sh "$jobfile" "$out_json"
  python3 scripts/parse_fio_json.py "$out_json" "$out_jsonl" "$tag"
}

# Optional: precondition (steady-state) - safe only if target is dedicated.
# Uncomment if you want steady-state for random write analysis.
# run_and_parse jobs/precondition_fill.fio precond

# Required experiments
run_and_parse jobs/baseline_qd1.fio baseline
run_and_parse jobs/sweep_blocksize_random.fio fio_blocksize_random
run_and_parse jobs/sweep_blocksize_sequential.fio fio_blocksize_sequential
run_and_parse jobs/sweep_mix_4k_random.fio fio_mix_4k_random
run_and_parse jobs/sweep_qd_4k_random_read.fio fio_qd_4k_rr
run_and_parse jobs/tail_latency.fio fio_tail_latency

# Combine jsonl
cat results/raw/*.jsonl > results/raw/combined.jsonl

# Tables
python3 scripts/summarize.py results/raw/combined.jsonl results/tables

# Figures
python3 scripts/plot_tradeoff.py results/raw/combined.jsonl results/figures/tradeoff_4k_rr.png
python3 scripts/plot_blocksize.py results/raw/combined.jsonl results/figures/_blocksize_prefix.png
python3 scripts/plot_mix.py results/raw/combined.jsonl results/figures/mix_4k_random.png
python3 scripts/plot_tail.py results/raw/combined.jsonl results/tables/tail_latency.csv results/figures/tail_latency.png

echo
echo "Done."
echo "Raw:     results/raw/"
echo "Tables:  results/tables/"
echo "Figures: results/figures/"
