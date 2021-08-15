///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Fractal Brownian Motion from "Texturing & Modeling - a procedural approach"
 *	\file		IceFractalBrownianMotion.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEFRACTALBROWNIANMOTION_H
#define ICEFRACTALBROWNIANMOTION_H

	class ICEMATHS_API FractalBrownianMotion : public Allocateable
	{
		public:
							FractalBrownianMotion();
							FractalBrownianMotion(float h, float lacunarity, float octaves);
							~FractalBrownianMotion();

				void		Init(float h, float lacunarity, float octaves);

				float		Compute(const Point& vector)	const;

		inline_	float		GetFractalIncrement()	const	{ return mFractalIncrement;	}
		inline_	float		GetLacunarity()			const	{ return mLacunarity;		}
		inline_	float		GetOctaves()			const	{ return mOctaves;			}

		protected:
				PerlinNoise	mNoise;
				float		mFractalIncrement;
				float		mLacunarity;
				float		mOctaves;

				float*		mExponentArray;
	};

#endif // ICEFRACTALBROWNIANMOTION_H
