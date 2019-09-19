set title "Raw response" 
set xlabel "event"
set ylabel "input/response"
set autoscale
set yrange [-1:+1]
set term pngcairo
set output "iirfilter_test2.png" 
plot  "iir_filter_test2.dat" using 1:2 w lines, \
   "iir_filter_test2.dat" using 1:3 w lines

