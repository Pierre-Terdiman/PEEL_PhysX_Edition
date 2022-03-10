///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintShapeRenderer.h"
#include "RenderModel.h"
#include "PintObjectsManager.h"

extern RenderModel* gCurrentRenderModel;

static const float gDefaultEnvSize = 400.0f;
static const float gDefaultEnvThickness = 10.0f;

float GetDefaultEnvironmentSize()
{
	return gDefaultEnvSize;
}

bool SetupDefaultEnvironment(Pint& pint, bool enabled)
{
	if(pint.mDefaultEnvHandle)
		return false;

	if(gCurrentRenderModel)
		gCurrentRenderModel->SetGroundPlane(enabled);

	if(!enabled)
		return true;

	const float BoxSize = gDefaultEnvSize;

	PINT_BOX_CREATE BoxDesc(BoxSize, gDefaultEnvThickness, BoxSize);
	if(gCurrentRenderModel)
	{
		if(gCurrentRenderModel->HasSpecialGroundPlane())
			BoxDesc.mRenderer	= CreateNullRenderer();
		else
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
	}

	PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
	ObjectDesc.mPosition.x		= 0.0f;
	ObjectDesc.mPosition.y		= -gDefaultEnvThickness;
	ObjectDesc.mPosition.z		= 0.0f;
	ObjectDesc.mMass			= 0.0f;
	ObjectDesc.mAddToDatabase	= false;	// We don't want to export this or show it in stats etc
//	ObjectDesc.mMass			= 10.0f;
//	ObjectDesc.mKinematic		= true;
	const PintActorHandle h = CreatePintObject(pint, ObjectDesc);
	ASSERT(!pint.mDefaultEnvHandle);
	pint.mDefaultEnvHandle = h;
	return h!=null;
}

bool ReleaseDefaultEnvironment(Pint& pint)
{
	if(!pint.mDefaultEnvHandle)
		return false;
	pint._ReleaseObject(pint.mDefaultEnvHandle);
	pint.mDefaultEnvHandle = null;

	if(gCurrentRenderModel)
		gCurrentRenderModel->SetGroundPlane(false);
	return true;
}
