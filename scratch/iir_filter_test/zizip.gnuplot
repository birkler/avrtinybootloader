#http://t16web.lanl.gov/Kawano/gnuplot/plot1-e.html

set title "Raw response" 
set xlabel ""
set ylabel "sessions/day"
set autoscale
set logscale y
set term pngcairo linewidth 2

set output "zisip.png" 
plot  "zizip.dat" using 1:2 w filledcurve  smooth bezier
   
