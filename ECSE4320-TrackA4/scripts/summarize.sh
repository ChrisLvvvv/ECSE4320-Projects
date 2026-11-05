#!/usr/bin/env bash
set -euo pipefail

IN=results/raw/a4_bench.csv
OUT=results/raw/a4_bench_mean.csv

# Output header
echo "impl,work,threads,nkeys,mean_ops_per_sec" > "$OUT"

# Skip header; group by impl,work,threads,nkeys and average ops_per_sec (col 9)
tail -n +2 "$IN" | awk -F',' '
{
  key=$1","$2","$3","$4;
  sum[key]+=$9;
  cnt[key]+=1;
}
END{
  for (k in sum) {
    printf "%s,%.10g\n", k, sum[k]/cnt[k];
  }
}' | sort -t, -k2,2 -k4,4n -k3,3n -k1,1 >> "$OUT"

echo "Wrote $OUT"
