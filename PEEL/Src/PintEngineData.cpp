///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintEngineData.h"
#include "Pint.h"

void EngineData::Init(Pint* engine)
{
	ASSERT(engine);
	mSQHelper.Init(engine);
	mVisHelper = ICE_NEW(VisibilityManager);
	mVisHelper->Init(engine);
	mEngine = engine;
	engine->mDefaultEnvHandle = null;
}

void EngineData::Reset()
{
	if(mEngine)
		mEngine->mDefaultEnvHandle = null;
	mSQHelper.Reset();
	if(mVisHelper)
	{
		mVisHelper->Reset();
		DELETESINGLE(mVisHelper);
	}
}
