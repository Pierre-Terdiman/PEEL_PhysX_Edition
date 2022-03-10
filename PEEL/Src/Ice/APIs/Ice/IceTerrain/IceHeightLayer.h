///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for height layers.
 *	\file		IceHeightLayer.h
 *	\author		Pierre Terdiman
 *	\date		March, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEHEIGHTLAYER_H
#define ICEHEIGHTLAYER_H

	enum HeightLayerType
	{
		HEIGHT_LAYER_TYPE_BITMAP,
		HEIGHT_LAYER_TYPE_CELLULAR,
		HEIGHT_LAYER_TYPE_CLAMP,
		HEIGHT_LAYER_TYPE_CONSTANT,
		HEIGHT_LAYER_TYPE_CRATER,
		HEIGHT_LAYER_TYPE_EROSION,
		HEIGHT_LAYER_TYPE_FBM,
		HEIGHT_LAYER_TYPE_FILTER,
		HEIGHT_LAYER_TYPE_MIDPOINT,
		HEIGHT_LAYER_TYPE_POWER,
		HEIGHT_LAYER_TYPE_RIDGED,
		HEIGHT_LAYER_TYPE_SCALE,
		HEIGHT_LAYER_TYPE_SINUS,
		HEIGHT_LAYER_TYPE_SPECTRAL,
		HEIGHT_LAYER_TYPE_TURBULENCE,
		HEIGHT_LAYER_TYPE_MADD,

		HEIGHT_LAYER_TYPE_UNDEFINED	= 0x7fffffff
	};

	enum HeightLayerOp
	{
		HEIGHT_LAYER_OP_SET,
		HEIGHT_LAYER_OP_ADD,
		HEIGHT_LAYER_OP_MULTIPLY,
	};

	struct ICETERRAIN_API HEIGHTLAYERCREATE
	{
		HEIGHTLAYERCREATE() : mWidth(0), mHeight(0), mAmplitude(0.0f), mOp(HEIGHT_LAYER_OP_ADD)
		{
		}

		udword			mWidth;		//!< Layer's width
		udword			mHeight;	//!< Layer's height
		float			mAmplitude;	//!< Layer's amplitude
		HeightLayerOp	mOp;		//!< Layer's op
	};

	class ICETERRAIN_API HeightLayer : public Allocateable
	{
		public:
								HeightLayer();
		virtual					~HeightLayer();

		virtual	bool			Init(const HEIGHTLAYERCREATE& create);
		virtual	bool			Update(float* field, udword width, udword height)	const	= 0;
		inline_	bool			Update(Heightfield& field)							const
								{
									if(!mEnabled)
										return false;
									return Update(field.GetHeights(), field.GetNbU(), field.GetNbV());
								}
		virtual	bool			SupportsCache()		const	{ return false;			}
		virtual	bool			SupportsOp()		const	{ return false;			}

		inline_	HeightLayerType	GetType()			const	{ return mType;			}
		inline_	udword			GetWidth()			const	{ return mWidth;		}
		inline_	udword			GetHeight()			const	{ return mHeight;		}
		inline_	float			GetAmplitude()		const	{ return mAmplitude;	}
		inline_	HeightLayerOp	GetOp()				const	{ return mOp;			}
		inline_	bool			IsCacheEnabled()	const	{ return mCacheEnabled;	}
		inline_	bool			IsLayerEnabled()	const	{ return mEnabled;		}

		inline_	void			SetAmplitude(float h)		{ mAmplitude = h;	MarkDirty();	}
		inline_	void			SetOp(HeightLayerOp op)		{ mOp = op;							}
		inline_	void			SetLayerEnabled(bool b)		{ mEnabled = b;						}

				void			SetCacheEnabled(bool b);

				void*			mUserData;
		protected:
				HeightLayerType	mType;
				udword			mWidth;
				udword			mHeight;
				float			mAmplitude;
				HeightLayerOp	mOp;

				float*			mCache;
				bool			mCacheEnabled;
		mutable	bool			mDirty;
				bool			mEnabled;

		inline_	void			MarkDirty()
								{
									mDirty = true;
								}

		virtual	void			ComputeValues(float* field, udword width, udword height, HeightLayerOp op)	const	{}
				bool			UpdateAndCache(float* field, udword width, udword height)					const;
	};

#endif // ICEHEIGHTLAYER_H
