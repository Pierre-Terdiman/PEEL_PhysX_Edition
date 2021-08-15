///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PEEL.h"
#include "PintObjectsManager.h"
#include "PintVisibilityManager.h"
#include "Tool.h"
#include "ToolInterface.h"
#include "TestScenes.h"
#include "DefaultControlInterface.h"

///////////////////////////////////////////////////////////////////////////////

PintActorHandle CreatePintObject(Pint& pint, const PINT_OBJECT_CREATE& desc)
{
	SPY_ZONE("CreatePintObject")

	if(0)
	{
		const PINT_SHAPE_CREATE* CurrentShape = desc.mShapes;
		while(CurrentShape)
		{
			if(!CurrentShape->mRenderer)
			{
				ASSERT(0);
			}

			CurrentShape = CurrentShape->mNext;
		}
	}

	const PintActorHandle handle = pint._CreateObject(desc);
	return handle;
}

// Experimental/temporary design for this one...
// Maybe a generic publish-subscribe thing would be better
void ReleasePintObject(Pint& pint, PintActorHandle removed_object, bool release_from_selection)
{
	SPY_ZONE("ReleasePintObject")

	extern PhysicsTest* gRunningTest;
	if(gRunningTest)
		gRunningTest->OnObjectReleased(pint, removed_object);

	ToolInterface* CurrentTool = GetCurrentTool();
	if(CurrentTool)
		CurrentTool->OnObjectReleased(pint, removed_object);

	if(release_from_selection)
		GetDefaultControlInterface().mSelMan.RemoveFromSelection(&pint, removed_object);

	// Make sure visibility helper doesn't keep an invalid reference to removed object.
	// We could make this more efficiently but making the object visible before deletion works.
	if(pint.mVisHelper)
		pint.mVisHelper->SetRenderable(removed_object, true);

	pint.ReleaseObject(removed_object);
}

bool IsDefaultEnv(Pint& pint, PintActorHandle h)
{
	return (h && h==pint.mDefaultEnvHandle);
}
