///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintRenderState.h"

static Point gEngineColor(0.0f, 0.0f, 0.0f);
static Point gMainColor(0.0f, 0.0f, 0.0f);

void SetEngineColor(const Point& color)
{
	gEngineColor = color;
	gMainColor = color;
	glColor4f(color.x, color.y, color.z, 1.0f);
}

void SetMainColor(const Point& color)
{
	gMainColor = color;
	glColor4f(color.x, color.y, color.z, 1.0f);
}

void SetMainColor(const RGBAColor& color)
{
	gMainColor.x = color.R;
	gMainColor.y = color.G;
	gMainColor.z = color.B;
	glColor4f(color.R, color.G, color.B, color.A);
}

void ResetMainColor()
{
	gMainColor = gEngineColor;
	glColor4f(gEngineColor.x, gEngineColor.y, gEngineColor.z, 1.0f);
}

Point GetMainColor()
{
	return gMainColor;
}
