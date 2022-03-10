///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains base class for render objects.
 *	\file		IceRenderBase.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERENDERBASE_H
#define ICERENDERBASE_H

	#define RENDER_STATISTICS

	// Forward declarations
	class Renderer;

	class ICERENDERER_API RenderBase// : public Allocateable	<= can't use this one because some classes inherit from Cell already
	{
		public:
							RenderBase();
		virtual				~RenderBase();

		// Renderer shortcut
		static	Renderer*	mRenderer;

#ifdef RENDER_STATISTICS

		// Statistics
		// Objects
		static	udword		mNbVertexBuffers;
		static	udword		mVBTotalSize;
		static	udword		mNbTextures;
		// Locks
		static	udword		mNbVBLocks;
		static	udword		mNbVBUnlocks;
		static	udword		mNbTextureLocks;
		static	udword		mNbTextureUnlocks;
		// Render calls
		static	udword		mNbDrawPrimitive;
		static	udword		mNbDrawIndexedPrimitive;
		static	udword		mNbDrawPrimitiveVB;
		static	udword		mNbDrawIndexedPrimitiveVB;

				void		ResetFrameRenderStats();
				void		ResetAllRenderStats();
#endif
	};

#endif // ICERENDERBASE_H
