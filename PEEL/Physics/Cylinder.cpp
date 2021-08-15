///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Common.h"
#include "Cylinder.h"

CylinderMesh::CylinderMesh() : mOrientation(ORIENTATION_XY), mRadius(0.0f), mHalfHeight(0.0f), mNbVerts(0), mVerts(null)
{
}

CylinderMesh::CylinderMesh(udword nb_circle_pts, float radius, float half_height, Orientation orientation) : mRadius(0.0f), mHalfHeight(0.0f), mNbVerts(0), mVerts(null)
{
	Generate(nb_circle_pts, radius, half_height, orientation);
}

CylinderMesh::~CylinderMesh()
{
	Reset();
}

void CylinderMesh::Reset()
{
	mOrientation = ORIENTATION_XY;
	mRadius = 0.0f;
	mHalfHeight = 0.0f;
	mNbVerts = 0;
	DELETEARRAY(mVerts);
}

void CylinderMesh::Generate(udword nb_circle_pts, float radius, float half_height, Orientation orientation)
{
	mOrientation = orientation;
	mRadius = radius;
	mHalfHeight = half_height;

	const udword TotalNbVerts = nb_circle_pts*2;
	mNbVerts = TotalNbVerts;

	DELETEARRAY(mVerts);
	Point* V = ICE_NEW(Point)[TotalNbVerts];
	mVerts = V;

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
	}
}
