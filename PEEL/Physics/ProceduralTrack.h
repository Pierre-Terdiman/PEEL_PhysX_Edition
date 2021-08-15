///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PROCEDURAL_TRACK_H
#define PROCEDURAL_TRACK_H

	class RaceTrack
	{
		public:
						RaceTrack();
						~RaceTrack();

			bool		Generate(float size = 2000.0f, float amplitude = 20.0f, float wall_height = 2.0f, float track_width = 15.0f);

			udword		mNbVerts;
			udword		mNbTris;
			Point*		mVerts;
			udword*		mIndices;
	};

#endif
