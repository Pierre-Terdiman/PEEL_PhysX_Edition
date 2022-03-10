///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for turbulence layers.
 *	\file		IceTurbulenceLayer.h
 *	\author		Pierre Terdiman
 *	\date		March, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICETURBULENCELAYER_H
#define ICETURBULENCELAYER_H

	struct ICETERRAIN_API TURBULENCELAYERCREATE : HEIGHTLAYERCREATE
	{
		TURBULENCELAYERCREATE() :
			mOffsetX	(0.0f),
			mOffsetY	(0.0f),
			mXFreq		(0.0f),
			mYFreq		(0.0f),
			mOctaves	(0.0f),
			mZ			(0.0f)
		{
		}

		float	mOffsetX;
		float	mOffsetY;
		float	mXFreq;		//!< Frequency of the turbulence function along the x axis
		float	mYFreq;		//!< Frequency of the turbulence function along the y axis
		float	mOctaves;	//!< Number of noise layers
		float	mZ;
	};

	class ICETERRAIN_API TurbulenceLayer : public HeightLayer
	{
		public:
							TurbulenceLayer();
		virtual				~TurbulenceLayer();

		virtual	bool		Init(const TURBULENCELAYERCREATE& create);
		virtual	bool		Update(float* field, udword width, udword height)	const;
		virtual	bool		SupportsCache()		const	{ return true;			}
		virtual	bool		SupportsOp()		const	{ return true;			}

		inline_	float		GetOffsetX()		const	{ return mOffsetX;		}
		inline_	float		GetOffsetY()		const	{ return mOffsetY;		}
		inline_	float		GetXFreq()			const	{ return mXFreq;		}
		inline_	float		GetYFreq()			const	{ return mYFreq;		}
		inline_	float		GetOctaves()		const	{ return mOctaves;		}
		inline_	float		GetZ()				const	{ return mZ;			}

		inline_	void		SetOffsetX(float offset)	{ mOffsetX = offset;	MarkDirty();	}
		inline_	void		SetOffsetY(float offset)	{ mOffsetY = offset;	MarkDirty();	}
		inline_	void		SetXFreq(float freq)		{ mXFreq = freq;		MarkDirty();	}
		inline_	void		SetYFreq(float freq)		{ mYFreq = freq;		MarkDirty();	}
		inline_	void		SetOctaves(float octaves)	{ mOctaves = octaves;	MarkDirty();	}
		inline_	void		SetZ(float z)				{ mZ = z;				MarkDirty();	}

		protected:
				PerlinNoise	mNoise;

				float		mOffsetX;
				float		mOffsetY;
				float		mXFreq;
				float		mYFreq;
				float		mOctaves;
				float		mZ;

		template<const HeightLayerOp op>
				void		ComputeValuesT(float* field, udword width, udword height)	const;

		virtual	void		ComputeValues(float* field, udword width, udword height, HeightLayerOp op)	const;
	};

#endif // ICETURBULENCELAYER_H
