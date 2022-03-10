///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for midpoint layers.
 *	\file		IceMidpointLayer.h
 *	\author		Pierre Terdiman
 *	\date		March, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMIDPOINTLAYER_H
#define ICEMIDPOINTLAYER_H

	struct ICETERRAIN_API MIDPOINTLAYERCREATE : HEIGHTLAYERCREATE
	{
		MIDPOINTLAYERCREATE() : mDisplacement(0.0f), mSmoothness(0.0f)
		{
		}

		float	mDisplacement;	//!< "Fracture" value
		float	mSmoothness;	//!< Normal value is 0.5
	};

	class ICETERRAIN_API MidpointLayer : public HeightLayer
	{
		public:
						MidpointLayer();
		virtual			~MidpointLayer();

		virtual	bool	Init(const MIDPOINTLAYERCREATE& create);
		virtual	bool	Update(float* field, udword width, udword height)	const;

		inline_	float	GetDisplacement()	const	{ return mDisplacement;	}
		inline_	float	GetSmoothness()		const	{ return mSmoothness;	}

		inline_	void	SetDisplacement(float v)	{ mDisplacement = v;	}
		inline_	void	SetSmoothness(float v)		{ mSmoothness = v;		}

		protected:
				float	mDisplacement;
				float	mSmoothness;

		mutable	bool*	mDone;

				void	_Compute(float* field, udword x0, udword y0, udword size, float value)	const;
	};

#endif // ICEMIDPOINTLAYER_H
