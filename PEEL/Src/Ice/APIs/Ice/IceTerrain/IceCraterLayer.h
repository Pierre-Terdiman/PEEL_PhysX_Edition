///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for crater layers.
 *	\file		IceCraterLayer.h
 *	\author		Pierre Terdiman
 *	\date		March, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECRATERLAYER_H
#define ICECRATERLAYER_H

	struct ICETERRAIN_API CRATERLAYERCREATE : HEIGHTLAYERCREATE
	{
		// TODO: expose more params here
		CRATERLAYERCREATE() : mNbCraters(0)	{}

		udword	mNbCraters;
	};

	class ICETERRAIN_API CraterLayer : public HeightLayer
	{
		public:
						CraterLayer();
		virtual			~CraterLayer();

		virtual	bool	Init(const CRATERLAYERCREATE& create);
		virtual	bool	Update(float* field, udword width, udword height)	const;

		inline_	udword	GetNbCraters()	const	{ return mNbCraters;	}
		inline_	void	SetNbCraters(udword nb)	{ mNbCraters = nb;		}

		protected:
				udword	mNbCraters;

		const	float	mA1;
		const	float	mB;
		const	float	mD;

				float	crater_profile(float nsq_rad)	const;
				float	dissolve(float nsq_rad)			const;
				void	distribute_craters(float *a, unsigned int how_many, int meshsize, bool wrap, float ch_scale)	const;
	};

#endif // ICECRATERLAYER_H
