///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for FBM layers.
 *	\file		IceFBMLayer.h
 *	\author		Pierre Terdiman
 *	\date		March, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEFBMLAYER_H
#define ICEFBMLAYER_H

	struct ICETERRAIN_API FBMLAYERCREATE : HEIGHTLAYERCREATE
	{
		FBMLAYERCREATE() :
			mOffsetX			(0.0f),
			mOffsetY			(0.0f),
			mXFreq				(0.0f),
			mYFreq				(0.0f),
			mFractalIncrement	(0.0f),
			mLacunarity			(0.0f),
			mOctaves			(0.0f),
			mZ					(0.0f)
		{
		}

		float	mOffsetX;
		float	mOffsetY;
		float	mXFreq;				//!< Frequency of the FBM function along the x axis
		float	mYFreq;				//!< Frequency of the FBM function along the y axis
		float	mFractalIncrement;
		float	mLacunarity;
		float	mOctaves;
		float	mZ;
	};

	class ICETERRAIN_API FBMLayer : public HeightLayer
	{
		public:
												FBMLayer();
		virtual									~FBMLayer();

		virtual	bool							Init(const FBMLAYERCREATE& create);
		virtual	bool							Update(float* field, udword width, udword height)	const;
		virtual	bool							SupportsCache()	const	{ return true;			}
		virtual	bool							SupportsOp()	const	{ return true;			}

		inline_	float							GetOffsetX()	const	{ return mOffsetX;		}
		inline_	float							GetOffsetY()	const	{ return mOffsetY;		}
		inline_	float							GetXFreq()		const	{ return mXFreq;		}
		inline_	float							GetYFreq()		const	{ return mYFreq;		}
		inline_	float							GetZ()			const	{ return mZ;			}

		inline_	void							SetOffsetX(float offset){ mOffsetX = offset;	MarkDirty();	}
		inline_	void							SetOffsetY(float offset){ mOffsetY = offset;	MarkDirty();	}
		inline_	void							SetXFreq(float freq)	{ mXFreq = freq;		MarkDirty();	}
		inline_	void							SetYFreq(float freq)	{ mYFreq = freq;		MarkDirty();	}
		inline_	void							SetZ(float z)			{ mZ = z;				MarkDirty();	}

		inline_	FractalBrownianMotion&			GetFBM()				{ return mFBM;			}
		inline_	const FractalBrownianMotion&	GetFBM()		const	{ return mFBM;			}

		inline_	void							SetFractalIncrement(float h)
												{
													MarkDirty();
													//const float FractalInc = mFBM.GetFractalIncrement();
													const float Lacunarity = mFBM.GetLacunarity();
													const float Octaves = mFBM.GetOctaves();
													mFBM.Init(h, Lacunarity, Octaves);
												}

		inline_	void							SetLacunarity(float lacunarity)
												{
													MarkDirty();
													const float FractalInc = mFBM.GetFractalIncrement();
													//const float Lacunarity = mFBM.GetLacunarity();
													const float Octaves = mFBM.GetOctaves();
													mFBM.Init(FractalInc, lacunarity, Octaves);
												}

		inline_	void							SetOctaves(float octaves)
												{
													MarkDirty();
													const float FractalInc = mFBM.GetFractalIncrement();
													const float Lacunarity = mFBM.GetLacunarity();
													//const float Octaves = mFBM.GetOctaves();
													mFBM.Init(FractalInc, Lacunarity, octaves);
												}

		inline_	void							SetCompatibility(bool b)	{ mCompatibility = b;	MarkDirty();	}

		protected:
				FractalBrownianMotion			mFBM;

				float							mOffsetX;
				float							mOffsetY;
				float							mXFreq;
				float							mYFreq;
				float							mZ;

				bool							mCompatibility;

		template<const HeightLayerOp op>
				void							ComputeValuesT(float* field, udword width, udword height)		const;
		template<const HeightLayerOp op>
				void							ComputeNewValuesT(float* field, udword width, udword height)	const;

		virtual	void							ComputeValues(float* field, udword width, udword height, HeightLayerOp op) const;
	};

#endif // ICEFBMLAYER_H
