///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TEST_SCENES_HEIGHTFIELD_H
#define TEST_SCENES_HEIGHTFIELD_H

#include "PintDef.h"
#include "TestScenes.h"

	class Pint;
	struct PintCaps;
	class GLIndexBufferCollection;

	class HeightfieldHelper
	{
		public:
											HeightfieldHelper(udword nb_u, udword nb_v);
											~HeightfieldHelper();

				bool						Init(float size_x, float size_z, float amplitude);
				void						Release();

				PintActorHandle				Setup(Pint& pint, const PintCaps& caps, const Point& pos);
				void						Close(Pint& pint);

				IndexedSurface				mSurface;
				GLIndexBufferCollection*	mIndexBuffers;
				const udword				mNbU;
				const udword				mNbV;
				float						mScaleU;
				float						mScaleV;
				float						mSizeX;
				float						mSizeZ;
	};

	class HeightfieldTest : public TestBase
	{
		public:
									HeightfieldTest(udword nb_u, udword nb_v);
		virtual						~HeightfieldTest();

				HeightfieldHelper	mHH;

		virtual	void				CommonRelease();
		virtual	void				Close(Pint& pint);
				bool				Init(Pint& pint, const PintCaps& caps, const Point& pos);
	};

#endif