///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TerrainRegionManager.h"
#include "PintObjectsManager.h"
#include "Terrain.h"

#ifdef USE_TERRAIN_TILE_RENDERER
	#include "PintTerrainTileRenderer.h"
	#include "Camera.h"
#endif

static const bool gEnableLOD = false;

TerrainRegionManager::RegionData::RegionData() :
#ifdef USE_TERRAIN_TILE_RENDERER
	mTTR			(null),
#endif
	mTerrainActor	(null),
	mTerrainShape	(null),
	mVertices		(null),
	mVertexNormals	(null),
	mDeform			(null),
//	mPos			(Point(0.0f, 0.0f, 0.0f))
	mInQueue		(false)
{
}

TerrainRegionManager::RegionData::~RegionData()
{
	ICE_FREE(mDeform);
	DELETEARRAY(mVertexNormals);
	DELETEARRAY(mVertices);
}

TerrainRegionManager::TerrainRegionManager(Pint& pint, const ManagedTexture* mt, const Params& params) :
	mPint					(pint),
	mMT						(mt),
//	mFBM					(0.8f, 0.9f, 12.0f),
	mFBM					(1.5f, 1.2f, 32.0f),
//	mFBM					(1.5f, 1.2f, 16.0f),
//	mFBM					(1.5f, 1.2f, 64.0f),
	mFocus					(Point(0.0f, 0.0f, 0.0f)),
	mParams					(params),
	mDebugDrawVertexNormals	(false)
{
#ifdef USE_TERRAIN_TILE_RENDERER
	IndexedSurface IS;
	//IS.MakePlane(nb_verts_per_side, nb_verts_per_side);
	MakePlane(IS, params.mNbVertsPerSide, params.mNbVertsPerSide, null, true);
	const SurfaceInterface SI = IS.GetSurfaceInterface();

	GLIndexBuffer* IB = ICE_NEW(GLIndexBuffer);
	IB->Create(SI.mNbFaces, SI.mDFaces, SI.mWFaces);
	mIndexBuffers.AddIndexBuffer(*IB);

	if(gEnableLOD)
	{
		udword lod=2;
		while(1)
		{
			const udword nbu = params.mNbVertsPerSide;
			const udword nbv = params.mNbVertsPerSide;
			const udword nbu2 = (nbu/lod)+1;
			const udword nbv2 = (nbv/lod)+1;
			if(nbu2==3 || nbv2==3)
				break;
			printf("Creating LOD %d*%d\n", nbu2, nbv2);
			const udword NbFaces = (nbu2-1)*(nbv2-1)*2;
			IndexedTriangle* Topo = ICE_NEW(IndexedTriangle)[NbFaces];
			CreateAlternativeGridTopologyLOD(Topo, NbFaces, nbu, nbv, nbu2, nbv2, lod);

			GLIndexBuffer* IB = ICE_NEW(GLIndexBuffer);
			IB->Create(NbFaces, Topo->mRef, null);
			mIndexBuffers.AddIndexBuffer(*IB);

			DELETEARRAY(Topo);

			lod *= 2;
		}
	}
#endif
}

TerrainRegionManager::~TerrainRegionManager()
{
#ifdef USE_TERRAIN_TILE_RENDERER
	const udword NbIndexBuffers = mIndexBuffers.GetNbIndexBuffers();
	for(udword i=0;i<NbIndexBuffers;i++)
	{
		const GLIndexBuffer* IB = mIndexBuffers.GetIndexBuffer(i);
		DELETESINGLE(IB);
	}
#endif
}

void TerrainRegionManager::SetRegionLOD(const StreamRegion& region, PintTerrainTileRenderer* ttr)
{
	Point Pos;
	region.mCellBounds.GetCenter(Pos);

	// ######## test
	//const Point CamPos = GetCameraPos();
	const Point& CamPos = mFocus;
	const float DistToCam = CamPos.Distance(Pos);
	float LOD = DistToCam/mParams.mWorldSize;
	LOD = sqrtf(LOD);
	if(LOD>1.0f)
		LOD = 1.0f;
	//LOD *= LOD;
	udword CurrentLOD = udword(LOD*mIndexBuffers.GetNbIndexBuffers()-1);
	//const GLIndexBuffer* IndexBuffer = mIndexBuffers.GetIndexBuffer(CurrentLOD);
	ttr->mLOD = CurrentLOD;
}

