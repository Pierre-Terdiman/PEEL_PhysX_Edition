///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains Vertex-Buffer base code.
 *	\file		IceVertexBuffer.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEVERTEXBUFFER_H
#define ICEVERTEXBUFFER_H

	// Forward declarations
	class Renderer;

	enum VBFlag
	{
		VBF_DYNAMIC			= (1<<0),
		VBF_SHARED			= (1<<1),
		VBF_OPTIMIZED		= (1<<2),

		VBF_FORCE_DWORD		= 0x7fffffff
	};

/*	//! Vertex Buffer status flags
	enum VBStatusFlag
	{
		VBSFLAG_OPTIMIZED	= (1<<0),		//!< The vertex buffer has been optimized [mostly obsolete]

		VBSFLAG_FORCE_DWORD	= 0x7fffffff
	};
*/
	//! Vertex buffer descriptor
	struct ICERENDERER_API VBDescriptor
	{
		inline_							VBDescriptor()	{ mSize = SIZEOFOBJECT; }

						udword			mSize;			//!< Size of this structure, in bytes. This member must be initialized before the structure is used.
						udword			mCaps;			//!< Combination of VBFlag
						VertexFormat	mFVF;			//!< Combination of VBFormatFlag
						udword			mNbVertices;	//!< The maximum number of vertices that this vertex buffer can contain. The maximum number of vertices allowed is 0xFFFF.

	};

	//! Vertex Buffer creation structure
	struct ICERENDERER_API VERTEXBUFFERCREATE
	{
		inline_							VERTEXBUFFERCREATE() : mFVF(0), mFlags(0), mNbVerts(0), mData(null)	{}

						VertexFormat	mFVF;			//!< Flexible Vertex Format (combination of VBFormatFlag)
						udword			mFlags;			//!< Creation flags (combination of VBFlag)
						udword			mNbVerts;		//!< Number of vertices
		const			void*			mData;			//!< Possible vertex data (mNbVerts * FVFSize bytes), else null (left to the user)
	};

	//! Vertex buffer class
	class ICERENDERER_API VertexBuffer : public Hardwired
	{
		protected:
										VertexBuffer();
		virtual							~VertexBuffer();

		public:

		// Creation
		virtual			VertexBuffer&	Create(const VERTEXBUFFERCREATE& create);
		virtual			bool			Release()									= 0;

		// Information
//		virtual			bool			GetDescriptor(VBDescriptor& descriptor)		= 0;

		// Data access
		inline_			udword			GetVertexSize()		const
										{
											if(mVertexSize)	return mVertexSize;
											// The vertex size is cached within the class, in mVertexSize.
											mVertexSize = ComputeFVFSize(mFVF);
											return mVertexSize;
										}
		inline_			VertexFormat	GetVertexFormat()	const	{ return mFVF;									}
		inline_			udword			GetNbVerts()		const	{ return mNbVerts;								}
		inline_			udword			GetSize()			const	{ return mNbVerts * GetVertexSize();			}
		inline_			uword			GetHandle()			const	{ return mVI;									}

		inline_			udword			GetFlags()			const	{ return mFlags;								}
		inline_			BOOL			IsDynamic()			const	{ return mFlags & VBF_DYNAMIC;					}
		inline_			BOOL			IsShared()			const	{ return mFlags & VBF_SHARED;					}
		inline_			BOOL			IsOptimized()		const	{ return mFlags & VBF_OPTIMIZED;				}
		inline_			void			SetOptimized()				{ mFlags|=VBF_OPTIMIZED;						}

		protected:
		// Creation settings
						VertexFormat	mFVF;			//!< Flexible Vertex Format
						udword			mFlags;			//!< Creation flags
						udword			mNbVerts;		//!< Total number of vertices in the vertex buffer
		// Lazy-evaluated data
		mutable			udword			mVertexSize;	//!< Size of a single vertex (according to mVF)

		private:		uword			mVI;			//!< Virtual index = buffer identifier
		static			XList*			mHandleManager;
	};

	//### experimental
	class ICERENDERER_API SlidingVB : public Allocateable
	{
		public:
						SlidingVB();
						~SlidingVB();

		bool			Init(Renderer* rd, VertexFormat fvf, udword nb_faces=5000);
		bool			Release();

		bool			Push(ubyte* data);
		bool			Render(Renderer* rd);

		private:
		VertexBuffer*	mVB;
		udword			mOffset;
		udword			mNbFaces;
		udword			mCurrentNb;
	};

#endif // ICEVERTEXBUFFER_H
