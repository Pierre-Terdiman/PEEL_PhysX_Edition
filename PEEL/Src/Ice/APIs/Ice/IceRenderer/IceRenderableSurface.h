///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a renderable surface.
 *	\file		IceRenderableSurface.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERENDERABLESURFACE_H
#define ICERENDERABLESURFACE_H

	//! A renderable surface.
	class ICERENDERER_API RenderableSurface : public ConsolidatedSurface
	{
		public:
										RenderableSurface();
		virtual							~RenderableSurface();

		// Destruction
				RenderableSurface&		FreeIndexBuffer();
				RenderableSurface&		FreeVertexBuffer();
				void					FreeRenderingData();

		// Refresh hardwired VB
				bool					RefreshVB();
		// Rendering
		inline_	bool					DrawSubset(udword i)	const
										{
											// Render from VB & IB

											// This failed when mIB was null because modified between batching & rendering
											// We could enforce some rules to save the tests..
//											return mRd->DrawIndexedPrimitive(mVB, mDesc, mIB, i);
											return mRd->DrawIndexedPrimitive(GetVBDesc(), GetIndexBuffer(), i);
										}

/*		inline_	bool					DrawSubset2(udword i)	const
										{
											const Consolidation* Data = GetRenderInfos();
											IndexBuffer* IB = GetIndexBuffer();
											VertexBuffer* VB = GetVertexBuffer();	// ### warning, creates a VB anyway

											// Render from the system memory copy
											uword* VRefs = (uword*)IB->GetIndices();
											Subset* SS = IB->GetSubset(i);
											return mRd->DrawIndexedPrimitive(
												IB->GetPrimType(),
												VB->GetVertexFormat(),
												Data->GetVBCopy() + VB->GetVertexSize()*SS->mMinIndex,
												Data->GetNbVerts() - SS->mMinIndex,
												&VRefs[SS->mOffset],
												SS->mCount,
												0);
										}*/

				bool					SortSubset(udword id, const Point& local_dir, TriangleSortKey sort);
		// Data access
		inline_	VertexBuffer*			GetVertexBuffer()	const
										{
											if(!mVB)	const_cast<RenderableSurface* const>(this)->CreateVertexBuffer();	// "mutable method"
											return mVB;
										}
		inline_	IndexBuffer*			GetIndexBuffer()	const
										{
											if(!mIB)	const_cast<RenderableSurface* const>(this)->CreateIndexBuffer();	// "mutable method"
											return mIB;
										}
		inline_	const VBDesc*			GetVBDesc()			const
										{
											if(!mDesc)	const_cast<RenderableSurface* const>(this)->CreateVertexBuffer();	// "mutable method"
											return mDesc;
										}

		inline_	void					RenderFaces()
										{
											udword Nb = GetIndexBuffer()->GetNbSubsets();
											for(udword i=0;i<Nb;i++)
											{
												DrawSubset(i);
											}
										}

		inline_	bool					IsSharedVB()		const	{ return mSharedVB;	}
		inline_	bool					IsStatic()			const	{ return mIsStatic;	}
		inline_	bool					UsesStrips()		const	{ return mUseStrips;}

		// Select lists/strips and save selection for a persistent result
		inline_	bool					SelectStrips(bool flag)		{ mUseStrips = flag; return CreateIndexBuffer();	}

		private:
		// Hardwired
				VertexBuffer*			mVB;			//!< Hardwired geometry	// ### could be removed ? use mDesc->mPool
				IndexBuffer*			mIB;			//!< Hardwired topology
		// VB desc
				VBDesc*					mDesc;
				bool					mSharedVB;
				bool					mIsStatic;
				bool					mUseStrips;
		// Shared data
		static	float					mSortingLimit;	//!< Limit |cos(angle)| beyond which sorting is performed
		static	udword					mFlags;			//!< Internal flags

		protected:
		// Creation
				bool					CreateVertexBuffer();
				bool					CreateIndexBuffer();
		override(ConsolidatedSurface)	bool		Consolidate(const RENDERABLESURFACECREATE& create);
	};

#endif // ICERENDERABLESURFACE_H
