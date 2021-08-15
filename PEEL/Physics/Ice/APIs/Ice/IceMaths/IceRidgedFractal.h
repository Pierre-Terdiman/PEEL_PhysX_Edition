///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Ridged Multifractal from "Texturing & Modeling - a procedural approach"
 *	\file		IceRidgedFractal.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERIDGEDFRACTAL_H
#define ICERIDGEDFRACTAL_H

	class ICEMATHS_API RidgedFractal : public Allocateable
	{
		public:
								RidgedFractal();
								RidgedFractal(float h, float lacunarity, float octaves, float offset, float gain);
								~RidgedFractal();

				void			Init(float h, float lacunarity, float octaves, float offset, float gain);

				float			Compute(const Point& vector)	const;

		inline_	float			GetFractalIncrement()	const	{ return mFractalIncrement;	}
		inline_	float			GetLacunarity()			const	{ return mLacunarity;		}
		inline_	float			GetOctaves()			const	{ return mOctaves;			}
		inline_	float			GetOffset()				const	{ return mOffset;			}
		inline_	float			GetGain()				const	{ return mGain;				}

		inline_	void			SetGain(float gain)				{ mGain = gain;				}
		inline_	void			SetOffset(float offset)			{ mOffset = offset;			}

		protected:
				PerlinNoise		mNoise;
				float			mFractalIncrement;
				float			mLacunarity;
				float			mOctaves;
				float			mOffset;
				float			mGain;

				float*			mExponentArray;
	};

#endif // ICERIDGEDFRACTAL_H
