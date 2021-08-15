///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScreenQuad_ColorSphere.h"
#include "Camera.h"
#include "PEEL.h"

///////////////////////////////////////////////////////////////////////////////

ScreenQuadColorSphere::ScreenQuadColorSphere() : mID(INVALID_ID)
{
	mTopColor = Point(1.0f, 0.5f, 0.25f);
	mMiddleColor = Point(0.8f, 0.8f, 0.8f);
	mBottomColor = Point(0.25f, 0.5f, 1.0f);
	mSpread = 0.8f;

	udword Color;
	if(GetColorByName("RoyalBlue1", Color))
	{
		mTopColor.z = float(Color & 0xff)/255.0f;
		mTopColor.y = float((Color>>8) & 0xff)/255.0f;
		mTopColor.x = float((Color>>16) & 0xff)/255.0f;
	}

	if(GetColorByName("Moccasin", Color))
	{
		mMiddleColor.z = float(Color & 0xff)/255.0f;
		mMiddleColor.y = float((Color>>8) & 0xff)/255.0f;
		mMiddleColor.x = float((Color>>16) & 0xff)/255.0f;
	}

	if(GetColorByName("Wheat4", Color))
//	if(GetColorByName("Tan4", Color))
	{
		mBottomColor.z = float(Color & 0xff)/255.0f;
		mBottomColor.y = float((Color>>8) & 0xff)/255.0f;
		mBottomColor.x = float((Color>>16) & 0xff)/255.0f;
	}
	mSpread = 1.0f;
}

ScreenQuadColorSphere::~ScreenQuadColorSphere()
{
}

void ScreenQuadColorSphere::Apply(udword screen_width, udword screen_height)
{
	if(!mProgram)
	{
		if(0)
		{
			//const GLShader::Loader vs(FindPEELFile("ScreenQuad_ColorSphere_vs.h"));
			//const GLShader::Loader ps(FindPEELFile("ScreenQuad_ColorSphere_ps.h"));
			//mProgram = GLShader::CompileProgram(vs.mData, ps.mData, null);
		}
		else
		{
			const GLShader::Loader vs(FindPEELFile("ScreenQuad_Default_vs.h"));
			const GLShader::Loader ps(FindPEELFile("ScreenQuad_ColorSphere2_ps.h"));
			const char* Parts[] = { vs.mData, ps.mData };
			mProgram = GLShader::CompileMultiFilesShader(2, Parts);
		}
	}

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	const Point CamPos = GetCameraPos();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glUseProgram(mProgram);
	GLShader::SetUniform3f(mProgram, "camPos", CamPos.x, CamPos.y, CamPos.z);
	GLShader::SetUniform1f(mProgram, "colorSpread", mSpread);
	GLShader::SetUniform3f(mProgram, "topCol", mTopColor.x, mTopColor.y, mTopColor.z);
	GLShader::SetUniform3f(mProgram, "middleCol", mMiddleColor.x, mMiddleColor.y, mMiddleColor.z);
	GLShader::SetUniform3f(mProgram, "bottomCol", mBottomColor.x, mBottomColor.y, mBottomColor.z);

	Draw2DScreenQuad(screen_width, screen_height);

	glUseProgram(0);
	glEnable(GL_CULL_FACE);	glCullFace(GL_BACK);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}

///////////////////////////////////////////////////////////////////////////////

void ScreenQuadColorSphere::OnNewColorSelected(ubyte r, ubyte g, ubyte b)
{
	const Point Color(float(r)/255.0f, float(g)/255.0f, float(b)/255.0f);

	if(mID==0)
		mTopColor = Color;
	else if(mID==1)
		mMiddleColor = Color;
	else
		mBottomColor = Color;
}

///////////////////////////////////////////////////////////////////////////////

#include "PintGUIHelper.h"
#include "GUI_Helpers.h"

void SetupColorPicker(const char* name, ColorPickerCallback* callback);

static void gButtonCallback(IceButton& button, void* user_data)
{
	ScreenQuadColorSphere* sq = reinterpret_cast<ScreenQuadColorSphere*>(user_data);
	const udword ID = button.GetID();
	sq->mID = ID;
	if(ID==0)
		SetupColorPicker("Select: color sphere top color", sq);
	else if(ID==1)
		SetupColorPicker("Select: color sphere middle color", sq);
	else
		SetupColorPicker("Select: color sphere bottom color", sq);
}

static const sdword LabelOffsetY = 2;
static const sdword OffsetX = 90;
static const sdword EditBoxWidth = 60;

static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
{
	ScreenQuadColorSphere* SQCB = reinterpret_cast<ScreenQuadColorSphere*>(user_data);
	SQCB->mSpread = GetFloat(SQCB->mSpread, &edit_box);
}

void ScreenQuadColorSphere::CreateUI(PintGUIHelper& helper, Widgets* widgets, IceWindow* parent)
{
	const sdword x = 16;

	sdword y2 = 10;
	helper.CreateButton(parent, 0, x, y2, 130, 20, "Change top color", widgets, gButtonCallback, this);
	y2 += 30;
	helper.CreateButton(parent, 1, x, y2, 130, 20, "Change middle color", widgets, gButtonCallback, this);
	y2 += 30;
	helper.CreateButton(parent, 2, x, y2, 130, 20, "Change bottom color", widgets, gButtonCallback, this);
	y2 += 30;

	helper.CreateLabel(parent, x+10, y2+LabelOffsetY, 100, 20, "Color spread:", widgets);
	IceEditBox* EB = helper.CreateEditBox(parent, 0, x+10+OffsetX, y2, EditBoxWidth, 20, helper.Convert(mSpread), widgets, EDITBOX_FLOAT_POSITIVE, gEBCallback, null/*gTooltip_ColorSpread*/);
	EB->SetUserData(this);
}

///////////////////////////////////////////////////////////////////////////////

