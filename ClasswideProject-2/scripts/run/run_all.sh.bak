#!/usr/bin/env bash
set -euo pipefail

PIN="taskset -c 0"
mkdir -p results/raw results/tables results/figures

ts() { date -u +"%Y-%m-%dT%H:%M:%SZ"; }

have() { [[ -f "$1" ]]; }

latest() { ./scripts/run/latest.sh "$1"; }

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

# 1) pchase working-set sweep
if have results/figures/pchase_workingset_latency.png && have results/tables/pchase_workingset_summary.csv; then
  echo "== [$(ts)] 1) Working-set pchase sweep: SKIP (already have table+figure)"
else
  step "1) Working-set pchase sweep" $PIN ./scripts/run/run_workingset_sweep.sh
  PCHASE_IN=$(latest "results/raw/pchase_workingset_*.jsonl")
  step "1b) Parse pchase" python3 scripts/parse/summarize_pchase.py "$PCHASE_IN" results/tables/pchase_workingset_summary.csv
  step "1c) Plot pchase" python3 scripts/plot/plot_pchase_workingset.py \
    results/tables/pchase_workingset_summary.csv results/figures/pchase_workingset_latency.png
fi

# 2) stride seq
if have results/tables/stride_sweep_summary.csv && have results/figures/stride_bandwidth_read.png && have results/figures/stride_latency_read.png; then
  echo "== [$(ts)] 2) Stride sweep (seq): SKIP"
else
  step "2) Stride sweep (seq)" $PIN ./scripts/run/run_stride_sweep.sh
  STRIDE_SEQ_IN=$(latest "results/raw/stride_sweep_*.jsonl")
  step "2b) Parse stride seq" python3 scripts/parse/summarize_stride.py "$STRIDE_SEQ_IN" results/tables/stride_sweep_summary.csv
  step "2c) Plot stride seq bandwidth" python3 scripts/plot/plot_stride_bandwidth.py \
    results/tables/stride_sweep_summary.csv results/figures/stride_bandwidth_read.png
  step "2d) Plot stride seq latency" python3 scripts/plot/plot_stride_latency.py \
    results/tables/stride_sweep_summary.csv results/figures/stride_latency_read.png
fi

# 3) stride random + compare
if have results/tables/stride_seq_vs_random_summary.csv && have results/figures/stride_bandwidth_read_compare.png && have results/figures/stride_latency_read_compare.png; then
  echo "== [$(ts)] 3) Stride sweep (random)+compare: SKIP"
else
  step "3) Stride sweep (random)" $PIN ./scripts/run/run_stride_random_sweep.sh
  STRIDE_RND_IN=$(latest "results/raw/stride_random_sweep_*.jsonl")
  step "3b) Compare seq vs random" python3 scripts/parse/compare_stride_seq_random.py \
    results/tables/stride_sweep_summary.csv "$STRIDE_RND_IN" results/tables/stride_seq_vs_random_summary.csv
  step "3c) Plot compare bandwidth" python3 scripts/plot/plot_stride_compare_bandwidth.py \
    results/tables/stride_seq_vs_random_summary.csv results/figures/stride_bandwidth_read_compare.png
  step "3d) Plot compare latency" python3 scripts/plot/plot_stride_compare_latency.py \
    results/tables/stride_seq_vs_random_summary.csv results/figures/stride_latency_read_compare.png
fi

# 4) R/W mix
if have results/tables/rw_mix_summary.csv && have results/figures/rw_mix_bandwidth_stride64_seq.png; then
  echo "== [$(ts)] 4) R/W mix sweep: SKIP"
else
  step "4) R/W mix sweep" $PIN ./scripts/run/run_rw_mix_sweep.sh
  RW_IN=$(latest "results/raw/rw_mix_sweep_*.jsonl")
  step "4b) Parse rw mix" python3 scripts/parse/summarize_rw_mix.py "$RW_IN" results/tables/rw_mix_summary.csv
  for s in 64 256 1024; do
    step "4c) Plot rw mix bandwidth stride=$s" python3 scripts/plot/plot_rw_mix_bandwidth.py \
      results/tables/rw_mix_summary.csv $s "results/figures/rw_mix_bandwidth_stride${s}_seq.png"
    step "4d) Plot rw mix latency stride=$s" python3 scripts/plot/plot_rw_mix_latency.py \
      results/tables/rw_mix_summary.csv $s "results/figures/rw_mix_latency_stride${s}_seq.png"
  done
fi

# 5) MLP intensity
if have results/tables/mlp_intensity_summary.csv && have results/figures/mlp_throughput_latency_knee.png; then
  echo "== [$(ts)] 5) MLP intensity sweep: SKIP"
else
  step "5) Intensity sweep (MLP)" $PIN ./scripts/run/run_mlp_intensity_sweep.sh
  MLP_IN=$(latest "results/raw/mlp_intensity_*.jsonl")
  step "5b) Parse mlp" python3 scripts/parse/summarize_mlp.py "$MLP_IN" results/tables/mlp_intensity_summary.csv
  step "5c) Plot mlp knee" python3 scripts/plot/plot_mlp_knee.py \
    results/tables/mlp_intensity_summary.csv results/figures/mlp_throughput_latency_knee.png
fi

# 6) cache miss impact
if have results/tables/cache_miss_summary.csv && have results/figures/cache_miss_latency.png; then
  echo "== [$(ts)] 6) Cache miss sweep: SKIP"
else
  step "6) Cache-miss impact sweep" $PIN ./scripts/run/run_cache_miss_sweep.sh
  CACHE_IN=$(latest "results/raw/cache_miss_sweep_*.jsonl")
  step "6b) Parse cache miss" python3 scripts/parse/summarize_cache_miss.py "$CACHE_IN" results/tables/cache_miss_summary.csv
  step "6c) Plot cache miss" python3 scripts/plot/plot_cache_miss.py \
    results/tables/cache_miss_summary.csv results/figures/cache_miss_latency.png
fi

# 7) tlb miss impact
if have results/tables/tlb_miss_summary.csv && have results/figures/tlb_miss_latency.png; then
  echo "== [$(ts)] 7) TLB miss sweep: SKIP"
else
  step "7) TLB-miss impact sweep" $PIN ./scripts/run/run_tlb_miss_sweep.sh
  TLB_IN=$(latest "results/raw/tlb_miss_sweep_*.jsonl")
  step "7b) Parse tlb miss" python3 scripts/parse/summarize_tlb_miss.py "$TLB_IN" results/tables/tlb_miss_summary.csv
  step "7c) Plot tlb miss" python3 scripts/plot/plot_tlb_miss.py \
    results/tables/tlb_miss_summary.csv results/figures/tlb_miss_latency.png
fi

echo
echo "== [$(ts)] Done =="
echo "Tables:  results/tables/"
echo "Figures: results/figures/"
