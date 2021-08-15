///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a decal manager.
 *	\file		IceDecalManager.h
 *	\author		Pierre Terdiman, with help from Jon Watte
 *	\date		January, 25, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEDECALMANAGER_H
#define ICEDECALMANAGER_H

	//#define DECAL_USE_PUSHER
	//#define DECAL_USE_VB
	//#define DECAL_USE_DP
	#define DECAL_USE_VBPOOL

//	typedef PNDT_Vertex	DecalVertex;

	class ICERENDERER_API DecalManager : public Allocateable
	{
		public:
								DecalManager();
								~DecalManager();

				bool			Init(Renderer* rd, udword max_nb_verts, VertexFormat fvf);

				bool			BindDecal(Cell* owner, udword nb_verts, const void* verts);
				bool			UnbindDecals(Cell* owner);
				bool			UnbindAll();

				bool			StartRender(Renderer* rd, bool set_render_states);
				bool			RenderMeshDecals(Renderer* rd, Cell* owner);
				bool			EndRender(Renderer* rd, bool set_render_states);

				bool			SetupRenderStates(Renderer* rd, bool start);

//		virtual	bool			RenderVerts(udword nb_verts, const DecalVertex* verts) = 0;

		inline_	udword			GetVertexSize()			const	{ return mFVFSize;				}

		// Stats
		inline_	void			ResetStats()					{ mNbRenderedDecals = 0;		}
		inline_	udword			GetNbRenderedDecals()	const	{ return mNbRenderedDecals;		}

		private:
				ProjMatrix		mSaved;
				udword			mNbVerts;			//!< Maximal number of decal-vertices in the pool
				udword			mWritePtr;			//!< Virtual running index
				uword*			mCounts;			//!< Cyclic-array of mNbVerts counts
				udword			mLastCookie;
				Cell*			mLastOwner;
				udword			mFVFSize;
#ifdef DECAL_USE_VBPOOL
				VertexBuffer*	mVBPool;
				Container		mIntervals;
#else
				DecalVertex*	mVertexPool;		//!< Cyclic-array of mNbVerts decal-vertices
#endif
				DataBinder		mDB;
				Container		mObsoleteIndices;

#ifdef DECAL_USE_PUSHER
				TriPusher		mPusher;
#endif
#ifdef DECAL_USE_VB
				VertexBuffer*	mVB;
				udword			mOffset;
#endif
			// Stats
				udword			mNbRenderedDecals;	//!< Number of decals rendered

		// Internal methods
				void			Release();
				void*			AllocVerts(udword nb_verts, udword& cookie);
		inline_	udword			ComputeRealIndex(udword virtual_index)	const
								{
									return mNbVerts ? virtual_index % mNbVerts : 0;
								}
	};

#endif // ICEDECALMANAGER_H
