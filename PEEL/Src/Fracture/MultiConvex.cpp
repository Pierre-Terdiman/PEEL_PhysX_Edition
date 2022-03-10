///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\Common.h"

#include "MultiConvex.h"
#include "Convex.h"
#include "FracturePattern.h"

#include "..\PintShapeRenderer.h"
#include "..\PintObjectsManager.h"

static PintActorHandle createRenderable(FractureConvex& convex, Pint& pint, const Matrix4x4& pose, bool isStatic, const Point& vel)
{
	const udword NbVerts = convex.GetNbVerts();
	const Point* Vertices = convex.GetVerts();

	if(!NbVerts || !Vertices)
		return null;

	// ### no way to pass tris directly for now
	PINT_CONVEX_CREATE ConvexCreate(NbVerts, Vertices);
	ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

	PR Pose = pose;

	PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
	ObjectDesc.mMass			= 1.0f;
	ObjectDesc.mPosition		= Pose.mPos;
//	ObjectDesc.mPosition		= Pose.mPos * 2.0f;
	ObjectDesc.mRotation		= Pose.mRot;
	ObjectDesc.mLinearVelocity	= vel;
	return CreatePintObject(pint, ObjectDesc);
}

MultiConvex::MultiConvex()
{
	clear();
}

MultiConvex::~MultiConvex()
{
	clear();
}

void MultiConvex::AddConvex(FractureConvex* c, Pint& pint, PintActorHandle actor, const Matrix4x4& pose)
{
	ConvexData cd;
	cd.mConvex	= c;
	cd.mActor	= actor;
	cd.mPint	= &pint;
	cd.mPose	= pose;

	mConvexData.pushBack(cd);
}

void MultiConvex::DeleteConvex(Pint* pint, PintActorHandle actor, FractureConvex* convex)
{
	if(actor && pint)
		ReleasePintObject(*pint, actor, true);

	convex->Release();
}

bool MultiConvex::DeleteConvex(Pint& pint, PintActorHandle actor)
{
	const udword Nb = mConvexData.size();
	for(udword i=0; i<Nb; i++)
	{
		if(mConvexData[i].mPint==&pint && mConvexData[i].mActor==actor)
		{
			mConvexData[i].mConvex->Release();

			mConvexData[i] = mConvexData[Nb-1];
			mConvexData.popBack();
			return true;
		}
	}
	return false;
}

Matrix4x4 MultiConvex::getActorPose(const ConvexData& cd) const
{
	if(cd.mActor)
	{
		assert(cd.mPint);
		return cd.mPint->GetWorldTransform(cd.mActor);
	}
	else
		return cd.mPose;
}

void MultiConvex::clear()
{
	const udword Nb = mConvexData.size();
	for(udword i=0; i<Nb; i++)
		DeleteConvex(mConvexData[i]);

	mConvexData.reset();
}

void MultiConvex::createFromConvex(Pint& pint, const FractureConvex* convex, const Point& offset)
{
	clear();
	FractureConvex* c = ICE_NEW(FractureConvex)(convex);
	
	// TODO: store render data in mConvexes too
	Matrix4x4 pose(Idt);
	pose.SetTrans(offset + c->centerAtZero());
	PintActorHandle actor = createRenderable(*c, pint, pose, false, Point(0.0f, 0.0f, 0.0f));
	ASSERT(actor);

	AddConvex(c, pint, actor, pose);
}

/*bool MultiConvex::rayCast(Pint& pint, const Ray& ray, float& dist, int& convexNr)
{
	dist = MAX_FLOAT;
	convexNr = -1;

	const udword Nb = mConvexes.GetNbEntries();
	for(udword i=0; i<Nb; i++)
	{
		Convex* c = reinterpret_cast<Convex*>(mConvexes[i]);

		float d;
		if(c->rayCast(pint, ray, d))
		{
			if (d < dist)
			{
				dist = d;
				convexNr = i;
			}
		}
	}
	return convexNr >= 0;
}*/

