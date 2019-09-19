/*
 * iir_filter_test.c
 *
 *  Created on: Jan 20, 2011
 *      Author: jorgen
 *
 *
 *      http://www.dsptutor.freeuk.com/dfilt14.htm
 *
 * __cxa_guard_acquire
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "q2_13_t.h"
#include "iir_filter.h"
#include "lfsr8.h"



//typedef fixed_point <int16_t,8> q2_13_t;

#define elements_of(array) (sizeof(array) / sizeof(array[0]))

#define q2_13_sqrt(a) q2_13_float(sqrt(q2_13_tofloat(a)))
#define sqr(_f) ((_f)*(_f))

typedef q2_13_t myfloat;
int32_t fixsqrt16(int32_t a);


myfloat input[1000];
myfloat output[elements_of(input)];


float fill_input(float fin, float fsample)
{
	float wstep = (fin/fsample) * 2.0 * M_PI;
	float w=0;
	unsigned int i;
	float sum = 0;

	for (i=0;i<elements_of(input);i++) {
		input[i]=q2_13_float(sin(w));             //[0..1.0)
		w += wstep;
		sum += sqr(q2_13_tofloat(input[i]));
	}
	return sqrt(sum);
}



float fill_input_square(int cycle)
{
	unsigned int i;
	float sum = 0;
	float val = 1;

	for (i=0;i<elements_of(input);i++) {
		if ((i % cycle) == 0) {
			val *= -1;
		}
		input[i]=q2_13_float(val);
		sum += sqr(q2_13_tofloat(input[i]));
	}
	return sqrt(sum);
}

float fill_input_simul(void )
{
	int cycle = 2;
	unsigned int i;
	float sum = 0;
	float val = 0.0;
	int mixer = 1;
	float background = 0.8;
	float offset = 0.0;

	for (i=0;i<elements_of(input);i++) {
		if ((i % cycle) == 0) {
			mixer *= -1;
		}
		if (i==200) {
			val = 0.01;
		}
		if (i==400) {
			offset = -0.5;
		}

		float meas = ((background + offset)) + ((mixer > 0) ? val : 0);
		input[i]=q2_13_float(meas);
		sum += sqr(q2_13_tofloat(input[i]));
	}
	return sqrt(sum);
}



float fill_input_lfsr(void)
{
	unsigned int i;
	float sum = 0;
	uint8_t val;

	for (i=0;i<elements_of(input);i++) {
		if ((i & 0x1) == 0) {
			val = prng_lfsr8_31();
		}
		float val2 = (val & 0x1) ? 1 : -1;
		input[i]=q2_13_float(val2);
		sum += sqr(q2_13_tofloat(input[i]));
	}
	return sqrt(sum);
}



float apply_filter(int n,myfloat input[], myfloat output[])
{
	float sum = 0;
	int i;
	filter_iir2_t filter_iir2_data = {0,0};
	for (i=0;i<n;i++) {
		output[i] = q2_13_filter_iir2_2_10_LP(&filter_iir2_data,input[i]);
		sum += sqr(q2_13_tofloat(output[i]));
	}
	return sqrt(sum);
}

float apply_hp_mix_lp(int n,myfloat input[], myfloat output[])
{
	float sum = 0;
	int i;
	myfloat mixer = q2_13_float(1.0);
	filter_iir2_t filter_iir2_data_hp = {0,0};
	filter_iir2_t filter_iir2_data_lp = {0,0};
	for (i=0;i<n;i++) {
		if ((i % 2) == 0) {
			mixer = q2_13_mul(mixer,q2_13_float(-1.0));
		}
		//Highpass filtering
		myfloat temp  = q2_13_filter_iir2_90per_HP(&filter_iir2_data_hp,input[i]);

		//Mix with "carrier wave"
		temp = q2_13_mul(temp,mixer);
		//Low pass
		temp = q2_13_filter_iir2(&filter_iir2_data_lp,temp);
		// output[i] = temp;
		output[i] = temp;
		sum += sqr(q2_13_tofloat(output[i]));
	}
	return sqrt(sum);
}




static void apply_filter_and_output_raw_response(float sum_in)
{
	int i;
	float sum_out = apply_hp_mix_lp(elements_of(input),input,output);
	printf("#\tin:%f\tout:%f\n",sum_in,sum_out);
	for (i = 0;i<500;i++) {
		printf("%d\t%f\t%f\n",i,q2_13_tofloat(input[i]),q2_13_tofloat(output[i]));
	}
}

int main(int argc,const  char* argv[])
{
	float fsample =200;
	float fin = 1;
	if (argc >= 2 && strcmp(argv[1],"--square") == 0) {
		float sum_in = fill_input_square(2);
		apply_filter_and_output_raw_response(sum_in);
	}
	if (argc >= 2 && strcmp(argv[1],"--simul") == 0) {
		float sum_in = fill_input_simul();
		apply_filter_and_output_raw_response(sum_in);
	}
	else if (argc >= 2 && strcmp(argv[1],"--lfsr") == 0) {
		float sum_in = fill_input_lfsr();
		apply_filter_and_output_raw_response(sum_in);
	}
	else {
		printf("#f\t\tin\tout\tdB\n");
		float sum_in = fill_input_lfsr();
		float sum_out = apply_filter(elements_of(input),input,output);
		printf("%.3f\t\t%.3f\t%.3f\t%f\n",(double)0,(sum_in),(sum_out),20*log10((sum_out)/(sum_in)));

		while (fin < fsample/2.0) {
			float sum_in = fill_input(fin,fsample);
			float sum_out = apply_filter(elements_of(input),input,output);
			printf("%.3f\t\t%.3f\t%.3f\t%f\n",(double)fin,(sum_in),(sum_out),20*log10((sum_out)/(sum_in)));
			fin = fin*1.1;
		}
	}
	return 0;
}

