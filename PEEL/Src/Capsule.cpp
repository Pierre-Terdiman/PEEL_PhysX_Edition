///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Common.h"
#include "Capsule.h"
#include "Sphere.h"

CapsuleMesh::CapsuleMesh() :
	mOrientation(ORIENTATION_XY),
	mRadius		(0.0f),
	mHalfHeight	(0.0f),
	mNbVerts	(0),
	mVerts		(null),
	mNormals	(null),
	mNbTris		(0),
	mTris		(null)
{
}

CapsuleMesh::CapsuleMesh(udword nb_circle_pts, float radius, float half_height, bool generate_normals) :
	mOrientation(ORIENTATION_XY),
	mRadius		(0.0f),
	mHalfHeight	(0.0f),
	mNbVerts	(0),
	mVerts		(null),
	mNormals	(null),
	mNbTris		(0),
	mTris		(null)
{
	Generate(nb_circle_pts, radius, half_height, generate_normals);
}

CapsuleMesh::~CapsuleMesh()
{
	Reset();
}

void CapsuleMesh::Reset()
{
	mOrientation = ORIENTATION_XY;
	mRadius = 0.0f;
	mHalfHeight = 0.0f;
	mNbVerts = 0;
	mNbTris = 0;
	DELETEARRAY(mTris);
	DELETEARRAY(mNormals);
	DELETEARRAY(mVerts);
}

