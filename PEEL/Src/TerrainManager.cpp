///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TerrainManager.h"
#include "Pint.h"
#include "PintSQ.h"
#include "DefaultControlInterface.h"
#include "PEEL.h"
#include "TerrainRegionManager.h"
#include "PintTerrainTileRenderer.h"

static const bool gDebugDrawTouchedShapes = false;

DeformManager::DeformManager(Pint& owner) : mOwner(owner)
{
}

DeformManager::~DeformManager()
{
}

bool DeformManager::Update(const PintVehicleData& vehicle, TerrainRegionManager& region_manager, float wheel_radius, float dt)
{
	SPY_ZONE("DeformManager::Update")

	Pint_Shape* ShapeAPI = mOwner.GetShapeAPI();
	if(!ShapeAPI)
		return false;

	// Derive overlap shapes from current wheel positions
	PintSphereOverlapData Overlaps[4];
	for(udword i=0;i<4;i++)
	{
		const PR WheelPose = ShapeAPI->GetWorldTransform(vehicle.mChassisActor, vehicle.mWheelShapes[i]);
		Overlaps[i].mSphere = Sphere(WheelPose.mPos, wheel_radius);	// TODO: per-wheel radius?
	}

	// Perform scene-level overlaps, get back set of touched actors/shapes
	// ### TODO: better filtering here, we could use a filter callback or at least ask for static actors only
	PintMultipleHits* Dest = mOwner.mSQHelper->PrepareSphereOverlapObjectsQuery(4, Overlaps);
	const udword Nb = mOwner.BatchSphereOverlapObjects(mOwner.mSQHelper->GetThreadContext(), 4, Dest, mOwner.mSQHelper->GetSphereOverlapObjectsStream(), Overlaps);
	//printf("Nb: %d\n", Nb);
	if(gDebugDrawTouchedShapes)	// Abuse selection code to draw touched actors
	{
		DefaultControlInterface& DCI = GetDefaultControlInterface();
		DCI.mSelMan.CancelSelection();
		const PintOverlapHit* AllHits = (const PintOverlapHit*)mOwner.mSQHelper->GetSphereOverlapObjectsStream().GetEntries();	// ####
		for(udword i=0;i<Nb;i++)
			DCI.mSelMan.AddToSelection(&mOwner, AllHits[i].mTouchedActor, 0);
	}

	if(1)
	{
		mObjects.clear();
		mTouchedTris.Reset();

		// Iterate over touched objects, fetch region for each of them, then store them in local hash. This is clumsy because:
		// - the actor/shape/region are already stored in a hashmap, in region_manager, so we could just access that one directly.
		// - but the map in region_manager contains all regions and we don't want to further iterate over all of them, we want to iterate over touched ones.
		// - we want to store touched regions only once, i.e. when crossing a region boundary two wheel overlaps could return the same (actor/shape) and we
		// don't want to see them appear twice in the array we're going to iterate over for deformation. Hence the local hashmap.
		// - still this is clumsy and could probably be done more efficiently.
		const PintOverlapHit* AllHits = (const PintOverlapHit*)mOwner.mSQHelper->GetSphereOverlapObjectsStream().GetEntries();	// ####
		for(udword i=0;i<Nb;i++)
		{
			const PintActorHandle TouchedActor = AllHits[i].mTouchedActor;
			const PintShapeHandle TouchedShape = AllHits[i].mTouchedShape;

			// ### clumsy
			TerrainRegionManager::RegionData* RD = region_manager.FindRegion(TouchedActor, TouchedShape);
			// The overlaps can return objects not associated with any region, like the vehicle itself.
			if(RD)
			{
				// We only want to keep the terrain tiles that can be deformed. In another context where the vehicle would have to push away
				// dynamic obstacles we could keep more actors here.
				if(RD->mTerrainActor==TouchedActor && RD->mTerrainShape==TouchedShape)
					mObjects.insert(ActorShape(TouchedActor, TouchedShape), RD);
			}
		}

		// Figure out deformation for each wheel
		for(udword i=0;i<4;i++)
		{
			Container TouchedTriangleIndices;

			for(ps::HashMap<ActorShape, void*>::Iterator iter = mObjects.getIterator(); !iter.done(); ++iter)
			{
				const PintActorHandle TerrainActor = iter->first.mActor;
				const PintShapeHandle TerrainShape = iter->first.mShape;

				SurfaceInterface SI;
				if(ShapeAPI->GetTriangleMeshData(SI, TerrainShape, true))
				{
					const PR TerrainPose = ShapeAPI->GetWorldTransform(TerrainActor, TerrainShape);

					TouchedTriangleIndices.Reset();
					if(ShapeAPI->FindTouchedTriangles(TouchedTriangleIndices, mOwner.mSQHelper->GetThreadContext(), TerrainShape, TerrainPose, Overlaps[i]))
					{
						const Matrix4x4 M = TerrainPose;

						const udword NbToucheddTris = TouchedTriangleIndices.GetNbEntries();
						mVertexIndices.Reset();
						for(udword i=0;i<NbToucheddTris;i++)
						{
							if(1)
							{
								// ### just for debug viz
								Triangle tri;
								bool status = ShapeAPI->GetTriangle(tri, TerrainShape, TouchedTriangleIndices[i]);
								ASSERT(status);
								mTouchedTris.AddVertex(tri.mVerts[0]*M);
								mTouchedTris.AddVertex(tri.mVerts[1]*M);
								mTouchedTris.AddVertex(tri.mVerts[2]*M);
							}

							IndexedTriangle itri;
							bool status = ShapeAPI->GetIndexedTriangle(itri, TerrainShape, TouchedTriangleIndices[i]);
							ASSERT(status);
							// ###
							mVertexIndices.AddUnique(itri.mRef[0]);
							mVertexIndices.AddUnique(itri.mRef[1]);
							mVertexIndices.AddUnique(itri.mRef[2]);
						}

						{
							TerrainRegionManager::RegionData* RD = reinterpret_cast<TerrainRegionManager::RegionData*>(iter->second);
							Point* V = const_cast<Point*>(SI.mVerts);

							//const float MaxDeform = 0.25f;
							//const float MaxDeform = 0.5f;

							udword NbVerts = mVertexIndices.GetNbEntries();
							const udword* VI = mVertexIndices.GetEntries();
							while(NbVerts--)
							{
								const float MaxDeform = UnitRandomFloat()*0.5f;
								//const float MaxDeform = UnitRandomFloat()*0.25f;
								const udword Index = *VI++;

								float D = RD->mDeform[Index];
								if(D<MaxDeform)
								{
									//D += 0.02f;
									D += 0.04f;
									if(1)
										V[Index].y = RD->mVertices[Index].y - D;
									RD->mVertexNormals[Index].y *= 1.2f;
										//RD->mVertexNormals[Index].Zero();
										//RD->mVertexNormals[Index] *= 0.01f;
										RD->mVertexNormals[Index] *= 0.25f;
									RD->mDeform[Index] = D;
								}
							}

							ShapeAPI->Refit(TerrainShape, TerrainActor);
							RD->mTTR->UpdateVerts(V, RD->mVertexNormals);
						}
					}
				}
			}
		}
	}
	mOwner.mSQHelper->ResetHitData();
	return true;
}
