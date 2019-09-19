#http://t16web.lanl.gov/Kawano/gnuplot/plot1-e.html

set title "Raw response" 
set xlabel "event"
set ylabel "output"
set y2label "input"
set autoscale
set yrange [-0.1:+0.1]
set y2range [-2:+2]
set y2tics auto
set ytics nomirror
set term pngcairo linewidth 2

set output "iirfilter_test3.png" 
plot  "iir_filter_test3.dat" using 1:2 w lines axis x1y2, \
   "iir_filter_test3.dat" using 1:3 w lines axis x1y1

