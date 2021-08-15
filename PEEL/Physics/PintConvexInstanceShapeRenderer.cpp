///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintConvexInstanceShapeRenderer.h"
#include "PintDLConvexShapeRenderer.h"
#include "GLShader.h"
#include "Camera.h"
#include "PintRenderState.h"

extern	udword	gBatchSize;
extern	bool	gWireframePass;

static PtrContainer* gConvexInstancesRenderers = null;

static PintShapeRenderer* RegisterConvexInstanceRenderer(PintShapeRenderer* renderer)
{
	ASSERT(renderer);

	if(!gConvexInstancesRenderers)
		gConvexInstancesRenderers = ICE_NEW(PtrContainer);
	ASSERT(gConvexInstancesRenderers);

	gConvexInstancesRenderers->AddPtr(renderer);
	return renderer;
}

void ReleaseAllConvexInstanceRenderers()
{
/*	if(gConvexInstancesRenderers)
	{
		const udword Size = gConvexInstancesRenderers->GetNbEntries();
		for(udword i=0;i<Size;i++)
		{
			PintShapeRenderer* renderer = (PintShapeRenderer*)gConvexInstancesRenderers->GetEntry(i);
			DELETESINGLE(renderer);
		}
		DELETESINGLE(gConvexInstancesRenderers);
	}*/
	DELETESINGLE(gConvexInstancesRenderers);
}

void StartInstanceRender()
{
}

