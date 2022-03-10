///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SurfaceManager.h"
#include "Common.h"
#include "ProgressBar.h"
#include "PintShapeRenderer.h"
#include "PintObjectsManager.h"

SurfaceManager::SurfaceManager()
{
	mGlobalBounds.SetEmpty();
}

SurfaceManager::~SurfaceManager()
{
}

///////////////////////////////////////////////////////////////////////////////

static const udword gNb = sizeof(SurfaceManager::SurfaceData)/sizeof(udword);

void SurfaceManager::RegisterSurface(const IndexedSurface* surface, PintShapeRenderer* renderer, const PR* pose, const PINT_MATERIAL_CREATE* material)
{
	ASSERT(surface);

	if(!renderer)
		renderer = CreateMeshRenderer(PintSurfaceInterface(surface->GetSurfaceInterface()));

	SurfaceData* SD = ICE_RESERVE(SurfaceData, mSurfaceData);
	SD->mSurface	= surface;
	SD->mRenderer	= renderer;
	SD->mMaterial	= material;
	if(pose)
		SD->mPose	= *pose;
	else
		SD->mPose.Identity();
}

udword SurfaceManager::GetNbRegisteredSurfaces() const
{
	return mSurfaceData.GetNbEntries()/gNb;
}

const SurfaceManager::SurfaceData* SurfaceManager::GetSurfaceData(udword i) const
{
	const udword Nb = mSurfaceData.GetNbEntries()/gNb;
	if(i>=Nb)
		return null;

	const SurfaceData* SD = reinterpret_cast<const SurfaceData*>(mSurfaceData.GetEntries());
	return SD + i;
}

///////////////////////////////////////////////////////////////////////////////

/*IndexedSurface* SurfaceManager::CreateManagedSurface()
{
	IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
	mSurfaces.AddPtr(IS);
	return IS;
}*/

void SurfaceManager::ReleaseManagedSurfaces()
{
//	DeleteOwnedObjects<IndexedSurface>(mSurfaces);

	udword Nb = mSurfaceData.GetNbEntries()/gNb;
	SurfaceData* SD = reinterpret_cast<SurfaceData*>(mSurfaceData.GetEntries());
	while(Nb--)
	{
		DELETESINGLE(SD->mSurface);
		SD++;
	}
	mSurfaceData.Empty();
}

///////////////////////////////////////////////////////////////////////////////

static const bool gRotateMeshes = false;

static float gMaxEdgeLength2 = 5.0f * 5.0f;
//static float gMaxEdgeLength2 = 10.0f * 10.0f;
//static float gMaxEdgeLength2 = 50.0f * 50.0f;
//static float gMaxEdgeLength2 = 100.0f * 100.0f;
static void TessellateTriangle(udword& nb_new_tris, const Triangle& tr, Triangles& triangles)
{
	udword code;
	{
		const Point edge0 = tr.mVerts[0] - tr.mVerts[1];
		const Point edge1 = tr.mVerts[1] - tr.mVerts[2];
		const Point edge2 = tr.mVerts[2] - tr.mVerts[0];
		const bool split0 = edge0.SquareMagnitude()>gMaxEdgeLength2;
		const bool split1 = edge1.SquareMagnitude()>gMaxEdgeLength2;
		const bool split2 = edge2.SquareMagnitude()>gMaxEdgeLength2;
		code = (udword(split2)<<2)|(udword(split1)<<1)|udword(split0);
	}

	const Point m0 = (tr.mVerts[0] + tr.mVerts[1])*0.5f;
	const Point m1 = (tr.mVerts[1] + tr.mVerts[2])*0.5f;
	const Point m2 = (tr.mVerts[2] + tr.mVerts[0])*0.5f;

	switch(code)
	{
		case 0:     // 000: no split
		{
			triangles.AddTri(tr);
			nb_new_tris += 1;
		}
		break;
		case 1:     // 001: split edge0
		{
			TessellateTriangle(nb_new_tris, Triangle(tr.mVerts[0], m0, tr.mVerts[2]),	triangles);
			TessellateTriangle(nb_new_tris, Triangle(m0, tr.mVerts[1], tr.mVerts[2]),	triangles);
		}
		break;
		case 2:     // 010: split edge1
		{
			TessellateTriangle(nb_new_tris, Triangle(tr.mVerts[0], tr.mVerts[1], m1),	triangles);
			TessellateTriangle(nb_new_tris, Triangle(tr.mVerts[0], m1, tr.mVerts[2]),	triangles);
		}
		break;
		case 3:     // 011: split edge0/edge1
		{
			TessellateTriangle(nb_new_tris, Triangle(tr.mVerts[0], m0, m1),				triangles);
			TessellateTriangle(nb_new_tris, Triangle(tr.mVerts[0], m1, tr.mVerts[2]),	triangles);
			TessellateTriangle(nb_new_tris, Triangle(m0, tr.mVerts[1], m1),				triangles);
		}
		break;
		case 4:     // 100: split edge2
		{
			TessellateTriangle(nb_new_tris, Triangle(tr.mVerts[0], tr.mVerts[1], m2),	triangles);
			TessellateTriangle(nb_new_tris, Triangle(tr.mVerts[1], tr.mVerts[2], m2),	triangles);
		}
		break;
		case 5:     // 101: split edge0/edge2
		{
			TessellateTriangle(nb_new_tris, Triangle(tr.mVerts[0], m0, m2),				triangles);
			TessellateTriangle(nb_new_tris, Triangle(m0, tr.mVerts[1], m2),				triangles);
			TessellateTriangle(nb_new_tris, Triangle(m2, tr.mVerts[1], tr.mVerts[2]),	triangles);
		}
		break;
		case 6:     // 110: split edge1/edge2
		{
			TessellateTriangle(nb_new_tris, Triangle(tr.mVerts[0], tr.mVerts[1], m1),	triangles);
			TessellateTriangle(nb_new_tris, Triangle(tr.mVerts[0], m1, m2),				triangles);
			TessellateTriangle(nb_new_tris, Triangle(m2, m1, tr.mVerts[2]),				triangles);
		}
		break;
		case 7:     // 111: split edge0/edge1/edge2
		{
			TessellateTriangle(nb_new_tris, Triangle(tr.mVerts[0], m0, m2),				triangles);
			TessellateTriangle(nb_new_tris, Triangle(m0, tr.mVerts[1], m1),				triangles);
			TessellateTriangle(nb_new_tris, Triangle(m2, m1, tr.mVerts[2]),				triangles);
			TessellateTriangle(nb_new_tris, Triangle(m0, m1, m2),						triangles);
		}
		break;
	};
}

