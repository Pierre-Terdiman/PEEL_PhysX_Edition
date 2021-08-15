///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for sinus layers.
 *	\file		IceSinusLayer.h
 *	\author		Pierre Terdiman
 *	\date		March, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESINUSLAYER_H
#define ICESINUSLAYER_H

	struct ICETERRAIN_API SINUSLAYERCREATE : HEIGHTLAYERCREATE
	{
		SINUSLAYERCREATE() :
			mXFreq	(0.0f),
			mYFreq	(0.0f),
			mPhaseX	(0.0f),
			mPhaseY	(0.0f)
		{
		}

		float	mXFreq;		//!< Frequency of the sinus function along the x axis
		float	mYFreq;		//!< Frequency of the sinus function along the y axis
		float	mPhaseX;
		float	mPhaseY;
	};

	class ICETERRAIN_API SinusLayer : public HeightLayer
	{
		public:
						SinusLayer();
		virtual			~SinusLayer();

		virtual	bool	Init(const SINUSLAYERCREATE& create);
		virtual	bool	Update(float* field, udword width, udword height)	const;

		inline_	float	GetXFreq()			const	{ return mXFreq;	}
		inline_	float	GetYFreq()			const	{ return mYFreq;	}
		inline_	float	GetPhaseX()			const	{ return mPhaseX;	}
		inline_	float	GetPhaseY()			const	{ return mPhaseY;	}

		inline_	void	SetXFreq(float freq)		{ mXFreq = freq;	}
		inline_	void	SetYFreq(float freq)		{ mYFreq = freq;	}
		inline_	void	SetPhaseX(float phase)		{ mPhaseX = phase;	}
		inline_	void	SetPhaseY(float phase)		{ mPhaseY = phase;	}

		protected:
				float	mXFreq;
				float	mYFreq;
				float	mPhaseX;
				float	mPhaseY;
	};

#endif // ICESINUSLAYER_H
