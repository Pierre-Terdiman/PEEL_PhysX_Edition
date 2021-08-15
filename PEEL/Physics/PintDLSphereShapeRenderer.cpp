///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintDLSphereShapeRenderer.h"
#include "Sphere.h"

// A basic display-list-based sphere renderer. Not batched. Includes vertex normals.

static void CreateDL(const Point* V, const udword* Indices, udword NbTris, bool uses_vertex_normals, bool geo_sphere)
{
	if(!geo_sphere)
	{
		udword NbQuads = NbTris/2;
		glBegin(GL_QUADS);
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
				Point n0 = p0;	n0.Normalize();
				Point n1 = p1;	n1.Normalize();
				Point n2 = p2;	n2.Normalize();
				Point n3 = p3;	n3.Normalize();

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
		while(NbTris--)
		{
			const udword i0 = *Indices++;
			const udword i1 = *Indices++;
			const udword i2 = *Indices++;

			const Point& p0 = V[i0];
			const Point& p1 = V[i1];
			const Point& p2 = V[i2];

			if(uses_vertex_normals)
			{
				Point n0 = p0;	n0.Normalize();
				Point n1 = p1;	n1.Normalize();
				Point n2 = p2;	n2.Normalize();

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

PintDLSphereShapeRenderer::PintDLSphereShapeRenderer(float radius, bool uses_vertex_normals, bool geo_sphere) : mData(radius, uses_vertex_normals, geo_sphere)
{
	glNewList(mDisplayListNum, GL_COMPILE);
	{
		if(0)
		{
			glutxSolidSphere(radius, 12, 12);
		}
		else
		{
			if(geo_sphere)
			{
				const GeoSphereMesh SM(radius);
				CreateDL(SM.mVerts, SM.mTris->mRef, SM.mNbTris, uses_vertex_normals, geo_sphere);
			}
			else
			{
				const udword NbCirclePts = 16;
				const udword NbRotations = 16;
				const SphereMesh SM(NbCirclePts, NbRotations, radius);
				CreateDL(SM.mVerts, SM.mTris->mRef, SM.mNbTris, uses_vertex_normals, geo_sphere);
			}
		}
	}
	glEndList();
}

/*		virtual	void				Render(const PR& pose)
		{
			if(gWireframePass)
				PintDLShapeRenderer::Render(pose);
			else
				GLPointRenderer2::BatchPoint(pose.mPos, mRadius);
		}*/
