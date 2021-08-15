///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef RAYTRACING_TEST_H
#define RAYTRACING_TEST_H

	class Pint;

//	udword RaytracingTest(Pint& pint, udword& total, udword screen_width, udword screen_height);
	udword RaytracingTest(Picture& pic, Pint& pint, udword& total_time, udword screen_width, udword screen_height, udword nb_rays, float max_dist);
	udword RaytracingTestMT(Picture& pic, Pint& pint, udword& total_time, udword screen_width, udword screen_height, udword nb_rays, float max_dist);

#endif
