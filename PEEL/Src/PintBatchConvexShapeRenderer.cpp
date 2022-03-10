///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintBatchConvexShapeRenderer.h"
#include "PintDLConvexShapeRenderer.h"

#include <xmmintrin.h>
#include <emmintrin.h>

extern	bool	gWireframePass;
extern	udword	gBatchSize;

static Container* gBatchIndices = null;
static Vertices* gBatchVertices = null;
static Vertices* gBatchNormals = null;
static udword gCurrentNbVerts = 0;
static udword gCurrentNbTris = 0;

void ReleaseBatchConvexRender()
{
	DELETESINGLE(gBatchNormals);
	DELETESINGLE(gBatchVertices);
	DELETESINGLE(gBatchIndices);
}

void StartBatchConvexRender()
{
	gCurrentNbVerts = 0;
	gCurrentNbTris = 0;
	if(!gBatchIndices)
		gBatchIndices = ICE_NEW(Container);
	if(!gBatchVertices)
		gBatchVertices = ICE_NEW(Vertices);
	if(!gBatchNormals/* && gCurrentRenderModel->NeedsVertexNormals()*/)
		gBatchNormals = ICE_NEW(Vertices);
}

void EndBatchConvexRender()
{
	if(gBatchIndices->GetEntries())
	{
		Matrix4x4 Idt;
		Idt.Identity();
		void SetGlobalPoseRenderModel(const float*);
		SetGlobalPoseRenderModel(&Idt.m[0][0]);

		glEnableClientState(GL_VERTEX_ARRAY);
		const Point* Normals = gBatchNormals->GetVertices();
		if(Normals)
//		if(gBatchNormals)
			glEnableClientState(GL_NORMAL_ARRAY);

		glVertexPointer(3, GL_FLOAT, sizeof(Point), gBatchVertices->GetVertices());
		if(Normals)
//		if(gBatchNormals)
			glNormalPointer(GL_FLOAT, sizeof(Point), Normals);
//			glNormalPointer(GL_FLOAT, sizeof(Point), gBatchNormals->GetVertices());
		glDrawElements(GL_TRIANGLES, 3*gCurrentNbTris, GL_UNSIGNED_INT, gBatchIndices->GetEntries());

		if(Normals)
//		if(gBatchNormals)
			glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	gBatchIndices->Reset();
	gBatchVertices->Reset();
//	if(gBatchNormals)
		gBatchNormals->Reset();
}

static Point* BatchRenderReservePoints(Vertices* pts, udword nb_pts)
{
	Point* P = (Point*)pts->Reserve(nb_pts*3+1);	// +1 for safe SIMD stores
	const udword CurrentSize = pts->GetNbEntries();
	pts->ForceSize(CurrentSize-1);
	return P;
}

static inline_ Point* BatchRenderReserveVerts(udword nb_verts)
{
	return BatchRenderReservePoints(gBatchVertices, nb_verts);
//	return (Point*)gBatchVertices->Reserve(nb_verts*3);
}

static inline_ Point* BatchRenderReserveNormals(udword nb_verts)
{
	return BatchRenderReservePoints(gBatchNormals, nb_verts);
//	return (Point*)gBatchNormals->Reserve(nb_verts*3);
}

static void BatchRender(udword nb_tris, udword nb_verts, const Point* verts, const Point* normals, const udword* indices)
{
	if(verts)
		CopyMemory(gBatchVertices->Reserve(nb_verts*3), verts, sizeof(Point)*nb_verts);
	if(normals)
		CopyMemory(gBatchNormals->Reserve(nb_verts*3), normals, sizeof(Point)*nb_verts);

	udword* BatchedIndices = gBatchIndices->Reserve(nb_tris*3);
	udword Nb = nb_tris*3;
	while(Nb--)
		*BatchedIndices++ = gCurrentNbVerts + *indices++;
/*	for(udword i=0;i<nb_tris*3;i++)
	{
		gBatchIndices->Add(gCurrentNbVerts+indices[i]);
	}*/

	gCurrentNbVerts += nb_verts;
	gCurrentNbTris += nb_tris;

	if(gCurrentNbTris>gBatchSize)
	{
		EndBatchConvexRender();
		StartBatchConvexRender();
	}
}

static	inline_ __m128 Multiply3x3V(const __m128 p, const Matrix4x4& mat)	
{
//	const __m128 xxxV = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(p), _MM_SHUFFLE(3,0,0,0)));
//	const __m128 yyyV = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(p), _MM_SHUFFLE(3,1,1,1)));
//	const __m128 zzzV = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(p), _MM_SHUFFLE(3,2,2,2)));
	const __m128 xxxV = _mm_shuffle_ps(p, p, _MM_SHUFFLE(3,0,0,0));
	const __m128 yyyV = _mm_shuffle_ps(p, p, _MM_SHUFFLE(3,1,1,1));
	const __m128 zzzV = _mm_shuffle_ps(p, p, _MM_SHUFFLE(3,2,2,2));

	__m128 ResV = _mm_mul_ps(xxxV, _mm_loadu_ps(&mat.m[0][0]));
	ResV = _mm_add_ps(ResV, _mm_mul_ps(yyyV, _mm_loadu_ps(&mat.m[1][0])));
	ResV = _mm_add_ps(ResV, _mm_mul_ps(zzzV, _mm_loadu_ps(&mat.m[2][0])));
	return ResV;
}

PintBatchConvexShapeRenderer::PintBatchConvexShapeRenderer(udword nb_verts, const Point* verts)
{
	Cvh* CHull = CreateConvexHull(nb_verts, verts);
	ASSERT(CHull);

	// For the wireframe part
	CreateHullDisplayList(mDisplayListNum, CHull);

	{
		udword TotalNbTris = 0;
#ifdef USE_DRAW_ELEMENTS
		udword TotalNbVerts = 0;
#endif
		const Point* ConvexVerts = CHull->GetVerts();
		const udword NbPolys = CHull->GetNbPolygons();
		for(udword i=0;i<NbPolys;i++)
		{
			const HullPolygon& PolygonData = CHull->GetPolygon(i);
			const udword NbVertsInPoly = PolygonData.mNbVerts;
#ifdef USE_DRAW_ELEMENTS
			TotalNbVerts += NbVertsInPoly;
#endif
			const udword NbTris = NbVertsInPoly - 2;
			TotalNbTris += NbTris;
		}

		mTotalNbTris = TotalNbTris;
#ifdef USE_DRAW_ELEMENTS
		mTotalNbVerts = TotalNbVerts;
//		mIndexedTris = ICE_NEW(IndexedTriangle16)[TotalNbTris];
		mIndexedTris = ICE_NEW(IndexedTriangle)[TotalNbTris];
		mSrcVerts = ICE_NEW(Point)[TotalNbVerts];
		mSrcNormals = ICE_NEW(Point)[TotalNbVerts];
#ifdef USE_CPU_TRANSFORM
#ifndef USE_BATCH_RENDER
		mDstVerts = ICE_NEW(Point)[TotalNbVerts];
		mDstNormals = ICE_NEW(Point)[TotalNbVerts];
#endif
#endif
#else
		mTris = ICE_NEW(Triangle)[TotalNbTris];
		mNormals = ICE_NEW(Triangle)[TotalNbTris];
#endif
	}

	{
		udword TotalNbTris = 0;
#ifdef USE_DRAW_ELEMENTS
		udword TotalNbVerts = 0;
#endif
		const Point* ConvexVerts = CHull->GetVerts();
		const udword NbPolys = CHull->GetNbPolygons();
		for(udword i=0;i<NbPolys;i++)
		{
			const HullPolygon& PolygonData = CHull->GetPolygon(i);
			const udword NbVertsInPoly = PolygonData.mNbVerts;
			const udword* Indices = PolygonData.mVRef;
#ifdef USE_DRAW_ELEMENTS
			udword Remap[1024];
			for(udword i=0;i<NbVertsInPoly;i++)
			{
				Remap[i] = TotalNbVerts;
				mSrcVerts[TotalNbVerts] = ConvexVerts[Indices[i]];
				mSrcNormals[TotalNbVerts] = PolygonData.mPlane.n;
				TotalNbVerts++;
			}
#endif
			const udword NbTris = NbVertsInPoly - 2;
			udword Offset = 1;
			for(udword i=0;i<NbTris;i++)
			{
#ifdef USE_DRAW_ELEMENTS
				const udword VRef0b = Remap[0];
				const udword VRef1b = Remap[Offset];
				const udword VRef2b = Remap[Offset+1];
				mIndexedTris[TotalNbTris].mRef[0] = VRef0b;
				mIndexedTris[TotalNbTris].mRef[1] = VRef1b;
				mIndexedTris[TotalNbTris].mRef[2] = VRef2b;
#else
				const udword VRef0 = Indices[0];
				const udword VRef1 = Indices[Offset];
				const udword VRef2 = Indices[Offset+1];

				mTris[TotalNbTris].mVerts[0] = ConvexVerts[VRef0];
				mTris[TotalNbTris].mVerts[1] = ConvexVerts[VRef1];
				mTris[TotalNbTris].mVerts[2] = ConvexVerts[VRef2];

				mNormals[TotalNbTris].mVerts[0] = PolygonData.mPlane.n;
				mNormals[TotalNbTris].mVerts[1] = PolygonData.mPlane.n;
				mNormals[TotalNbTris].mVerts[2] = PolygonData.mPlane.n;
#endif
				Offset++;
				TotalNbTris++;
			}
		}
		ASSERT(TotalNbVerts==mTotalNbVerts);
		ASSERT(TotalNbTris==mTotalNbTris);
	}
	DELETESINGLE(CHull);
}

PintBatchConvexShapeRenderer::~PintBatchConvexShapeRenderer()
{
#ifdef USE_DRAW_ELEMENTS
	DELETEARRAY(mIndexedTris);
#ifdef USE_CPU_TRANSFORM
#ifndef USE_BATCH_RENDER
	DELETEARRAY(mDstNormals);
	DELETEARRAY(mDstVerts);
#endif
#endif
	DELETEARRAY(mSrcVerts);
	DELETEARRAY(mSrcNormals);
#else
	DELETEARRAY(mNormals);
	DELETEARRAY(mTris);
#endif
}

void PintBatchConvexShapeRenderer::_Render(const PR& pose) const
{
	if(gWireframePass)
	{
		PintDLShapeRenderer::_Render(pose);
		return;
	}

#ifndef USE_CPU_TRANSFORM
	glPushMatrix();
	SetupGLMatrix(pose);
#endif

#ifdef USE_DRAW_ELEMENTS
#ifndef USE_BATCH_RENDER
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
#endif

#ifdef USE_CPU_TRANSFORM
	const bool NeedsNormals = true;//gCurrentRenderModel->NeedsVertexNormals();

	const Matrix4x4 M(pose);	// PT: TODO: SIMD
	const Point* SV = mSrcVerts;
	const Point* SN = mSrcNormals;
	udword N = mTotalNbVerts;
#ifdef USE_BATCH_RENDER
	Point* DV = BatchRenderReserveVerts(N);
	Point* DN = NeedsNormals ? BatchRenderReserveNormals(N) : null;
#else
	Point* DV = mDstVerts;
	Point* DN = mDstNormals;
#endif
	const __m128 transV = _mm_loadu_ps(&M.GetTrans().x);
	if(NeedsNormals)
	{
		while(N--)
		{
			const __m128 v = _mm_add_ps(Multiply3x3V(_mm_loadu_ps(&SV->x), M), transV);
			_mm_storeu_ps(&DV->x, v);
			SV++;
			DV++;

			const __m128 n = Multiply3x3V(_mm_loadu_ps(&SN->x), M);
			_mm_storeu_ps(&DN->x, n);
			SN++;
			DN++;

//			TransformPoint4x3(*DV++, *SV++, M);
//			TransformPoint3x3(*DN++, *SN++, M);
		}
	}
	else
	{
		while(N--)
		{
			const __m128 v = _mm_add_ps(Multiply3x3V(_mm_loadu_ps(&SV->x), M), transV);
			_mm_storeu_ps(&DV->x, v);
			SV++;
			DV++;
		}
	}
#ifdef USE_BATCH_RENDER
	BatchRender(mTotalNbTris, mTotalNbVerts, null, null, mIndexedTris->mRef);
#else
	glVertexPointer(3, GL_FLOAT, sizeof(Point), mDstVerts);
	glNormalPointer(GL_FLOAT, sizeof(Point), mDstNormals);
#endif

#else
	glVertexPointer(3, GL_FLOAT, sizeof(Point), mSrcVerts);
	glNormalPointer(GL_FLOAT, sizeof(Point), mSrcNormals);
#endif

#ifndef USE_BATCH_RENDER
	glDrawElements(GL_TRIANGLES, 3*mTotalNbTris, GL_UNSIGNED_INT, mIndexedTris);
//	glDrawElements(GL_TRIANGLES, 3*mTotalNbTris, GL_UNSIGNED_SHORT, mIndexedTris);

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
#endif
#else
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(Point), mTris);
	glNormalPointer(GL_FLOAT, sizeof(Point), mNormals);

	glDrawArrays(GL_TRIANGLES, 0, 3*mTotalNbTris);

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
#endif
#ifndef USE_CPU_TRANSFORM
	glPopMatrix();
#endif
}






