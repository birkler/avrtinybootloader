/*
 * q2_13_t.h
 *
 *  Created on: Jan 25, 2011
 *      Author: jorgen
 */

#ifndef q2_13_T_H_
#define q2_13_T_H_
#include <stdint.h>
typedef int32_t q2_13_t;



#define q2_13_mul(a,b) ((q2_13_t)(((int32_t)(a) * (b)) >> 13))
#define q2_13_div(a,b) ((q2_13_t)((((int32_t)(a))<<(int32_t)13L)/(b)))
#define q2_13_add(a,b) ((a) + (b))
#define q2_13_sum(a,b,c,d,e) ((a) + (b) + (c) + (d) + (e))

#define q2_13_float(_f) ((q2_13_t)lrint(((_f) * (1L<<13))))
#define q2_13_tofloat(_f) (((float)(_f))/((float)(1L<<13)))



#endif /* q2_13_T_H_ */
