///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolRayBased.h"
#include "Tool.h"
#include "Camera.h"

///////////////////////////////////////////////////////////////////////////////

ToolRayBased::ToolRayBased() :
	mDir	(0.0f, 0.0f, 0.0f),
	mOrigin	(0.0f, 0.0f, 0.0f),
	mX		(0),
	mY		(0),
	mOldX	(0),
	mOldY	(0)
{
}

ToolRayBased::~ToolRayBased()
{
}

void ToolRayBased::SetMouseData(const MouseInfo& mouse)
{
	const int x = mouse.mMouseX;	mX = x;
	const int y = mouse.mMouseY;	mY = y;
	mOldX = mouse.mOldMouseX;
	mOldY = mouse.mOldMouseY;
	mDir = ComputeWorldRay(x, y);
	mOrigin = GetCameraPos();
}

///////////////////////////////////////////////////////////////////////////////