void CapsuleMesh::Generate(udword nb_circle_pts, float radius, float half_height, bool generate_normals)
{
	Reset();

	const Orientation orientation = ORIENTATION_XZ;

	mOrientation = orientation;
	mRadius = radius;
	mHalfHeight = half_height;

	const udword NbCylinderVerts = nb_circle_pts*2;
	const udword NbCylinderTris = nb_circle_pts*2;

	SphereMesh SM(nb_circle_pts, nb_circle_pts/2, radius, true);

	const udword NbSphereVerts = SM.mNbVerts;
	mNbVerts = NbSphereVerts*2 + NbCylinderVerts;
	mVerts = ICE_NEW(Point)[mNbVerts];
	if(generate_normals)
		mNormals = ICE_NEW(Point)[mNbVerts];

	for(udword i=0;i<NbSphereVerts;i++)
	{
		mVerts[i].x = SM.mVerts[i].x;
		mVerts[i].y = -SM.mVerts[i].z + half_height;
		mVerts[i].z = SM.mVerts[i].y;

		mVerts[i+NbSphereVerts].x = SM.mVerts[i].x;
		mVerts[i+NbSphereVerts].y = SM.mVerts[i].z - half_height;
		mVerts[i+NbSphereVerts].z = SM.mVerts[i].y;

		if(mNormals)
		{
			mNormals[i] = mVerts[i];
			mNormals[i].y -= half_height;
			mNormals[i].Normalize();

			mNormals[i+NbSphereVerts] = mVerts[i+NbSphereVerts];
			mNormals[i+NbSphereVerts].y += half_height;
			mNormals[i+NbSphereVerts].Normalize();
		}
	}

	// Cylinder body verts
	{
		Point* V = mVerts + NbSphereVerts*2;
		GeneratePolygon(nb_circle_pts, V, sizeof(Point), orientation, radius);

		Point Offset(0.0f, 0.0f, 0.0f);
		if(orientation==ORIENTATION_XY)
			Offset.z = half_height;
		else if(orientation==ORIENTATION_YZ)
			Offset.x = half_height;
		else
			Offset.y = half_height;
		Point* V2 = V + nb_circle_pts;
		for(udword i=0;i<nb_circle_pts;i++)
		{
			const Point P = V[i];
			V[i] = P - Offset;
			V2[i] = P + Offset;

			if(mNormals)
			{
				Point* N = mNormals + NbSphereVerts*2;
				Point* N2 = N + nb_circle_pts;
//				N[i].Zero();
//				N2[i].Zero();
				N[i] = V[i];
				N[i].y = 0.0f;
				N[i].Normalize();

				N2[i] = V2[i];
				N2[i].y = 0.0f;
				N2[i].Normalize();
			}
		}
	}

	const udword NbSphereTris = SM.mNbTris;
	mNbTris = NbSphereTris*2 + NbCylinderTris;
	mTris = ICE_NEW(IndexedTriangle)[mNbTris];


	IndexedTriangle* T = mTris;
	{
//		IndexedTriangle* T = mTris + NbSphereTris*2;
		const udword BaseOffset = NbSphereVerts*2;
//		udword k=0;
		for(udword i=0;i<nb_circle_pts;i++)
		{
			const udword j = (i + 1)%nb_circle_pts;


			const udword Ref0 = BaseOffset + i;
			const udword Ref3 = BaseOffset + nb_circle_pts + i;
			const udword Ref2 = BaseOffset + nb_circle_pts + j;
			const udword Ref1 = BaseOffset + j;

			T->mRef[0] = Ref0;
			T->mRef[1] = Ref1;
			T->mRef[2] = Ref2;
			T++;

			T->mRef[0] = Ref0;
			T->mRef[1] = Ref2;
			T->mRef[2] = Ref3;
			T++;



/*			const udword Ref0 = BaseOffset + i;
			const udword Ref2 = BaseOffset + nb_circle_pts + i;
			const udword Ref1 = BaseOffset + nb_circle_pts + j;
			const udword Ref3 = BaseOffset + j;

//			T[k].mRef[0] = Ref0;
//			T[k].mRef[1] = Ref1;
//			T[k].mRef[2] = Ref2;

//			T[k+1].mRef[0] = Ref1;
//			T[k+1].mRef[1] = Ref0;
//			T[k+1].mRef[2] = Ref3;

			T->mRef[0] = Ref0;
			T->mRef[1] = Ref1;
			T->mRef[2] = Ref2;
			T++;

			T->mRef[0] = Ref1;
			T->mRef[1] = Ref0;
			T->mRef[2] = Ref3;
			T++;*/

//			k += 2;
		}
	}

	{
		for(udword i=0;i<NbSphereTris;i++)
		{
			const udword Ref0 = SM.mTris[i].mRef[0];
			const udword Ref1 = SM.mTris[i].mRef[1];
			const udword Ref2 = SM.mTris[i].mRef[2];
			T->mRef[0] = Ref0;
			T->mRef[1] = Ref1;
			T->mRef[2] = Ref2;
			T++;
		}
		for(udword i=0;i<NbSphereTris/2;i++)
		{
			const udword Ref0 = SM.mTris[i*2].mRef[0];
			const udword Ref1 = SM.mTris[i*2].mRef[1];
			const udword Ref2 = SM.mTris[i*2].mRef[2];

			const udword Ref0b = SM.mTris[i*2+1].mRef[0];
			const udword Ref1b = SM.mTris[i*2+1].mRef[1];
			const udword Ref2b = SM.mTris[i*2+1].mRef[2];

			T->mRef[0] = Ref0b + NbSphereVerts;
			T->mRef[1] = Ref2b + NbSphereVerts;
			T->mRef[2] = Ref1b + NbSphereVerts;
			T++;

			T->mRef[0] = Ref0 + NbSphereVerts;
			T->mRef[1] = Ref2 + NbSphereVerts;
			T->mRef[2] = Ref1 + NbSphereVerts;
			T++;

		}
	}

	if(0)
	{
		for(udword i=0;i<NbSphereTris;i++)
		{
			const udword Ref0 = SM.mTris[i].mRef[0];
			const udword Ref1 = SM.mTris[i].mRef[1];
			const udword Ref2 = SM.mTris[i].mRef[2];
/*			mTris[i].mRef[0] = Ref0;
			mTris[i].mRef[1] = Ref1;
			mTris[i].mRef[2] = Ref2;
			mTris[i+NbSphereTris].mRef[0] = Ref0 + NbSphereVerts;
			mTris[i+NbSphereTris].mRef[1] = Ref2 + NbSphereVerts;
			mTris[i+NbSphereTris].mRef[2] = Ref1 + NbSphereVerts;*/
			T[i].mRef[0] = Ref0;
			T[i].mRef[1] = Ref1;
			T[i].mRef[2] = Ref2;
			T[i+NbSphereTris].mRef[0] = Ref0 + NbSphereVerts;
			T[i+NbSphereTris].mRef[1] = Ref2 + NbSphereVerts;
			T[i+NbSphereTris].mRef[2] = Ref1 + NbSphereVerts;
		}
	}
}
