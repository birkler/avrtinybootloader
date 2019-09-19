

all:
	cd app_hid; make
#	cd scratch/iir_filter_test; make

clean:
	cd app_hid; make clean
	cd scratch/iir_filter_test; make clean

scanner-info2:
	cd app_hid; make -s scanner-info2

scanner-info:
	cd app_hid; make -s scanner-info
	
	
program:
	cd app_hid; make program
	