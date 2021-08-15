///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code approximate standard functions.
 *	\file		IceApprox.h
 *	\author		Dave Eberly
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// The approximation formulas are from
//
//   Handbook of Mathematical Functions
//   Edited by M. Abramowitz and I.A. Stegun.
//   Dover Publications, Inc.
//   New York, NY
//   Ninth printing, December 1972

// For the inverse tangent calls, all approximations are valid for |t| <= 1.
// To compute ATAN(t) for t > 1, use ATAN(t) = PI/2 - ATAN(1/t).  For t < -1,
// use ATAN(t) = -PI/2 - ATAN(1/t).

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEAPPROX_H
#define ICEAPPROX_H

inline_ float Sin0(float fT)
{
	// assert:  0 <= fT <= PI/2
	// maximum absolute error = 1.6415e-04
	// speedup = 1.91

	float fTSqr = fT*fT;
	float fResult = 7.61e-03f;
	fResult *= fTSqr;
	fResult -= 1.6605e-01f;
	fResult *= fTSqr;
	fResult += 1.0f;
	fResult *= fT;
	return fResult;
}

inline_ float Sin1(float fT)
{
	// assert:  0 <= fT <= PI/2
	// maximum absolute error = 2.3279e-09
	// speedup = 1.40

	float fTSqr = fT*fT;
	float fResult = -2.39e-08f;
	fResult *= fTSqr;
	fResult += 2.7526e-06f;
	fResult *= fTSqr;
	fResult -= 1.98409e-04f;
	fResult *= fTSqr;
	fResult += 8.3333315e-03f;
	fResult *= fTSqr;
	fResult -= 1.666666664e-01f;
	fResult *= fTSqr;
	fResult += 1.0f;
	fResult *= fT;
	return fResult;
}

inline_ float Cos0(float fT)
{
	// assert:  0 <= fT <= PI/2
	// maximum absolute error = 1.1880e-03
	// speedup = 2.14

	float fTSqr = fT*fT;
	float fResult = 3.705e-02f;
	fResult *= fTSqr;
	fResult -= 4.967e-01f;
	fResult *= fTSqr;
	fResult += 1.0f;
	return fResult;
}

inline_ float Cos1(float fT)
{
	// assert:  0 <= fT <= PI/2
	// maximum absolute error = 2.3082e-09
	// speedup = 1.47

	float fTSqr = fT*fT;
	float fResult = -2.605e-07f;
	fResult *= fTSqr;
	fResult += 2.47609e-05f;
	fResult *= fTSqr;
	fResult -= 1.3888397e-03f;
	fResult *= fTSqr;
	fResult += 4.16666418e-02f;
	fResult *= fTSqr;
	fResult -= 4.999999963e-01f;
	fResult *= fTSqr;
	fResult += 1.0f;
	return fResult;
}

inline_ float Tan0(float fT)
{
	// assert:  0 <= fT <= PI/4
	// maximum absolute error = 8.0613e-04
	// speedup = 2.51

	float fTSqr = fT*fT;
	float fResult = 2.033e-01f;
	fResult *= fTSqr;
	fResult += 3.1755e-01f;
	fResult *= fTSqr;
	fResult += 1.0f;
	fResult *= fT;
	return fResult;
}

inline_ float Tan1(float fT)
{
	// assert:  0 <= fT <= PI/4
	// maximum absolute error = 1.8897e-08
	// speedup = 1.71

	float fTSqr = fT*fT;
	float fResult = 9.5168091e-03f;
	fResult *= fTSqr;
	fResult += 2.900525e-03f;
	fResult *= fTSqr;
	fResult += 2.45650893e-02f;
	fResult *= fTSqr;
	fResult += 5.33740603e-02f;
	fResult *= fTSqr;
	fResult += 1.333923995e-01f;
	fResult *= fTSqr;
	fResult += 3.333314036e-01f;
	fResult *= fTSqr;
	fResult += 1.0f;
	fResult *= fT;
	return fResult;
}

inline_ float InvTan0(float fT)
{
	// assert:  |fT| <= 1
	// maximum absolute error = 4.8830e-03
	// speedup = 2.14

	float fTSqr = fT*fT;
	return fT/(1.0f + 0.28f*fTSqr);
}

inline_ float InvTan1(float fT)
{
	// assert:  |fT| <= 1
	// maximum absolute error = 1.1492e-05
	// speedup = 2.16

	float fTSqr = fT*fT;
	float fResult = 0.0208351f;
	fResult *= fTSqr;
	fResult -= 0.085133f;
	fResult *= fTSqr;
	fResult += 0.180141f;
	fResult *= fTSqr;
	fResult -= 0.3302995f;
	fResult *= fTSqr;
	fResult += 0.999866f;
	fResult *= fT;
	return fResult;
}

inline_ float InvTan2(float fT)
{
	// assert:  |fT| <= 1
	// maximum absolute error = 1.3593e-08
	// speedup = 1.50

	float fTSqr = fT*fT;
	float fResult = 0.0028662257f;
	fResult *= fTSqr;
	fResult -= 0.0161657367f;
	fResult *= fTSqr;
	fResult += 0.0429096138f;
	fResult *= fTSqr;
	fResult -= 0.0752896400f;
	fResult *= fTSqr;
	fResult += 0.1065626393f;
	fResult *= fTSqr;
	fResult -= 0.1420889944f;
	fResult *= fTSqr;
	fResult += 0.1999355085f;
	fResult *= fTSqr;
	fResult -= 0.3333314528f;
	fResult *= fTSqr;
	fResult += 1.0f;
	fResult *= fT;
	return fResult;
}

#endif // ICEAPPROX_H