PintConvexBatchRendererCPUTransformNoNormals::PintConvexBatchRendererCPUTransformNoNormals(udword nb_verts, const Point* verts)
{
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
		const Point* ConvexVerts = CHull->GetVerts();
		const udword NbPolys = CHull->GetNbPolygons();
		for(udword i=0;i<NbPolys;i++)
		{
			const HullPolygon& PolygonData = CHull->GetPolygon(i);
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
}

PintConvexBatchRendererCPUTransformNoNormals::~PintConvexBatchRendererCPUTransformNoNormals()
{
	DELETEARRAY(mIndexedTris);
	DELETEARRAY(mSrcVerts);
//	DELETEARRAY(mSrcNormals);
}

void PintConvexBatchRendererCPUTransformNoNormals::_Render(const PR& pose) const
{
	if(gWireframePass)
	{
		PintDLShapeRenderer::_Render(pose);
		return;
	}

	const Matrix4x4 M(pose);	// PT: TODO: SIMD
	const Point* SV = mSrcVerts;
	udword N = mNbVerts;
	Point* DV = BatchRenderReserveVerts(N);
	const __m128 transV = _mm_loadu_ps(&M.GetTrans().x);
	while(N--)
	{
		const __m128 v = _mm_add_ps(Multiply3x3V(_mm_loadu_ps(&SV->x), M), transV);
		_mm_storeu_ps(&DV->x, v);
		SV++;
		DV++;
	}
	BatchRender(mNbTris, mNbVerts, null, null, mIndexedTris->mRef);
}
