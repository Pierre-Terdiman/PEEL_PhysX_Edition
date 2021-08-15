///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TERRAIN_REGION_MANAGER_H
#define TERRAIN_REGION_MANAGER_H

#include "Streaming.h"
#include "TerrainManager.h"
#include "GLMesh.h"

#define USE_TERRAIN_TILE_RENDERER

	class Pint;
	class ManagedTexture;
#ifdef USE_TERRAIN_TILE_RENDERER
	class PintTerrainTileRenderer;
#endif

	class TerrainRegionManager : public StreamInterface
	{
		Pint&					mPint;
		const ManagedTexture*	mMT;
		GLIndexBufferCollection	mIndexBuffers;
		FractalBrownianMotion	mFBM;

		public:	// ### "temp"
		struct RegionData : public Allocateable
		{
			RegionData();
			~RegionData();

			// TODO: use a single struct?
			PtrContainer				mRegionActors;
			PtrContainer				mShapeRenderers;
			Container					mMeshIndices;
#ifdef USE_TERRAIN_TILE_RENDERER
			PintTerrainTileRenderer*	mTTR;
#endif
			PintActorHandle				mTerrainActor;
			PintShapeHandle				mTerrainShape;
			Point*						mVertices;
			Point*						mVertexNormals;
			float*						mDeform;
//			Point						mPos;
			bool						mInQueue;
		};

		public:

			struct Params
			{
				inline_	Params(	udword nb_added_regions_per_frame,
								udword nb_side_verts,
								float world_size,
								bool create_all_initial_tiles) :
					mNbAddedRegionsPerFrame	(nb_added_regions_per_frame),
					mNbVertsPerSide			(nb_side_verts),
					mWorldSize				(world_size),
					mCreateAllInitialTiles	(create_all_initial_tiles)
					{}

				udword	mNbAddedRegionsPerFrame;
				udword	mNbVertsPerSide;
				float	mWorldSize;
				bool	mCreateAllInitialTiles;
			};

							TerrainRegionManager(Pint& pint, const ManagedTexture* mt, const Params& params);
		virtual				~TerrainRegionManager();

		virtual	void		PreUpdate(const AABB& streaming_bounds)		override;
		virtual	void		PostUpdate(udword timestamp)				override;
		virtual	void		AddRegion(StreamRegion& region)				override;
		virtual	void		UpdateRegion(const StreamRegion& region)	override;
		virtual	void		RemoveRegion(const StreamRegion& region)	override;
		virtual	void		RenderDebugRegion(PintRender& render, const Streamer& owner, const StreamRegion& region)	override;

				void		SetRegionLOD(const StreamRegion& region, PintTerrainTileRenderer* ttr);
				void		AddRegionInternal(const StreamRegion& region);

				RegionData*	FindRegion(PintActorHandle actor, PintShapeHandle shape)	const;
				ps::HashMap<ActorShape, RegionData*>	mMap;

				ps::Array<StreamRegion>	mQueue;

				Point		mFocus;
		const	Params		mParams;
				bool		mDebugDrawVertexNormals;
	};

#endif
