///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_ENGINE_DATA_H
#define PINT_ENGINE_DATA_H

#include "PintTiming.h"
#include "PintSQ.h"
#include "PintVisibilityManager.h"

	struct EngineData
	{
		EngineData() :
			mEngine					(null),
			mVisHelper				(null),
			mEnabled				(true),
			mSupportsCurrentTest	(true)
		{
		}

		void				Init(Pint* engine);
		void				Reset();

		Pint*				mEngine;
		PintSQ				mSQHelper;
		PintTiming			mTiming;
		VisibilityManager*	mVisHelper;
		bool				mEnabled;
		bool				mSupportsCurrentTest;
	};

#endif