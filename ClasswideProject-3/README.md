# Class-wide Project 3 — SSD Performance Profiling (fio)

This project profiles SSD performance using **fio**, focusing on the throughput–latency trade-off as queue depth
and workload characteristics change. All results are reproducible from scripts and raw outputs in this repository.

## Environment & Setup

- OS: Ubuntu (WSL2)
- Tool: fio
- Plotting: Python 3 + matplotlib
- Target: Dedicated test file (64 GiB)
  - `/home/chrislyu/ssdtest/fio_testfile`

All experiments use **direct I/O (`direct=1`)** to bypass the page cache. Runs are time-based with warm-up
(`runtime=30s`, `ramp_time=5s`) unless otherwise stated.

> **Safety note:** Benchmarks were run against a dedicated file, not a live partition.

## Methodology

Common configuration:
- `ioengine=libaio`, `numjobs=1`
- Random workloads disable reuse (`randrepeat=0`, `norandommap=1`)
- Latency percentiles: p50 / p95 / p99 / p99.9
- Fixed data pattern to reduce compressibility effects

fio concurrency (`iodepth`) is treated as the experimental knob. Host-side scripts run single-process.

## Experiments & Results

### 1. Zero-Queue Baselines (QD = 1)

Workloads:
- 4 KiB random read
- 4 KiB random write
- 128 KiB sequential read
- 128 KiB sequential write

Reported metrics:
- Mean latency
- p95 / p99 latency
- Throughput (IOPS for 4 KiB, MiB/s for 128 KiB)

Results table:
- `results/tables/baseline_qd1.csv`

These measurements isolate device behavior with minimal queueing.

### 2. Block Size Sweep (Pattern Fixed)

Block sizes:
- 4 KiB → 256 KiB

Patterns (separate runs):
- Random read
- Sequential read

From the same runs, we report throughput and mean latency to show the transition from IOPS-dominated
to bandwidth-dominated regimes.

Figures:
- `results/figures/blocksize_random_*`
- `results/figures/blocksize_sequential_*`

### 3. Read/Write Mix Sweep

Fixed conditions:
- 4 KiB random
- `iodepth=32`, `numjobs=1`

Mixes:
- 100% read
- 100% write
- 70/30
- 50/50

Figures:
- `results/figures/mix_4k_random_iops.png`
- `results/figures/mix_4k_random_lat.png`

Writes and mixed workloads show higher latency and variance due to buffering and flash program/erase behavior.

### 4. Queue-Depth Sweep (Throughput–Latency Trade-off)

Workload:
- 4 KiB random read

Queue depths:
- 1, 2, 4, 8, 16, 32, 64, 128, 256

Figure:
- `results/figures/tradeoff_4k_rr.png`

Throughput increases rapidly at low QD, then flattens while latency grows sharply. The knee appears
around **QD ≈ 32**, where additional concurrency yields diminishing throughput gains but significant
latency cost.

This behavior is consistent with Little’s Law: once the device saturates, increasing concurrency mainly
increases waiting time rather than service rate.

### 5. Tail Latency

Workload:
- 4 KiB random read

Queue depths:
- Mid-QD (16)
- Near-knee / high-QD (64)

Reported percentiles:
- p50 / p95 / p99 / p99.9

Artifacts:
- `results/tables/tail_latency.csv`
- `results/figures/tail_latency.png`

Tail latency widens significantly near saturation, highlighting the SLA impact of operating at high QD.

## Reproducibility

Main experiment suite:
```bash
./scripts/run_all.sh
```

Queue-depth sweep:
```bash
./scripts/run_qd_sweep.sh
```

All plots are regenerated from `results/raw/*.json` via parsing and plotting scripts.

## Limitations

- **WSL2 + file-backed target:** Results are collected inside WSL2 using a file target rather than a raw NVMe
  namespace. Absolute values may differ from bare-metal Linux, but relative trends and knees remain clear.
- **Device state effects:** Consumer SSD caching, background GC, and thermal behavior can affect steady-state
  results. A preconditioning fill job is provided for further analysis.

---