void TerrainRegionManager::PreUpdate(const AABB& streaming_bounds)
{
	streaming_bounds.GetCenter(mFocus);
}

void TerrainRegionManager::PostUpdate(udword timestamp)
{
	const udword Nb = mQueue.size();
	if(!Nb)
		return;

	if(timestamp==0 && mParams.mCreateAllInitialTiles)
	{
		for(udword i=0;i<Nb;i++)
			AddRegionInternal(mQueue[i]);
		mQueue.clear();
	}
	else
	{
		// ######## test
//		const Point CamPos = GetCameraPos();
		const Point& CamPos = mFocus;

		float* Keys = new float[Nb];
		for(udword i=0;i<Nb;i++)
		{
			Point Pos;
			mQueue[i].mCellBounds.GetCenter(Pos);
			Keys[i] = CamPos.SquareDistance(Pos);
		}

		// 256 / 64 / 65

		RadixSort RS;
		const udword* Sorted = RS.Sort(Keys, Nb).GetRanks();
		DELETEARRAY(Keys);

		const udword NbAddedPerFrame = TMin(mParams.mNbAddedRegionsPerFrame, Nb);
		for(udword i=0;i<NbAddedPerFrame;i++)
		{
			StreamRegion& R = mQueue[Sorted[i]];
			AddRegionInternal(R);
			ASSERT(R.mUserData);
			R.mUserData = null;	// Mark for removal below, we cannot remove immediately since it would invalidate sorted indices
		}

		udword NbRemoved = 0;
		udword NbToGo = Nb;
		udword i=0;
		while(NbToGo--)
		{
			if(!mQueue[i].mUserData)
			{
				mQueue.replaceWithLast(i);
				NbRemoved++;
				if(NbRemoved==NbAddedPerFrame)
					break;
			}
			else
				i++;
		}
	}
}

void TerrainRegionManager::AddRegion(StreamRegion& region)
{
	SPY_ZONE("TerrainRegionManager::AddRegion")

	if(1)
	{
		RegionData* RD = ICE_NEW(RegionData);
		RD->mInQueue = true;
		region.mUserData = RD;
		mQueue.pushBack(region);
		return;
	}
}

