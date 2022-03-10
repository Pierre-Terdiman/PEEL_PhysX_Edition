///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolObjectCreate.h"
#include "ToolRayBased.h"
#include "PintShapeRenderer.h"
#include "PintObjectsManager.h"
#include "Camera.h"
#include "GLRenderHelpers.h"

///////////////////////////////////////////////////////////////////////////////

static	float	gBoxSize = 1.0f;
static	float	gBoxMass = 1.0f;
//static	float	gBoxVelocity = 100.0f;

ToolCreateObject::ToolCreateObject() :
	mDir	(0.0f, 0.0f, 0.0f),
	mOrigin	(0.0f, 0.0f, 0.0f),
	mImpact	(0.0f, 0.0f, 0.0f),
	mX		(0),
	mY		(0)
{
}

ToolCreateObject::~ToolCreateObject()
{
}

//void ToolCreateObject::SetMouseData(const MouseInfo& mouse)
void ToolCreateObject::MouseMoveCallback(const MouseInfo& mouse)
{
//	printf("CHECKPOINT %d %d\n", mouse.mMouseX, mouse.mMouseY);
	const int x = mouse.mMouseX;	mX = x;
	const int y = mouse.mMouseY;	mY = y;
	mDir = ComputeWorldRay(x, y);
	mOrigin = GetCameraPos();
}

void ToolCreateObject::RenderCallback(PintRender& render, Pint& pint, udword pint_index)
{
	PintRaycastHit Hit;
	if(!mDir.IsZero() && Raycast(pint, Hit, mOrigin, mDir))
	{
		mImpact = Hit.mImpact;
		if(0)
		{
			mImpact.x -= fmodf(mImpact.x, gBoxSize);
			mImpact.y -= fmodf(mImpact.y, gBoxSize);
			mImpact.z -= fmodf(mImpact.z, gBoxSize);
		}
		GLRenderHelpers::DrawFrame(mImpact, 1.0f);
	}
}

void ToolCreateObject::RightDownCallback(Pint& pint, udword pint_index)
{
	// TODO: consider sharing this
	PINT_BOX_CREATE BoxDesc(gBoxSize, gBoxSize, gBoxSize);
	BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

	PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
	ObjectDesc.mMass			= gBoxMass;
	ObjectDesc.mPosition		= mImpact;
//	ObjectDesc.mLinearVelocity	= mDir * gBoxVelocity;
	CreatePintObject(pint, ObjectDesc);
}

///////////////////////////////////////////////////////////////////////////////

#include "GUI_Helpers.h"

static const char* gTooltip_BoxSize		= "Size of shot box";
static const char* gTooltip_BoxMass		= "Mass of shot box";
static const char* gTooltip_BoxVelocity	= "Velocity of shot box";

void ToolCreateObject::CreateUI(PintGUIHelper& helper, IceWidget* parent, Widgets& owner)
{
	const sdword OffsetX = 100;
	const sdword EditBoxWidth = 60;
	const sdword LabelOffsetY = 2;
	const sdword YStep = 20;
	const sdword x = 4;

	struct Local
	{
		static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			const udword ID = edit_box.GetID();
			if(ID==0)
				gBoxSize = GetFloat(gBoxSize, &edit_box);
			else if(ID==1)
				gBoxMass = GetFloat(gBoxMass, &edit_box);
//			else if(ID==2)
//				gBoxVelocity = GetFloat(gBoxVelocity, &edit_box);
		}
	};

	sdword y = 0;
	helper.CreateLabel(parent, x, y+LabelOffsetY, 100, 20, "Box size:", &owner);
	helper.CreateEditBox(parent, 0, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gBoxSize), &owner, EDITBOX_FLOAT_POSITIVE, Local::gEBCallback, gTooltip_BoxSize);
	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 100, 20, "Box mass:", &owner);
	helper.CreateEditBox(parent, 1, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gBoxMass), &owner, EDITBOX_FLOAT_POSITIVE, Local::gEBCallback, gTooltip_BoxMass);
	y += YStep;

//	helper.CreateLabel(parent, 0, y+LabelOffsetY, 100, 20, "Box velocity:", &owner);
//	helper.CreateEditBox(parent, 2, OffsetX, y, EditBoxWidth, 20, helper.Convert(gBoxVelocity), &owner, EDITBOX_FLOAT_POSITIVE, Local::gEBCallback, gTooltip_BoxVelocity);
//	y += YStep;
}


