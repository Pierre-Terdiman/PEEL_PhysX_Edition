///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef MY_CONVEX_H
#define MY_CONVEX_H

	// Support for old convex files from optimized SAT demo

	enum ConvexIndex
	{
		CONVEX_INDEX_0,
		CONVEX_INDEX_1,
		CONVEX_INDEX_2,
		CONVEX_INDEX_3,
		CONVEX_INDEX_4,
		CONVEX_INDEX_5,
		CONVEX_INDEX_6,
		CONVEX_INDEX_7,
		CONVEX_INDEX_8,
		CONVEX_INDEX_9,
		CONVEX_INDEX_10,
		CONVEX_INDEX_11,
		CONVEX_INDEX_12,
		CONVEX_INDEX_13,
	};

	class MyPoly : public Allocateable
	{
		public:
					MyPoly() : mNbVerts(0), mIndices(NULL)
					{
					}
					~MyPoly()
					{
						DELETEARRAY(mIndices);
					}

		short		mNbVerts;
		char*		mIndices;
		float		mPlane[4];
		float		mMin;
	};

	class MyConvex : public Allocateable
	{
		public:
								MyConvex();
								~MyConvex();

				bool			LoadFromFile(const char* filename);
				bool			LoadFile(udword i);
				void			Release();
				void			Scale(float s);

				int				mNbVerts;
				Point*			mVerts;
				int				mNbPolys;
				MyPoly*			mPolys;
	};

#endif
