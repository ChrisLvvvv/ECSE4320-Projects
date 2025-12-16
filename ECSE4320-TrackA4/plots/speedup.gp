set datafile separator ","
set terminal pngcairo size 900,650
set output out

set key left top
set grid
set xlabel "Threads"
set ylabel "Speedup vs 1 thread"
set title sprintf("Speedup vs Threads (work=%s, nkeys=%d)", work, nkeys)

# CSV columns:
# 1 impl, 2 work, 3 threads, 4 nkeys, 5 speedup
plot \
  'results/raw/a4_speedup_mean.csv' using ( (stringcolumn(1) eq "coarse"  && stringcolumn(2) eq work && int(column(4)) == nkeys) ? column(3) : 1/0 ):( (stringcolumn(1) eq "coarse"  && stringcolumn(2) eq work && int(column(4)) == nkeys) ? column(5) : 1/0 ) with linespoints title "coarse", \
  'results/raw/a4_speedup_mean.csv' using ( (stringcolumn(1) eq "striped" && stringcolumn(2) eq work && int(column(4)) == nkeys) ? column(3) : 1/0 ):( (stringcolumn(1) eq "striped" && stringcolumn(2) eq work && int(column(4)) == nkeys) ? column(5) : 1/0 ) with linespoints title "striped"
