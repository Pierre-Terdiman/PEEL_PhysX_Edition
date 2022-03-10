///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for various filters.
 *	\file		IceFilters.h
 *	\author		Pierre Terdiman
 *	\date		October, 10, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEFILTERS_H
#define ICEFILTERS_H

	enum FunctionType
	{
		FUNC_NONE,
		FUNC_IDENTITY,
		FUNC_SIN,
		FUNC_TRIANGLE,
		FUNC_SQUARE,
		FUNC_SAWTOOTH,
		FUNC_INVERSESAWTOOTH,
		FUNC_NOISE,

		FUNC_FORCE_DWORD	= 0x7fffffff
	};

	inline_	float	FuncStep(float t, float a)
	{
		return float(t>=a);
	}

	inline_	float	FuncPulse(float t, float a, float b)
	{
		return FuncStep(t, a) - FuncStep(t, b);
	}

	inline_	float	FuncClamp(float t, float a, float b)
	{
		return (t<a ? a : (t>b ? b : t));
	}

	inline_	float	FuncSmoothStep(float t, float a, float b)
	{
		if(t<a)		return 0.0f;
		if(t>=b)	return 1.0f;
		t = (t-a)/(b-a);
		return t*t * (3.0f - 2.0f*t);
	}

	inline_	float	Mod(float a, float b)
	{
		int n = (int)(a/b);
		a -= n*b;
		if(a<0.0f)	a+=b;
		return a;
	}

	inline_	float	FuncSawTooth(float t)
	{
		return Mod(t, 1.0f);
	}

	inline_	float	FuncInverseSawTooth(float t)
	{
		return 1.0f - Mod(t, 1.0f);
	}

	inline_	float	FuncSquare(float t)
	{
		return FuncPulse(Mod(t, 1.0f), 0.0f, 0.5f);
	}

	//inline_	float	FuncTriangle(float t)					{ return FuncSawTooth(t)}


	class ICEMATHS_API Function : public Allocateable
	{
		public:
		inline_			Function()	{}
		inline_			~Function()	{}

		FunctionType	mFunc;
		float			mBase;
		float			mAmplitude;
		float			mPhase;
		float			mFrequency;

		// F(t) = Base + Amplitude * Function(Phase + t * Frequency)
		float F(float t)	const
		{
			switch(mFunc)
			{
				case FUNC_NONE:				return mBase;
				case FUNC_IDENTITY:			return mBase + mAmplitude * (mPhase + t * mFrequency);
				case FUNC_SIN:				return mBase + mAmplitude * sinf(mPhase + t * mFrequency);

				case FUNC_SQUARE:			return mBase + mAmplitude * FuncSquare(mPhase + t * mFrequency);
				case FUNC_SAWTOOTH:			return mBase + mAmplitude * FuncSawTooth(mPhase + t * mFrequency);
				case FUNC_INVERSESAWTOOTH:	return mBase + mAmplitude * FuncInverseSawTooth(mPhase + t * mFrequency);

//				case FUNC_TRIANGLE:			return mBase + TriangleFunc(mPhase + t * mFrequency);

			}
			return 0.0f;
		}
	};

#endif	// ICEFILTERS_H

