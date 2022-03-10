///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a Vertex-Buffer factory.
 *	\file		IceVertexBufferFactory.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEVERTEXBUFFERFACTORY_H
#define ICEVERTEXBUFFERFACTORY_H

	//! a.k.a. "vertex bubble"
	class ICERENDERER_API VBDesc : public Allocateable
	{
		public:
		inline_							VBDesc() : mPool(null), mStartIndex(0), mSize(0)	{}
		inline_							~VBDesc()											{}

						VertexBuffer*	mPool;			//!< Vertex pool
						udword			mStartIndex;	//!< Index of first used vertex
						udword			mSize;			//!< Number of used vertices
	};

	//! Vertex buffer factory
	class ICERENDERER_API VBFactory : public Allocateable
	{
		private:
										VBFactory();
										~VBFactory();
		public:
						VBDesc*			GetVertexBuffer(const VERTEXBUFFERCREATE& create, Renderer* rd);
						bool			ReleaseVertexBuffer(VBDesc*& desc);

						void			ReleaseVertexBuffers();
		private:
#ifndef NEW_VB_MANAGEMENT
						Container		mVBS;			//!< List of vertex buffers
						Container		mVBDesc;
		// Internal methods
						udword			GetNbVertexBuffers()	const	{ return mVBS.GetNbEntries()>>1;	}
#else
						Container		mVBS;			//!< List of vertex buffers
						Container		mUsedIntervals;
						Container		mFreeIntervals;
		// Internal methods
						VBDesc*			GetInterval(VertexFormat fvf, udword required_size);
						bool			ReleaseInterval(VBDesc* it);
#endif

		friend			class			Renderer;
	};

#endif // ICEVERTEXBUFFERFACTORY_H
