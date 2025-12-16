import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results/raw/smt.csv")

agg = df.groupby(["mode", "case"])["time_sec"].agg(["mean", "std"]).reset_index()
agg.to_csv("results/tables/smt_summary.csv", index=False)

# compute slowdown table
pivot = agg.pivot(index="mode", columns="case", values="mean")
pivot["slowdown"] = pivot["interf"] / pivot["alone"]
pivot.to_csv("results/tables/smt_slowdown.csv")

for mode in sorted(df["mode"].unique()):
    sub = df[df["mode"] == mode]
    plt.figure()
    sub.boxplot(column="time_sec", by="case")
    plt.suptitle("")
    plt.title(f"SMT interference: {mode}")
    plt.xlabel("case")
    plt.ylabel("time (s)")
    plt.tight_layout()
    plt.savefig(f"results/figures/smt_{mode}.png", dpi=200)

print(pivot)
