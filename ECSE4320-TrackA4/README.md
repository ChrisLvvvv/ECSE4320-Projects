# Project A4: Concurrent Data Structures and Memory Coherence  
## Coarse-Grained vs. Striped Locking Analysis

This project examines how synchronization design impacts the scalability of shared-memory data structures on modern multicore systems. Although hardware cache coherence guarantees correctness at the cache-line level, overall performance is largely determined by how software synchronization is implemented. By building a concurrent hash table with two different locking strategies—Coarse-Grained locking and Striped locking—this project evaluates how lock granularity affects throughput and speedup. The results show that achieving atomic correctness alone is not sufficient for scalable parallel performance, and the observed behavior can be explained using Amdahl’s Law and cache-coherence effects.

## Implementation and Design

The core data structure is a thread-safe chained hash table supporting `insert`, `lookup`, and `erase` operations.  
The **Coarse-Grained** implementation serves as the baseline and uses a single global mutex to protect the entire table. This approach is straightforward and deadlock-free, but it effectively serializes all operations, turning the structure into a performance bottleneck under concurrency.

The **Striped Locking** implementation improves concurrency by dividing the table into multiple independent stripes, each guarded by its own mutex. An operation only acquires the lock corresponding to the stripe of the target key. Since each operation holds at most one mutex, deadlock is impossible. This design allows concurrent access to disjoint regions of the table and reduces the likelihood of lock contention.

## Methodology

All experiments were conducted on a Linux x86-64 system using GCC and pthreads. Three workload types were evaluated:

- **Lookup-only:** read-dominated workload  
- **Insert-only:** write-dominated stress test  
- **Mixed:** 70% lookup and 30% insert  

The benchmark sweeps thread counts from 1 to 16 and dataset sizes from \(10^4\) to \(10^6\) keys. Throughput (operations per second) and relative speedup are measured to characterize scaling behavior. Each configuration is executed multiple times, and mean performance values are reported to reduce noise.

## Results and Analysis

### Throughput and Scalability Limits

The throughput results reveal a clear divergence between the two synchronization strategies. The Coarse-Grained implementation quickly reaches a performance wall: adding threads provides little benefit and often results in throughput degradation. This behavior is visible across all workloads and becomes more pronounced as thread count increases.

In contrast, the Striped implementation shows clear throughput improvements, particularly for lookup-heavy and mixed workloads. However, as the dataset size grows to \(10^6\) keys, throughput decreases for both designs. This indicates that memory hierarchy effects, such as last-level cache misses and memory latency, begin to dominate execution time regardless of the locking strategy.

### Speedup in the Lookup-Only Case

The lookup-only workload represents an ideal scenario for shared-memory concurrency, yet it highlights the severity of the coarse-grained bottleneck. Under coarse locking, even read-only operations are serialized, leading to speedup values consistently below 1.0 as more threads are added. The Striped implementation allows lookups on different buckets to proceed in parallel and achieves a speedup of roughly 3× at 16 threads. While this is a significant improvement, it remains sub-linear, suggesting that memory bandwidth and lock acquisition overhead limit further scaling.

### Cost of Writes and Coherence Effects

The insert-only workload places the highest stress on the memory system. Writes trigger cache coherence traffic, causing cache lines to be invalidated across cores. In the Coarse-Grained design, the global mutex becomes a hotspot, and the cache line containing the lock frequently migrates between cores. This results in severe performance degradation.

Striped locking mitigates this effect by distributing synchronization across multiple locks. However, scalability remains limited, with speedup peaking around 1.5–2×. This plateau likely reflects a combination of false sharing (if locks reside on adjacent cache lines) and the intrinsic cost of dynamic memory allocation during insert operations.

### Realistic Mixed Workloads

The mixed workload confirms that the benefits of lock striping persist in more realistic scenarios. The Striped implementation consistently outperforms the Coarse-Grained baseline and avoids the dramatic collapse in performance seen with a single global lock. Nevertheless, the observed speedup remains well below linear scaling, indicating that Amdahl’s Law applies: the sequential portions of the program, such as memory allocation and collision handling, ultimately bound achievable performance.

## Discussion

These results demonstrate that correctness in concurrent programming does not imply scalability. While the Coarse-Grained approach is easy to reason about and guarantees correctness, it performs poorly under contention due to lock serialization and excessive coherence traffic. Striped locking reduces contention and improves scalability, but it does not eliminate the fundamental costs associated with shared-memory coherence. Achieving better scalability would require reducing not only lock contention, but also the amount of shared state that must be kept coherent across cores.

## Reproducibility

The project includes scripts to fully reproduce all experiments:

```bash
# Build
make clean && make -j

# Run the full benchmark sweep
./scripts/run_bench.sh

# Aggregate results and generate plots
python3 plots/speedup.py results/raw/a4_bench.csv
gnuplot plots/throughput.gp
gnuplot plots/speedup.gp

