///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Basic implementation of Steve Worley's noise function; see proceedings of SIGGRAPH 96.
 *	This software is placed in the public domain and is provided as is without express or implied warranty.
 *	\file		IcePharrNoise.h
 *	\author		Copyright (C) 1998, Matt Pharr <mmp@graphics.stanford.edu>
 *	\date		1998
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPHARRNOISE_H
#define ICEPHARRNOISE_H

	// Return the distance from the point p to the nth closest randomly distributed point defined by the noise function.  Density sets how
	// densely the noise points are in three-space.
	ICEMATHS_API float wNoise(Point& p, int nth, float density = 3.0f);

	// Similarly, return the vector the nth closest noise point.
	ICEMATHS_API Point wVNoise(Point& p, int nth, float density = 3.0f);

	// IMPLEMENTATION

	inline float lerp(float t, float a, float b)			{ return (1.0f - t) * a + t * b; }
	inline float distance_squared(Point& a, Point& b)		{ return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z); }
	inline float frac(float d)								{ return d >= 0.0f ? d - int(d) : frac(-d); }
	inline float sqr(float a)								{ return a * a; }

	void doVoxel(int x, int y, int z, Point& p, float density, int num, float* dist, Point* vec);
	void doWNoise(Point& p, float density, int num, int dimensions, float* dist, Point* vec);

#endif // ICEPHARRNOISE_H
