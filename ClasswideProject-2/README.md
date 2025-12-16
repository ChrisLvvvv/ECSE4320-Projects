# Project 2 — Cache & Memory Performance Profiling

This project characterizes cache and DRAM behavior on a modern x86 CPU by measuring latency, bandwidth,
and their interaction under different access patterns. All experiments are single-threaded and focus
on exposing memory-hierarchy effects rather than parallel scaling.

---

## Environment

- OS: Ubuntu (WSL2)
- CPU: Intel Core i7-12700F (Alder Lake)
- Compiler: g++ (C++20)
- Build flags: `-O3 -march=native`
- Execution:
  - Single-threaded
  - Core pinned using `taskset`
  - NUMA node local
- Timing:
  - Wall-clock timing using a lightweight cycle-based timer
  - Each configuration repeated ≥3 times (mean ± std reported)

---

## Repository Structure

ClasswideProject-2/
├── src/ # C++20 microbenchmarks
│ ├── bench/ # timing utilities
│ └── mem/ # memory access kernels
├── include/ # headers
├── scripts/
│ ├── parse/ # JSONL → CSV summarization
│ └── plot/ # plotting scripts
├── results/
│ ├── raw/ # canonical raw measurements (JSONL)
│ ├── tables/ # summarized CSV tables
│ └── figures/ # final plots
└── README.md


Only canonical raw files (without timestamps), final tables, and figures are tracked.

---

## Experiments

### 1. Zero-Queue / Working-Set Latency (Pointer Chasing)

A pointer-chasing benchmark is used to measure effective load latency as the working set grows.
Randomized pointer traversal prevents hardware prefetching and enforces true dependency chains.

**Goal**
- Identify latency plateaus corresponding to L1, L2, L3, and DRAM.

**Output**
- Raw: `results/raw/pchase_workingset.jsonl`
- Table: `results/tables/pchase_workingset_summary.csv`
- Figure: `results/figures/pchase_workingset_latency.png`

---

### 2. Access Pattern & Stride Sweep (Sequential)

A streaming memory kernel reads a fixed-size buffer with increasing strides.

**Parameters**
- Strides: ~64B, ~256B, ~1024B
- Read-only accesses

**Goal**
- Observe spatial locality, cache line utilization, and bandwidth scaling.

**Output**
- Raw: `results/raw/stride_seq.jsonl`
- Table: `results/tables/stride_sweep_summary.csv`
- Figures:
  - `stride_bandwidth_read.png`
  - `stride_latency_read.png`

---

### 3. Sequential vs Random Access Comparison

The same stride sweep is repeated with randomized access order.

**Goal**
- Quantify the effect of prefetching and spatial locality.

**Output**
- Raw: `results/raw/stride_random.jsonl`
- Table: `results/tables/stride_seq_vs_random_summary.csv`
- Figures:
  - `stride_bandwidth_read_compare.png`
  - `stride_latency_read_compare.png`

---

### 4. Read/Write Mix Sweep

The stride kernel is extended to include configurable read/write ratios.

**Ratios Tested**
- 100% Read
- 70% Read / 30% Write
- 50% Read / 50% Write
- 100% Write

**Goal**
- Study write-allocate effects and store buffer pressure.

**Output**
- Raw: `results/raw/rw_mix.jsonl`
- Table: `results/tables/rw_mix_summary.csv`
- Figures:
  - `rw_mix_bandwidth_stride*.png`
  - `rw_mix_latency_stride*.png`

---

### 5. Throughput–Latency Trade-off (MLP Sweep)

Multiple independent pointer-chasing chains are used to increase memory-level parallelism (MLP).

**Parameters**
- Number of chains: 1 → 32
- Fixed stride and working set in DRAM

**Goal**
- Demonstrate the throughput–latency trade-off
- Identify the saturation “knee” using Little’s Law

**Output**
- Raw: `results/raw/mlp_intensity.jsonl`
- Table: `results/tables/mlp_intensity_summary.csv`
- Figure: `results/figures/mlp_throughput_latency_knee.png`

---

### 6. Cache Miss Impact

The streaming kernel is executed with increasing working-set sizes to induce cache misses.

**Goal**
- Correlate performance degradation with cache miss behavior
- Interpret results using the AMAT model

**Output**
- Raw: `results/raw/cache_miss.jsonl`
- Table: `results/tables/cache_miss_summary.csv`
- Figure: `results/figures/cache_miss_latency.png`

---

### 7. TLB Miss Impact

A page-stride access pattern (4KB stride) is used to stress the DTLB.

**Goal**
- Measure the effect of limited TLB reach
- Observe latency increases once page working set exceeds TLB capacity

**Output**
- Raw: `results/raw/tlb_miss.jsonl`
- Table: `results/tables/tlb_miss_summary.csv`
- Figure: `results/figures/tlb_miss_latency.png`

---

## Methodological Notes

- CPU frequency scaling was left enabled; repeated runs reduce transient noise.
- Hardware prefetchers significantly affect sequential patterns and are intentionally suppressed in
  pointer-chase and random-access experiments.
- Results reflect effective latency under dependency constraints rather than architectural minimums.

---

## Limitations

- Measurements are performed under WSL2; absolute values may differ from bare-metal Linux.
- `perf` counters and Intel MLC are not available in this environment.
- Results focus on relative trends rather than peak advertised bandwidth.

---

## Summary

This project exposes key properties of the memory hierarchy:
- Distinct cache and DRAM latency regimes
- The importance of spatial locality and prefetching
- Write traffic asymmetry
- The throughput–latency trade-off governed by memory-level parallelism
- Performance penalties from cache and TLB misses

All experiments are reproducible using the provided source code, scripts, and canonical raw data.