void EndInstanceRender()
{
	if(gConvexInstancesRenderers)
	{
		const udword Size = gConvexInstancesRenderers->GetNbEntries();
		for(udword i=0;i<Size;i++)
		{
			PintConvexInstanceRenderer* renderer = (PintConvexInstanceRenderer*)gConvexInstancesRenderers->GetEntry(i);
			renderer->DrawBatch();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

#ifdef QUAT2MATRIXSHADER
static const char* testVertexShader = "#version 330 core\n" STRINGIFY(
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec4 instanceRot;
	layout (location = 2) in vec4 instancePos;

	uniform mat4 projection;
	uniform mat4 view;
	uniform mat4 viewProj;

	out vec4 varyPos;

	void main()
	{
		vec3 p_ = vec3(instanceRot.xyz);

		vec3 xs_ys_zs = p_ + p_;
//		float xs = p.x + p.x;
//		float ys = p.y + p.y;
//		float zs = p.z + p.z;

		vec3 wx_wy_wz = vec3(xs_ys_zs.x * instanceRot.w, xs_ys_zs.y * instanceRot.w, xs_ys_zs.z * instanceRot.w);
//		float wx = w * xs;
//		float wy = w * ys;
//		float wz = w * zs;

		vec3 xx_xy_xz = vec3(xs_ys_zs.x * instanceRot.x, xs_ys_zs.y * instanceRot.x, xs_ys_zs.z * instanceRot.x);
//		float xx = p.x * xs;
//		float xy = p.x * ys;
//		float xz = p.x * zs;

		vec3 yy_yz_zz = vec3(xs_ys_zs.y * instanceRot.y, xs_ys_zs.z * instanceRot.y, xs_ys_zs.z * instanceRot.z);
//		float yy = p.y * ys;
//		float yz = p.y * zs;
//		float zz = p.z * zs;

		vec4 row0 = vec4(	1.0 - yy_yz_zz.x - yy_yz_zz.z,
					xx_xy_xz.y + wx_wy_wz.z,
					xx_xy_xz.z - wx_wy_wz.y,
					0.0);

		vec4 row1 = vec4(	xx_xy_xz.y - wx_wy_wz.z,
			1.0 -	xx_xy_xz.x - yy_yz_zz.z,
					yy_yz_zz.y + wx_wy_wz.x,
					0.0);

		vec4 row2 = vec4(	xx_xy_xz.z + wx_wy_wz.y,
					yy_yz_zz.y - wx_wy_wz.x,
			1.0 -	xx_xy_xz.x - yy_yz_zz.x,
					0.0);

		vec4 row3 = vec4( instancePos.xyz, 1.0 );

		mat4 instanceMatrix = mat4(row0, row1, row2, row3);

		vec4 p = instanceMatrix * vec4(aPos, 1.0);
//		vec4 p = row3 + vec4(aPos, 1.0);
//		vec4 p = vec4(aPos, 1.0);

//		gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
//		gl_Position = vec4(aPos, 1.0);
//		gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
		gl_Position = viewProj * p;
//		gl_Position = projection * view * vec4(aPos, 1.0);

		varyPos = view * p;
	}
);
#else
static const char* testVertexShader = "#version 330 core\n" STRINGIFY(
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in mat4 instanceMatrix;

	uniform mat4 projection;
	uniform mat4 view;
	uniform mat4 viewProj;

	out vec4 varyPos;

	void main()
	{
		vec4 p = instanceMatrix * vec4(aPos, 1.0);

//		gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
//		gl_Position = vec4(aPos, 1.0);
//		gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
		gl_Position = viewProj * p;
//		gl_Position = projection * view * vec4(aPos, 1.0);

		varyPos = view * p;
	}
);
#endif

static const char* testPixelShader = "#version 330 core\n" STRINGIFY(
	out vec4 FragColor;
	in vec4 varyPos;
	uniform vec3 parallelLightDir;
	uniform vec3 mainColor;

	void main()
	{
		vec3 dx = dFdx(varyPos.xyz);
		vec3 dy = dFdy(varyPos.xyz);
		vec3 normal = normalize(cross(dx, dy));

//		float diffuse = (0.3 + 0.7 * max(dot(normal, -parallelLightDir),0.0));
		float diffuse = (0.3 + 0.7 * max(dot(normal, parallelLightDir), 0.0));
		diffuse = clamp(diffuse, 0.0, 1.0);
		FragColor = vec4(mainColor.xyz*diffuse, 1.0);
	//	float diffuse = max(dot(normal, -parallelLightDir), 0.0);
//		diffuse = clamp(diffuse, 0.0, 1.0);

		
//		FragColor = vec4(0.8*diffuse, 0.75*diffuse, 0.9*diffuse, 1.0);
//		FragColor = vec4(parallelLightDir, 1.0);
//		FragColor = vec4(normal, 1.0f);
//		FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
	} 
);

	// TODO: revisit this
	static GLuint gConvexInstanceRendererPrg = 0;

/*	class GLInstanceBuffer
	{
		public:
		GLInstanceBuffer();
		~GLInstanceBuffer();

#ifdef QUAT2MATRIXSHADER
				Transform*			mPoses;
#else
				Matrix4x4*			mMatrices;
#endif
	};

GLInstanceBuffer::GLInstanceBuffer()
{
}

GLInstanceBuffer::~GLInstanceBuffer()
{
}*/

#define ACCESS				GL_DYNAMIC_DRAW
//#define ACCESS				GL_STATIC_DRAW
PintConvexInstanceRenderer::PintConvexInstanceRenderer(udword nb_verts, const Point* verts) :
#ifdef QUAT2MATRIXSHADER
	mPoses				(null),
#else
	mMatrices			(null),
#endif
	mIndexedTris		(null),
	mSrcVerts			(null),
	mNbVerts			(0),
	mNbTris				(0),
	mCurrentNbInstances	(0),
	mMaxNbInstances		(gBatchSize),
	mVBO				(0),
	mVAO				(0),
	mEBO				(0),
	mIBO				(0)
{
	RegisterConvexInstanceRenderer(this);

	const udword NbInstances = mMaxNbInstances;
#ifdef QUAT2MATRIXSHADER
	mPoses = ICE_NEW(Transform)[NbInstances];
	ZeroMemory(mPoses, sizeof(Transform)*NbInstances);
#else
	mMatrices = ICE_NEW(Matrix4x4)[NbInstances];
	ZeroMemory(mMatrices, sizeof(Matrix4x4)*NbInstances);
#endif
	Cvh* CHull = CreateConvexHull(nb_verts, verts);
	ASSERT(CHull);

	// For the wireframe part
	CreateHullDisplayList(mDisplayListNum, CHull);

	const udword NbVerts = CHull->GetNbVerts();
	const Point* ConvexVerts = CHull->GetVerts();
	mSrcVerts = ICE_NEW(Point)[NbVerts];
	CopyMemory(mSrcVerts, ConvexVerts, sizeof(Point)*NbVerts);
	mNbVerts = NbVerts;

	{
		udword TotalNbTris = 0;
//		udword TotalNbVerts = 0;
		const udword NbPolys = CHull->GetNbPolygons();
		for(udword i=0;i<NbPolys;i++)
		{
			const HullPolygon& PolygonData = CHull->GetPolygon(i);
			const udword NbVertsInPoly = PolygonData.mNbVerts;
//			TotalNbVerts += NbVertsInPoly;
			const udword NbTris = NbVertsInPoly - 2;
			TotalNbTris += NbTris;
		}

		mNbTris = TotalNbTris;
//		mTotalNbTris = TotalNbTris;
//		mTotalNbVerts = TotalNbVerts;
		mIndexedTris = ICE_NEW(IndexedTriangle)[TotalNbTris];
//		mSrcVerts = ICE_NEW(Point)[TotalNbVerts];
//		mSrcNormals = ICE_NEW(Point)[TotalNbVerts];
	}

	{
		udword TotalNbTris = 0;
//		udword TotalNbVerts = 0;
		//const Point* ConvexVerts = CHull->GetVerts();
		const udword NbPolys = CHull->GetNbPolygons();
		for(udword j=0;j<NbPolys;j++)
		{
			const HullPolygon& PolygonData = CHull->GetPolygon(j);
			const udword NbVertsInPoly = PolygonData.mNbVerts;
			const udword NbTris = NbVertsInPoly - 2;
			const udword* Indices = PolygonData.mVRef;

			udword Offset = 1;
			for(udword i=0;i<NbTris;i++)
			{
				const udword VRef0b = Indices[0];
				const udword VRef1b = Indices[Offset];
				const udword VRef2b = Indices[Offset+1];
				mIndexedTris[TotalNbTris].mRef[0] = VRef0b;
				mIndexedTris[TotalNbTris].mRef[1] = VRef1b;
				mIndexedTris[TotalNbTris].mRef[2] = VRef2b;
				Offset++;
				TotalNbTris++;
			}
		}
//		ASSERT(TotalNbVerts==mTotalNbVerts);
		ASSERT(TotalNbTris==mNbTris);
	}
	DELETESINGLE(CHull);

	if(!gConvexInstanceRendererPrg)
		gConvexInstanceRendererPrg = GLShader::CompileProgram(testVertexShader, testPixelShader, null);

	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);
	glGenBuffers(1, &mIBO);

	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Point)*mNbVerts, mSrcVerts, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mNbTris*sizeof(IndexedTriangle), mIndexedTris->mRef, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0); 
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
	glBindVertexArray(0); 

//	glDeleteVertexArrays(1, &VAO);
//	glDeleteBuffers(1, &VBO);
//	 glDeleteBuffers(1, &EBO);

	glBindBuffer(GL_ARRAY_BUFFER, mIBO);
#ifdef QUAT2MATRIXSHADER
	glBufferData(GL_ARRAY_BUFFER, NbInstances * sizeof(Transform), mPoses, ACCESS);
#else
	glBufferData(GL_ARRAY_BUFFER, NbInstances * sizeof(Matrix4x4), mMatrices, ACCESS);
#endif
		glBindVertexArray(mVAO);
#ifdef QUAT2MATRIXSHADER
		const GLsizei vec4Size = 4*4;
		const udword Stride = sizeof(Transform);
#else
		const GLsizei vec4Size = 4*4;
		const udword Stride = sizeof(Matrix4x4);
#endif
		const udword baseLoc = 1;
		glEnableVertexAttribArray(baseLoc+0);	glVertexAttribPointer(baseLoc+0, 4, GL_FLOAT, GL_FALSE, Stride, (void*)0);
		glEnableVertexAttribArray(baseLoc+1);	glVertexAttribPointer(baseLoc+1, 4, GL_FLOAT, GL_FALSE, Stride, (void*)(vec4Size));
#ifndef QUAT2MATRIXSHADER
		glEnableVertexAttribArray(baseLoc+2);	glVertexAttribPointer(baseLoc+2, 4, GL_FLOAT, GL_FALSE, Stride, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(baseLoc+3);	glVertexAttribPointer(baseLoc+3, 4, GL_FLOAT, GL_FALSE, Stride, (void*)(3 * vec4Size));
#endif
		glVertexAttribDivisor(baseLoc+0, 1);
		glVertexAttribDivisor(baseLoc+1, 1);
#ifndef QUAT2MATRIXSHADER
		glVertexAttribDivisor(baseLoc+2, 1);
		glVertexAttribDivisor(baseLoc+3, 1);
#endif
		glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0); 

	mCurrentNbInstances = 0;
}

