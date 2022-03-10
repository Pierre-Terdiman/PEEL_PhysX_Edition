///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestScenes.h"
#include "TestScenesHelpers.h"
#include "Loader_Bin.h"
#include "PintSQ.h"
#include "PintShapeRenderer.h"

///////////////////////////////////////////////////////////////////////////////

struct MidphaseHits : Allocateable
{
	MidphaseHits(PintActorHandle mesh_handle, PintActorHandle query_shape_handle) : mMeshHandle(mesh_handle), mQueryShapeHandle(query_shape_handle)	
	{
		mHits.SetNoHit();
	}

	PintMultipleHits	mHits;
	Container			mStream;
	PintActorHandle		mMeshHandle;
	PintActorHandle		mQueryShapeHandle;
};

class MidphaseTestzoneTest : public TestBase
{
	public:
							MidphaseTestzoneTest()		{								}
	virtual					~MidphaseTestzoneTest()		{								}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_MIDPHASE;		}
	virtual	udword			GetProfilingFlags()	const	{ return PROFILING_TEST_UPDATE;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)	override
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.44f, -0.57f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()	override
	{
		TestBase::CommonSetup();
		LoadMeshesFromFile_(*this, "testzone.bin");
		return true;
	}

	virtual	void	Close(Pint& pint)	override
	{
		MidphaseHits* MH = reinterpret_cast<MidphaseHits*>(pint.mUserData);
		DELETESINGLE(MH);
	}

	virtual	void	DrawDebugInfo(Pint& pint, PintRender& render)	override
	{
		if(!pint.mUserData)
			return;
		const MidphaseHits* MH = reinterpret_cast<const MidphaseHits*>(pint.mUserData);

		const PintActorHandle Handle = MH->mMeshHandle;
		const Matrix4x4 GlobalPose = pint.GetWorldTransform(Handle);

		const Point Color(1.0f, 1.0f, 1.0f);
		Pint_Actor* ActorAPI = pint.GetActorAPI();
		if(ActorAPI)
		{
			PintShapeHandle ShapeHandle = ActorAPI->GetShape(Handle, 0);	// ####

			Pint_Shape* ShapeAPI = pint.GetShapeAPI();
			if(ShapeAPI)
			{
				Triangle tri;
				const udword NbHits = MH->mHits.mNbHits;
				for(udword i=0;i<NbHits;i++)
				{
					bool Status = ShapeAPI->GetTriangle(tri, ShapeHandle, MH->mStream[i]);

					const Point v0 = tri.mVerts[0] * GlobalPose;
					const Point v1 = tri.mVerts[1] * GlobalPose;
					const Point v2 = tri.mVerts[2] * GlobalPose;

					render.DrawLine(v0, v1, Color);
					render.DrawLine(v1, v2, Color);
					render.DrawLine(v2, v0, Color);
				}
			}
		}
	}
};

///////////////////////////////////////////////////////////////////////////////

static bool MidphaseSetup(Pint& pint, SurfaceManager& sm, const PintCaps& caps, PintActorHandle query_shape_handle)
{
	PtrContainer Objects;
	if(!sm.CreateMeshesFromRegisteredSurfaces(pint, caps, null, &Objects))
		return false;

	ASSERT(Objects.GetNbEntries()==1);
	if(!Objects.GetNbEntries())
		return false;
	PintActorHandle MeshHandle = PintActorHandle(Objects[0]);
	ASSERT(MeshHandle);

	ASSERT(!pint.mUserData);
	MidphaseHits* MH = ICE_NEW(MidphaseHits)(MeshHandle, query_shape_handle);
	pint.mUserData = MH;
	return true;
}

static const char* gDesc_SphereOverlapTriangles_TestZone = "MIDPHASE: TestZone. Sphere overlap triangles.";

class SphereOverlapTriangles_TestZone : public MidphaseTestzoneTest
{
			const float		mRadius;
	public:
							SphereOverlapTriangles_TestZone() : mRadius(40.0f)	{								}
	virtual					~SphereOverlapTriangles_TestZone()					{								}
	virtual	const char*		GetName()					const	{ return "SphereOverlapTriangles_TestZone";		}
	virtual	const char*		GetDescription()			const	{ return gDesc_SphereOverlapTriangles_TestZone;	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphereOverlaps)
			return false;

		PINT_SPHERE_CREATE Create(mRadius);
		Create.mRenderer = CreateNullRenderer();
		const PintActorHandle H = CreateStaticObject(pint, &Create, Point(0.0f, 0.0f, 0.0f));

