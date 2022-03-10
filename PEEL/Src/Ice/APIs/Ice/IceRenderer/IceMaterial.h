///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a hardware material class.
 *	\file		IceMaterial.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMATERIAL_H
#define ICEMATERIAL_H

	//! This structure holds the material properties. You'll send a pointer to such a structure to your rendering API.
	//! You can't directly use a Material pointer since the v-table adds one dword.
	class ICERENDERER_API MaterialProps
	{
		public:
		//! Constructor
									MaterialProps()		{	SetDefault();	}
		//! Destructor
									~MaterialProps()	{}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Setups default material properties.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					void			SetDefault();

		//! Operator for "if(MaterialProps==MaterialProps)"
					bool			operator==(const MaterialProps& obj);
		//! Operator for "if(MaterialProps!=MaterialProps)"
		inline_		bool			operator!=(const MaterialProps& obj)	{ return !(*this==obj);	}

					RGBAColor		mDiffuseColor;			//!< Diffuse color RGBA
					RGBAColor		mAmbientColor;			//!< Ambient color RGB
					RGBAColor		mSpecularColor;			//!< Specular 'shininess'
					RGBAColor		mEmissiveColor;			//!< Emissive color RGB
					float			mPower;					//!< Sharpness if specular highlight
	};

	struct ICERENDERER_API MATERIALCREATE
	{
						MATERIALCREATE();

		MaterialProps	mProps;							//!< Material properties
		CULLMODE		mCullMode;						//!< Culling mode
	};

	class ICERENDERER_API Material : public Allocateable
	{
		public:
		//! Constructor
										Material() : mCullMode(CULL_FORCE_DWORD), mID(0xffff), mRefCount(0)	{}
		//! Destructor
		virtual							~Material()															{}

		//! Operator for "if(Material==Material)"
					bool				operator==(const Material& obj);
		//! Operator for "if(Material!=Material)"
		inline_		bool				operator!=(const Material& obj)	{ return !(*this==obj);	}

					MaterialProps		mProps;			//!< Material properties
					CULLMODE			mCullMode;		//!< Culling mode
		// Management
					uword				mID;
					uword				mRefCount;
	};

#endif // ICEMATERIAL_H
