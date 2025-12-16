import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results/raw/numa.csv")

summary = df.groupby("mode")[["time_sec", "bandwidth"]].agg(["mean", "std"])
summary.to_csv("results/tables/numa_summary.csv")

plt.figure()
summary["bandwidth"]["mean"].plot(kind="bar", yerr=summary["bandwidth"]["std"])
plt.ylabel("bandwidth (GB/s)")
plt.title("NUMA memory access bandwidth")
plt.tight_layout()
plt.savefig("results/figures/numa_bandwidth.png", dpi=200)

plt.figure()
summary["time_sec"]["mean"].plot(kind="bar", yerr=summary["time_sec"]["std"])
plt.ylabel("time (s)")
plt.title("NUMA memory access time")
plt.tight_layout()
plt.savefig("results/figures/numa_time.png", dpi=200)

print(summary)
