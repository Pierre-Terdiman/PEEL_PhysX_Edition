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
#include "PEEL_MenuBar.h"

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

	UpdateSelectionItems();
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


#ifdef REMOVED

namespace
{
/*	struct SceneActorShape
	{
		PX_FORCE_INLINE	SceneActorShape(uint64_t scene, uint64_t actor, uint64_t shape) : mScene(scene), mActor(actor), mShape(shape)	{}

		uint64_t	mScene;
		uint64_t	mActor;
		uint64_t	mShape;
	};

	PX_FORCE_INLINE bool operator==(const Nvd::SceneActorShape& h0, const Nvd::SceneActorShape& h1)
	{
		if(h0.mScene!=h1.mScene)
			return false;
		if(h0.mActor!=h1.mActor)
			return false;
		if(h0.mShape!=h1.mShape)
			return false;
		return true;
	}

	PX_FORCE_INLINE uint32_t hash(const Nvd::SceneActorShape& key)
	{
		// ### not great but ...
		const uint32_t id0 = uint32_t(key.mActor);
		const uint32_t id1 = uint32_t(key.mShape);
		const uint64_t mix = (uint64_t(id0)<<32)|uint64_t(id1);
		return ps::hash(mix);
	}*/

}

using namespace physx;
using namespace shdfnd;
//using namespace Nvd;

/*

bool VisibilityManager::isRenderable(const SceneActorShape& handle) const
{
	return !mInvisibles2.contains(handle);
}

void VisibilityManager::setRenderable(const SceneActorShape& handle, bool visible)
{
	if(visible)
		mInvisibles2.erase(handle);
	else
		mInvisibles2.insert(handle);
}*/


#endif
