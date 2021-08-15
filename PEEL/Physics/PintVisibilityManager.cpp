///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintVisibilityManager.h"
#include "PINT_Editor.h"

VisibilityManager::VisibilityManager() :
	mEngine				(null),
	mNeedsTranslation	(false)
{
}

VisibilityManager::~VisibilityManager()
{
}

void VisibilityManager::Init(Pint* engine)
{
	mEngine = engine;
	mNeedsTranslation = IsEditor(engine);
	engine->mVisHelper = this;
}

void VisibilityManager::Reset()
{
	mInvisibles.clear();
	mEngine->ResetSQFilters();
}

void VisibilityManager::SetRenderable(PintActorHandle handle, bool visible)
{
	Pint_Actor* API = mEngine->GetActorAPI();
	if(API)
	{
		API->SetDebugVizFlag(handle, visible);
//		API->SetSimulationFlag(handle, visible);
	}

	mEngine->SetSQFlag(handle, visible);

	if(mNeedsTranslation)
		handle = Editor_GetNativeHandle(handle);

	if(visible)
		mInvisibles.erase(handle);
	else
		mInvisibles.insert(handle);
}

bool VisibilityManager::IsRenderable(PintActorHandle handle) const
{
	return !mInvisibles.contains(handle);
}

bool VisibilityManager::IsRenderable2(PintActorHandle handle) const
{
	if(mNeedsTranslation)
		handle = Editor_GetNativeHandle(handle);

	return !mInvisibles.contains(handle);
}

