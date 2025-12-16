#!/usr/bin/env bash
set -euo pipefail

PIN="taskset -c 0"
mkdir -p results/raw results/tables results/figures

ts() { date -u +"%Y-%m-%dT%H:%M:%SZ"; }
have() { [[ -f "$1" ]]; }

step() {
  local name="$1"; shift
  echo
  echo "== [$(ts)] $name =="
  time "$@"
}

echo "== [$(ts)] Build =="
make -j build >/dev/null

echo "== [$(ts)] Tool availability =="
./scripts/env/check_tools.sh || true

# Canonical raw outputs (no timestamps)
RAW_PCHASE="results/raw/pchase_workingset.jsonl"
RAW_STRIDE_SEQ="results/raw/stride_seq.jsonl"
RAW_STRIDE_RND="results/raw/stride_random.jsonl"
RAW_RWMIX="results/raw/rw_mix.jsonl"
RAW_MLP="results/raw/mlp_intensity.jsonl"
RAW_CACHE="results/raw/cache_miss.jsonl"
RAW_TLB="results/raw/tlb_miss.jsonl"

# 1) Working-set pchase
if have results/tables/pchase_workingset_summary.csv && have results/figures/pchase_workingset_latency.png; then
  echo "== [$(ts)] 1) Working-set pchase: SKIP"
else
  step "1) Working-set pchase" $PIN ./scripts/run/run_workingset_sweep.sh --out "$RAW_PCHASE"
  step "1b) Parse pchase" python3 scripts/parse/summarize_pchase.py "$RAW_PCHASE" results/tables/pchase_workingset_summary.csv
  step "1c) Plot pchase" python3 scripts/plot/plot_pchase_workingset.py \
    results/tables/pchase_workingset_summary.csv results/figures/pchase_workingset_latency.png
fi

# 2) Stride sweep (sequential)
if have results/tables/stride_sweep_summary.csv && have results/figures/stride_bandwidth_read.png && have results/figures/stride_latency_read.png; then
  echo "== [$(ts)] 2) Stride sweep (seq): SKIP"
else
  step "2) Stride sweep (seq)" $PIN ./scripts/run/run_stride_sweep.sh --out "$RAW_STRIDE_SEQ"
  step "2b) Parse stride seq" python3 scripts/parse/summarize_stride.py "$RAW_STRIDE_SEQ" results/tables/stride_sweep_summary.csv
  step "2c) Plot stride seq bandwidth" python3 scripts/plot/plot_stride_bandwidth.py \
    results/tables/stride_sweep_summary.csv results/figures/stride_bandwidth_read.png
  step "2d) Plot stride seq latency" python3 scripts/plot/plot_stride_latency.py \
    results/tables/stride_sweep_summary.csv results/figures/stride_latency_read.png
fi

# 3) Stride sweep (random) + compare
if have results/tables/stride_seq_vs_random_summary.csv && have results/figures/stride_bandwidth_read_compare.png && have results/figures/stride_latency_read_compare.png; then
  echo "== [$(ts)] 3) Stride sweep (random)+compare: SKIP"
else
  step "3) Stride sweep (random)" $PIN ./scripts/run/run_stride_random_sweep.sh --out "$RAW_STRIDE_RND"
  step "3b) Compare seq vs random" python3 scripts/parse/compare_stride_seq_random.py \
    results/tables/stride_sweep_summary.csv "$RAW_STRIDE_RND" results/tables/stride_seq_vs_random_summary.csv
  step "3c) Plot compare bandwidth" python3 scripts/plot/plot_stride_compare_bandwidth.py \
    results/tables/stride_seq_vs_random_summary.csv results/figures/stride_bandwidth_read_compare.png
  step "3d) Plot compare latency" python3 scripts/plot/plot_stride_compare_latency.py \
    results/tables/stride_seq_vs_random_summary.csv results/figures/stride_latency_read_compare.png
fi

# 4) R/W mix
if have results/tables/rw_mix_summary.csv && have results/figures/rw_mix_bandwidth_stride64_seq.png; then
  echo "== [$(ts)] 4) R/W mix: SKIP"
else
  step "4) R/W mix sweep" $PIN ./scripts/run/run_rw_mix_sweep.sh --out "$RAW_RWMIX"
  step "4b) Parse rw mix" python3 scripts/parse/summarize_rw_mix.py "$RAW_RWMIX" results/tables/rw_mix_summary.csv
  for s in 64 256 1024; do
    step "4c) Plot rw mix bandwidth stride=$s" python3 scripts/plot/plot_rw_mix_bandwidth.py \
      results/tables/rw_mix_summary.csv $s "results/figures/rw_mix_bandwidth_stride${s}_seq.png"
    step "4d) Plot rw mix latency stride=$s" python3 scripts/plot/plot_rw_mix_latency.py \
      results/tables/rw_mix_summary.csv $s "results/figures/rw_mix_latency_stride${s}_seq.png"
  done
fi

# 5) MLP intensity sweep
if have results/tables/mlp_intensity_summary.csv && have results/figures/mlp_throughput_latency_knee.png; then
  echo "== [$(ts)] 5) MLP intensity: SKIP"
else
  step "5) MLP intensity sweep" $PIN ./scripts/run/run_mlp_intensity_sweep.sh --out "$RAW_MLP"
  step "5b) Parse mlp" python3 scripts/parse/summarize_mlp.py "$RAW_MLP" results/tables/mlp_intensity_summary.csv
  step "5c) Plot mlp knee" python3 scripts/plot/plot_mlp_knee.py \
    results/tables/mlp_intensity_summary.csv results/figures/mlp_throughput_latency_knee.png
fi

# 6) Cache-miss sweep
if have results/tables/cache_miss_summary.csv && have results/figures/cache_miss_latency.png; then
  echo "== [$(ts)] 6) Cache miss: SKIP"
else
  step "6) Cache-miss sweep" $PIN ./scripts/run/run_cache_miss_sweep.sh --out "$RAW_CACHE"
  step "6b) Parse cache miss" python3 scripts/parse/summarize_cache_miss.py "$RAW_CACHE" results/tables/cache_miss_summary.csv
  step "6c) Plot cache miss" python3 scripts/plot/plot_cache_miss.py \
    results/tables/cache_miss_summary.csv results/figures/cache_miss_latency.png
fi

# 7) TLB-miss sweep
if have results/tables/tlb_miss_summary.csv && have results/figures/tlb_miss_latency.png; then
  echo "== [$(ts)] 7) TLB miss: SKIP"
else
  step "7) TLB-miss sweep" $PIN ./scripts/run/run_tlb_miss_sweep.sh --out "$RAW_TLB"
  step "7b) Parse tlb miss" python3 scripts/parse/summarize_tlb_miss.py "$RAW_TLB" results/tables/tlb_miss_summary.csv
  step "7c) Plot tlb miss" python3 scripts/plot/plot_tlb_miss.py \
    results/tables/tlb_miss_summary.csv results/figures/tlb_miss_latency.png
fi

echo
echo "== [$(ts)] Done =="
