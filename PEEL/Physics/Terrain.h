///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TERRAIN_H
#define TERRAIN_H

#include "PintDef.h"

	struct /*ICETERRAIN_API*/ CUSTOMLAYERCREATE : HEIGHTLAYERCREATE
	{
		const float*	mHeights;
	};

	class /*ICETERRAIN_API*/ CustomLayer : public HeightLayer
	{
		public:
						CustomLayer();
		virtual			~CustomLayer();

		virtual	bool	Init(const CUSTOMLAYERCREATE& create);
		virtual	bool	Update(float* field, udword width, udword height)	const;

		inline_	float*	GetHeights()	{ return mHeights;	}

		protected:
				float*	mHeights;
	};



//#define SUPPORT_TERRAIN_TEXTURE

#ifdef SUPPORT_TERRAIN_TEXTURE
	class PintGLMeshShapeRendererEx;
	class ManagedTexture;
#else
	class PintGLMeshShapeRenderer;
#endif

	// This class captures the engine-independent terrain data, i.e.
	// the terrain data that can be shared between all physics engines.
	class TerrainData : public Allocateable
	{
		public:
											TerrainData();
											~TerrainData();

				bool						Init(udword nb_x, udword nb_y, float scale_x, float scale_y);
				void						Release();

				void						AddLayer(const HeightLayer* layer);
				bool						RemoveLayer(const HeightLayer* layer);
		inline_	udword						GetNbLayers()		const	{ return mLayers.GetNbEntries();									}
		inline_	HeightLayer*				GetLayer(udword i)			{ return reinterpret_cast<HeightLayer*>(mLayers.GetEntry(i));		}
		inline_	const HeightLayer*			GetLayer(udword i)	const	{ return reinterpret_cast<const HeightLayer*>(mLayers.GetEntry(i));	}

				void						Evaluate(Heightfield& hf)	const;

				void						UpdateVertices(Point* V);
				void						UpdateRenderer();

				// Master surface
#ifdef SUPPORT_TERRAIN_TEXTURE
				MultiSurface				mSurface;
				// At the moment the PintGLMeshShapeRenderer is the only one that supports deformable meshes
				PintGLMeshShapeRendererEx*	mRenderer;
				const ManagedTexture*		mTexture;
#else
				IndexedSurface				mSurface;
				// At the moment the PintGLMeshShapeRenderer is the only one that supports deformable meshes
				PintGLMeshShapeRenderer*	mRenderer;
#endif
				PtrContainer				mLayers;
				udword						mNbX, mNbY;
	};

	bool			RegisterTerrainData(PintActorHandle owner, TerrainData* terrain_data);
	bool			UnregisterTerrainData(PintActorHandle owner);
	TerrainData*	GetTerrainData(PintActorHandle owner);
	void			UnregisterAllTerrainData();

	TerrainData*	CreateTerrainData();
	void			ReleaseAllTerrainData();

	// modified IndexedSurface::MakePlane
	bool			MakePlane(IndexedSurface& surface, udword nbu, udword nbv, Quad* fit, bool alternative_topology);
	void			CreateAlternativeGridTopologyLOD(IndexedTriangle* tris, udword nb_faces, udword nbu, udword nbv, udword nbu2, udword nbv2, udword lod);

	float			Terrain_Evaluate(const FractalBrownianMotion& fbm, const float ScaleY, const float Scale, float x, float z);
	void			CreateProceduralTerrain(Point* V, Point* N, udword nbu, udword nbv, const Point& min, const Point& max, const FractalBrownianMotion& fbm, const Point& offset);

#endif
