set title "Filter response" 
set xlabel "Freq (Hz)"
set ylabel "out/in rms (dB)"
set xrange [0.001:0.005]
set yrange [20:500]
set autoscale
set term pngcairo
set output "iirfilter_test.png" 
plot  "iir_filter_test.dat" using 1:4 with lines smooth bezier


