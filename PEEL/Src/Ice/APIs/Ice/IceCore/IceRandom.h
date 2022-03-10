///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for random generators.
 *	\file		IceRandom.h
 *	\author		Pierre Terdiman
 *	\date		August, 9, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERANDOM_H
#define ICERANDOM_H

	// Wrappers for system random functions
	FUNCTION ICECORE_API	void	SRand(udword seed);
	FUNCTION ICECORE_API	udword	Rand();

	//! Returns a random floating-point value f such as 0.0f <= f <= 1.0f
	inline_ float UnitRandomFloat()	{ return float(Rand()) * ONE_OVER_RAND_MAX;	}

	//! Returns a random number i so that 0 <= i < max_value
	FUNCTION ICECORE_API	udword GetRandomNumber(udword max_value=0xffffffff);

	//! Returns a random bool (0 or 1)
	FUNCTION ICECORE_API	udword GetRandomBool();

	//! Returns a pseudo random number i so that 0 <= i < max_value
	//! Uses a better random number generator than "GetRandomNumber".
	FUNCTION ICECORE_API	udword GetPseudoRandomNumber(udword max_value=0xffffffff);

	//! Returns a real random number i so that 0 <= i < max_value
	//! The set of numbers returned by this function will always be different
	//! each time you run your app.
	FUNCTION ICECORE_API	udword GetRealRandomNumber(udword max_value=0xffffffff);

	// This one is not very good....
	class ICECORE_API BasicRandom
	{
		public:

		//! Constructor
		inline_				BasicRandom(udword seed=0)	: mRnd(seed)	{}
		//! Destructor
		inline_				~BasicRandom()								{}

		inline_	void		SetSeed(udword seed)		{ mRnd = seed;											}
		inline_	udword		GetCurrentValue()	const	{ return mRnd;											}
		inline_	udword		Randomize()					{ mRnd = mRnd * 2147001325 + 715136305; return mRnd;	}

		inline_	float		RandomFloat()				{ return (float(Randomize() & 0xffff)/65535.0f) - 0.5f;	}

		private:
				udword		mRnd;
	};

	// Following is a set of better random generators.

	// Base class for random number generators
	class ICECORE_API RandomGenerator
	{
		protected:

							RandomGenerator();
		virtual				~RandomGenerator();

		mutable	sdword		mSeed;
		public:

				void		SetNewSeed();
		inline_	sdword		GetSeed()		const		{ return mSeed; }

		virtual	void		SetSeed(sdword s) = 0;

		virtual	udword		RandI()			const	= 0;	// 0..2^31
		virtual	float		RandF()			const			// 0.0 .. 1.0 float generator
							{
								// default: multiply by 1/(2^31)
								return float(RandI()) * float(1.0f/2147483647.0f);
							}

				sdword		RandI(sdword i, sdword n)		// i..n integer generator
							{
								ASSERT(i<=n);
								return (sdword)(i + (RandI() % (n - i + 1)) );
							}   

				float		RandF(float i, float n)			// i..n float generator
							{
								ASSERT(i<=n);
								return (i + (n - i) * RandF());   
							}   
	};

	// Linear Congruential Method, the "minimal standard generator"
	// Fast, farly good random numbers (better than using rand)
	// Park & Miller, 1988, Comm of the ACM, 31(10), pp. 1192-1201
	class ICECORE_API RandomLCG : public RandomGenerator
	{
		public:
							RandomLCG();
							RandomLCG(sdword s);

		virtual	void		SetSeed(sdword s);
		virtual	udword		RandI()			const;

		protected:
		static	const sdword msQuotient;
		static	const sdword msRemainder;
	};

	//--------------------------------------
	// Fast, very good random numbers
	//
	// Period = 2^249
	//
	// Kirkpatrick, S., and E. Stoll, 1981; A Very Fast Shift-Register
	//       Sequence Random Number Generator, Journal of Computational Physics,
	// V. 40.
	//
	// Maier, W.L., 1991; A Fast Pseudo Random Number Generator,
	// Dr. Dobb's Journal, May, pp. 152 - 157

	class ICECORE_API RandomR250: public RandomGenerator
	{
		public:
						RandomR250(); 
						RandomR250(sdword s); 

		virtual	void	SetSeed(sdword s);
		virtual	udword	RandI()	const;

		private:
		mutable	udword	mBuffer[250];
		mutable	sdword	mIndex;
	};

#endif // ICERANDOM_H

