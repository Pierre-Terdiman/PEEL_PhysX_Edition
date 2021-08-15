///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintPointSpriteSphereShapeRenderer.h"
#include "GLPointRenderer2.h"

// This point-sprite sphere isn't perspective correct and cannot be combined with the DL version....
// so we need the real raytaced version, and for that we need to compute the proper screen bounds.

PintPointSpriteSphereShapeRenderer::PintPointSpriteSphereShapeRenderer(float radius) : mRadius(radius)
{
}

void PintPointSpriteSphereShapeRenderer::_Render(const PR& pose) const
{
//	glPushMatrix();
//	GLRenderHelpers::SetupGLMatrix(pose);
//	GLPointRenderer2::DrawPoint(pose.mPos, mRadius);
	GLPointRenderer2::BatchPoint(pose.mPos, mRadius);
//	glPopMatrix();
}

