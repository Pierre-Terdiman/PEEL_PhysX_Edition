///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestScenes_Heightfield.h"

#include "PintTerrainTileRenderer.h"
#include "Terrain.h"
#include "PintObjectsManager.h"

HeightfieldHelper::HeightfieldHelper(udword nb_u, udword nb_v) :
	mIndexBuffers	(null),
	mNbU			(nb_u),
	mNbV			(nb_v),
	mScaleU			(1.0f),
	mScaleV			(1.0f),
	mSizeX			(0.0f),
	mSizeZ			(0.0f)
{
}

HeightfieldHelper::~HeightfieldHelper()
{
}

bool HeightfieldHelper::Init(float size_x, float size_z, float amplitude)
{
	mSizeX = size_x;
	mSizeZ = size_z;
	{
		const float MinX = 0.0f;
		const float MinZ = 0.0f;
		const float MaxX = size_x;
		const float MaxZ = size_z;
		mScaleU = MaxX/float(mNbU-1);
		mScaleV = MaxZ/float(mNbV-1);

//			const Point p0(MinX, 0.0f, MinZ);
//			const Point p1(MinX, 0.0f, MaxZ);
//			const Point p2(MaxX, 0.0f, MinZ);
//			const Point p3(MaxX, 0.0f, MaxZ);

		const Point p0(MinZ, 0.0f, MinX);
		const Point p1(MinZ, 0.0f, MaxX);
		const Point p2(MaxZ, 0.0f, MinX);
		const Point p3(MaxZ, 0.0f, MaxX);

		// Interpolate from x-min to x-max first with z=MinZ, then again increasing z
//			const Point p0(MinX, 0.0f, MinZ);
//			const Point p1(MaxX, 0.0f, MinZ);
//			const Point p2(MinX, 0.0f, MaxZ);
//			const Point p3(MaxX, 0.0f, MaxZ);

//			const Point p0(MinX, 0.0f, MaxZ);
//			const Point p1(MinX, 0.0f, MinZ);
//			const Point p2(MaxX, 0.0f, MaxZ);
//			const Point p3(MaxX, 0.0f, MinZ);

		/*const*/ Quad Q(p0, p1, p2, p3);

		MakePlane(mSurface, mNbU, mNbV, &Q, false);
		//mSurface.Flip();
	}

	{
		Point* V = mSurface.GetVerts();
		udword Offset = 0;
		for(udword j=0;j<mNbV;j++)
		{
			const float CoeffV = float(j)/float(mNbV);
			for(udword i=0;i<mNbU;i++)
			{
				const float CoeffU = float(i)/float(mNbU);

				//V[Offset++].y = sinf(float(i) * float(j) * 0.1f);
				//V[Offset++].y = sinf(float(i*1.0f) * 1.0f) - (CoeffU+CoeffV)*3.0f;
				V[Offset++].y = amplitude * sinf(CoeffU*TWOPI*2.0f) * cosf(CoeffV*TWOPI*2.0f);
			}
		}
	}

	const SurfaceInterface SI = mSurface.GetSurfaceInterface();

	GLIndexBuffer* IB = ICE_NEW(GLIndexBuffer);
	IB->Create(SI.mNbFaces, SI.mDFaces, SI.mWFaces);

	mIndexBuffers = ICE_NEW(GLIndexBufferCollection);
	mIndexBuffers->AddIndexBuffer(*IB);

	return true;
}

void HeightfieldHelper::Release()
{
	mSurface.Clean();
	const udword NbIndexBuffers = mIndexBuffers->GetNbIndexBuffers();
	for(udword i=0;i<NbIndexBuffers;i++)
	{
		const GLIndexBuffer* IB = mIndexBuffers->GetIndexBuffer(i);
		DELETESINGLE(IB);
	}
	DELETESINGLE(mIndexBuffers);
}

void HeightfieldHelper::Close(Pint& pint)
{
	PintTerrainTileRenderer* TTR = reinterpret_cast<PintTerrainTileRenderer*>(pint.mUserData);
	DELETESINGLE(TTR);
}

PintActorHandle HeightfieldHelper::Setup(Pint& pint, const PintCaps& caps, const Point& pos)
{
	if(!caps.mSupportHeightfields)
		return null;

	const udword NbVerts = mSurface.GetNbVerts();
	const Point* V = mSurface.GetVerts();

	if(0)
	{
		PINT_MESH_DATA_CREATE MeshDataDesc;
		MeshDataDesc.SetSurfaceData(mSurface.GetSurfaceInterface());

		PintMeshIndex MeshIndex;
		const PintMeshHandle MeshHandle = pint.CreateMeshObject(MeshDataDesc, &MeshIndex);

		PINT_MESH_CREATE2 MeshDesc(MeshHandle);

		PintTerrainTileRenderer* TTR = ICE_NEW(PintTerrainTileRenderer)(*mIndexBuffers, NbVerts, V, null);
		MeshDesc.mRenderer	= TTR;
		ASSERT(!pint.mUserData);
		pint.mUserData = TTR;

		PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
		ObjectDesc.mMass		= 0.0f;
		ObjectDesc.mPosition	= pos + Point(-mSizeZ*0.5f, 0.0f, -mSizeX*0.5f);
		return CreatePintObject(pint, ObjectDesc);
	}
	else
	{
		PintHeightfieldHandle HFHandle;
		PintHeightfieldData HFData;
		{
			float* Heights = new float[mNbU*mNbV];
			for(udword i=0;i<mNbU*mNbV;i++)
				Heights[i] = V[i].y;

			PINT_HEIGHTFIELD_DATA_CREATE HFDataDesc(mNbU, mNbV, Heights);

			PintHeightfieldIndex HFIndex;
			HFHandle = pint.CreateHeightfieldObject(HFDataDesc, HFData, &HFIndex);

			DELETEARRAY(Heights);
		}

		PINT_HEIGHTFIELD_CREATE HFDesc(HFHandle);
		HFDesc.mScaleU	= mScaleU;
		HFDesc.mScaleV	= mScaleV;
		{
			Point* NewPts = ICE_NEW(Point)[mNbU*mNbV];
			for(udword i=0;i<mNbU*mNbV;i++)
			{
				NewPts[i] = V[i];
				NewPts[i].y -= HFData.mMinHeight;
			}

			PintTerrainTileRenderer* TTR = ICE_NEW(PintTerrainTileRenderer)(*mIndexBuffers, NbVerts, NewPts, null);
			HFDesc.mRenderer	= TTR;
			ASSERT(!pint.mUserData);
			pint.mUserData = TTR;

			DELETEARRAY(NewPts);
		}

		PINT_OBJECT_CREATE ObjectDesc(&HFDesc);
		ObjectDesc.mMass		= 0.0f;
		ObjectDesc.mPosition	= pos + Point(-mSizeZ*0.5f, 0.0f, -mSizeX*0.5f);
		return CreatePintObject(pint, ObjectDesc);
	}
}

///////////////////////////////////////////////////////////////////////////////

HeightfieldTest::HeightfieldTest(udword nb_u, udword nb_v) : mHH(nb_u, nb_v)
{
}

HeightfieldTest::~HeightfieldTest()
{
}

void HeightfieldTest::CommonRelease()
{
	mHH.Release();

	TestBase::CommonRelease();
}

void HeightfieldTest::Close(Pint& pint)
{
	mHH.Close(pint);

	TestBase::Close(pint);
}

bool HeightfieldTest::Init(Pint& pint, const PintCaps& caps, const Point& pos)
{
	return mHH.Setup(pint, caps, pos)!=null;
}
