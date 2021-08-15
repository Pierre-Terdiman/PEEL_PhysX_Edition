///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for constant layers.
 *	\file		IceConstantLayer.h
 *	\author		Pierre Terdiman
 *	\date		March, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECONSTANTLAYER_H
#define ICECONSTANTLAYER_H

	// Note: this layer is obsolete, replaced with the multiply-add layer.

	struct ICETERRAIN_API CONSTANTLAYERCREATE : HEIGHTLAYERCREATE
	{
		CONSTANTLAYERCREATE() : mConstantHeight(0.0f)	{}

		float	mConstantHeight;	//!< Constant height
	};

	class ICETERRAIN_API ConstantLayer : public HeightLayer
	{
		public:
						ConstantLayer();
		virtual			~ConstantLayer();

		virtual	bool	Init(const CONSTANTLAYERCREATE& create);
		virtual	bool	Update(float* field, udword width, udword height)	const;

		inline_	float	GetConstantHeight()	const	{ return mConstantHeight;	}
		inline_	void	SetConstantHeight(float h)	{ mConstantHeight = h;		}

		protected:
				float	mConstantHeight;
	};

#endif // ICECONSTANTLAYER_H
