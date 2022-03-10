///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProceduralTrack.h"

RaceTrack::RaceTrack() :
	mNbVerts	(0),
	mNbTris		(0),
	mVerts		(null),
	mIndices	(null)
{
}

RaceTrack::~RaceTrack()
{
	ICE_FREE(mIndices);
	DELETEARRAY(mVerts);
}

static inline_ udword index(const Point* base, const Point& p)
{
	return udword(&p - base);
}

bool RaceTrack::Generate(float size, float amplitude, float wall_height, float track_width)
{
	const Point pos(0.0f, 50.0f, 0.0f);
	const udword NbSegments = 512;
	const float Scale = size;

	mNbVerts = NbSegments*5;
	mVerts = ICE_NEW(Point)[mNbVerts];

	Point* pts = mVerts;
	Point* pts0 = mVerts + NbSegments;
	Point* pts1 = mVerts + NbSegments*2;
	Point* pts0b = mVerts + NbSegments*3;
	Point* pts1b = mVerts + NbSegments*4;

//	const float Amplitude = 0.0f;
//	const float Amplitude = 5.0f;
	const float Amplitude = amplitude;
//	const float Amplitude = 40.0f;
	for(udword i=0;i<NbSegments;i++)
	{
		const float Angle = TWOPI*float(i)/float(NbSegments);
//		const float Stretch = 1.0f+cosf(Angle)*sinf(2.0f*Angle)*0.5f;
		const float Stretch = 1.0f+cosf(Angle)*sinf(2.0f*Angle)*sinf(4.0f*Angle)*0.25f;
//		const float Stretch = 1.0f+cosf(Angle)*sinf(2.0f*Angle)*1.0f;
		pts[i].x = pos.x + cosf(Angle)*Scale*Stretch;
		pts[i].y = pos.y + cosf(Angle)*sinf(2.0f*Angle)*Amplitude;
		pts[i].z = pos.z + sinf(Angle)*Scale*Stretch;
	}

	for(udword i=0;i<NbSegments;i++)
	{
		const udword j = (i+1)%NbSegments;

		const Point Dir = (pts[j] - pts[i]).Normalize();

		Point Right, Up;
		ComputeBasis(Dir, Right, Up);

//		const float WallHeight = 20.0f;
		const float WallHeight = wall_height;
//		const float WallHeight = i ? 2.0f : 20.0f;
		const float TrackWidth = track_width;
		pts0[i]		= pts[i]+Right*TrackWidth;
		pts0b[i]	= pts0[i] + Point(0.0f, WallHeight, 0.0f);
		pts1[i]		= pts[i]-Right*TrackWidth;
		pts1b[i]	= pts1[i] + Point(0.0f, WallHeight, 0.0f);
	}

	mNbTris = NbSegments*4*2;
	mIndices = (udword*)ICE_ALLOC(mNbTris*sizeof(udword)*3);

	udword* run = mIndices;
	for(udword i=0;i<NbSegments;i++)
	{
		const udword j = (i+1)%NbSegments;

		// Main track
		*run++ = index(mVerts, pts1[i]);
		*run++ = index(mVerts, pts[j]);
		*run++ = index(mVerts, pts[i]);

		*run++ = index(mVerts, pts1[i]);
		*run++ = index(mVerts, pts1[j]);
		*run++ = index(mVerts, pts[j]);

		*run++ = index(mVerts, pts[i]);
		*run++ = index(mVerts, pts0[j]);
		*run++ = index(mVerts, pts0[i]);

		*run++ = index(mVerts, pts[i]);
		*run++ = index(mVerts, pts[j]);
		*run++ = index(mVerts, pts0[j]);

		// Left border
		*run++ = index(mVerts, pts1b[i]);
		*run++ = index(mVerts, pts1[j]);
		*run++ = index(mVerts, pts1[i]);

		*run++ = index(mVerts, pts1b[i]);
		*run++ = index(mVerts, pts1b[j]);
		*run++ = index(mVerts, pts1[j]);

		// Right border
		*run++ = index(mVerts, pts0[i]);
		*run++ = index(mVerts, pts0b[j]);
		*run++ = index(mVerts, pts0b[i]);

		*run++ = index(mVerts, pts0[i]);
		*run++ = index(mVerts, pts0[j]);
		*run++ = index(mVerts, pts0b[j]);
	}
	return true;
}
