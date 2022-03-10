///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for cellular layers.
 *	\file		IceCellularLayer.h
 *	\author		Pierre Terdiman
 *	\date		March, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECELLULARLAYER_H
#define ICECELLULARLAYER_H

	struct ICETERRAIN_API CELLULARLAYERCREATE : HEIGHTLAYERCREATE
	{
		CELLULARLAYERCREATE() :
			mOffsetX	(0.0f),
			mOffsetY	(0.0f),
			mScaleX		(0.0f),
			mScaleY		(0.0f),
			mZ			(0.0f)
		{
		}

		float	mOffsetX;
		float	mOffsetY;
		float	mScaleX;
		float	mScaleY;
		float	mZ;
	};

	class ICETERRAIN_API CellularLayer : public HeightLayer
	{
		public:
							CellularLayer();
		virtual				~CellularLayer();

		virtual	bool		Init(const CELLULARLAYERCREATE& create);
		virtual	bool		Update(float* field, udword width, udword height)	const;
		virtual	bool		SupportsCache()	const	{ return true;			}
		virtual	bool		SupportsOp()	const	{ return true;			}

		inline_	float		GetOffsetX()	const	{ return mOffsetX;		}
		inline_	float		GetOffsetY()	const	{ return mOffsetY;		}
		inline_	float		GetScaleX()		const	{ return mScaleX;		}
		inline_	float		GetScaleY()		const	{ return mScaleY;		}
		inline_	float		GetZ()			const	{ return mZ;			}

		inline_	void		SetOffsetX(float offset){ mOffsetX = offset;	MarkDirty();	}
		inline_	void		SetOffsetY(float offset){ mOffsetY = offset;	MarkDirty();	}
		inline_	void		SetScaleX(float scale)	{ mScaleX = scale;		MarkDirty();	}
		inline_	void		SetScaleY(float scale)	{ mScaleY = scale;		MarkDirty();	}
		inline_	void		SetZ(float z)			{ mZ = z;				MarkDirty();	}

		protected:
				WorleyNoise	mNoise;
				float		mOffsetX;
				float		mOffsetY;
				float		mScaleX;
				float		mScaleY;
				float		mZ;

		template<const HeightLayerOp op>
				void		ComputeValuesT(float* field, udword width, udword height)	const;

		virtual	void		ComputeValues(float* field, udword width, udword height, HeightLayerOp op)	const;
	};

#endif // ICECELLULARLAYER_H
