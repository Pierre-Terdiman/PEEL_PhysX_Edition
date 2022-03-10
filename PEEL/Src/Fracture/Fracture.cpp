///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Fracture.h"
#include "Convex.h"

#include "PintShapeRenderer.h"
#include "PintObjectsManager.h"

FractureManager::FractureManager()
{
}

FractureManager::~FractureManager()
{
}

void FractureManager::RegisterConvex(Pint& pint, FractureConvex* cn, float mass, const Point& pos, const Quat& rot, const Point& lin_vel, const Point& ang_vel, bool kine)
{
	PINT_CONVEX_CREATE ConvexCreate(cn->GetNbVerts(), cn->GetVerts());
	ConvexCreate.mRenderer = CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

	PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
	ObjectDesc.mMass			= mass;
	ObjectDesc.mPosition		= pos;
	ObjectDesc.mRotation		= rot;
	ObjectDesc.mLinearVelocity	= lin_vel;
	ObjectDesc.mAngularVelocity	= ang_vel;
	ObjectDesc.mKinematic		= kine;
	const PintActorHandle Handle= CreatePintObject(pint, ObjectDesc);

	cn->mKinematic = kine;

	Link* L = (Link*)mLinks.Reserve(sizeof(Link)/sizeof(udword));
	L->mConvex = cn;
	L->mHandle = Handle;
}

void FractureManager::Release()
{
	const udword Nb = mLinks.GetNbEntries()/(sizeof(Link)/sizeof(udword));
	Link* Links = (Link*)mLinks.GetEntries();
	for(udword i=0;i<Nb;i++)
	{
		Links[i].mConvex->Release();
	}
	mLinks.Empty();
}

FractureConvex* FractureManager::FindConvex(PintActorHandle handle, udword* index)	const
{
	const udword Nb = mLinks.GetNbEntries()/(sizeof(Link)/sizeof(udword));
	const Link* Links = (const Link*)mLinks.GetEntries();
	for(udword i=0;i<Nb;i++)
	{
		if(Links[i].mHandle==handle)
		{
			if(index)
				*index = i;
			return Links[i].mConvex;
		}
	}
	if(index)
		*index = INVALID_ID;
	return null;
}

void FractureManager::RemoveLink(udword index)
{
	const udword Coeff = sizeof(Link)/sizeof(udword);
	const udword Nb = mLinks.GetNbEntries()/Coeff;
	ASSERT(index<Nb);
	Link* Links = (Link*)mLinks.GetEntries();
	Links[index] = Links[Nb-1];
	mLinks.ForceSize((Nb-1)*Coeff);
}
