///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains viewport-related code.
 *	\file		IceViewport.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEVIEWPORT_H
#define ICEVIEWPORT_H

	//! Clear flags
	enum ClearFlag
	{
		CLEAR_TARGETBUFFER	= 0x00000001,	//!< Clear the target buffer (color buffer or texture)
		CLEAR_ZBUFFER		= 0x00000002,	//!< Clear the Z-Buffer
		CLEAR_STENCILBUFFER	= 0x00000004,	//!< Clear the stencil buffer

		CLEAR_FORCE_DWORD	= 0x7fffffff
	};

	//! Viewport structure
	class ICERENDERER_API Viewport
	{
		public:
							Viewport();
		/* NO VIRTUAL */	~Viewport();

		inline_	float		ComputeRatio()			const	{ return float(mWidth)/float(mHeight);	}
		inline_	float		ComputeInverseRatio()	const	{ return float(mHeight)/float(mWidth);	}
				bool		ComputeWorldRay(sdword xs, sdword ys, float fov, const Point& pos, const ViewMatrix& view, Point& ray) const;

		// MUST MAP D3D VIEWPORT
		// Viewport position
					udword	mX;						//!< Viewport x position
					udword	mY;						//!< Viewport y position
		// Viewport dimensions
					udword	mWidth;					//!< Viewport's width
					udword	mHeight;				//!< Viewport's height
		// Clip volume
					float	mMinZ;					//!< Depth range
					float	mMaxZ;					//!< Depth range
		// ~MUST MAP D3D VIEWPORT
		// Clear flags
				udword		mFlags;
		// Clear values
				udword		mClearValueTarget;		//!< Background color
				float		mClearValueZBuffer;		//!< ZBuffer initial value
				udword		mClearValueStencil;		//!< Stencil buffer initial value
	};

	class ICERENDERER_API Viewports : public Allocateable
	{
		public:
							Viewports()						{}
							~Viewports()					{}

		inline_	void		ResetViewports()				{ mContainer.Reset();													}
		inline_	udword		GetNbViewports()	const		{ return mContainer.GetNbEntries()/(sizeof(Viewport)/sizeof(udword));	}
		inline_	Viewport*	GetViewports()		const		{ return (Viewport*)mContainer.GetEntries();							}

				Viewports&	AddViewport(const Viewport& vp);
		private:
				Container	mContainer;
	};

#endif // ICEVIEWPORT_H
