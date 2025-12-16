import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results/raw/affinity.csv")

# boxplot style summary
for metric in ["compute_sec", "mem_sec"]:
    plt.figure()
    df.boxplot(column=metric, by="pin")
    plt.suptitle("")
    plt.title(metric)
    plt.xlabel("pin")
    plt.ylabel("seconds")
    plt.tight_layout()
    out = f"results/figures/affinity_{metric}.png"
    plt.savefig(out, dpi=200)

# also dump a small table
summary = df.groupby("pin")[["compute_sec", "mem_sec"]].agg(["mean", "std"])
summary.to_csv("results/tables/affinity_summary.csv")
print(summary)