void TerrainRegionManager::AddRegionInternal(const StreamRegion& region)
{
	SPY_ZONE("TerrainRegionManager::AddRegionInternal")

	const udword SeedPart0 = udword(region.mKey);
	const udword SeedPart1 = udword(region.mKey>>32);
	const udword Seed = SeedPart0 ^ SeedPart1;
	BasicRandom Rnd(Seed);

/*
uint32_t pcg32(uint64_t* s) {
uint32_t x = ((*s >> 18u) ^ *s) >> 27u;
uint32_t r = *s >> 59u;
*s  = *s * 6364136223846793005ULL + 1;
return (x >> r) | (x << ((-r) & 31));
}
*/


	//printf("AddRegion %d\n", region.mKey);

//	RegionData* RD = ICE_NEW(RegionData);
	RegionData* RD = reinterpret_cast<RegionData*>(region.mUserData);
	ASSERT(RD->mInQueue);
	RD->mInQueue = false;

	Point C,E;
	region.mCellBounds.GetCenter(C);
	region.mCellBounds.GetExtents(E);

	if(0)
	{
		PINT_BOX_CREATE BoxDesc(E.x, 0.5f, E.z);
		BoxDesc.mRenderer = CreateBoxRenderer(BoxDesc.mExtents);

/*		RGBAColor color;
		color.R = float((0.5f + Rnd.RandomFloat())*255.0f);
		color.G = float((0.5f + Rnd.RandomFloat())*255.0f);
		color.B = float((0.5f + Rnd.RandomFloat())*255.0f);
		color.A = 1.0f;
		BoxDesc.mRenderer = CreateColorShapeRenderer(BoxDesc.mRenderer, color);*/

		RD->mShapeRenderers.AddPtr(BoxDesc.mRenderer);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass		= 0.0f;
		ObjectDesc.mPosition.x	= C.x;
		ObjectDesc.mPosition.y	= 0.0f;
		ObjectDesc.mPosition.z	= C.z;
		const PintActorHandle h = CreatePintObject(mPint, ObjectDesc);
		RD->mRegionActors.AddPtr(h);
	}

	if(1)
	{
		const Point p0(region.mCellBounds.mMin.x, 0.0f, region.mCellBounds.mMin.z);
		const Point p1(region.mCellBounds.mMin.x, 0.0f, region.mCellBounds.mMax.z);
		const Point p2(region.mCellBounds.mMax.x, 0.0f, region.mCellBounds.mMin.z);
		const Point p3(region.mCellBounds.mMax.x, 0.0f, region.mCellBounds.mMax.z);

		/*const*/ Quad Q(p0, p1, p2, p3);

		const udword N = mParams.mNbVertsPerSide;
		const udword NbU = N;
		const udword NbV = N;

#ifdef USE_TEXTURE
		MultiSurface MS;
		MS.MakePlane(NbU, NbV, true, &Q);
		Point* V = MS.GetVerts();
#else
		IndexedSurface IS;
//		IS.MakePlane(NbU, NbV, &Q);
			MakePlane(IS, NbU, NbV, &Q, true);
		Point* V = IS.GetVerts();
#endif

		const udword NbVerts = IS.GetNbVerts();
		RD->mVertices = ICE_NEW(Point)[NbVerts];
		RD->mVertexNormals = ICE_NEW(Point)[NbVerts];
		RD->mDeform = (float*)ICE_ALLOC(sizeof(float)*NbVerts);
		ZeroMemory(RD->mDeform, sizeof(float)*NbVerts);
		//for(udword i=0;i<NbVerts;i++)
		//	RD->mVertexNormals[i] = Point(0.0f, 1.0f, 0.0f);

		if(1)
		{
			CreateProceduralTerrain(V, RD->mVertexNormals, NbU, NbV, region.mCellBounds.mMin, region.mCellBounds.mMax, mFBM, C);
			CopyMemory(RD->mVertices, V, sizeof(Point)*NbU*NbV);
		}

		// This creates normals in the wrong direction
		if(0)
		{
			const SurfaceInterface& surface = IS.GetSurfaceInterface();

			NORMALSCREATE Create;
			Create.NbVerts		= surface.mNbVerts;
			Create.Verts		= surface.mVerts;
			Create.NbFaces		= surface.mNbFaces;
			Create.dFaces		= surface.mDFaces;
			Create.wFaces		= surface.mWFaces;
			Create.UseAngles	= true;

			SmoothNormals SN;
			SN.Compute(Create);
			const Point* Normals = SN.GetVertexNormals();
			CopyMemory(RD->mVertexNormals, Normals, sizeof(Point)*surface.mNbVerts);
		}

		if(0)
		{
			const SurfaceInterface& surface = IS.GetSurfaceInterface();
			BuildSmoothNormals(surface.mNbFaces, surface.mNbVerts, surface.mVerts, surface.mDFaces, surface.mWFaces, RD->mVertexNormals, true);
		}

		if(0)
		{
			const SurfaceInterface& surface = IS.GetSurfaceInterface();

			MeshBuilder2 Builder;

			MBCreate Create;
			// Main surface
			Create.NbVerts		= surface.mNbVerts;
			Create.NbFaces		= surface.mNbFaces;
			Create.Verts		= surface.mVerts;
			Create.IndexedGeo	= true;

			Create.KillZeroAreaFaces		= true;
			Create.UseW						= false;
			Create.ComputeVNorm				= true;
			Create.ComputeFNorm				= true;
			Create.WeightNormalWithAngles	= true;

			bool status = Builder.Init(Create);
			ASSERT(status);

			MBFaceData FD;
			for(udword i=0;i<Create.NbFaces;i++)
			{
				FD.Index	= i;

				udword VRefs[3];
				if(surface.mDFaces)
				{
					VRefs[0] = surface.mDFaces[i*3+0];
					VRefs[1] = surface.mDFaces[i*3+1];
					VRefs[2] = surface.mDFaces[i*3+2];
				}
				else if(surface.mWFaces)
				{
					VRefs[0] = surface.mWFaces[i*3+0];
					VRefs[1] = surface.mWFaces[i*3+1];
					VRefs[2] = surface.mWFaces[i*3+2];
				}
				FD.VRefs	= VRefs;

				status = Builder.AddFace(FD);
				ASSERT(status);
			}

			MBResult Consolidated;
			status = Builder.Build(Consolidated);
			ASSERT(status);

			const udword NbVerts = Consolidated.Geometry.NbVerts;
			/*const*/ udword NbFaces = Consolidated.Topology.NbFaces;
			const udword* VRefs = Consolidated.Topology.VRefs;
			const Point* Verts = reinterpret_cast<const Point*>(Consolidated.Geometry.Verts);
			const Point* Normals = reinterpret_cast<const Point*>(Consolidated.Geometry.Normals);
			const udword* VertsRefs = Consolidated.Geometry.VertsRefs;

			IS.Clean();
			IS.Init(NbFaces, NbVerts, null, (const IndexedTriangle*)VRefs);

			Point* dstVerts = IS.GetVerts();
			for(udword i=0;i<NbVerts;i++)
			{
				udword VRef = VertsRefs[i];
				ASSERT(VRef<Consolidated.Geometry.NbGeomPts);
				RD->mVertices[i] = dstVerts[i] = Verts[VRef];
				RD->mVertexNormals[i] = Normals[i];
			}
		}


		PINT_MESH_DATA_CREATE mdc;
#ifdef USE_TEXTURE
		mdc.mSurface = MS.GetSurfaceInterface();
#else
		mdc.SetSurfaceData(IS.GetSurfaceInterface());	//### we probably don't need to compute the CRC32 here
#endif
		// ### this uses up more memory for some reason?
		mdc.mDeformable	= true;
		PintMeshIndex MeshIndex;
		PintMeshHandle MeshHandle;
		{
			SPY_ZONE("CreateMeshObject")
			MeshHandle = mPint.CreateMeshObject(mdc, &MeshIndex);
		}

		PINT_MESH_CREATE2 MeshDesc;
		// Disable shape sharing because:
		// - there's no need to waste time trying to do the sharing, we know our tiles are unique
		// - there's a bug in the Pint plugin, shared shapes aren't purged from the internal arrays on release
		MeshDesc.mSharing = SHAPE_SHARING_NO;
		//MeshDesc.mSurface = IS.GetSurfaceInterface();
		MeshDesc.mTriangleMesh = MeshHandle;

#ifdef USE_TERRAIN_TILE_RENDERER
		PintTerrainTileRenderer* TTR = ICE_NEW(PintTerrainTileRenderer)(mIndexBuffers, mdc.GetSurface().mNbVerts, mdc.GetSurface().mVerts, RD->mVertexNormals);
		MeshDesc.mRenderer = TTR;
		if(gEnableLOD)
			SetRegionLOD(region, TTR);
#else
		//MeshDesc.mRenderer = CreateMeshRenderer(MeshDesc.mSurface);
	#ifdef USE_TEXTURE
		MeshDesc.mRenderer = CreateMeshRenderer(MS);
	#else
		MeshDesc.mRenderer = CreateMeshRenderer(mdc.mSurface, RD->mVertexNormals);
	#endif

		if(0)
		{
			RGBAColor color;
			color.R = float(0.5f + Rnd.RandomFloat());
			color.G = float(0.5f + Rnd.RandomFloat());
			color.B = float(0.5f + Rnd.RandomFloat());
			color.A = 1.0f;
			MeshDesc.mRenderer = CreateColorShapeRenderer(MeshDesc.mRenderer, color, mMT);
		}
#endif

		if(0)
		{
			// ### mainly needed for debug rendering, we can grab vertices from PhysX
			DELETEARRAY(RD->mVertices);
			DELETEARRAY(RD->mVertexNormals);
		}

#ifdef USE_TERRAIN_TILE_RENDERER
		//RD->mShapeRenderers.AddPtr(MeshDesc.mRenderer);
		RD->mTTR = TTR;
#else
		// ### can't share this then
		RD->mShapeRenderers.AddPtr(MeshDesc.mRenderer);
#endif
		RD->mMeshIndices.Add(MeshIndex);

		PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
		ObjectDesc.mMass		= 0.0f;
ObjectDesc.mPosition.x	= C.x;
ObjectDesc.mPosition.y	= 0.0f;
ObjectDesc.mPosition.z	= C.z;
//RD->mPos = ObjectDesc.mPosition;
		const PintActorHandle h = CreatePintObject(mPint, ObjectDesc);
		RD->mRegionActors.AddPtr(h);

		{
			RD->mTerrainActor = h;
			Pint_Actor* ActorAPI = mPint.GetActorAPI();
			if(ActorAPI && ActorAPI->GetNbShapes(RD->mTerrainActor)==1)
				RD->mTerrainShape = ActorAPI->GetShape(RD->mTerrainActor, 0);
		}

		mMap.insert(ActorShape(RD->mTerrainActor, RD->mTerrainShape), RD);
	}

	if(0)
	{
		//const float BoxScale = 0.4f;
		//const float BoxScale = 0.2f;
		const float BoxScale = 0.1f;
		const Point Ext(BoxScale, BoxScale, BoxScale);
		PintShapeRenderer* r = CreateBoxRenderer(Ext);

		//const float ScaleY = 1.5f;
		const float ScaleY = 16.0f;
		const float Scale = 0.02f;
		//const float ScaleY = 32.0f;
		//const float Scale = 0.005f;

/*				const float CoeffX = float(i)/float(NbU-1);
				const float x = region.mCellBounds.mMin.x + CoeffX * (region.mCellBounds.mMax.x - region.mCellBounds.mMin.x);

					const float CoeffZ = float(j)/float(NbV-1);
					const float z = region.mCellBounds.mMin.z + CoeffZ * (region.mCellBounds.mMax.z - region.mCellBounds.mMin.z);
					float val = Evaluate(mFBM, ScaleY, Scale, x, z);*/

		for(udword i=0;i<64;i++)
		{
			PINT_BOX_CREATE BoxDesc(Ext);
			BoxDesc.mRenderer = r;

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mPosition.x	= C.x + Rnd.RandomFloat() * E.x * 2.0f;
			ObjectDesc.mPosition.y	= 0.5f;// + Scale;
			ObjectDesc.mPosition.z	= C.z + Rnd.RandomFloat() * E.z * 2.0f;
			//ObjectDesc.mMass		= 1.0f;

				float val = Terrain_Evaluate(mFBM, ScaleY, Scale, ObjectDesc.mPosition.x, ObjectDesc.mPosition.z);
				ObjectDesc.mPosition.y = val;

			ObjectDesc.mRotation.p.x = Rnd.RandomFloat();
			ObjectDesc.mRotation.p.y = Rnd.RandomFloat();
			ObjectDesc.mRotation.p.z = Rnd.RandomFloat();
			ObjectDesc.mRotation.w = Rnd.RandomFloat();
			ObjectDesc.mRotation.Normalize();

			const PintActorHandle h = CreatePintObject(mPint, ObjectDesc);
			RD->mRegionActors.AddPtr(h);
		}
	}

//	region.mUserData = RD;
}

