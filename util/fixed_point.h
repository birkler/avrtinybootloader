#ifndef FIXEDP_CLASS_H_INCLUDED
#define FIXEDP_CLASS_H_INCLUDED

#ifdef _MSC_VER
#pragma once
#endif

#include "fixed_func.h"

namespace fixedpoint {

// The template argument p in all of the following functions refers to the
// fixed point precision (e.g. p = 8 gives 24.8 fixed point functions).

template <class t_t, int p>
struct fixed_point {
	t_t intValue;

	fixed_point() {}

	/*explicit*/ fixed_point(const int8_t& i) : intValue(i << p) {}
	/*explicit*/ fixed_point(const int& i) : intValue(i << p) {}
	/*explicit*/ //fixed_point(const float& f) : intValue(float2fix<p>(f)) {}
	/*explicit*/ fixed_point(const double& f) : intValue((t_t)(f * (1 << p))) {}

	fixed_point& operator += (const fixed_point& r) { intValue += r.intValue; return *this; }
	fixed_point& operator -= (const fixed_point& r) { intValue -= r.intValue; return *this; }
	fixed_point& operator *= (const fixed_point& r) { intValue = fixmul<t_t,p>(intValue, r.intValue); return *this; }
	fixed_point& operator /= (const fixed_point& r) { intValue = fixdiv<t_t,p>(intValue, r.intValue); return *this; }

	fixed_point& operator *= (int32_t r) { intValue *= r; return *this; }
	fixed_point& operator /= (int32_t r) { intValue /= r; return *this; }

	fixed_point operator - () const { fixed_point x; x.intValue = -intValue; return x; }
	fixed_point operator + (const fixed_point&  r) const { fixed_point x = *this; x += r; return x;}
	fixed_point operator - (const fixed_point&  r) const { fixed_point x = *this; x -= r; return x;}
	fixed_point operator * (const fixed_point& r) const { fixed_point x = *this; x *= r; return x;}
	fixed_point operator / (const fixed_point&  r) const { fixed_point x = *this; x /= r; return x;}

	bool operator == (const fixed_point&  r) const { return intValue == r.intValue; }
	bool operator != (const fixed_point&  r) const { return !(*this == r); }
	bool operator <  (const fixed_point&  r) const { return intValue < r.intValue; }
	bool operator >  (const fixed_point&  r) const { return intValue > r.intValue; }
	bool operator <= (const fixed_point&  r) const { return intValue <= r.intValue; }
	bool operator >= (const fixed_point&  r) const { return intValue >= r.intValue; }

	fixed_point operator + (int32_t r) const { fixed_point x = *this; x += r; return x;}
	fixed_point operator - (int32_t r) const { fixed_point x = *this; x -= r; return x;}
	fixed_point operator * (int32_t r) const { fixed_point x = *this; x *= r; return x;}
	fixed_point operator / (int32_t r) const { fixed_point x = *this; x /= r; return x;}
	operator float() {
		return ((float)intValue / (1<<p));
	}
};

// Specializations for use with plain integers
template <class t_t,int p>
inline fixed_point<t_t,p> operator + (t_t a, fixed_point<t_t,p> b)
{ return b + a; }

template <class t_t,int p>
inline fixed_point<t_t,p> operator - (int32_t a, fixed_point<t_t,p> b)
{ return -b + a; }

template <class t_t,int p>
inline fixed_point<t_t,p> operator * (int32_t a, fixed_point<t_t,p> b)
{ return b * a; }

template <class t_t,int p>
inline fixed_point<t_t,p> operator / (int32_t a, fixed_point<t_t,p> b)
{ fixed_point<t_t,p> r(a); r /= b; return r; }

// math functions
// no default implementation

template <class t_t,int p>
inline fixed_point<t_t,p> sin(fixed_point<t_t,p> a);

template <class t_t,int p>
inline fixed_point<t_t,p> cos(fixed_point<t_t,p> a);

template <class t_t,int p>
inline fixed_point<t_t,p> sqrt(fixed_point<t_t,p> a);

template <class t_t,int p>
inline fixed_point<t_t,p> rsqrt(fixed_point<t_t,p> a);

template <class t_t,int p>
inline fixed_point<t_t,p> inv(fixed_point<t_t,p> a);

template <class t_t,int p>
inline fixed_point<t_t,p> abs(fixed_point<t_t,p> a)
{
	fixed_point<t_t,p> r;
	r.intValue = a.intValue > 0 ? a.intValue : -a.intValue;
	return r;
}

// specializations for 16.16 format

template <>
inline fixed_point<int32_t,16> sin(fixed_point<int32_t,16> a)
{
	fixed_point<int32_t,16> r;
	r.intValue = fixsin16(a.intValue);
	return r;
}

template <>
inline fixed_point<int32_t,16> cos(fixed_point<int32_t,16> a)
{
	fixed_point<int32_t,16> r;
	r.intValue = fixcos16(a.intValue);
	return r;
}


template <>
inline fixed_point<int32_t,16> sqrt(fixed_point<int32_t,16> a)
{
	fixed_point<int32_t,16> r;
	r.intValue = fixsqrt16(a.intValue);
	return r;
}

template <>
inline fixed_point<int16_t,8> sqrt(fixed_point<int16_t,8> a)
{
	fixed_point<int16_t,8> r;
	r.intValue = fixsqrt16(a.intValue);
	return r;
}


template <>
inline fixed_point<int8_t,6> sqrt(fixed_point<int8_t,6> a)
{
	fixed_point<int8_t,6> r;
	r.intValue = fixsqrtQ1_6(a.intValue);
	return r;
}



template <>
inline fixed_point<int32_t,16> rsqrt(fixed_point<int32_t,16> a)
{
	fixed_point<int32_t,16> r;
	r.intValue = fixrsqrt16(a.intValue);
	return r;
}

template <>
inline fixed_point<int32_t,16> inv(fixed_point<int32_t,16> a)
{
	fixed_point<int32_t,16> r;
	r.intValue = fixinv<16>(a.intValue);
	return r;
}

// The multiply accumulate case can be optimized.
template <int8_t,int p>
inline fixed_point<int8_t,p> multiply_accumulate(
	int count,
	const fixed_point<int8_t,p> *a,
	const fixed_point<int8_t,p> *b)
{
	int16_t result = 0;
	for (int i = 0; i < count; ++i)
		result += static_cast<int16_t>(a[i].intValue) * b[i].intValue;
	fixed_point<int8_t,p> r;
	r.intValue = static_cast<int8_t>(result >> p);
	return r;
}

} // end namespace fixedpoint

#endif