		return MidphaseSetup(pint, *this, caps, H);
	}

	void GetSphere(Sphere& sphere, Pint& pint, const MidphaseHits* MH)	const
	{
		const PintActorHandle Handle = MH->mQueryShapeHandle;
		const PR GlobalPose = pint.GetWorldTransform(Handle);
		sphere.mCenter = GlobalPose.mPos;
		sphere.mRadius = mRadius;
	}

	virtual	void	DrawDebugInfo(Pint& pint, PintRender& render)	override
	{
		MidphaseTestzoneTest::DrawDebugInfo(pint, render);

		const MidphaseHits* MH = reinterpret_cast<const MidphaseHits*>(pint.mUserData);
		if(MH)
		{
			Sphere S;
			GetSphere(S, pint, MH);
			render.DrawWireframeSphere(S, Point(1.0f, 1.0f, 1.0f));
		}
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		MidphaseHits* MH = reinterpret_cast<MidphaseHits*>(pint.mUserData);
		const PintActorHandle MeshHandle = MH->mMeshHandle;
		ASSERT(MeshHandle);

		PintSphereOverlapData Data;
		GetSphere(Data.mSphere, pint, MH);

		MH->mStream.Reset();
		return pint.FindTriangles_MeshSphereOverlap(pint.mSQHelper->GetThreadContext(), MeshHandle, 1, &MH->mHits, MH->mStream, &Data);
	}

END_TEST(SphereOverlapTriangles_TestZone)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_BoxOverlapTriangles_TestZone = "MIDPHASE: TestZone. Box overlap triangles.";

class BoxOverlapTriangles_TestZone : public MidphaseTestzoneTest
{
			const Point		mExtents;
	public:
							BoxOverlapTriangles_TestZone() : mExtents(30.0f, 40.0f, 30.0f)	{					}
	virtual					~BoxOverlapTriangles_TestZone()									{					}
	virtual	const char*		GetName()					const	{ return "BoxOverlapTriangles_TestZone";		}
	virtual	const char*		GetDescription()			const	{ return gDesc_BoxOverlapTriangles_TestZone;	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportBoxOverlaps)
			return false;

		Matrix3x3 Rot;
		Rot.RotYX(0.42f, 0.27f);
		const Quat Q = Rot;

		PINT_BOX_CREATE Create(mExtents);
		Create.mRenderer = CreateNullRenderer();
		const PintActorHandle H = CreateStaticObject(pint, &Create, Point(0.0f, 0.0f, 0.0f), &Q);

		return MidphaseSetup(pint, *this, caps, H);
	}

	void GetBox(OBB& box, Pint& pint, const MidphaseHits* MH)	const
	{
		const PintActorHandle Handle = MH->mQueryShapeHandle;
		const PR Pose = pint.GetWorldTransform(Handle);

		box.mCenter = Pose.mPos;
		box.mExtents = mExtents;
		box.mRot = Pose.mRot;
	}

	virtual	void	DrawDebugInfo(Pint& pint, PintRender& render)	override
	{
		MidphaseTestzoneTest::DrawDebugInfo(pint, render);

		const MidphaseHits* MH = reinterpret_cast<const MidphaseHits*>(pint.mUserData);
		if(MH)
		{
			OBB Box;
			GetBox(Box, pint, MH);
			render.DrawWireframeOBB(Box, Point(1.0f, 1.0f, 1.0f));
		}
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		MidphaseHits* MH = reinterpret_cast<MidphaseHits*>(pint.mUserData);
		const PintActorHandle MeshHandle = MH->mMeshHandle;
		ASSERT(MeshHandle);

		PintBoxOverlapData Data;
		GetBox(Data.mBox, pint, MH);

		MH->mStream.Reset();
		return pint.FindTriangles_MeshBoxOverlap(pint.mSQHelper->GetThreadContext(), MeshHandle, 1, &MH->mHits, MH->mStream, &Data);
	}

END_TEST(BoxOverlapTriangles_TestZone)

///////////////////////////////////////////////////////////////////////////////

// TODO: refactor this one like the above two

static const char* gDesc_CapsuleOverlapTriangles_TestZone = "MIDPHASE: TestZone. Capsule overlap triangles.";

class CapsuleOverlapTriangles_TestZone : public MidphaseTestzoneTest
{
	public:
							CapsuleOverlapTriangles_TestZone()	{													}
	virtual					~CapsuleOverlapTriangles_TestZone()	{													}
	virtual	const char*		GetName()					const	{ return "CapsuleOverlapTriangles_TestZone";		}
	virtual	const char*		GetDescription()			const	{ return gDesc_CapsuleOverlapTriangles_TestZone;	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCapsuleOverlaps)
			return false;

		return MidphaseSetup(pint, *this, caps, null);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		MidphaseHits* MH = reinterpret_cast<MidphaseHits*>(pint.mUserData);
		const PintActorHandle MeshHandle = MH->mMeshHandle;
		ASSERT(MeshHandle);

		PintCapsuleOverlapData Data;
		Data.mCapsule.mP0 = Point(0.0f, 0.0f, 0.0f);
		Data.mCapsule.mP1 = Point(10.0f, 10.0f, 10.0f);
		Data.mCapsule.mRadius = 40.0f;

		MH->mStream.Reset();
		return pint.FindTriangles_MeshCapsuleOverlap(pint.mSQHelper->GetThreadContext(), MeshHandle, 1, &MH->mHits, MH->mStream, &Data);
	}

END_TEST(CapsuleOverlapTriangles_TestZone)

///////////////////////////////////////////////////////////////////////////////