void TerrainRegionManager::UpdateRegion(const StreamRegion& region)
{
	SPY_ZONE("TerrainRegionManager::UpdateRegion")

	ASSERT(region.mUserData);
	RegionData* RD = reinterpret_cast<RegionData*>(region.mUserData);
	if(!RD)
		return;

	if(RD->mInQueue)
		return;

#ifdef USE_TERRAIN_TILE_RENDERER
	if(gEnableLOD)
		SetRegionLOD(region, RD->mTTR);

	if(0)
	{
		Pint_Shape* ShapeAPI = mPint.GetShapeAPI();
		SurfaceInterface SI;
		if(ShapeAPI->GetTriangleMeshData(SI, RD->mTerrainShape, true))	//###
		{
			Point* V = const_cast<Point*>(SI.mVerts);

			udword NbVerts = mParams.mNbVertsPerSide*mParams.mNbVertsPerSide;
			for(udword i=0;i<NbVerts;i++)
			{
				V[i].y *= 0.99f;
				//RD->mVertices[i].y *= 0.99f;
			}

			ShapeAPI->Refit(RD->mTerrainShape, RD->mTerrainActor);
			RD->mTTR->UpdateVerts(V, null);
		}
	}
#endif
}

void TerrainRegionManager::RemoveRegion(const StreamRegion& region)
{
	SPY_ZONE("TerrainRegionManager::RemoveRegion")

	//printf("RemoveRegion %d\n", region.mKey);

	ASSERT(region.mUserData);
	RegionData* RD = reinterpret_cast<RegionData*>(region.mUserData);
	if(!RD)
		return;

	if(RD->mInQueue)
	{
		const udword NbInQueue = mQueue.size();
		for(udword i=0;i<NbInQueue;i++)
		{
			if(mQueue[i].mKey==region.mKey)
			{
				mQueue.replaceWithLast(i);
				DELETESINGLE(RD);
				return;
			}
		}
		ASSERT(!"Delayed region not found in queue");
		return;
	}

	mMap.erase(ActorShape(RD->mTerrainActor, RD->mTerrainShape));

	if(1)
	{
		const udword Nb = RD->mRegionActors.GetNbEntries();
		for(udword i=0;i<Nb;i++)
		{
			PintActorHandle h = reinterpret_cast<PintActorHandle>(RD->mRegionActors[i]);
			//mPint.ReleaseObject(h);
			ReleasePintObject(mPint, h, true);
		}
	}

	if(1)
	{
#ifdef USE_TERRAIN_TILE_RENDERER
		// ### TODO: recycle
		DELETESINGLE(RD->mTTR);
#endif
		const udword Nb = RD->mShapeRenderers.GetNbEntries();
		for(udword i=0;i<Nb;i++)
		{
			PintShapeRenderer* SR = reinterpret_cast<PintShapeRenderer*>(RD->mShapeRenderers[i]);
			// ### todo: optimize
			bool Status = ReleaseShapeRenderer(SR);
			ASSERT(Status);
		}
	}

	if(1)
	{
		const udword Nb = RD->mMeshIndices.GetNbEntries();
		for(udword i=0;i<Nb;i++)
		{
			const PintMeshIndex MeshIndex = RD->mMeshIndices[i];
			bool Status = mPint.DeleteMeshObject(null, &MeshIndex);
			ASSERT(Status);
		}
	}

	DELETESINGLE(RD);
}

