///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a screen-quad API.
 *	\file		IceScreenQuad.h
 *	\author		Pierre Terdiman
 *	\date		September, 24, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESCREENQUAD_H
#define ICESCREENQUAD_H

	enum ScreenQuadFlag
	{
		SCREEN_QUAD_ENABLE_LIGHTING			= (1<<0),	//!< Setup lighting, or leave it untouched
		SCREEN_QUAD_ENABLE_ALPHA_BLENDING	= (1<<1),	//!< Setup default alpha blending mode, or leave it untouched
		SCREEN_QUAD_ENABLE_SHADOWS			= (1<<2),	//!< Setup stencil mode, else leave it untouched
		SCREEN_QUAD_SHADOWS_INIT			= (1<<3),	//!< If SCREEN_QUAD_SHADOWS_ON, mark the init or end of shadowing
		SCREEN_QUAD_DISABLE_COLOR_BUFFER	= (1<<4),	//!< Enable or disable color buffer updates
		SCREEN_QUAD_DISABLE_TEXTURING		= (1<<5),	//!< Enable or disable texturing
		SCREEN_QUAD_ENABLE_ALPHA_ONE		= (1<<6),	//!< 
	};

	class ICERENDERER_API ScreenQuad
	{
		public:
				ScreenQuad();
		inline_	~ScreenQuad()	{}

		udword	mLeftUpColor;		//!< Color for left-up vertex
		udword	mLeftDownColor;		//!< Color for left-down vertex
		udword	mRightUpColor;		//!< Color for right-up vertex
		udword	mRightDownColor;	//!< Color for right-down vertex
		float	mAlpha;				//!< Alpha value used if > 0.0f
		udword	mFlags;				//!< Combination of ScreenQuadFlags
		float	mX0, mY0;			//!< Up-left coordinates
		float	mX1, mY1;			//!< Bottom-right coordinates
		float	mZoomOffset;		//!< Offset for zoomed texture
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Renders a 2D-quad over the whole screen.
	 *	\param		renderer	[in] current renderer
	 *	\param		screen_quad	[in] screen quad parameters
	 *	\return		true if success
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ICERENDERER_API bool RenderScreenQuad(Renderer* renderer, const ScreenQuad& screen_quad);

#endif // ICESCREENQUAD_H
