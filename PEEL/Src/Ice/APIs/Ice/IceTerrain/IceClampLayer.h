///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for clamp layers.
 *	\file		IceClampLayer.h
 *	\author		Pierre Terdiman
 *	\date		March, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECLAMPLAYER_H
#define ICECLAMPLAYER_H

	struct ICETERRAIN_API CLAMPLAYERCREATE : HEIGHTLAYERCREATE
	{
		CLAMPLAYERCREATE() : mCutGreaterThan(false)
		{
		};

		// Clamp height is going to be mAmplitude
		bool	mCutGreaterThan;
	};

	class ICETERRAIN_API ClampLayer : public HeightLayer
	{
		public:
						ClampLayer();
		virtual			~ClampLayer();

		virtual	bool	Init(const CLAMPLAYERCREATE& create);
		virtual	bool	Update(float* field, udword width, udword height)	const;

		inline_	float	GetClampHeight()	const	{ return mAmplitude;		}
		inline_	bool	GetCutGreaterThan()	const	{ return mCutGreaterThan;	}

		inline_	void	SetClampHeight(float h)		{ mAmplitude = h;			}
		inline_	void	SetCutGreaterThan(bool b)	{ mCutGreaterThan = b;		}

		protected:
				bool	mCutGreaterThan;
	};

#endif // ICECLAMPLAYER_H
