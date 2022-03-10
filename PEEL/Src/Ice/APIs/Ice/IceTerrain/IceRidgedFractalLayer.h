///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for ridged fractal layers.
 *	\file		IceRidgedFractalLayer.h
 *	\author		Pierre Terdiman
 *	\date		March, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERIDGEDFRACTALLAYER_H
#define ICERIDGEDFRACTALLAYER_H

	struct ICETERRAIN_API RIDGEDLAYERCREATE : HEIGHTLAYERCREATE
	{
		RIDGEDLAYERCREATE() :
			mOffsetX			(0.0f),
			mOffsetY			(0.0f),
			mXFreq				(0.0f),
			mYFreq				(0.0f),
			mFractalIncrement	(0.0f),
			mLacunarity			(0.0f),
			mOctaves			(0.0f),
			mOffset				(0.0f),
			mGain				(0.0f),
			mZ					(0.0f)
		{
		}

		float	mOffsetX;
		float	mOffsetY;
		float	mXFreq;				//!< Frequency of the ridged function along the x axis
		float	mYFreq;				//!< Frequency of the ridged function along the y axis
		float	mFractalIncrement;
		float	mLacunarity;
		float	mOctaves;
		float	mOffset;
		float	mGain;
		float	mZ;
	};

	class ICETERRAIN_API RidgedFractalLayer : public HeightLayer
	{
		public:
										RidgedFractalLayer();
		virtual							~RidgedFractalLayer();

		virtual	bool					Init(const RIDGEDLAYERCREATE& create);
		virtual	bool					Update(float* field, udword width, udword height)	const;
		virtual	bool					SupportsCache()		const	{ return true;			}
		virtual	bool					SupportsOp()		const	{ return true;			}

		inline_	float					GetOffsetX()		const	{ return mOffsetX;		}
		inline_	float					GetOffsetY()		const	{ return mOffsetY;		}
		inline_	float					GetXFreq()			const	{ return mXFreq;		}
		inline_	float					GetYFreq()			const	{ return mYFreq;		}
		inline_	float					GetZ()				const	{ return mZ;			}

		inline_	void					SetOffsetX(float offset)	{ mOffsetX = offset;	MarkDirty();	}
		inline_	void					SetOffsetY(float offset)	{ mOffsetY = offset;	MarkDirty();	}
		inline_	void					SetXFreq(float freq)		{ mXFreq = freq;		MarkDirty();	}
		inline_	void					SetYFreq(float freq)		{ mYFreq = freq;		MarkDirty();	}
		inline_	void					SetZ(float z)				{ mZ = z;				MarkDirty();	}

		inline_	RidgedFractal&			GetRidgedFractal()			{ return mRidged;		}
		inline_	const RidgedFractal&	GetRidgedFractal()	const	{ return mRidged;		}

		inline_	void					SetGain(float gain)			{ mRidged.SetGain(gain);		MarkDirty();	}
		inline_	void					SetOffset(float offset)		{ mRidged.SetOffset(offset);	MarkDirty();	}

		inline_	void					SetFractalIncrement(float h)
										{
											MarkDirty();
											//const float FractalInc = mRidged.GetFractalIncrement();
											const float Lacunarity = mRidged.GetLacunarity();
											const float Octaves = mRidged.GetOctaves();
											mRidged.Init(h, Lacunarity, Octaves, mRidged.GetOffset(), mRidged.GetGain());
										}

		inline_	void					SetLacunarity(float lacunarity)
										{
											MarkDirty();
											const float FractalInc = mRidged.GetFractalIncrement();
											//const float Lacunarity = mRidged.GetLacunarity();
											const float Octaves = mRidged.GetOctaves();
											mRidged.Init(FractalInc, lacunarity, Octaves, mRidged.GetOffset(), mRidged.GetGain());
										}

		inline_	void					SetOctaves(float octaves)
										{
											MarkDirty();
											const float FractalInc = mRidged.GetFractalIncrement();
											const float Lacunarity = mRidged.GetLacunarity();
											//const float Octaves = mRidged.GetOctaves();
											mRidged.Init(FractalInc, Lacunarity, octaves, mRidged.GetOffset(), mRidged.GetGain());
										}
		protected:
				RidgedFractal			mRidged;

				float					mOffsetX;
				float					mOffsetY;
				float					mXFreq;
				float					mYFreq;
				float					mZ;

		template<const HeightLayerOp op>
				void					ComputeValuesT(float* field, udword width, udword height)	const;

		virtual	void					ComputeValues(float* field, udword width, udword height, HeightLayerOp op)	const;
	};

#endif // ICERIDGEDFRACTALLAYER_H
