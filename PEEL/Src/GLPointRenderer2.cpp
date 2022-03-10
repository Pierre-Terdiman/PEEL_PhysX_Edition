///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GLPointRenderer2.h"
#include "Camera.h"
#include "GLShader.h"
#include "PEEL.h"

// The problem with this one is that the points aren't perspective-correct

static const char* vs = "#version 130\n" STRINGIFY(
	void main()
	{
		gl_FrontColor = gl_Color;
//		gl_BackColor = gl_Color;

		gl_Position = ftransform();
	}
);

static const char* ps = "#version 130\n" STRINGIFY(

	uniform vec3 spherePos;
	uniform float sphereRadius;
	uniform float constantScale;

	void main()
	{
//		gl_FragColor = vec4(gl_FragCoord.x/768.0, 0.0, 0.0, 1.0);
//		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
//		gl_FragColor = gl_Color;
//		gl_FragColor = vec4(gl_Color.x, gl_Color.y, 0.0, 1.0);

		// gl_Color.x = u between 0.0 and 1.0
		// gl_Color.y = v between 0.0 and 1.0
//		vec3 normal = vec3(gl_Color.x - 0.5, gl_Color.y - 0.5, 0.0f);
//		float mag = dot(normal.xy, normal.xy);
//		if(mag > 1.0)
//			discard;

		vec3 normal;
//		normal.xy = gl_Color.xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
		normal.xy = gl_Color.zw*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
		float mag = dot(normal.xy, normal.xy);
		if(mag > 1.0)
		{
//			gl_FragColor = vec4(1.0, 0, 0, 1.0);
			discard;   // kill pixels outside circle
		}
		else
		{
			normal.z = sqrt(1.0-mag);

			float radius = sphereRadius;
			vec4 viewPos = gl_ModelViewMatrix*vec4(spherePos.xyz, 1.0);
			vec3 eyePos = viewPos.xyz + normal*radius;
			vec4 ndcPos = gl_ProjectionMatrix * vec4(eyePos, 1.0);

//			float coeff = 1.0f * ndcPos.z;
//			float coeff = 1.0f / viewPos.z;

			ndcPos.z /= ndcPos.w;
			float depth = ndcPos.z*0.5 + 0.5;
			gl_FragDepth = depth;

			// depth = 0 => 0.95
			// depth = 1000 => 0.9
//			float limit = 1.0 - depth*0.1;
//			float limit = 1.0 - depth*0.04;
//			float limit = 1.0 - 1.0/depth;

			//limit = a*depth+b;
			//depth = 0 => b = 0.95
			//depth = 1000 => limit = 0.5 => 0.5 = a*1000 + 0.95 => a = (0.5 - 0.95)/1000
			float lowLimit = 0.95;
			float highLimit = 0.05;
			float a = (highLimit - lowLimit)/100;
			float b = lowLimit;
			float limit = a*depth+b;

//			limit = 0.9*0.001f/constantScale;
			limit = 10.0;

//			float coeff = 1.0f * ndcPos.z;
			if(mag > limit)
//			if(mag > 0.95)
//			if(mag > 0.9*coeff)
			{
				gl_FragColor = vec4(1.0, 0, 0, 1.0);
			}
			else
			{
				vec3 L = vec3(0.5, 0.5, 0.5);
				normalize(L);

				float d = dot(L, normal);
				gl_FragColor = vec4(d, d, d, 1.0);
//				gl_FragColor = vec4(depth, depth, depth, 1.0);
			}

		}
	}
);

static GLuint Program = 0;
static Container* gBatched = null;

void GLPointRenderer2::Init()
{
	Program = GLShader::CompileProgram(vs, ps, null);

	gBatched = ICE_NEW(Container);
}

void GLPointRenderer2::Close()
{
	DELETESINGLE(gBatched);
}

void GLPointRenderer2::DrawPoint(const Point& pos, float radius)
{
	//SetupProjectionMatrix();
	//SetupModelViewMatrix();

	Matrix4x4 ViewMatrix;
	GetModelViewMatrix(ViewMatrix);

	Matrix4x4 ProjMatrix;
	GetProjMatrix(ProjMatrix);

	// ICE: Extract the camera's up and right vector
	// TODO: move to camera file
	Point cy, cx;
	{
		cx.x = ViewMatrix.m[0][0]; cx.y = ViewMatrix.m[1][0]; cx.z = ViewMatrix.m[2][0]; // Right
		cy.x = ViewMatrix.m[0][1]; cy.y = ViewMatrix.m[1][1]; cy.z = ViewMatrix.m[2][1]; // Up
	}


	float Scale = ComputeConstantScale(pos, ViewMatrix, ProjMatrix);

	const Point Ex = cx*radius;
	const Point Ey = cy*radius;

	const Point p0 = pos - Ex - Ey;
	const Point p1 = pos - Ex + Ey;
	const Point p2 = pos + Ex + Ey;
	const Point p3 = pos + Ex - Ey;

//	glColor3f(0.0f, 1.0f, 0.0f);
//	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glUseProgram(Program);
	GLShader::SetUniform3f(Program, "spherePos", pos.x, pos.y, pos.z);
	GLShader::SetUniform1f(Program, "sphereRadius", radius);
	GLShader::SetUniform1f(Program, "constantScale", Scale);

	// TODO: technically we don't need to pass all the zs, they're the same
	// TODO: use color x/y for something else, it's free right now
	glBegin(GL_TRIANGLES);
//		glColor3f(1.0f, 0.0f, 0.0f);
//		glColor3f(0.0f, 0.0f, 0.0f);
		glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
		glVertex3f(p0.x, p0.y, p0.z);

//		glColor3f(0.0f, 1.0f, 0.0f);
//		glColor3f(0.0f, 1.0f, 0.0f);
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		glVertex3f(p1.x, p1.y, p1.z);

//		glColor3f(0.0f, 0.0f, 1.0f);
//		glColor3f(1.0f, 1.0f, 0.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glVertex3f(p2.x, p2.y, p2.z);

//		glColor3f(1.0f, 0.0f, 0.0f);
//		glColor3f(0.0f, 0.0f, 0.0f);
		glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
		glVertex3f(p0.x, p0.y, p0.z);

//		glColor3f(0.0f, 1.0f, 0.0f);
//		glColor3f(1.0f, 1.0f, 0.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glVertex3f(p2.x, p2.y, p2.z);

//		glColor3f(0.0f, 0.0f, 1.0f);
//		glColor3f(1.0f, 0.0f, 0.0f);
		glColor4f(1.0f, 0.0f, 1.0f, 0.0f);
		glVertex3f(p3.x, p3.y, p3.z);
	glEnd();
	glUseProgram(0);
	glEnable(GL_CULL_FACE);	glCullFace(GL_BACK);
//	glEnable(GL_LIGHTING);
}

