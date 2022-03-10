///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef REVOLUTION_SHAPE_H
#define REVOLUTION_SHAPE_H

#include "Pint.h"

	class RevolutionShape
	{
		public:
									RevolutionShape();
									~RevolutionShape();

			bool					Init(const char* filename, const Point& offset, float scale);

			udword					mNbSlices;
			Point**					mVertsCopy;
			PINT_CONVEX_CREATE**	mShapes;
	};

#endif
