///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Loader_ShapeProfile.h"

ShapeProfile::ShapeProfile(udword nb, const Pt* pts) : mNbPts(nb)
{
	mPts = ICE_NEW(Pt)[nb];
	if(pts)
		CopyMemory(mPts, pts, sizeof(Pt)*nb);

	mSegments = ICE_NEW(Seg)[nb];
	ZeroMemory(mSegments, sizeof(Seg)*nb);

/*	mBounds.SetEmpty();
	for(udword i=0;i<nb;i++)
	{
		mBounds.Add();
	}*/
}

ShapeProfile::~ShapeProfile()
{
	DELETEARRAY(mSegments);
	DELETEARRAY(mPts);
}

