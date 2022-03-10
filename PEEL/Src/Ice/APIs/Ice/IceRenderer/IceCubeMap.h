///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains base cubemap code.
 *	\file		IceCubemap.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECUBEMAP_H
#define ICECUBEMAP_H

	struct CubeRenderParams
	{
	};

	typedef bool (*RENDERCALLBACK)(udword user_data, CubeRenderParams& params);

	//! Cubemap base class
	class ICERENDERER_API CubeMap
	{
		public:
										CubeMap() : mUserdata(0), mRenderer(null), mWidth(0), mHeight(0)	{ mViewPoint.Zero(); }
		virtual							~CubeMap()	{}

		// Initialization
		virtual			bool			Init(TEXTURECREATE& create) = 0;

		// Face selection
		virtual			bool			SelectCubeMap(udword i)		= 0;

		// Rendering
		virtual			bool			Render(RENDERCALLBACK callback=null)	= 0;

		// Settings
		inline_			void			SetViewPoint(Point& p)			{ mViewPoint = p;			}
		inline_			void			SetUserData(udword user_data)	{ mUserdata = user_data;	}
		inline_			void			SetRenderer(Renderer* renderer)	{ mRenderer = renderer;		}

		// Data access
		virtual			void*			GetSurface()		= 0;

		protected:
						udword			mUserdata;		//!< Userdata sent to the render callback
						udword			mWidth;			//!< Cubemap width
						udword			mHeight;		//!< Cubemap height
						Point			mViewPoint;		//!< Viewpoint used to generate the cubemap
						Renderer*		mRenderer;		//!< Renderer shortcut
	};


#endif // ICECUBEMAP_H
