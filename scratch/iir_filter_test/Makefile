
CFLAGS+=-g -Wall -pedantic
LFLAGS+=-lm

all:
	make iir_filter_test

clean:
	rm *.o
	rm iir_filter_test
	rm iir_filter_test.exe

iir_filter_test: iir_filter_test.cpp ../../util/q2_13_t.h  ../../util/iir_filter.h plot.gnuplot  plot2.gnuplot  plot3.gnuplot
	g++ $(CFLAGS) -fno-threadsafe-statics -I../../util iir_filter_test.cpp ../../util/fixed_func.cpp -o iir_filter_test $(LFLAGS)
	./iir_filter_test > iir_filter_test.dat
	./iir_filter_test --lfsr > iir_filter_test2.dat
	./iir_filter_test --simul > iir_filter_test3.dat
	gnuplot plot.gnuplot
	gnuplot plot2.gnuplot
	gnuplot plot3.gnuplot
	