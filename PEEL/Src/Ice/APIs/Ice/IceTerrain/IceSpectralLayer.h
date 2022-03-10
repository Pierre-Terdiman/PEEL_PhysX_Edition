///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for spectral layers.
 *	\file		IceSpectralLayer.h
 *	\author		Pierre Terdiman
 *	\date		March, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESPECTRALLAYER_H
#define ICESPECTRALLAYER_H

	struct ICETERRAIN_API SPECTRALLAYERCREATE : HEIGHTLAYERCREATE
	{
		SPECTRALLAYERCREATE() : mLevel(0), mSmoothness(0.0f)
		{
		}

		udword	mLevel;
		float	mSmoothness;
	};

	class ICETERRAIN_API SpectralLayer : public HeightLayer
	{
		public:
						SpectralLayer();
		virtual			~SpectralLayer();

		virtual	bool	Init(const SPECTRALLAYERCREATE& create);
		virtual	bool	Update(float* field, udword width, udword height)	const;

		inline_	udword	GetLevel()			const	{ return mLevel;		}
		inline_	float	GetSmoothness()		const	{ return mSmoothness;	}

		inline_	void	SetLevel(udword level)		{ mLevel = level;		}
		inline_	void	SetSmoothness(float s)		{ mSmoothness = s;		}

		protected:
				udword	mLevel;
				float	mSmoothness;
	};

#endif // ICESPECTRALLAYER_H
