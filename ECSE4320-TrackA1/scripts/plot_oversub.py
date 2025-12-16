import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results/raw/oversub.csv")

agg = df.groupby(["pin", "threads"]).agg(
    mean_time=("time_sec", "mean"),
    std_time=("time_sec", "std"),
    mean_gops=("gops", "mean"),
    std_gops=("gops", "std"),
).reset_index()

agg.to_csv("results/tables/oversub_summary.csv", index=False)

# Throughput plot
plt.figure()
for pin in sorted(agg["pin"].unique()):
    sub = agg[agg["pin"] == pin].sort_values("threads")
    plt.plot(sub["threads"], sub["mean_gops"], marker="o", label=pin)
plt.xlabel("threads")
plt.ylabel("throughput (Gops/s)")
plt.title("Oversubscription: throughput vs threads")
plt.legend()
plt.tight_layout()
plt.savefig("results/figures/oversub_throughput.png", dpi=200)

# Time plot
plt.figure()
for pin in sorted(agg["pin"].unique()):
    sub = agg[agg["pin"] == pin].sort_values("threads")
    plt.plot(sub["threads"], sub["mean_time"], marker="o", label=pin)
plt.xlabel("threads")
plt.ylabel("time (s)")
plt.title("Oversubscription: time vs threads")
plt.legend()
plt.tight_layout()
plt.savefig("results/figures/oversub_time.png", dpi=200)

print(agg.head(12))
