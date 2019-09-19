/*
Copyright (c) 2007, Markus Trenkwalder

Portions taken from the Vicent 3D rendering library
Copyright (c) 2004, David Blythe, Hans Martin Will

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the library's copyright owner nor the names of its
  contributors may be used to endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "fixed_func.h"

namespace fixedpoint {

//static const int32_t FIX16_2PI	= float2fix<int32_t,16>(6.28318530717958647692f);
//static const int32_t FIX16_R2PI = float2fix<int32_t,16>(1.0f/6.28318530717958647692f);


int8_t fixmulQ1_6(int8_t a, int8_t b)
{
	return (int8_t)((a*b)>>6);
}

int8_t fixdivQ1_6(int8_t a, int8_t b)
{
	const uint8_t p = 6;
#if 0
	return (int32_t)((((int64_t)a) << p) / b);
#else
	// The following produces the same results as the above but gcc 4.0.3
	// generates fewer instructions (at least on the ARM processor).
	union {
		int16_t a;
		struct {
			int8_t l;
			int8_t h;
		} parts;
	} x;

	x.parts.l = a << p;
	x.parts.h = a >> (sizeof(int8_t) * 8 - p);
	return (int8_t)(x.a / b);
#endif
}



/*
int32_t fixcos16(int32_t a)
{
    int32_t v;
    //reduce to [0,1)
    while (a < 0) a += FIX16_2PI;
    a = fixmul<16>(a, FIX16_R2PI);
    a += 0x4000;

    // now in the range [0, 0xffff], reduce to [0, 0xfff]
    a >>= 4;

    v = (a & 0x400) ? sin_tab[0x3ff - (a & 0x3ff)] : sin_tab[a & 0x3ff];
	v = fixmul<16>(v, 1L << 16);
    return (a & 0x800) ? -v : v;
}

*/

/*
int32_t fixsin16(int32_t a)
{
    int32_t v;

    // reduce to [0,1)
    while (a < 0) a += FIX16_2PI;
    a = fixmul<16>(a, FIX16_R2PI);

    // now in the range [0, 0xffff], reduce to [0, 0xfff]
    a >>= 4;

    v = (a & 0x400) ? sin_tab[0x3ff - (a & 0x3ff)] : sin_tab[a & 0x3ff];
    v = fixmul<16>(v, 1 << 16);
    return (a & 0x800) ? -v : v;
}

*/

int8_t fixsqrtQ1_6(int8_t a){
	return (int8_t)fixrsqrt16(a);
}

int32_t fixrsqrt16(int32_t a)
{
    int32_t x;

    static const uint16_t rsq_tab[] = { /* domain 0.5 .. 1.0-1/16 */
		0xb504, 0xaaaa, 0xa1e8, 0x9a5f, 0x93cd, 0x8e00, 0x88d6, 0x8432,
    };

    int32_t i, exp;
    if (a == 0) return 0x7fffffff;
    if (a == (1L<<16)) return a;

	exp = detail::CountLeadingZeros(a);
    x = rsq_tab[(a>>(28-exp))&0x7]<<1;

	exp -= 16;
    if (exp <= 0)
		x >>= -exp>>1;
    else
		x <<= (exp>>1)+(exp&1);
    if (exp&1) x = fixmul<int32_t,16>(x, (int32_t)rsq_tab[0]);


    /* newton-raphson */
    /* x = x/2*(3-(a*x)*x) */
    i = 0;
    do {

		x = fixmul<int32_t,16>(
				(x>>1),((1L<<16)*3 - fixmul<int32_t,16>(
						fixmul<int32_t,16>(a,x),x)));
    } while(++i < 3);

    return x;
}

static inline int32_t fast_div16(int32_t a, int32_t b)
{
	if ((b >> 24) && (b >> 24) + 1) {
		return fixmul<int32_t,16>(a >> 8, fixinv<16>(b >> 8));
	} else {
		return fixmul<int32_t,16>(a, fixinv<16>(b));
	}
}

int32_t fixsqrt16(int32_t a)
{
    int32_t s;
    int32_t i;
    s = (a + (1L<<16)) >> 1;
    /* 6 iterations to converge */
    for (i = 0; i < 6; i++)
		s = (s + fast_div16(a, s)) >> 1;
    return s;
}

} // end namespace fixedpoint
