#!/usr/bin/env bash
set -euo pipefail

IN=results/raw/a4_bench_mean.csv
OUT=results/raw/a4_speedup_mean.csv

echo "impl,work,threads,nkeys,speedup" > "$OUT"

# Build lookup table base[impl,work,nkeys] = throughput at threads=1
awk -F',' '
NR==1{next}
{
  impl=$1; work=$2; th=$3; nkeys=$4; thr=$5;
  key=impl","work","nkeys;
  if (th==1) base[key]=thr;
  rows[NR]=$0;
}
END{
  for (i in rows) {
    split(rows[i], f, ",");
    impl=f[1]; work=f[2]; th=f[3]; nkeys=f[4]; thr=f[5];
    key=impl","work","nkeys;
    if (base[key] > 0) {
      printf "%s,%s,%s,%s,%.10g\n", impl, work, th, nkeys, thr/base[key];
    }
  }
}' "$IN" | sort -t, -k2,2 -k4,4n -k3,3n -k1,1 >> "$OUT"

echo "Wrote $OUT"
