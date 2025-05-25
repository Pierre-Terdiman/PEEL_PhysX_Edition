#ifndef ROCK_CORE_TEMPLATES_H
#define ROCK_CORE_TEMPLATES_H

#include <Core/RockCommon.h>
#include <float.h>
#include <math.h>

namespace Rock
{
	template <class T>
	inline_	void	SwapT(T& x, T& y)
	{
		const T tmp = x;
		x = y;
		y = tmp;
	}

	template <typename A>
	struct Equal
	{
		inline_	bool operator()(const A& a, const A& b) const
		{
			return a == b;
		}
	};

	template <typename A>
	struct Less
	{
		inline_	bool operator()(const A& a, const A& b) const
		{
			return a < b;
		}
	};

	template <typename A>
	struct Greater
	{
		inline_	bool operator()(const A& a, const A& b) const
		{
			return a > b;
		}
	};

	template <class F, class S>
	class Pair
	{
		public:
		F	mFirst;
		S	mSecond;

		inline_ Pair() : mFirst(F()), mSecond(S())
		{
		}

		inline_ Pair(const F& f, const S& s) : mFirst(f), mSecond(s)
		{
		}

		inline_ Pair(const Pair& p) : mFirst(p.mFirst), mSecond(p.mSecond)
		{
		}

		inline_ Pair& operator=(const Pair& p)
		{
			mFirst = p.mFirst;
			mSecond = p.mSecond;
			return *this;
		}

		inline_ bool operator==(const Pair& p) const
		{
			return mFirst == p.mFirst && mSecond == p.mSecond;
		}

		inline_ bool operator<(const Pair& p) const
		{
			if(mFirst < p.mFirst)
				return true;
			else
				return !(p.mFirst < mFirst) && (mSecond < p.mSecond);
		}
	};

	template <class T>
	inline_	T	MaxT(T a, T b)					{ return a < b ? b : a;						}

	template <class T>
	inline_	T	MinT(T a, T b)					{ return a < b ? a : b;						}

	template <class T>
	inline_	T	ClampT(T v, T lo, T hi)
	{
		ROCK_ASSERT(lo <= hi);
		return MinT(hi, MaxT(lo, v));
	}

	inline_	float	Abs(float v)				{ return ::fabsf(v);						}
	inline_	double	Abs(double v)				{ return ::fabs(v);							}

	inline_	float	Sqrt(float v)				{ return ::sqrtf(v);						}
	inline_	double	Sqrt(double v)				{ return ::sqrt(v);							}
	
	inline_	float	RecipSqrt(float v)			{ return 1.0f / ::sqrtf(v);					}
	inline_	double	RecipSqrt(double v)			{ return 1.0 / ::sqrt(v);					}

	inline_	float	Sin(float v)				{ return ::sinf(v);							}
	inline_	double	Sin(double v)				{ return ::sin(v);							}

	inline_	float	Cos(float v)				{ return ::cosf(v);							}
	inline_	double	Cos(double v)				{ return ::cos(v);							}

	inline_	float	Tan(float v)				{ return ::tanf(v);							}
	inline_	double	Tan(double v)				{ return ::tan(v);							}

	inline_	float	ASin(float v)				{ return ::asinf(ClampT(v, -1.0f, 1.0f));	}
	inline_	double	ASin(double v)				{ return ::asin(ClampT(v, -1.0, 1.0));		}

	inline_	float	ACos(float v)				{ return ::acosf(ClampT(v, -1.0f, 1.0f));	}
	inline_	double	ACos(double v)				{ return ::acos(ClampT(v, -1.0, 1.0));		}

	inline_	float	ATan(float a)				{ return ::atanf(a);						}
	inline_	double	ATan(double a)				{ return ::atan(a);							}

	inline_	float	ATan2(float x, float y)		{ return ::atan2f(x, y);					}
	inline_	double	ATan2(double x, double y)	{ return ::atan2(x, y);						}
}

#endif
