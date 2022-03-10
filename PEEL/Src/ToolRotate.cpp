///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Tool.h"
#include "ToolRotate.h"
#include "Camera.h"
#include "GLRenderHelpers.h"
#include "PintObjectsManager.h"
#include "GUI_PosEdit.h"

using namespace IceRenderer;

///////////////////////////////////////////////////////////////////////////////

ToolRotate::ToolRotate()
{
}

ToolRotate::~ToolRotate()
{
}

void ToolRotate::Select()
{
}

void ToolRotate::Deselect()
{
	Reset(INVALID_ID);
}

void ToolRotate::Reset(udword pint_index)
{
#ifdef SUPPORT_ROTATE_TOOL_UI
	if(mEditPos)
	{
		mEditPos->mHasEdit = false;
		mEditPos->SetPos(Point(0.0f, 0.0f, 0.0f));
	}
#endif

	if(pint_index==INVALID_ID)
	{
		for(udword i=0;i<MAX_NB_ENGINES;i++)
			mData[i].Reset();
	}
	else
	{
		ASSERT(pint_index<MAX_NB_ENGINES)
		mData[pint_index].Reset();
	}
}

void ToolRotate::OnObjectReleased(Pint& pint, PintActorHandle removed_object)
{
	for(udword i=0;i<MAX_NB_ENGINES;i++)
	{
		RotateData& PD = mData[i];

		if(PD.mObject==removed_object)
		{
			PD.Reset();
			break;
		}
	}
}

void ToolRotate::KeyboardCallback(Pint& pint, udword pint_index, unsigned char key, bool down)
{
	if(down)
	{
		if(key==127)
		{
			ASSERT(pint_index<MAX_NB_ENGINES)
			RotateData& PD = mData[pint_index];

			if(PD.mObject)
			{
				ReleasePintObject(pint, PD.mObject, true);
				PD.Reset();
			}
		}
	}
}

void ToolRotate::RightDownCallback(Pint& pint, udword pint_index)
{
	ASSERT(pint_index<MAX_NB_ENGINES)
	RotateData& PD = mData[pint_index];

	PintRaycastHit Hit;
	if(Raycast(pint, Hit, mOrigin, mDir) && !IsDefaultEnv(pint, Hit.mTouchedActor))
	{
		PD.mObject = Hit.mTouchedActor;

#ifdef SUPPORT_ROTATE_TOOL_UI
		if(mEditPos)
		{
			const PR Pose = pint.GetWorldTransform(Hit.mTouchedActor);
			mEditPos->SetEulerAngles(Pose.mRot);
		}
#endif

		PD.mTransformer.Start(pint, Hit.mTouchedActor);
	}
	else
	{
		PD.Reset();
#ifdef SUPPORT_ROTATE_TOOL_UI
		if(mEditPos)
			mEditPos->Reset();
#endif
	}
}

void ToolRotate::RightDragCallback(Pint& pint, udword pint_index)
{
	const Point mdir_old = ComputeWorldRay(mOldX, mOldY);
	const Point mdir_new = ComputeWorldRay(mX, mY);

	ASSERT(pint_index<MAX_NB_ENGINES)
	RotateData& RD = mData[pint_index];
	if(RD.mObject)
	{
		PR Pose = pint.GetWorldTransform(RD.mObject);

		Quat q; VirtualTrackBall(q, GetCameraPos(), Pose.mPos, mdir_old, mdir_new, 1.0f/*WorldSphere.mRadius*/);
		q *= Pose.mRot;

		Pose.mRot = q;
#ifdef SUPPORT_ROTATE_TOOL_UI
		if(mEditPos)
			mEditPos->SetEulerAngles(q);
#endif

		RD.mTransformer.SetPose(pint, RD.mObject, Pose);
	}
}

void ToolRotate::RightUpCallback(Pint& pint, udword pint_index)
{
	RotateData& RD = mData[pint_index];

	if(RD.mObject)
		RD.mTransformer.Stop(pint, RD.mObject);

//	mData[pint_index].Reset();
}

void ToolRotate::RenderCallback(PintRender& render, Pint& pint, udword pint_index)
{
	ASSERT(pint_index<MAX_NB_ENGINES)
	RotateData& RD = mData[pint_index];
	if(RD.mObject)
		DrawBoxCorners(pint, RD.mObject, TOOLS_BOX_CORNERS_COLOR);

#ifdef SUPPORT_ROTATE_TOOL_UI
	if(mEditPos)
		mEditPos->SetEnabled(RD.mObject!=null);

	if(mEditPos && mEditPos->mHasEdit)
	{
		RotateData& RD = mData[pint_index];
		if(RD.mObject)
		{
			PR Pose = pint.GetWorldTransform(RD.mObject);
			mEditPos->GetEulerAngles(Pose.mRot);

			RD.mTransformer.SetPose(pint, RD.mObject, Pose);
		}
	}
#endif
}

void ToolRotate::PostRenderCallback()
{
#ifdef SUPPORT_ROTATE_TOOL_UI
	if(mEditPos)
		mEditPos->mHasEdit = false;
#endif
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SUPPORT_ROTATE_TOOL_UI
#include "GUI_Helpers.h"

void ToolRotate::CreateUI(PintGUIHelper& helper, IceWidget* parent, Widgets& owner)
{
	const sdword OffsetX = 20;
	const sdword EditBoxWidth = 60;
	const sdword LabelOffsetY = 2;
	const sdword YStep = 20;
	const sdword x = 4;

	sdword y = 4;

	WindowDesc WD;
	WD.mParent	= parent;
	WD.mX		= x;
	WD.mY		= y;
	WD.mWidth	= 100;
	WD.mHeight	= 70;
	WD.mType	= WINDOW_NORMAL;
	WD.mStyle	= WSTYLE_BORDER;
	WD.mStyle	= WSTYLE_CLIENT_EDGES;
	WD.mStyle	= WSTYLE_STATIC_EDGES;
	mEditPos = ICE_NEW(EditPosWindow)(WD);
	owner.Register(mEditPos);
}
#endif
