///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SCRIPT_H
#define SCRIPT_H

	class PhysicsTest;
	struct ParseContext;

	struct AutomatedTest
	{
		void	Init(PhysicsTest* test)
		{
			mTest				= test;
			mNbFrames			= INVALID_ID;
			mSoundPos			= INVALID_ID;
			mNextCameraPos		= INVALID_ID;
			mWireframeOverlay	= true;
			mTool				= INVALID_ID;
			mSubScene			= null;
		}

		PhysicsTest*	mTest;
		udword			mNbFrames;
		udword			mSoundPos;
		udword			mNextCameraPos;
		udword			mWireframeOverlay;
		udword			mTool;
		String*			mSubScene;
	};

	class AutomatedTests : public Allocateable
	{
		public:
							AutomatedTests(const ParseContext&);
							~AutomatedTests();

			bool			IsValid()			const;
			AutomatedTest*	GetCurrentTest()	const;
			AutomatedTest*	SelectNextTest();

			Container		mTests;
			//StringTable		mStrings;
			udword			mDefaultNbFrames;
			udword			mIndex;
			bool			mRendering;
			bool			mRandomizeOrder;
			bool			mTrashCache;
			bool			mSaveExcelFile;
	};

	AutomatedTests* GetAutomatedTests();
	void ReleaseAutomatedTests();

	void RunScript(const char* filename);
	void UpdateAutomatedTests(udword frame_nb, bool menu_is_visible);

	void StartTest(const char* name);

#endif
