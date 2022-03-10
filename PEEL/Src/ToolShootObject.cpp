///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolShootObject.h"
#include "PintShapeRenderer.h"
#include "PintObjectsManager.h"

///////////////////////////////////////////////////////////////////////////////

static	float	gObjectSize = 1.0f;
static	float	gObjectMass = 1.0f;
static	float	gObjectVelocity = 100.0f;

ToolShootObject::ToolShootObject()
{
}

ToolShootObject::~ToolShootObject()
{
}

void ToolShootObject::CreateShotObject(Pint& pint, const PINT_SHAPE_CREATE& create)
{
	PINT_OBJECT_CREATE ObjectDesc(&create);
	ObjectDesc.mMass			= gObjectMass;
	ObjectDesc.mPosition		= mOrigin;
	ObjectDesc.mLinearVelocity	= mDir * gObjectVelocity;
	CreatePintObject(pint, ObjectDesc);
}

void ToolShootObject::RightDownCallback(Pint& pint, udword pint_index)
{
	if(!mShapeComboBox)
		return;

	const PintShape ps = PintShape(mShapeComboBox->GetSelectedIndex());

	// TODO: consider sharing renderer
	if(ps==PINT_SHAPE_SPHERE)
	{
		PINT_SPHERE_CREATE Create(gObjectSize);
		Create.mRenderer	= CreateSphereRenderer(Create.mRadius);
		CreateShotObject(pint, Create);
	}
	else if(ps==PINT_SHAPE_CAPSULE)
	{
		PINT_CAPSULE_CREATE Create(gObjectSize, gObjectSize);
		Create.mRenderer	= CreateCapsuleRenderer(Create.mRadius, Create.mHalfHeight*2.0f);
		CreateShotObject(pint, Create);
	}
	else if(ps==PINT_SHAPE_BOX)
	{
		PINT_BOX_CREATE Create(gObjectSize, gObjectSize, gObjectSize);
		Create.mRenderer	= CreateBoxRenderer(Create.mExtents);
		CreateShotObject(pint, Create);
	}
}

///////////////////////////////////////////////////////////////////////////////

#include "GUI_Helpers.h"

static const char* gTooltip_ObjectSize		= "Size of shot object";
static const char* gTooltip_ObjectMass		= "Mass of shot object";
static const char* gTooltip_ObjectVelocity	= "Velocity of shot object";

void ToolShootObject::CreateUI(PintGUIHelper& helper, IceWidget* parent, Widgets& owner)
{
	const sdword OffsetX = 100;
	const sdword EditBoxWidth = 60;
	const sdword LabelWidth = 100;
	const sdword LabelOffsetY = 2;
	const sdword YStep = 20;
	const sdword x = 4;

	struct Local
	{
		static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			const udword ID = edit_box.GetID();
			if(ID==0)
				gObjectSize = GetFloat(gObjectSize, &edit_box);
			else if(ID==1)
				gObjectMass = GetFloat(gObjectMass, &edit_box);
			else if(ID==2)
				gObjectVelocity = GetFloat(gObjectVelocity, &edit_box);
		}
	};

	sdword y = 0;

	helper.CreateLabel(parent, x, y+LabelOffsetY, LabelWidth, 20, "Shape:", &owner);
	mShapeComboBox = CreateShapeTypeComboBox(parent, x+OffsetX, y, true, SSM_SPHERE|SSM_CAPSULE|SSM_BOX);
	owner.Register(mShapeComboBox);
	mShapeComboBox->Select(PINT_SHAPE_BOX);
	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 100, 20, "Object size:", &owner);
	helper.CreateEditBox(parent, 0, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gObjectSize), &owner, EDITBOX_FLOAT_POSITIVE, Local::gEBCallback, gTooltip_ObjectSize);
	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 100, 20, "Object mass:", &owner);
	helper.CreateEditBox(parent, 1, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gObjectMass), &owner, EDITBOX_FLOAT_POSITIVE, Local::gEBCallback, gTooltip_ObjectMass);
	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 100, 20, "Object velocity:", &owner);
	helper.CreateEditBox(parent, 2, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gObjectVelocity), &owner, EDITBOX_FLOAT_POSITIVE, Local::gEBCallback, gTooltip_ObjectVelocity);
	y += YStep;
}