bool SurfaceManager::CreateMeshesFromRegisteredSurfaces(Pint& pint, const PintCaps& caps, const PINT_MATERIAL_CREATE* material, PtrContainer* created_objects, const char* name)
{
	if(!caps.mSupportMeshes)
		return false;

	DWORD time = TimeGetTime();

	// Take a deep breath... and down the hatch
//	const udword Nb = GetNbSurfaces();
		const udword Nb2 = mSurfaceData.GetNbEntries()/gNb;
		const SurfaceData* SD = reinterpret_cast<const SurfaceData*>(mSurfaceData.GetEntries());
	CreateProgressBar(/*Nb+*/Nb2, _F("Loading... Please wait for: %s", pint.GetName()));

	const bool Regular = true;
	const bool MergeAll = false;
	const bool MergeGrid = false;
	const bool Tessellate = false;

//	IndexedSurface*const* Surfaces = reinterpret_cast<IndexedSurface**>(mSurfaces.GetEntries());

	if(Regular)
	{
/*		for(udword i=0;i<Nb;i++)
		{
			SetProgress(i);

			const IndexedSurface* IS = Surfaces[i];

			PINT_MESH_CREATE MeshDesc;
			MeshDesc.mSurface	= IS->GetSurfaceInterface();
			MeshDesc.mRenderer	= CreateMeshRenderer(MeshDesc.mSurface);
			MeshDesc.mMaterial	= material;

			PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
			ObjectDesc.mName		= name;
			ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
			if(gRotateMeshes)//###MEGADEBUG
			{
//				ObjectDesc.mPosition	= Point(10.2f, -2.0f, 3.15f);
				ObjectDesc.mRotation	= Quat(1.1f, 2.2f, 3.3f, 4.4f);
				ObjectDesc.mRotation.Normalize();

				Matrix3x3 RotX;
				RotX.RotX(HALFPI*0.5f);
				ObjectDesc.mRotation = RotX;
				ObjectDesc.mPosition = Point(0.1f, -0.2f, 0.3f);
			}
			ObjectDesc.mMass		= 0.0f;
			CreatePintObject(pint, ObjectDesc, keep_track_of_objects);
		}*/

		for(udword i=0;i<Nb2;i++)
		{
			SetProgress(/*Nb+*/i);

			const IndexedSurface* IS = SD[i].mSurface;

			PINT_MESH_CREATE MeshDesc;
			MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
			MeshDesc.mRenderer	= SD[i].mRenderer;
			MeshDesc.mMaterial	= SD[i].mMaterial ? SD[i].mMaterial : material;
			//MeshDesc.mDeformable	= true;

			PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
			ObjectDesc.mName		= name;
			ObjectDesc.mPosition	= SD[i].mPose.mPos;
			ObjectDesc.mRotation	= SD[i].mPose.mRot;
			ObjectDesc.mMass		= 0.0f;
//				ObjectDesc.mMass		= 1.0f;
//				ObjectDesc.mKinematic	= true;
			PintActorHandle h = CreatePintObject(pint, ObjectDesc);
			if(created_objects && h)
				created_objects->AddPtr(h);
		}
	}

	if(Tessellate)
	{
		ASSERT(!Nb2);
/*		for(udword i=0;i<Nb;i++)
		{
			SetProgress(i);

			const IndexedSurface* IS = Surfaces[i];

			Triangles NewTris;
			const Point* V = IS->GetVerts();
			udword TotalExtraTris = 0;
			for(udword j=0;j<IS->GetNbFaces();j++)
			{
				const IndexedTriangle* T = IS->GetFace(j);
				const Triangle Tr(V[T->mRef[0]], V[T->mRef[1]], V[T->mRef[2]]);
				udword NbNewTris = 0;
				TessellateTriangle(NbNewTris, Tr, NewTris);
				ASSERT(NbNewTris);
				TotalExtraTris += NbNewTris-1;
			}
			printf("TotalExtraTris: %d\n", TotalExtraTris);

			Permutation P(NewTris.GetNbTriangles()*3);
			P.Identity();
			const udword* Indices = P.GetList();

			PINT_MESH_CREATE MeshDesc;
//			MeshDesc.mSurface	= IS->GetSurfaceInterface();
			MeshDesc.mSurface.mNbFaces	= NewTris.GetNbTriangles();
			MeshDesc.mSurface.mDFaces	= Indices;
			MeshDesc.mSurface.mNbVerts	= NewTris.GetNbTriangles()*3;
			MeshDesc.mSurface.mVerts	= NewTris.GetTriangles()->mVerts;
			MeshDesc.mRenderer			= CreateMeshRenderer(MeshDesc.mSurface);
			MeshDesc.mMaterial			= material;

			PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
			ObjectDesc.mName		= name;
			ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
			ObjectDesc.mMass		= 0.0f;
			CreatePintObject(pint, ObjectDesc, keep_track_of_objects);
		}*/
	}
	
	if(MergeAll)
	{
		ASSERT(!Nb2);
/*		udword TotalNbVerts = 0;
		udword TotalNbTris = 0;
		for(udword i=0;i<Nb;i++)
		{
			const IndexedSurface* IS = Surfaces[i];
			TotalNbVerts += IS->GetNbVerts();
			TotalNbTris += IS->GetNbFaces();
		}

		IndexedSurface Merged;
		bool Status = Merged.Init(TotalNbTris, TotalNbVerts);
		ASSERT(Status);

		Point* V = Merged.GetVerts();
		IndexedTriangle* T = const_cast<IndexedTriangle*>(Merged.GetFaces());

		udword Offset = 0;
		for(udword i=0;i<Nb;i++)
		{
			SetProgress(i);
			const IndexedSurface* IS = Surfaces[i];
//			Merged.Merge(IS);

			const udword LocalNbVerts = IS->GetNbVerts();
			const Point* LocalV = IS->GetVerts();
			CopyMemory(V, LocalV, LocalNbVerts*sizeof(Point));
			V += LocalNbVerts;

			const udword LocalNbTris = IS->GetNbFaces();
			const IndexedTriangle* LocalT = IS->GetFaces();
			for(udword j=0;j<LocalNbTris;j++)
			{
				T[j].mRef[0] = LocalT[j].mRef[0] + Offset;
				T[j].mRef[1] = LocalT[j].mRef[1] + Offset;
				T[j].mRef[2] = LocalT[j].mRef[2] + Offset;
			}
			T += LocalNbTris;
			Offset += LocalNbVerts;
		}

		PINT_MESH_CREATE MeshDesc;
		MeshDesc.mSurface	= Merged.GetSurfaceInterface();
		MeshDesc.mRenderer	= CreateMeshRenderer(MeshDesc.mSurface);
		MeshDesc.mMaterial	= material;

		PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
		ObjectDesc.mName		= name;
		ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
		ObjectDesc.mMass		= 0.0f;
		CreatePintObject(pint, ObjectDesc, keep_track_of_objects);*/
	}

	if(MergeGrid)
	{
		ASSERT(!Nb2);
/*		AABB GlobalBounds;
		GlobalBounds.SetEmpty();

		udword TotalNbVerts = 0;
		udword TotalNbTris = 0;
		for(udword i=0;i<Nb;i++)
		{
			const IndexedSurface* IS = Surfaces[i];
			TotalNbVerts += IS->GetNbVerts();
			TotalNbTris += IS->GetNbFaces();

			for(udword j=0;j<IS->GetNbVerts();j++)
				GlobalBounds.Extend(*IS->GetVertex(j));
		}

		IndexedSurface Merged;
		bool Status = Merged.Init(TotalNbTris, TotalNbVerts);
		ASSERT(Status);

		Point* V = Merged.GetVerts();
		IndexedTriangle* T = const_cast<IndexedTriangle*>(Merged.GetFaces());

		udword Offset = 0;
		for(udword i=0;i<Nb;i++)
		{
			SetProgress(i);
			const IndexedSurface* IS = Surfaces[i];
//			Merged.Merge(IS);

			const udword LocalNbVerts = IS->GetNbVerts();
			const Point* LocalV = IS->GetVerts();
			CopyMemory(V, LocalV, LocalNbVerts*sizeof(Point));
			V += LocalNbVerts;

			const udword LocalNbTris = IS->GetNbFaces();
			const IndexedTriangle* LocalT = IS->GetFaces();
			for(udword j=0;j<LocalNbTris;j++)
			{
				T[j].mRef[0] = LocalT[j].mRef[0] + Offset;
				T[j].mRef[1] = LocalT[j].mRef[1] + Offset;
				T[j].mRef[2] = LocalT[j].mRef[2] + Offset;
			}
			T += LocalNbTris;
			Offset += LocalNbVerts;
		}

	Point Min, Max;
	GlobalBounds.GetMin(Min);
	GlobalBounds.GetMax(Max);
	Min -= Point(1.0f, 1.0f, 1.0f);
	Max += Point(1.0f, 1.0f, 1.0f);

	udword nb_subdiv = 16;
	const float dx = (Max.x - Min.x) / float(nb_subdiv);
	const float dy = (Max.y - Min.y) / float(nb_subdiv);
	const float dz = (Max.z - Min.z) / float(nb_subdiv);
	const udword NbCells = nb_subdiv*nb_subdiv*nb_subdiv;
	AABB* Cells = ICE_NEW(AABB)[NbCells];
//	udword* Counters = new udword[NbCells];
//	ZeroMemory(Counters, sizeof(udword)*NbCells);
	Container* CellTris = ICE_NEW(Container)[NbCells];
	Offset = 0;
	for(udword z=0;z<nb_subdiv;z++)
	{
		for(udword y=0;y<nb_subdiv;y++)
		{
			for(udword x=0;x<nb_subdiv;x++)
			{
				const Point CurrentMin(	Min.x + dx * float(x),
										Min.y + dy * float(y),
										Min.z + dz * float(z));
				const Point CurrentMax(	Min.x + dx * float(x+1),
										Min.y + dy * float(y+1),
										Min.z + dz * float(z+1));

				Cells[Offset].SetMinMax(CurrentMin, CurrentMax);
				Offset++;
			}
		}
	}


		V = Merged.GetVerts();
		T = const_cast<IndexedTriangle*>(Merged.GetFaces());

//		udword* Sorted = new udword[TotalNbTris];
		for(udword i=0;i<TotalNbTris;i++)
		{
			Point TriCenter;
			T[i].Center(V, TriCenter);

			udword CellIndex = INVALID_ID;
			for(udword j=0;j<NbCells;j++)
			{
				if(Cells[j].ContainsPoint(TriCenter))
				{
					CellIndex = j;
					break;
				}
			}
			ASSERT(CellIndex!=INVALID_ID);

//			Sorted[i] = CellIndex;
//			Counters[CellIndex]++;

			Container& C = CellTris[CellIndex];
			IndexedTriangle* Buffer = (IndexedTriangle*)C.Reserve(sizeof(IndexedTriangle)/sizeof(udword));
			*Buffer = T[i];
		}

		for(udword i=0;i<NbCells;i++)
		{
			const Container& C = CellTris[i];
			if(C.GetNbEntries())
			{
				udword NbTris = C.GetNbEntries()/(sizeof(IndexedTriangle)/sizeof(udword));
				if(NbTris==1)
					continue;

				IndexedSurface GridMesh;
				GridMesh.Init(NbTris, TotalNbVerts, Merged.GetVerts(), (const IndexedTriangle*)C.GetEntries());
				GridMesh.Optimize();

				PINT_MESH_CREATE MeshDesc;
				MeshDesc.mSurface	= GridMesh.GetSurfaceInterface();
				MeshDesc.mRenderer	= CreateMeshRenderer(MeshDesc.mSurface);
				MeshDesc.mMaterial	= material;

				PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
				ObjectDesc.mName		= name;
				ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
				ObjectDesc.mMass		= 0.0f;
				CreatePintObject(pint, ObjectDesc, keep_track_of_objects);
			}
		}


//		DELETEARRAY(Sorted);
	DELETEARRAY(CellTris);
//	DELETEARRAY(Counters);
	DELETEARRAY(Cells);*/
	}

	ReleaseProgressBar();

	time = TimeGetTime() - time;
	printf("Mesh creation time: %d (%s)\n", time, pint.GetName());

	return true;
}

///////////////////////////////////////////////////////////////////////////////
