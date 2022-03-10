///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Tool.h"
#include "ToolTranslate.h"
#include "Camera.h"
#include "GLRenderHelpers.h"
#include "GUI_PosEdit.h"
#include "PintObjectsManager.h"

///////////////////////////////////////////////////////////////////////////////

ToolTranslate::ToolTranslate() :
	mCachedOrigin	(Point(0.0f, 0.0f, 0.0f)),
	mCachedDir		(Point(0.0f, 0.0f, 0.0f)),
	mDrag			(false)
{
}

ToolTranslate::~ToolTranslate()
{
}

void ToolTranslate::Select()
{
//	printf("ToolTranslate selected\n");
}

void ToolTranslate::Deselect()
{
//	printf("ToolTranslate deselected\n");
	Reset(INVALID_ID);
}

void ToolTranslate::Reset(udword pint_index)
{
	mCachedOrigin	= Point(0.0f, 0.0f, 0.0f);
	mCachedDir		= Point(0.0f, 0.0f, 0.0f);

#ifdef SUPPORT_TRANSLATE_TOOL_UI
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

void ToolTranslate::OnObjectReleased(Pint& pint, PintActorHandle removed_object)
{
	for(udword i=0;i<MAX_NB_ENGINES;i++)
	{
		TranslateData& TD = mData[i];

		if(TD.mObject==removed_object)
		{
			TD.Reset();
			break;
		}
	}
}

void ToolTranslate::KeyboardCallback(Pint& pint, udword pint_index, unsigned char key, bool down)
{
	if(down)
	{
		if(key==127)
		{
			ASSERT(pint_index<MAX_NB_ENGINES)
			TranslateData& TD = mData[pint_index];

			if(TD.mObject)
			{
				ReleasePintObject(pint, TD.mObject, true);
				TD.Reset();
			}
		}
	}
}

void ToolTranslate::RightDownCallback(Pint& pint, udword pint_index)
{
	ASSERT(pint_index<MAX_NB_ENGINES)
	TranslateData& TD = mData[pint_index];

	PintRaycastHit Hit;
	if(Raycast(pint, Hit, mOrigin, mDir) && !IsDefaultEnv(pint, Hit.mTouchedActor))
	{
		TD.mObject = Hit.mTouchedActor;
		TD.mDistance = Hit.mDistance;

		const PR Pose = pint.GetWorldTransform(Hit.mTouchedActor);
		TD.mDelta = Hit.mImpact - Pose.mPos;
		TD.mCaptured = true;
		TD.mTransformer.Start(pint, Hit.mTouchedActor);
	}
	else
	{
		TD.Reset();
#ifdef SUPPORT_TRANSLATE_TOOL_UI
		if(mEditPos)
			mEditPos->Reset();
#endif
	}
}

void ToolTranslate::RightDragCallback(Pint& pint, udword pint_index)
{
	mDrag = true;
}

void ToolTranslate::RightUpCallback(Pint& pint, udword pint_index)
{
	TranslateData& TD = mData[pint_index];

	if(TD.mObject)
	{
		TD.mCaptured = false;
		TD.mTransformer.Stop(pint, TD.mObject);
	}
//	TD.Reset();
}

void ToolTranslate::RenderCallback(PintRender& render, Pint& pint, udword pint_index)
{
	// TODO: do this only once
	// Recompute camera vectors to avoid stuttering when the camera moves (say because of key presses) while one object is manipulated.
	mDir = ComputeWorldRay(mX, mY);
	mOrigin = GetCameraPos();

	ASSERT(pint_index<MAX_NB_ENGINES)
	TranslateData& TD = mData[pint_index];
	if(TD.mObject)
	{
#ifdef SUPPORT_TRANSLATE_TOOL_UI
		if(mEditPos)
		{
			mEditPos->SetEnabled(true);
			GLRenderHelpers::DrawFrame(mEditPos->mEdited, 1.0f);
		}
#endif
		PR Pose = pint.GetWorldTransform(TD.mObject);
		//GLRenderHelpers::DrawFrame(Pose.mPos);

#ifdef SUPPORT_TRANSLATE_TOOL_UI
		if(mEditPos && mEditPos->mHasEdit)
		{
			Pose.mPos = mEditPos->mEdited;

			TD.mTransformer.SetPose(pint, TD.mObject, Pose);
		}
#endif

		const bool CameraHasMoved = (mCachedOrigin!=mOrigin) || (mCachedDir!=mDir);
		if(TD.mCaptured && (mDrag || CameraHasMoved))
		{
			mCachedDir = mDir;
			mCachedOrigin = mOrigin;

			const Point NewPos = mOrigin + mDir * TD.mDistance;
			Pose.mPos = NewPos - TD.mDelta;

			TD.mTransformer.SetPose(pint, TD.mObject, Pose);

#ifdef SUPPORT_TRANSLATE_TOOL_UI
			if(mEditPos)
				mEditPos->SetPos(Pose.mPos);
#endif
		}

		DrawBoxCorners(pint, TD.mObject, TOOLS_BOX_CORNERS_COLOR);
	}
#ifdef SUPPORT_TRANSLATE_TOOL_UI
	else
	{
		if(mEditPos)
			mEditPos->SetEnabled(false);
	}
#endif
}

void ToolTranslate::PostRenderCallback()
{
	mDrag = false;
#ifdef SUPPORT_TRANSLATE_TOOL_UI
	if(mEditPos)
		mEditPos->mHasEdit = false;
#endif
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SUPPORT_TRANSLATE_TOOL_UI
#include "GUI_Helpers.h"

void ToolTranslate::CreateUI(PintGUIHelper& helper, IceWidget* parent, Widgets& owner)
{
	const sdword OffsetX = 20;
	const sdword EditBoxWidth = 60;
	const sdword LabelOffsetY = 2;
	const sdword YStep = 20;
	const sdword x = 4;

/*	struct Local
	{
		static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			if(param!=IceEditBox::CB_PARAM_RETURN)
				return;

			//printf("gEBCallback\n");
			const udword ID = edit_box.GetID();
			if(ID==X_)
				gEdited.x = GetFloat(gEdited.x, &edit_box);
			else if(ID==Y_)
				gEdited.y = GetFloat(gEdited.y, &edit_box);
			else if(ID==Z_)
				gEdited.z = GetFloat(gEdited.z, &edit_box);
			gHasEdit = true;
		}
	};*/

	sdword y = 4;

/*	helper.CreateLabel(parent, x, y+LabelOffsetY, 90, 20, "X:", &owner);
	mEditBoxX = helper.CreateEditBox(parent, X_, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gEdited.x), &owner, EDITBOX_FLOAT, Local::gEBCallback, null);
	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 90, 20, "Y:", &owner);
	mEditBoxY = helper.CreateEditBox(parent, Y_, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gEdited.y), &owner, EDITBOX_FLOAT, Local::gEBCallback, null);
	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 90, 20, "Z:", &owner);
	mEditBoxZ = helper.CreateEditBox(parent, Z_, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gEdited.z), &owner, EDITBOX_FLOAT, Local::gEBCallback, null);
	y += YStep;*/

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
