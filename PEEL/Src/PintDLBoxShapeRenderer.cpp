///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintDLBoxShapeRenderer.h"
#include "GLRenderHelpers.h"

extern bool	gWireframePass;

// A basic display-list-based box renderer. Not batched.

PintDLBoxShapeRenderer::PintDLBoxShapeRenderer(const Point& extents, bool uses_vertex_normals) :
	mData				(extents, uses_vertex_normals),
	mDisplayListNum2	(0)
{
	// We always need this one (for the wireframe pass if nothing else)
	glNewList(mDisplayListNum, GL_COMPILE);
	{
		glScalef(extents.x, extents.y, extents.z);
		glutxSolidCube(2.0f);
	}
	glEndList();

	// Then we may need another one without normals
	if(!uses_vertex_normals)
	{
		mDisplayListNum2 = glGenLists(1);

		glNewList(mDisplayListNum2, GL_COMPILE);
		{
			AABB Box;
			Box.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), extents);
			Point Pts[8];
			Box.ComputePoints(Pts);
			const udword* Indices = Box.GetTriangles();

			glBegin(GL_TRIANGLES);
			for(udword i=0;i<12;i++)
			{
				const udword VRef0 = *Indices++;
				const udword VRef1 = *Indices++;
				const udword VRef2 = *Indices++;

				const Point& v0 = Pts[VRef0];
				const Point& v1 = Pts[VRef1];
				const Point& v2 = Pts[VRef2];

//				glNormal3f(n0.x, n0.y, n0.z);
				glVertex3f(v0.x, v0.y, v0.z);

//				glNormal3f(n1.x, n1.y, n1.z);
				glVertex3f(v1.x, v1.y, v1.z);

//				glNormal3f(n2.x, n2.y, n2.z);
				glVertex3f(v2.x, v2.y, v2.z);
			}
			glEnd();
		}
		glEndList();
	}
}

PintDLBoxShapeRenderer::~PintDLBoxShapeRenderer()
{
	if(!mData.mUsesVertexNormals)
		glDeleteLists(mDisplayListNum2, 1);
}

void PintDLBoxShapeRenderer::_Render(const PR& pose) const
{
	if(mData.mUsesVertexNormals || gWireframePass)
	{
		PintDLShapeRenderer::_Render(pose);
		return;
	}

	glPushMatrix();
	{
		GLRenderHelpers::SetupGLMatrix(pose);
		glCallList(mDisplayListNum2);
	}
	glPopMatrix();
}