set datafile separator ","
set terminal pngcairo size 1200,450
set output out

set key left top
set grid
set xlabel "Threads"
set ylabel "Throughput (ops/s)"
set multiplot layout 1,3 title sprintf("Throughput vs Threads (nkeys=%d)", nkeys)

# CSV columns in a4_bench_mean.csv:
# 1 impl, 2 work, 3 threads, 4 nkeys, 5 mean_ops_per_sec

# ----- lookup -----
set title "work=lookup"
plot \
  'results/raw/a4_bench_mean.csv' using ( (stringcolumn(1) eq "coarse"  && stringcolumn(2) eq "lookup" && int(column(4))==nkeys) ? column(3) : 1/0 ):( (stringcolumn(1) eq "coarse"  && stringcolumn(2) eq "lookup" && int(column(4))==nkeys) ? column(5) : 1/0 ) with linespoints title "coarse", \
  'results/raw/a4_bench_mean.csv' using ( (stringcolumn(1) eq "striped" && stringcolumn(2) eq "lookup" && int(column(4))==nkeys) ? column(3) : 1/0 ):( (stringcolumn(1) eq "striped" && stringcolumn(2) eq "lookup" && int(column(4))==nkeys) ? column(5) : 1/0 ) with linespoints title "striped"

# ----- insert -----
set title "work=insert"
plot \
  'results/raw/a4_bench_mean.csv' using ( (stringcolumn(1) eq "coarse"  && stringcolumn(2) eq "insert" && int(column(4))==nkeys) ? column(3) : 1/0 ):( (stringcolumn(1) eq "coarse"  && stringcolumn(2) eq "insert" && int(column(4))==nkeys) ? column(5) : 1/0 ) with linespoints title "coarse", \
  'results/raw/a4_bench_mean.csv' using ( (stringcolumn(1) eq "striped" && stringcolumn(2) eq "insert" && int(column(4))==nkeys) ? column(3) : 1/0 ):( (stringcolumn(1) eq "striped" && stringcolumn(2) eq "insert" && int(column(4))==nkeys) ? column(5) : 1/0 ) with linespoints title "striped"

# ----- mixed -----
set title "work=mixed"
plot \
  'results/raw/a4_bench_mean.csv' using ( (stringcolumn(1) eq "coarse"  && stringcolumn(2) eq "mixed" && int(column(4))==nkeys) ? column(3) : 1/0 ):( (stringcolumn(1) eq "coarse"  && stringcolumn(2) eq "mixed" && int(column(4))==nkeys) ? column(5) : 1/0 ) with linespoints title "coarse", \
  'results/raw/a4_bench_mean.csv' using ( (stringcolumn(1) eq "striped" && stringcolumn(2) eq "mixed" && int(column(4))==nkeys) ? column(3) : 1/0 ):( (stringcolumn(1) eq "striped" && stringcolumn(2) eq "mixed" && int(column(4))==nkeys) ? column(5) : 1/0 ) with linespoints title "striped"

unset multiplot
