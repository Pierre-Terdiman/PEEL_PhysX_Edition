///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX3 plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "PINT_CommonPhysX3_Mesh.h"
#include "..\PintShapeRenderer.h"

MeshManager::MeshManager()
{
}

MeshManager::~MeshManager()
{
}

void MeshManager::ReleaseMeshes()
{
	{
		const udword Size = udword(mConvexes.size());
		for(udword i=0;i<Size;i++)
		{
			const ConvexRender& Current = mConvexes[i];
			if(Current.mConvexMesh)
				Current.mConvexMesh->release();
		}
		mConvexes.clear();
	}
	{
		const udword Size = udword(mMeshes.size());
		for(udword i=0;i<Size;i++)
		{
			const MeshRender& Current = mMeshes[i];
			if(Current.mTriangleMesh)
				Current.mTriangleMesh->release();
		}
		mMeshes.clear();
	}
}

PxConvexMesh* MeshManager::CreateConvexMesh(const Point* verts, udword nb_verts, PxConvexFlags flags, PintShapeRenderer* renderer, bool share_meshes)
{
	if(share_meshes && renderer && renderer->_CanBeShared())
	{
		const udword Size = udword(mConvexes.size());
		for(udword i=0;i<Size;i++)
		{
			const ConvexRender& CurrentConvex = mConvexes[i];
			if(CurrentConvex.mRenderer==renderer)
			{
//				printf("Sharing convex mesh\n");
				return CurrentConvex.mConvexMesh;
			}
		}
	}

	PxConvexMesh* NewConvex = CreatePhysXConvex(nb_verts, verts, flags);

	if(renderer)
		mConvexes.push_back(ConvexRender(NewConvex, renderer));

	return NewConvex;
}

PxTriangleMesh* MeshManager::CreateTriangleMesh(const PintSurfaceInterface& surface, PintShapeRenderer* renderer, bool deformable, bool share_meshes, bool dynamic)
{
	if(share_meshes && renderer && renderer->_CanBeShared())
	{
		const udword Size = udword(mMeshes.size());
		for(udword i=0;i<Size;i++)
		{
			const MeshRender& CurrentMesh = mMeshes[i];
			if(CurrentMesh.mRenderer==renderer)
			{
				return CurrentMesh.mTriangleMesh;
			}
		}
	}

	PxTriangleMesh* NewMesh = CreatePhysXMesh(surface, deformable, dynamic);

#ifdef PHYSX_SUPPORT_PMAP_XP
	//if(dynamic)
	{
		NewMesh->createPMap(64);
	}
#endif

	if(renderer)
		mMeshes.push_back(MeshRender(NewMesh, renderer));

//	printf("Nb registered meshes: %d\n", mMeshes.size());

	return NewMesh;
}

///////////////////////////////////////////////////////////////////////////////

udword MeshObjectManager::AddObject(void* object)
{
	udword Index;
	if(mFirstFree==INVALID_ID)
	{
		Index = mObjects.GetNbEntries();
		mObjects.AddPtr(object);
	}
	else
	{
		Index = mFirstFree;
		mFirstFree = udword(size_t(mObjects[Index]));
		mObjects[Index] = object;
	}

	return Index;
}

bool MeshObjectManager::DeleteObject(void* object, const udword* index)
{
	udword Index;
	if(index)
	{
		if(*index>=mObjects.GetNbEntries())
			return false;
		Index = *index;
	}
	else
	{
		if(!mObjects.Contains(object, &Index))
			return false;
	}

	if(object && mObjects[Index]!=object)
		return false;

	mObjects[Index] = reinterpret_cast<void*>(size_t(mFirstFree));
	mFirstFree = Index;
	return true;
}
