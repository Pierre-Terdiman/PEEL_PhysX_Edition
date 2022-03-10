///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintObjectsManager.h"
#include "PEEL.h"
#include "PintVisibilityManager.h"
#include "Tool.h"
#include "ToolInterface.h"
#include "TestScenes.h"
#include "DefaultControlInterface.h"

///////////////////////////////////////////////////////////////////////////////

void DeletePintObjectShapes(PINT_OBJECT_CREATE& desc)
{
	class DeleteShapeCallback : public PintShapeEnumerateCallback
	{
		public:
		virtual	void	ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data)	override
		{
			PINT_SHAPE_CREATE* Create = const_cast<PINT_SHAPE_CREATE*>(&create);
			DELETESINGLE(Create);
		}
	}cb;
	desc.GetNbShapes(&cb);
}

///////////////////////////////////////////////////////////////////////////////

PintActorHandle CreatePintObject(Pint& pint, const PINT_OBJECT_CREATE& desc)
{
	SPY_ZONE("CreatePintObject")

	/*if(0)
	{
		const PINT_SHAPE_CREATE* CurrentShape = desc.GetFirstShape();
		while(CurrentShape)
		{
			if(!CurrentShape->mRenderer)
			{
				ASSERT(0);
			}

			CurrentShape = CurrentShape->GetNext();
		}
	}*/

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

	// WARNING: by default we release all joints connected to released actor.
	ReleasePintJoints(pint, removed_object);

	// Finally release object
	pint._ReleaseObject(removed_object);
}

bool ReleasePintJoints(Pint& pint, PintActorHandle actor)
{
	SPY_ZONE("ReleasePintJoints")

	Pint_Actor* ActorAPI = pint.GetActorAPI();
	ASSERT(ActorAPI);
	if(!ActorAPI)
		return false;

	const udword NbJoints = ActorAPI->GetNbJoints(actor);
	if(NbJoints)
	{
		// We don't know if each plugin will support deleting contained items while iterating the container, so we copy the handles first.
		PintJointHandle* Handles = reinterpret_cast<PintJointHandle*>(StackAlloc(sizeof(PintJointHandle)*NbJoints));
		for(udword i=0;i<NbJoints;i++)
			Handles[i] = ActorAPI->GetJoint(actor, i);

		// And then we delete in second pass
		for(udword i=0;i<NbJoints;i++)
			pint.ReleaseJoint(Handles[i]);
	}
	return true;
}

bool IsDefaultEnv(Pint& pint, PintActorHandle h)
{
	return (h && h==pint.mDefaultEnvHandle);
}
