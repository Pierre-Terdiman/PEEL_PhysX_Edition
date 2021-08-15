///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for intervals.
 *	\file		IceInterval.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEINTERVAL_H
#define ICEINTERVAL_H

//#define INTERVAL_USE_FCOMI

	// From Redon's thesis:
	// [a,b] + [c,d] = [a+c, b+d]
	// [a,b] - [c,d] = [a-d, b-c]
	// [a,b] * [c,d] = [min(ac, ad, bc, bd), max(ac, ad, bc, bd)]
	// 1 / [a,b] = [1/b, 1/a] if a>0 or b<0
	// [a,b] / [c,d] = [a,b] * (1, [c,d]) if c>0 or d<0
	// [a,b] <= [c,d] if b<=c

	class ICEMATHS_API IceInterval : public Allocateable
	{
		public:
		inline_						IceInterval()											{}
		inline_						IceInterval(const IceInterval& it) : a(it.a), b(it.b)	{}
		inline_						IceInterval(float f) : a(f), b(f)						{}
		inline_						IceInterval(float _a, float _b) : a(_a), b(_b)			{}
		inline_						~IceInterval()											{}

		inline_	float				Width()									const		{ return b - a;									}
		inline_	float				MidPoint()								const		{ return (a+b)*0.5f;							}

		// Arithmetic operators

		//! Operator for Interval Plus = Interval + Interval.
		inline_	IceInterval			operator+(const IceInterval& it)		const		{ return IceInterval(a + it.a, b + it.b);		}

		//! Operator for Interval Plus = Interval + float.
		inline_	IceInterval			operator+(float f)						const		{ return IceInterval(a + f, b + f);				}

		//! Operator for Interval Minus = Interval - Interval.
		inline_	IceInterval			operator-(const IceInterval& it)		const		{ return IceInterval(a - it.b, b - it.a);		}

		//! Operator for Interval Minus = Interval - float.
		inline_	IceInterval			operator-(float f)						const		{ return IceInterval(a - f, b - f);				}

		//! Operator for Interval Mul = Interval * Interval.
		inline_	IceInterval			operator*(const IceInterval& it)		const
									{
										float ac = a*it.a;
										float ad = a*it.b;
										float bc = b*it.a;
										float bd = b*it.b;
#ifdef INTERVAL_USE_FCOMI
										float Min = FCMin4(ac, ad, bc, bd);
										float Max = FCMax4(ac, ad, bc, bd);
#else
										float Min = ac;
										if(ad<Min)	Min = ad;
										if(bc<Min)	Min = bc;
										if(bd<Min)	Min = bd;

										float Max = ac;
										if(ad>Max)	Max = ad;
										if(bc>Max)	Max = bc;
										if(bd>Max)	Max = bd;
#endif
										return IceInterval(Min, Max);
									}

		//! Operator for Interval Scale = Interval * float.
		inline_	IceInterval			operator*(float s)						const
									{
										float Min = a*s;
										float Max = b*s;
										if(Min>Max)	TSwap(Min,Max);

										return IceInterval(Min, Max);
									}

		//! Operator for Interval Scale = float * Interval.
		inline_ friend	IceInterval operator*(float s, const IceInterval& it)
									{
										float Min = it.a*s;
										float Max = it.b*s;
										if(Min>Max)	TSwap(Min,Max);

										return IceInterval(Min, Max);
									}

		//! Operator for Interval Scale = float / Interval.
		inline_	friend	IceInterval operator/(float s, const IceInterval& it)			{ return IceInterval(s/it.b, s/it.a);					}

		//! Operator for Point Scale = Point / float.
//		inline_	Point			operator/(float s)					const		{ s = 1.0f / s; return Point(x * s, y * s, z * s);	}

		//! Operator for IceInterval Div = IceInterval / IceInterval.
		inline_	IceInterval			operator/(const IceInterval& it)		const		{ return (*this) * (1.0f / it);						}



		//! Operator for IceInterval += IceInterval.
		inline_	IceInterval&		operator+=(const IceInterval& it)					{ a += it.a; b += it.b;			return *this;		}

		//! Operator for IceInterval += float.
		inline_	IceInterval&		operator+=(float f)									{ a += f; b += f;				return *this;		}

		//! Operator for IceInterval -= IceInterval.
		inline_	IceInterval&		operator-=(const IceInterval& it)					{ a -= it.b; b -= it.a;			return *this;		}

		//! Operator for IceInterval -= float.
		inline_	IceInterval&		operator-=(float f)									{ a -= f; b -= f;				return *this;		}

		//! Operator for IceInterval *= IceInterval.
		inline_	IceInterval&		operator*=(const IceInterval& it)
									{
										float ac = a*it.a;
										float ad = a*it.b;
										float bc = b*it.a;
										float bd = b*it.b;
#ifdef INTERVAL_USE_FCOMI
										a = FCMin4(ac, ad, bc, bd);
										b = FCMax4(ac, ad, bc, bd);
#else
										a = ac;
										if(ad<a)	a = ad;
										if(bc<a)	a = bc;
										if(bd<a)	a = bd;

										b = ac;
										if(ad>b)	b = ad;
										if(bc>b)	b = bc;
										if(bd>b)	b = bd;
#endif
										return *this;
									}

		//! Operator for IceInterval /= IceInterval.
		inline_	IceInterval&		operator/=(const IceInterval& it)
									{
										*this *= 1.0f / it;
										return *this;
									}

		//! Operator for "IceInterval A = IceInterval B"
		inline_	void				operator = (const IceInterval& interval)
									{
										a = interval.a;
										b = interval.b;
									}
		// Logical operators

		//! Operator for "if(IceInterval<=IceInterval)"
		inline_	bool				operator<=(const IceInterval& it)		const		{ return b <= it.a;	}
		//! Operator for "if(IceInterval<IceInterval)"
		inline_	bool				operator<(const IceInterval& it)		const		{ return b < it.a;	}
		//! Operator for "if(IceInterval>=IceInterval)"
		inline_	bool				operator>=(const IceInterval& it)		const		{ return a >= it.b;	}
		//! Operator for "if(IceInterval>IceInterval)"
		inline_	bool				operator>(const IceInterval& it)		const		{ return a > it.b;	}

		float	a, b;
	};

	FUNCTION ICEMATHS_API bool IntervalReduce(udword nb, const IceInterval** list, Container& pairs);

#endif // ICEINTERVAL_H
