///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GLScaledCylinder.h"
#include "GLRenderHelpers.h"

static GLuint gDL = 0;

void DrawCylinderInternal();

void InitScaledCylinders()
{
	gDL = glGenLists(1);

	glNewList(gDL, GL_COMPILE);
		//GLRenderHelpers::DrawCylinder(1.0f, 1.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		DrawCylinderInternal();
	glEndList();
}

void ReleaseScaledCylinders()
{
	glDeleteLists(gDL, 1);
}

void RenderScaledCylinder(const PR& pose, const Point& trans, const Point& scale, const Point& color)
{
	glPushMatrix();
	{
		GLRenderHelpers::SetupGLMatrix(pose);
		glTranslatef(trans.x, trans.y, trans.z);
		glScalef(scale.x, scale.y, scale.z);
		glColor4f(color.x, color.y, color.z, 1.0f);
		glCallList(gDL);
	}
	glPopMatrix();
}