void GLPointRenderer2::BatchPoint(const Point& pos, float radius)
{
	Sphere* S = (Sphere*)gBatched->Reserve(4);
	S->mCenter = pos;
	S->mRadius = radius;
}

void GLPointRenderer2::DrawBatchedPoints()
{
	udword NbSpheres = gBatched->GetNbEntries()/4;
	if(NbSpheres)
	{
		//SetupProjectionMatrix();
		//SetupModelViewMatrix();

		Matrix4x4 ViewMatrix;
		GetModelViewMatrix(ViewMatrix);

		Matrix4x4 ProjMatrix;
		GetProjMatrix(ProjMatrix);

		// ICE: Extract the camera's up and right vector
		// TODO: move to camera file
		Point cy, cx;
		{
			cx.x = ViewMatrix.m[0][0]; cx.y = ViewMatrix.m[1][0]; cx.z = ViewMatrix.m[2][0]; // Right
			cy.x = ViewMatrix.m[0][1]; cy.y = ViewMatrix.m[1][1]; cy.z = ViewMatrix.m[2][1]; // Up
		}

	//	glColor3f(0.0f, 1.0f, 0.0f);
	//	glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glUseProgram(Program);

		float Scale = 0.0f;//ComputeConstantScale(pos, ViewMatrix, ProjMatrix);
		GLShader::SetUniform1f(Program, "constantScale", Scale);

		const Sphere* S = (const Sphere*)gBatched->GetEntries();

		while(NbSpheres--)
		{
			const Point& pos = S->mCenter;
			const float radius = S->mRadius;

			const Point Ex = cx*radius;
			const Point Ey = cy*radius;

			const Point p0 = pos - Ex - Ey;
			const Point p1 = pos - Ex + Ey;
			const Point p2 = pos + Ex + Ey;
			const Point p3 = pos + Ex - Ey;

			GLShader::SetUniform3f(Program, "spherePos", pos.x, pos.y, pos.z);
			GLShader::SetUniform1f(Program, "sphereRadius", radius);

			// TODO: technically we don't need to pass all the zs, they're the same
			// TODO: use color x/y for something else, it's free right now
/*			glBegin(GL_TRIANGLES);
		//		glColor3f(1.0f, 0.0f, 0.0f);
		//		glColor3f(0.0f, 0.0f, 0.0f);
				glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
				glVertex3f(p0.x, p0.y, p0.z);

		//		glColor3f(0.0f, 1.0f, 0.0f);
		//		glColor3f(0.0f, 1.0f, 0.0f);
				glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
				glVertex3f(p1.x, p1.y, p1.z);

		//		glColor3f(0.0f, 0.0f, 1.0f);
		//		glColor3f(1.0f, 1.0f, 0.0f);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				glVertex3f(p2.x, p2.y, p2.z);

		//		glColor3f(1.0f, 0.0f, 0.0f);
		//		glColor3f(0.0f, 0.0f, 0.0f);
				glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
				glVertex3f(p0.x, p0.y, p0.z);

		//		glColor3f(0.0f, 1.0f, 0.0f);
		//		glColor3f(1.0f, 1.0f, 0.0f);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				glVertex3f(p2.x, p2.y, p2.z);

		//		glColor3f(0.0f, 0.0f, 1.0f);
		//		glColor3f(1.0f, 0.0f, 0.0f);
				glColor4f(1.0f, 0.0f, 1.0f, 0.0f);
				glVertex3f(p3.x, p3.y, p3.z);
			glEnd();*/

			glBegin(GL_TRIANGLE_STRIP);
//				glColor3f(1.0f, 0.0f, 0.0f);
//				glColor3f(0.0f, 0.0f, 0.0f);
				glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
				glVertex3f(p0.x, p0.y, p0.z);

//				glColor3f(0.0f, 1.0f, 0.0f);
//				glColor3f(0.0f, 1.0f, 0.0f);
				glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
				glVertex3f(p1.x, p1.y, p1.z);

//				glColor3f(0.0f, 0.0f, 1.0f);
//				glColor3f(1.0f, 0.0f, 0.0f);
				glColor4f(1.0f, 0.0f, 1.0f, 0.0f);
				glVertex3f(p3.x, p3.y, p3.z);

//				glColor3f(0.0f, 0.0f, 1.0f);
//				glColor3f(1.0f, 1.0f, 0.0f);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				glVertex3f(p2.x, p2.y, p2.z);
			glEnd();

			S++;
		}
		glUseProgram(0);
		glEnable(GL_CULL_FACE);	glCullFace(GL_BACK);
	//	glEnable(GL_LIGHTING);
	}
	gBatched->Reset();
}