void TerrainRegionManager::RenderDebugRegion(PintRender& render, const Streamer& owner, const StreamRegion& region)
{
	StreamInterface::RenderDebugRegion(render, owner, region);

	ASSERT(region.mUserData);
	const RegionData* RD = reinterpret_cast<const RegionData*>(region.mUserData);
	if(!RD)
		return;
	if(RD->mInQueue)
		return;

	if(mDebugDrawVertexNormals)
	{
		Point C;
		region.mCellBounds.GetCenter(C);

		const Point Color(1.0f, 0.0f, 0.0f);

		const udword NbNormals = mParams.mNbVertsPerSide*mParams.mNbVertsPerSide;
		Point* Lines = (Point*)StackAlloc(NbNormals*2*sizeof(Point));
		for(udword i=0;i<NbNormals;i++)
		{
			const Point Pos = C + RD->mVertices[i];
			const Point Nrm = RD->mVertexNormals[i];
			Lines[i*2+0] = Pos;
			Lines[i*2+1] = Pos + Nrm;
		}
		render.DrawLines(NbNormals, Lines, Color);
	}
}

TerrainRegionManager::RegionData* TerrainRegionManager::FindRegion(PintActorHandle actor, PintShapeHandle shape) const
{
	const ps::HashMap<ActorShape, RegionData*>::Entry* e = mMap.find(ActorShape(actor, shape));
	return e ? e->second : null;
}