bool MultiConvex::randomFracture(Pint& pint, const int convexNr, const Ray &ray, int numCuts, float vel)
{
	if (convexNr < 0 || convexNr >= int(mConvexData.size()))
		return false;

	ConvexData cd = mConvexData[convexNr];	// Copy on purpose
	FractureConvex* c = cd.mConvex;
//	if (c->getActor() == NULL)
//		return false;

//	Matrix4x4 pose = c->getActorPose(/*pint*/);
	Matrix4x4 pose = getActorPose(cd);

	Ray r;
	ComputeLocalRay(r, ray, pose);

	std::vector<FractureConvex*> newConvexes;
	FractureConvex* cNew = ICE_NEW(FractureConvex)(c);
	newConvexes.push_back(cNew);

	for (int i = 0; i < numCuts; i++)
	{
		// generate random cut plane
		const float RndX = (UnitRandomFloat()-0.5f)*2.0f;
		const float RndY = (UnitRandomFloat()-0.5f)*2.0f;
		const float RndZ = (UnitRandomFloat()-0.5f)*2.0f;
//		Point planeN(NxMath::rand(-1.0f, 1.0f), NxMath::rand(-1.0f, 1.0f), NxMath::rand(-1.0f, 1.0f));
		Point planeN(RndX, RndY, RndZ);

		planeN -= r.mDir * r.mDir.Dot(planeN);
		planeN.Normalize();
		float planeD = planeN.Dot(r.mOrig);

		int num = int(newConvexes.size());
		for (int j = 0; j < num; j++)
		{
			FractureConvex *c0 = NULL, *c1 = NULL;
			if (!newConvexes[j]->cut(planeN, planeD, c0, c1))
				continue;
			newConvexes[j]->Release();
			newConvexes[j] = c0;
			newConvexes.push_back(c1);
		}
	}	
	for (int i = 0; i < (int)newConvexes.size(); i++)
	{
		FractureConvex* c = newConvexes[i];
		Point off = c->centerAtZero();
		Matrix4x4 posei = pose;
//		posei.t += pose.M * off;
			Point Tmp = posei.GetTrans();
			Tmp += off * Matrix3x3(pose);
			posei.SetTrans(Tmp);
		Point v = off;
		v.Normalize();
		v *= vel;

		PintActorHandle actor = createRenderable(*c, pint, posei, false, v);
		if(!actor)
		{
			c->Release();
			continue;
		}

		if (i == 0)
		{
			//delete mConvexes[convexNr];	//### this is deleting a void ptr now
			DeleteConvex(cd);

			//mConvexes[convexNr] = c;
			mConvexData[convexNr].mConvex = c;
			mConvexData[convexNr].mActor = actor;
			mConvexData[convexNr].mPint = &pint;
			mConvexData[convexNr].mPose = posei;
		}
		else
			//mConvexes.AddPtr(c);
			AddConvex(c, pint, actor, posei);
	}

	return true;
}

bool MultiConvex::RayCast(const ConvexData& cd, const Ray& ray, float& dist) const
{
	Ray r = ray;
	{
		const Matrix4x4 pose = getActorPose(cd);
		ComputeLocalRay(r, ray, pose);
	}

	const FractureConvex* c = cd.mConvex;

	const udword numFaces = c->GetNbFaces();

	const Point* Vertices = c->GetVerts();
	const udword* FirstIds = c->GetFirstIds();
	const udword* Indices = c->GetIndices();

	for(udword i=0; i<numFaces; i++)
	{
		const int first = FirstIds[i];
		const int num = FirstIds[i+1] - first;
		if (num < 3)
			continue;
		const udword* ids = &Indices[first];
		const Point n = (Vertices[ids[1]] - Vertices[ids[0]])^(Vertices[ids[2]] - Vertices[ids[0]]);
		const float dot = n.Dot(r.mDir);
		if (dot >= 0.0f)
			continue;

		const float t = (Vertices[ids[0]] - r.mOrig).Dot(n) / dot;
		if (t < 0.0f)
			continue;
		const Point hit = r.mOrig + t * r.mDir;

		bool out = false;
		for (int j = 0; j < num; j++)
		{
			const int i0 = ids[j];
			const int i1 = ids[(j+1)%num];
			const Point& p0 = Vertices[i0];
			const Point& p1 = Vertices[i1];
			if (((p1-p0)^(hit-p0)).Dot(n) < 0)
			{
				out = true;
				break;
			}
		}
		if (!out)
		{
			dist = t;
			return true;
		}
	}
	return false;
}

bool MultiConvex::patternFracture(Pint& pint, const int convexNr, const Ray& ray, const FracturePattern& pattern, float scale, float vel)
{
	if (convexNr < 0 || convexNr >= int(mConvexData.size()))
		return false;

	ConvexData cd = mConvexData[convexNr];	// Copy on purpose
	FractureConvex* c = cd.mConvex;
//	if (c->getActor() == NULL)
//		return false;

	float dist;
	RayCast(cd, ray, dist);

	Point hit = ray.mOrig + ray.mDir * dist;
	Point localHit;
//	Matrix4x4 pose = c->getActorPose(/*pint*/);
	Matrix4x4 pose = getActorPose(cd);
//	pose.multiplyByInverseRT(hit, localHit);
		Matrix4x4 InvPose;
		InvertPRMatrix(InvPose, pose);
		localHit = hit * InvPose;

	PtrContainer newConvexes;
	pattern.getConvexIntersection(c, localHit, 1.0f, newConvexes);

	const udword NbNewConvexes = newConvexes.GetNbEntries();

	if(!NbNewConvexes)
		return false;

	for(udword i=0; i<NbNewConvexes; i++)
	{
		FractureConvex* cn = reinterpret_cast<FractureConvex*>(newConvexes[i]);

		Point off = cn->centerAtZero();
		Matrix4x4 posei = pose;
//			posei.t += pose.M * off;
			Point Tmp = posei.GetTrans();
			Tmp += off * Matrix3x3(pose);
			posei.SetTrans(Tmp);
//			Point v = posei.t - hit;
		Point v = Tmp - hit;
		v.Normalize();
		v *= vel;

		PintActorHandle actor = createRenderable(*cn, pint, posei, false, v);
		if(!actor)
		{
			cn->Release();
			continue;
		}

		if (i == 0)
		{
			//DeleteConvex(cd);

			//mConvexes[convexNr] = cn;
			mConvexData[convexNr].mConvex = cn;
			mConvexData[convexNr].mActor = actor;
			mConvexData[convexNr].mPint = &pint;
			mConvexData[convexNr].mPose = posei;
		}
		else
//			mConvexes.AddPtr(cn);
			AddConvex(cn, pint, actor, posei);
	}
	//c->Release();
	DeleteConvex(cd);

	return true;
}

