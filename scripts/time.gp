reset
set title "Time"
set terminal png font "Times_New_Roman"
set xlabel "n sequence in Fibonacci"
set ylabel "time (ns)"
set output "time.png"
set xrange [0:1000]
set key left



plot \
"data/final.txt" using 1:2 with linespoints linewidth 2 title "bn-fast-doubling", \