///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolAddImpulse.h"
#include "PintObjectsManager.h"

///////////////////////////////////////////////////////////////////////////////

static	float	gImpulseMagnitude = 100.0f;

ToolAddImpulse::ToolAddImpulse()
{
}

ToolAddImpulse::~ToolAddImpulse()
{
}

void ToolAddImpulse::RightDownCallback(Pint& pint, udword pint_index)
{
	PintRaycastHit Hit;
	if(Raycast(pint, Hit, mOrigin, mDir) && !IsDefaultEnv(pint, Hit.mTouchedActor))
		pint.AddWorldImpulseAtWorldPos(Hit.mTouchedActor, mDir*gImpulseMagnitude, Hit.mImpact);
}

///////////////////////////////////////////////////////////////////////////////

#include "GUI_Helpers.h"

static const char* gTooltip_ImpulseMagnitude = "Magnitude of impulse applied to object";

void ToolAddImpulse::CreateUI(PintGUIHelper& helper, IceWidget* parent, Widgets& owner)
{
	const sdword OffsetX = 100;
	const sdword EditBoxWidth = 60;
	const sdword LabelOffsetY = 2;
	const sdword x = 4;

	helper.CreateLabel(parent, x, LabelOffsetY, 100, 20, "Impulse magnitude:", &owner);

	struct Local
	{
		static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			gImpulseMagnitude = GetFloat(gImpulseMagnitude, &edit_box);
		}
	};

	helper.CreateEditBox(parent, 0, x+OffsetX, 0, EditBoxWidth, 20, helper.Convert(gImpulseMagnitude), &owner, EDITBOX_FLOAT_POSITIVE, Local::gEBCallback, gTooltip_ImpulseMagnitude);
}
