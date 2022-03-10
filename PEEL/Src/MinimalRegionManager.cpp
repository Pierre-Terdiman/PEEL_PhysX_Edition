///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MinimalRegionManager.h"
#include "PintObjectsManager.h"
#include "PintShapeRenderer.h"

MinimalRegionManager::MinimalRegionManager(Pint& pint) : mPint(pint)
{
}

void MinimalRegionManager::AddRegion(StreamRegion& region)
{
	//printf("AddRegion %d\n", region.mKey);

	Point C,E;
	region.mCellBounds.GetCenter(C);
	region.mCellBounds.GetExtents(E);

	PINT_BOX_CREATE BoxDesc(E.x, 0.5f, E.z);
	BoxDesc.mRenderer = CreateBoxRenderer(BoxDesc.mExtents);

	PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
	ObjectDesc.mMass		= 0.0f;
	ObjectDesc.mPosition.x	= C.x;
	ObjectDesc.mPosition.y	= 0.0f;
	ObjectDesc.mPosition.z	= C.z;
	region.mUserData = CreatePintObject(mPint, ObjectDesc);
}

void MinimalRegionManager::UpdateRegion(const StreamRegion& region)
{
}

void MinimalRegionManager::RemoveRegion(const StreamRegion& region)
{
	//printf("RemoveRegion %d\n", region.mKey);

//	mPint.ReleaseObject(PintActorHandle(region.mUserData));
	ReleasePintObject(mPint, PintActorHandle(region.mUserData), true);
}

