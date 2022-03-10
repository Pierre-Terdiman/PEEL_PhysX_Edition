///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef LOOP_THE_LOOP_H
#define LOOP_THE_LOOP_H

	class LoopTheLoop
	{
		public:
						LoopTheLoop();
						~LoopTheLoop();

			bool		Generate(float radius, float width, udword nb_circle_pts, bool generate_tube);

			udword		mNbVerts;
			udword		mNbTris;
			Point*		mVerts;
			udword*		mIndices;
	};

#endif
