///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScreenQuad_BackColor.h"

///////////////////////////////////////////////////////////////////////////////

ScreenQuadBackColor::ScreenQuadBackColor(const Point& back_color) :
	mBackColor	(back_color)
{
}

ScreenQuadBackColor::~ScreenQuadBackColor()
{
}

void ScreenQuadBackColor::Apply(udword screen_width, udword screen_height)
{
	glClearColor(mBackColor.x, mBackColor.y, mBackColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void ScreenQuadBackColor::OnNewColorSelected(ubyte r, ubyte g, ubyte b)
{
	mBackColor = Point(float(r)/255.0f, float(g)/255.0f, float(b)/255.0f);
}

///////////////////////////////////////////////////////////////////////////////

#include "PintGUIHelper.h"

void SetupColorPicker(const char* name, ColorPickerCallback* callback);

static void gButtonCallback(IceButton& button, void* user_data)
{
	ScreenQuadBackColor* sq = reinterpret_cast<ScreenQuadBackColor*>(user_data);
	SetupColorPicker("Select: back color", sq);
}

void ScreenQuadBackColor::CreateUI(PintGUIHelper& helper, Widgets* widgets, IceWindow* parent)
{
	const sdword x = 16;

	sdword y2 = 10;
	helper.CreateButton(parent, 0, x, y2, 130, 20, "Change back color", widgets, gButtonCallback, this);
	y2 += 30;

#ifdef REMOVED
	if(0)
	{
		helper.CreateLabel(parent, x, y2+LabelOffsetY, 130, 20, "Back color:", widgets);
		IceButton* B = helper.CreateButton(parent, COLOR_PICKER_BACK_COLOR, x+80, y2, 40, 20, " ", widgets, gButtonCallback, "Select: back color"/*tab_control*/);
		const ScreenQuadBackColor* SQBC = static_cast<const ScreenQuadBackColor*>(gScreenQuadModes[SCREEN_QUAD_BACK_COLOR]);
		B->EnableBitmap();
		B->SetBitmapColor(ubyte(SQBC->mBackColor.x*255.0f), ubyte(SQBC->mBackColor.y*255.0f), ubyte(SQBC->mBackColor.z*255.0f));
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////

