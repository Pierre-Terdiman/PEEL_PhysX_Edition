///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LoopTheLoop.h"

LoopTheLoop::LoopTheLoop() :
	mNbVerts	(0),
	mNbTris		(0),
	mVerts		(null),
	mIndices	(null)
{
}

LoopTheLoop::~LoopTheLoop()
{
	ICE_FREE(mIndices);
	DELETEARRAY(mVerts);
}

bool LoopTheLoop::Generate(float radius, float width, udword nb_circle_pts, bool generate_tube)
{
	mNbVerts = nb_circle_pts*2;
	mVerts = ICE_NEW(Point)[nb_circle_pts*2];

	GeneratePolygon(nb_circle_pts, mVerts, sizeof(Point), ORIENTATION_XY, radius, PI);
	for(udword i=0;i<nb_circle_pts;i++)
	{
		const float Coeff = float(i)/float(nb_circle_pts-1);
		if(!generate_tube)
			mVerts[i].z += Coeff * width;
		mVerts[nb_circle_pts+i] = mVerts[i] + Point(0.0f, 0.0f, width);
	}

	const udword NbTris = nb_circle_pts*4;
	mNbTris = NbTris;
	mIndices = (udword*)ICE_ALLOC(NbTris*3*sizeof(udword));
	udword* Indices = mIndices;
	for(udword i=0;i<nb_circle_pts;i++)	
	{
		const udword j = (i+1)%nb_circle_pts;

		*Indices++ = i;
		*Indices++ = j;
		*Indices++ = j+nb_circle_pts;

		*Indices++ = i;
		*Indices++ = j+nb_circle_pts;
		*Indices++ = i+nb_circle_pts;

		*Indices++ = i;
		*Indices++ = j+nb_circle_pts;
		*Indices++ = j;

		*Indices++ = i;
		*Indices++ = i+nb_circle_pts;
		*Indices++ = j+nb_circle_pts;
	}
	return true;
}