PintConvexInstanceRenderer::~PintConvexInstanceRenderer()
{
	DELETEARRAY(mIndexedTris);
	DELETEARRAY(mSrcVerts);
//	DELETEARRAY(mSrcNormals);
#ifdef QUAT2MATRIXSHADER
	DELETEARRAY(mPoses);
#else
	DELETEARRAY(mMatrices);
#endif
}

void PintConvexInstanceRenderer::DrawBatch() const
{
	if(mCurrentNbInstances)
	{
		GLint curProg;
		glGetIntegerv(GL_CURRENT_PROGRAM, &curProg);

		glUseProgram(gConvexInstanceRendererPrg);

		// We must set these after selecting the prg
		// ### but we don't need to do it once per batch!!
		{
			Matrix4x4 MV;
			GetModelViewMatrix(MV);

			Matrix4x4 P;
			GetProjMatrix(P);

			udword loc = glGetUniformLocation(gConvexInstanceRendererPrg, "view");
			glUniformMatrix4fv(loc, 1, GL_FALSE, &MV.m[0][0]);

			loc = glGetUniformLocation(gConvexInstanceRendererPrg, "projection");
			glUniformMatrix4fv(loc, 1, GL_FALSE, &P.m[0][0]);

			const Matrix4x4 ViewProj = MV * P;
			loc = glGetUniformLocation(gConvexInstanceRendererPrg, "viewProj");
			glUniformMatrix4fv(loc, 1, GL_FALSE, &ViewProj.m[0][0]);

			// TODO: put this in a better place
//				Point gBackLightDir(0.0f, -1.0f, 0.0f);
				Point gBackLightDir(0.0f, 1.0f, 0.0f);
//				const PxMat44 camTrans(mCamModelView);
//				PxVec3 eyeDir = camTrans.rotate(mBackLightDir); eyeDir.normalize();
				Matrix3x3 m = MV;
				Point eyeDir = gBackLightDir * m;
//				Point eyeDir = gBackLightDir;
				eyeDir.Normalize();
				GLShader::SetUniform3f(gConvexInstanceRendererPrg, "parallelLightDir", eyeDir.x, eyeDir.y, eyeDir.z);

			const Point MainColor = GetMainColor();
			GLShader::SetUniform3f(gConvexInstanceRendererPrg, "mainColor", MainColor.x, MainColor.y, MainColor.z);
		}

//		if(!(gBatchSize&1))
		{
			glBindBuffer(GL_ARRAY_BUFFER, mIBO);
#ifdef QUAT2MATRIXSHADER
			glBufferData(GL_ARRAY_BUFFER, mCurrentNbInstances * sizeof(Transform), mPoses, ACCESS);
#else
			glBufferData(GL_ARRAY_BUFFER, mCurrentNbInstances * sizeof(Matrix4x4), mMatrices, ACCESS);
#endif
			glBindBuffer(GL_ARRAY_BUFFER, 0); 
		}

		glBindVertexArray(mVAO);
	//	glDrawArrays(GL_TRIANGLES, 0, 3);
	//	glDrawElements(GL_TRIANGLES, NbTris*3, GL_UNSIGNED_INT, 0);
		glDrawElementsInstanced(GL_TRIANGLES, mNbTris*3, GL_UNSIGNED_INT, 0, mCurrentNbInstances);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glUseProgram(0);
		glBindVertexArray(0);

		mCurrentNbInstances = 0;

		glUseProgram(curProg);
	}
}

void PintConvexInstanceRenderer::_Render(const PR& pose) const
{
	if(gWireframePass)
	{
		PintDLShapeRenderer::_Render(pose);
		return;
	}

#ifdef QUAT2MATRIXSHADER
	const udword Nb = mCurrentNbInstances++;
	mPoses[Nb].p = pose.mPos;
	mPoses[Nb].q = pose.mRot;
#else
	mMatrices[mCurrentNbInstances++] = Matrix4x4(pose);
#endif
	if(mCurrentNbInstances==mMaxNbInstances)
		DrawBatch();
}

