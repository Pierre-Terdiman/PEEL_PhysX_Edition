///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScreenQuad_ColorGradient.h"
#include "GLRenderHelpers.h"

///////////////////////////////////////////////////////////////////////////////

ScreenQuadColorGradient::ScreenQuadColorGradient(const Point& top_color, const Point& bottom_color) :
	mTopColor	(top_color),
	mBottomColor(bottom_color),
	mID			(INVALID_ID)
{
}

ScreenQuadColorGradient::~ScreenQuadColorGradient()
{
}

void ScreenQuadColorGradient::Apply(udword screen_width, udword screen_height)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glDisable(GL_CULL_FACE);
	GLRenderHelpers::DrawRectangle(0.0f, 1.0f, 0.0f, 1.0f, mBottomColor, mTopColor, 1.0f, screen_width, screen_height, false, GLRenderHelpers::SQT_DISABLED);
}

void ScreenQuadColorGradient::OnNewColorSelected(ubyte r, ubyte g, ubyte b)
{
	const Point Color(float(r)/255.0f, float(g)/255.0f, float(b)/255.0f);

	if(!mID)
		mTopColor = Color;
	else
		mBottomColor = Color;
}

///////////////////////////////////////////////////////////////////////////////

#include "PintGUIHelper.h"

void SetupColorPicker(const char* name, ColorPickerCallback* callback);

static void gButtonCallback(IceButton& button, void* user_data)
{
	ScreenQuadColorGradient* sq = reinterpret_cast<ScreenQuadColorGradient*>(user_data);
	const udword ID = button.GetID();
	sq->mID = ID;
	if(!ID)
		SetupColorPicker("Select: screen quad top color", sq);
	else
		SetupColorPicker("Select: screen quad bottom color", sq);
}

void ScreenQuadColorGradient::CreateUI(PintGUIHelper& helper, Widgets* widgets, IceWindow* parent)
{
	const sdword x = 16;

	sdword y2 = 10;
	helper.CreateButton(parent, 0, x, y2, 130, 20, "Change top color", widgets, gButtonCallback, this);
	y2 += 30;
	helper.CreateButton(parent, 1, x, y2, 130, 20, "Change bottom color", widgets, gButtonCallback, this);
}

///////////////////////////////////////////////////////////////////////////////

