/*
 * iir_filter.h
 *
 *  Created on: Jan 25, 2011
 *      Author: jorgen
 *
 *
 *
 *
 *
 *      http://faculty.cua.edu/regalia/regalia-perso_files/sp-sep-91.pdf
 *
 *      http://www.dsptutor.freeuk.com/IIRFilterDesign/IIRFilterDesign.html
 *
 *      http://ntur.lib.ntu.edu.tw/bitstream/246246/142473/1/63.pdf
 */

#ifndef IIR_FILTER_H_
#define IIR_FILTER_H_

#include "q7_8_t.h"
#include "q2_13_t.h"

typedef struct {
	q7_8_t y_1;
	q7_8_t y_2;
	q7_8_t x_1;
	q7_8_t x_2;
} filter_iir2_t;


//Chebyshev IIR filter
//
//Filter type: HP
//Passband: 400 - 4000 Hz
//Passband ripple: 1.0 dB
//Order: 2
//
//Coefficients
//
//a[0] = 0.7550102     	b[0] = 1.0
//a[1] = -1.5100204     	b[1] = -1.655717
//a[2] = 0.7550102     	b[2] = 0.73281693
static q2_13_t q2_13_filter_iir2_90per_HP(filter_iir2_t* iir2,q2_13_t in)
{
	const q2_13_t a0 = q2_13_float(0.7550102);
	const q2_13_t a1 = q2_13_float(-1.5100204);
	const q2_13_t a2 = q2_13_float( 0.7550102);
	const q2_13_t inv_b0 = q2_13_float(1/1.0);
	const q2_13_t _b1 = -q2_13_float(-1.655717);
	const q2_13_t _b2 = -q2_13_float(0.73281693);

	q2_13_t res;
	res =  q2_13_mul(inv_b0,q2_13_sum(
			q2_13_mul(a0,in),
			q2_13_mul(a1,iir2->x_1),
			q2_13_mul(a2,iir2->x_2),
			q2_13_mul(_b1,iir2->y_1),
			q2_13_mul(_b2,iir2->y_2)));
	//printf("%f,",iir2->b0);
	iir2->y_2 = iir2->y_1;
	iir2->y_1 = res;
	iir2->x_2 = iir2->x_1;
	iir2->x_1 = in;
	return res;
}



static q2_13_t q2_13_filter_iir2_50_BP(filter_iir2_t* iir2,q2_13_t in)
{
	const q2_13_t a0 = q2_13_float(0.037804753 );
	const q2_13_t a1 = q2_13_float(0.0  );
	const q2_13_t a2 = q2_13_float(-0.037804753 );
	const q2_13_t inv_b0 = q2_13_float(1/1.0);
	const q2_13_t _b1 = -q2_13_float(-1.1792585E-16);
	const q2_13_t _b2 = -q2_13_float( 0.9243905);

	q2_13_t res;
	res =  q2_13_mul(inv_b0,q2_13_sum(
			q2_13_mul(a0,in),
			q2_13_mul(a1,iir2->x_1),
			q2_13_mul(a2,iir2->x_2),
			q2_13_mul(_b1,iir2->y_1),
			q2_13_mul(_b2,iir2->y_2)));
	//printf("%f,",iir2->b0);
	iir2->y_2 = iir2->y_1;
	iir2->y_1 = res;
	iir2->x_2 = iir2->x_1;
	iir2->x_1 = in;
	return res;
}

/*

static q2_13_t q2_13_filter_iir2_2_10_LP(filter_iir2_t* iir2,q2_13_t in)
{
	const q2_13_t a0 = q2_13_float(0.0014603166);
	const q2_13_t a1 = q2_13_float(0.0029206332 );
	const q2_13_t a2 = q2_13_float(0.0014603166);
	const q2_13_t inv_b0 = q2_13_float(1/1.0);
	const q2_13_t _b1 = -q2_13_float(-1.8890331);
	const q2_13_t _b2 = -q2_13_float( 0.89487433);

	q2_13_t res;
	res =  q2_13_mul(inv_b0,q2_13_sum(
			q2_13_mul(a0,in),
			q2_13_mul(a1,iir2->x_1),
			q2_13_mul(a2,iir2->x_2),
			q2_13_mul(_b1,iir2->y_1),
			q2_13_mul(_b2,iir2->y_2)));
	//printf("%f,",iir2->b0);
	iir2->y_2 = iir2->y_1;
	iir2->y_1 = res;
	iir2->x_2 = iir2->x_1;
	iir2->x_1 = in;
	return res;
}
*/
/*
q2_13_t q2_13_filter_iir2(filter_iir2_t* iir2,q2_13_t in)
{
	const q2_13_t a0 = q2_13_float(0.020517392);
	const q2_13_t a1 = q2_13_float(0.041034784);
	const q2_13_t a2 = q2_13_float(0.020517392);
	const q2_13_t inv_b0 = q2_13_float(1/1.0);
	const q2_13_t _b1 = -q2_13_float(-1.6185197);
	const q2_13_t _b2 = -q2_13_float(0.71059346);

	q2_13_t res;
	res =  q2_13_mul(inv_b0,q2_13_sum(
			q2_13_mul(a0,in),
			q2_13_mul(a1,iir2->x_1),
			q2_13_mul(a2,iir2->x_2),
			q2_13_mul(_b1,iir2->y_1),
			q2_13_mul(_b2,iir2->y_2)));
	//printf("%f,",iir2->b0);
	iir2->y_2 = iir2->y_1;
	iir2->y_1 = res;
	iir2->x_2 = iir2->x_1;
	iir2->x_1 = in;
	return res;
}
*/

q7_8_t filter_iir2(filter_iir2_t* iir2,q7_8_t in)
{
	const q7_8_t a0 = q7_8_float(0.020517392);
	const q7_8_t a1 = q7_8_float(0.041034784);
	const q7_8_t a2 = q7_8_float(0.020517392);
	const q7_8_t inv_b0 = q7_8_float(1/1.0);
	const q7_8_t _b1 = -q7_8_float(-1.6185197);
	const q7_8_t _b2 = -q7_8_float(0.71059346);

	q7_8_t res;
	res =  q7_8_mul(inv_b0,q7_8_sum(
			q7_8_mul(a0,in),
			q7_8_mul(a1,iir2->x_1),
			q7_8_mul(a2,iir2->x_2),
			q7_8_mul(_b1,iir2->y_1),
			q7_8_mul(_b2,iir2->y_2)));
	//printf("%f,",iir2->b0);
	iir2->y_2 = iir2->y_1;
	iir2->y_1 = res;
	iir2->x_2 = iir2->x_1;
	iir2->x_1 = in;
	return res;
}



#endif /* IIR_FILTER_H_ */
