///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintDLCapsuleShapeRenderer.h"
#include "GLRenderHelpers.h"
#include "Capsule.h"

// A basic display-list-based capsule renderer. Not batched.

PintDLCapsuleShapeRenderer::PintDLCapsuleShapeRenderer(float r, float h, bool uses_vertex_normals) : mData(r, h, uses_vertex_normals)
{
	glNewList(mDisplayListNum, GL_COMPILE);
	{
		if(0)
		{
			GLRenderHelpers::DrawCapsule(r, h);
		}
		else
		{
			const udword NbCirclePts = 16;
			const CapsuleMesh CM(NbCirclePts, r, h*0.5f, uses_vertex_normals);

			const Point* V = CM.mVerts;
			const Point* N = CM.mNormals;
			const udword* Indices = CM.mTris->mRef;

			if(1)
			{
				glBegin(GL_QUADS);
				udword NbQuads = CM.mNbTris/2;
				while(NbQuads--)
				{
					const udword i0 = *Indices++;
					const udword i1 = *Indices++;
					const udword i2 = *Indices++;
					const udword i0b = *Indices++;
					const udword i2b = *Indices++;
					const udword i3 = *Indices++;

					const Point& p0 = V[i0];
					const Point& p1 = V[i1];
					const Point& p2 = V[i2];
					const Point& p3 = V[i3];

					if(uses_vertex_normals)
					{
						const Point& n0 = N[i0];
						const Point& n1 = N[i1];
						const Point& n2 = N[i2];
						const Point& n3 = N[i3];

						glNormal3f(n0.x, n0.y, n0.z);
						glVertex3f(p0.x, p0.y, p0.z);

						glNormal3f(n1.x, n1.y, n1.z);
						glVertex3f(p1.x, p1.y, p1.z);

						glNormal3f(n2.x, n2.y, n2.z);
						glVertex3f(p2.x, p2.y, p2.z);

						glNormal3f(n3.x, n3.y, n3.z);
						glVertex3f(p3.x, p3.y, p3.z);
					}
					else
					{
						glVertex3f(p0.x, p0.y, p0.z);
						glVertex3f(p1.x, p1.y, p1.z);
						glVertex3f(p2.x, p2.y, p2.z);
						glVertex3f(p3.x, p3.y, p3.z);
					}
				}
				glEnd();
			}
			else
			{
				glBegin(GL_TRIANGLES);
				for(udword i=0;i<CM.mNbTris;i++)
				{
					const udword i0 = CM.mTris[i].mRef[0];
					const udword i1 = CM.mTris[i].mRef[1];
					const udword i2 = CM.mTris[i].mRef[2];

					const Point& p0 = V[i0];
					const Point& p1 = V[i1];
					const Point& p2 = V[i2];

					if(uses_vertex_normals)
					{
						const Point& n0 = N[i0];
						const Point& n1 = N[i1];
						const Point& n2 = N[i2];

						glNormal3f(n0.x, n0.y, n0.z);
						glVertex3f(p0.x, p0.y, p0.z);

						glNormal3f(n1.x, n1.y, n1.z);
						glVertex3f(p1.x, p1.y, p1.z);

						glNormal3f(n2.x, n2.y, n2.z);
						glVertex3f(p2.x, p2.y, p2.z);
					}
					else
					{
						glVertex3f(p0.x, p0.y, p0.z);
						glVertex3f(p1.x, p1.y, p1.z);
						glVertex3f(p2.x, p2.y, p2.z);
					}
				}
				glEnd();
			}
		}
	}
	glEndList();
}

