///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintDLCylinderShapeRenderer.h"
#include "GLRenderHelpers.h"

// A basic display-list-based cylinder renderer. Not batched.

PintDLCylinderShapeRenderer::PintDLCylinderShapeRenderer(float r, float h) : mData(r, h)
{
	glNewList(mDisplayListNum, GL_COMPILE);
		GLRenderHelpers::DrawCylinder(r, h);
	glEndList();
}
