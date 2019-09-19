/*
 * q7_8_t.h
 *
 *  Created on: Jan 25, 2011
 *      Author: jorgen
 */

#ifndef Q7_8_T_H_
#define Q7_8_T_H_
#include <stdint.h>
typedef int16_t q7_8_t;



#define q7_8_mul(a,b) ((int16_t)(((int32_t)(a) * (b)) >> 8))
#define q7_8_div(a,b) ((uint16_t)((((int32)(a))<<8)/(b)))
#define q7_8_add(a,b) ((a) + (b))
#define q7_8_sum(a,b,c,d,e) ((a) + (b) + (c) + (d) + (e))

#define q7_8_float(_f) ((int16_t)((_f) * (1<<8)))
#define q7_8_tofloat(_f) (((float)(_f))/(1<<8))



#endif /* Q7_8_T_H_ */
