///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintShapeRenderer.h"
#include "TestScenes.h"
#include "TestScenesHelpers.h"
#include "PintSQ.h"
#include "SourceRay.h"
#include "Loader_RepX.h"
#include "PintObjectsManager.h"
#include "GUI_Helpers.h"
#include "Zcb2_RenderData.h"
#include "DefaultEnv.h"

static const bool gShowDisabledTests = true;

//bool gRaycastClosest = true;
udword gRaycastMode = 0;

///////////////////////////////////////////////////////////////////////////////

#define MAX_NB_TESTS	512
static udword gNbTests			= 0;	// Total #tests including disabled ones
static udword gNbAvailableTests = 0;	// Total #tests excluding disabled ones
static PhysicsTest*	gTests[MAX_NB_TESTS];

udword GetNbTests()				{ return gNbAvailableTests;							}
PhysicsTest* GetTest(udword i)	{ return i<gNbAvailableTests ? gTests[i] : null;	}

void InitTests()
{
	if(!gNbTests)
		return;

	udword* Keys = (udword*)ICE_ALLOC_TMP(sizeof(void*)*gNbTests);
	{
		gNbAvailableTests = 0;	// Number of tests visible to the UI
		for(udword i=0;i<gNbTests;i++)
		{
			const TestCategory CT = gTests[i]->GetCategory();
			Keys[i] = CT;
			if(CT!=CATEGORY_INTERNAL)	// Always hide internal tests
			{
				// Potentially show disabled tests when doing internal dev
				if(gShowDisabledTests || CT!=CATEGORY_DISABLED)
					gNbAvailableTests++;
			}
		}

		// Sort tests by category - put disabled ones last

		RadixSort RS;
		const udword* Ranks = RS.Sort(Keys, gNbTests, RADIX_UNSIGNED).GetRanks();

#ifdef _WIN64
		PhysicsTest** tmp = (PhysicsTest**)ICE_ALLOC(sizeof(PhysicsTest*)*gNbTests);
		CopyMemory(tmp, gTests, sizeof(PhysicsTest*)*gNbTests);

		for(udword i=0;i<gNbTests;i++)
			gTests[i] = tmp[Ranks[i]];

		ICE_FREE(tmp);
#else
		for(udword i=0;i<gNbTests;i++)
			Keys[i] = reinterpret_cast<udword>(gTests[i]);

		for(udword i=0;i<gNbTests;i++)
			gTests[i] = reinterpret_cast<PhysicsTest*>(Keys[Ranks[i]]);
#endif
	}
	ICE_FREE(Keys);
}

static void RegisterTest(PhysicsTest& test)
{
	ASSERT(gNbTests<MAX_NB_TESTS);
	if(gNbTests==MAX_NB_TESTS)
	{
		printf("Skipping test: %s\n", test.GetName());
		return;
	}
//	printf("Registering test: %s\n", test.GetName());
	gTests[gNbTests++] = &test;
}

///////////////////////////////////////////////////////////////////////////////

void PhysicsTest::CloseUI()
{
	mUIElems.Release();
}

void PhysicsTest::AddResetButton(IceWindow* parent, sdword x, sdword y, sdword width)
{
	ButtonDesc BD;
	BD.mID			= 0;
	BD.mParent		= parent;
	BD.mX			= x;
	BD.mY			= y;
	BD.mWidth		= width;
	BD.mHeight		= 20;
	BD.mLabel		= "Reset test";
	IceButton* IB = ICE_NEW(ResetButton)(*this, BD);
	RegisterUIElement(IB);
	IB->SetVisible(true);
}

///////////////////////////////////////////////////////////////////////////////

TestBase::PlanarMeshHelper::PlanarMeshHelper() :
	mSurface	(null),
	mRenderer	(null)
{
}

TestBase::PlanarMeshHelper::~PlanarMeshHelper()
{
	ASSERT(!mSurface);
}

void TestBase::PlanarMeshHelper::Release()
{
	DELETESINGLE(mSurface);
}

void TestBase::PlanarMeshHelper::Generate(udword nb_x, udword nb_z, const Point& scale, float sine_freq, float sine_amplitude)
{
	{
		IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
		bool status = IS->MakePlane(nb_x, nb_z);
		ASSERT(status);

		if(sine_amplitude!=0.0f)
		{
			const float Coeff = sine_freq;
			Point* V = IS->GetVerts();
			for(udword j=0;j<nb_z;j++)
			{
				for(udword i=0;i<nb_x;i++)
				{
					V[i+j*nb_x].y = sine_amplitude * sinf(float(i)*Coeff) * cosf(float(j)*Coeff);
				}
			}
		}

		IS->Scale(scale);
		IS->Flip();
		mSurface = IS;
	}

	// ### TODO: is the CRC needed here?
	mRenderer = CreateMeshRenderer(PintSurfaceInterface(mSurface->GetSurfaceInterface()));
}

void TestBase::PlanarMeshHelper::Generate(udword nb, float scale, float sine_freq, float sine_amplitude)
{
	Generate(nb, nb, Point(scale, 1.0f, scale), sine_freq, sine_amplitude);

/*	{
		IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
		bool status = IS->MakePlane(nb, nb);
		ASSERT(status);

		if(0)
		{
//			const float Coeff = 1.0f;
			const float Coeff = 0.5f;
			Point* V = IS->GetVerts();
			for(udword j=0;j<nb;j++)
			{
				for(udword i=0;i<nb;i++)
				{
					V[i+j*nb].y = 10.0f * sinf(float(i)*Coeff) * cosf(float(j)*Coeff);
				}
			}
		}

		IS->Scale(Point(scale, 1.0f, scale));
		IS->Flip();
		mSurface = IS;
	}

	mRenderer = CreateMeshRenderer(mSurface->GetSurfaceInterface());
//	mRenderer = CreateMeshRenderer(mSurface->GetSurfaceInterface(), true, true);*/
}

PintActorHandle TestBase::PlanarMeshHelper::CreatePlanarMesh(Pint& pint, float altitude, const PINT_MATERIAL_CREATE* material)
{
	PINT_MESH_CREATE MeshDesc;
	MeshDesc.SetSurfaceData(mSurface->GetSurfaceInterface());
	MeshDesc.mRenderer	= mRenderer;
	MeshDesc.mMaterial	= material;

	PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
	ObjectDesc.mPosition	= Point(0.0f, altitude, 0.0f);
	ObjectDesc.mMass		= 0.0f;
	return CreatePintObject(pint, ObjectDesc);
}

///////////////////////////////////////////////////////////////////////////////

TestBase::TestBase() :
	mHighFrictionMaterial	(1.0f, 1.0f, 0.0f),
	mZeroFrictionMaterial	(0.0f, 0.0f, 0.0f),
	mRepX					(null),
	mZB2Factory				(null),
	mCurrentTime			(0.0f)
{
	RegisterTest(*this);
}

TestBase::~TestBase()
{
}

void TestBase::GetSceneParams(PINT_WORLD_CREATE& params)
{
//	params.mGravity	= Point(0.0f, -9.81f, 0.0f);
//	params.mGravity	= UI_GetGravity();
}

bool TestBase::Init(Pint& pint, bool create_def_env)
{
	PintCaps Caps;
	pint.GetCaps(Caps);

	// We need to call the test setup before creating the default environment, to give
	// them a chance to properly create the collision filters before anything gets added
	// to the physics world (which is a requirement in some physics engines).
	if(!Setup(pint, Caps))
		return false;

	return SetupDefaultEnvironment(pint, create_def_env);
}

void TestBase::Close(Pint& pint)
{
}

void TestBase::RegisterAABB(const AABB& aabb)
{
	AABB* Box = ICE_RESERVE(AABB, mAABBs);

	*Box = aabb;
}

void TestBase::RenderAllAABBs(PintRender& renderer)
{
	const Point Color(0.0f, 1.0f, 0.0f);

	udword NbBoxes = GetNbAABBs();
	const AABB* Boxes = GetAABBs();
	renderer.DrawWireframeAABB(NbBoxes, Boxes, Color);
}

/////

void TestBase::RegisterRaycast(const Point& origin, const Point& dir, float max_dist)
{
	PintRaycastData* Data = ICE_RESERVE(PintRaycastData, mRaycastData);

	Data->mOrigin	= origin;
	Data->mDir		= dir;
	Data->mMaxDist	= max_dist;
}

udword TestBase::GetNbRegisteredRaycasts() const
{
	return mRaycastData.GetNbEntries()/(sizeof(PintRaycastData)/sizeof(udword));
}

PintRaycastData* TestBase::GetRegisteredRaycasts() const
{
	return (PintRaycastData*)mRaycastData.GetEntries();
}

/////

void TestBase::RegisterBoxSweep(const OBB& box, const Point& dir, float max_dist)
{
	PintBoxSweepData* Data = ICE_RESERVE(PintBoxSweepData, mBoxSweepData);

	Data->mBox		= box;
	Data->mDir		= dir;
	Data->mMaxDist	= max_dist;
}

udword TestBase::GetNbRegisteredBoxSweeps() const
{
	return mBoxSweepData.GetNbEntries()/(sizeof(PintBoxSweepData)/sizeof(udword));
}

PintBoxSweepData* TestBase::GetRegisteredBoxSweeps() const
{
	return (PintBoxSweepData*)mBoxSweepData.GetEntries();
}

/////

void TestBase::RegisterSphereSweep(const Sphere& sphere, const Point& dir, float max_dist)
{
	PintSphereSweepData* Data = ICE_RESERVE(PintSphereSweepData, mSphereSweepData);
	Data->mSphere	= sphere;
	Data->mDir		= dir;
	Data->mMaxDist	= max_dist;
}

udword TestBase::GetNbRegisteredSphereSweeps() const
{
	return mSphereSweepData.GetNbEntries()/(sizeof(PintSphereSweepData)/sizeof(udword));
}

PintSphereSweepData* TestBase::GetRegisteredSphereSweeps() const
{
	return (PintSphereSweepData*)mSphereSweepData.GetEntries();
}

/////

void TestBase::RegisterCapsuleSweep(const LSS& capsule, const Point& dir, float max_dist)
{
	PintCapsuleSweepData* Data = ICE_RESERVE(PintCapsuleSweepData, mCapsuleSweepData);

	Data->mCapsule	= capsule;
	Data->mDir		= dir;
	Data->mMaxDist	= max_dist;
}

udword TestBase::GetNbRegisteredCapsuleSweeps() const
{
	return mCapsuleSweepData.GetNbEntries()/(sizeof(PintCapsuleSweepData)/sizeof(udword));
}

PintCapsuleSweepData* TestBase::GetRegisteredCapsuleSweeps() const
{
	return (PintCapsuleSweepData*)mCapsuleSweepData.GetEntries();
}

/////

void TestBase::RegisterConvexSweep(const udword convex_object_index, PintShapeRenderer* renderer, const PR& pr, const Point& dir, float max_dist)
{
	PintConvexSweepData* Data = ICE_RESERVE(PintConvexSweepData, mConvexSweepData);

	Data->mRenderer				= renderer;
	Data->mConvexObjectIndex	= convex_object_index;
	Data->mTransform			= pr;
	Data->mDir					= dir;
	Data->mMaxDist				= max_dist;
}

udword TestBase::GetNbRegisteredConvexSweeps() const
{
	return mConvexSweepData.GetNbEntries()/(sizeof(PintConvexSweepData)/sizeof(udword));
}

PintConvexSweepData* TestBase::GetRegisteredConvexSweeps() const
{
	return (PintConvexSweepData*)mConvexSweepData.GetEntries();
}

/////

void TestBase::RegisterSphereOverlap(const Sphere& sphere)
{
	PintSphereOverlapData* Data = ICE_RESERVE(PintSphereOverlapData, mSphereOverlapData);

	Data->mSphere	= sphere;
}

udword TestBase::GetNbRegisteredSphereOverlaps() const
{
	return mSphereOverlapData.GetNbEntries()/(sizeof(PintSphereOverlapData)/sizeof(udword));
}

PintSphereOverlapData* TestBase::GetRegisteredSphereOverlaps() const
{
	return (PintSphereOverlapData*)mSphereOverlapData.GetEntries();
}

/////

void TestBase::RegisterBoxOverlap(const OBB& box)
{
	PintBoxOverlapData* Data = ICE_RESERVE(PintBoxOverlapData, mBoxOverlapData);

	Data->mBox	= box;
}

udword TestBase::GetNbRegisteredBoxOverlaps() const
{
	return mBoxOverlapData.GetNbEntries()/(sizeof(PintBoxOverlapData)/sizeof(udword));
}

PintBoxOverlapData* TestBase::GetRegisteredBoxOverlaps() const
{
	return (PintBoxOverlapData*)mBoxOverlapData.GetEntries();
}

/////

void TestBase::RegisterCapsuleOverlap(const LSS& capsule)
{
	PintCapsuleOverlapData* Data = ICE_RESERVE(PintCapsuleOverlapData, mCapsuleOverlapData);

	Data->mCapsule	= capsule;
}

udword TestBase::GetNbRegisteredCapsuleOverlaps() const
{
	return mCapsuleOverlapData.GetNbEntries()/(sizeof(PintCapsuleOverlapData)/sizeof(udword));
}

PintCapsuleOverlapData* TestBase::GetRegisteredCapsuleOverlaps() const
{
	return (PintCapsuleOverlapData*)mCapsuleOverlapData.GetEntries();
}

/////

void TestBase::RegisterConvexOverlap(const udword convex_object_index, PintShapeRenderer* renderer, const PR& pr)
{
	PintConvexOverlapData* Data = ICE_RESERVE(PintConvexOverlapData, mConvexOverlapData);

	Data->mRenderer				= renderer;
	Data->mConvexObjectIndex	= convex_object_index;
	Data->mTransform			= pr;
}

udword TestBase::GetNbRegisteredConvexOverlaps() const
{
	return mConvexOverlapData.GetNbEntries()/(sizeof(PintConvexOverlapData)/sizeof(udword));
}

PintConvexOverlapData* TestBase::GetRegisteredConvexOverlaps() const
{
	return (PintConvexOverlapData*)mConvexOverlapData.GetEntries();
}

/////

void TestBase::RegisterRenderer(PintShapeRenderer* renderer)
{
	mRenderers.AddPtr(renderer);
}

udword TestBase::GetNbRegisteredRenderers() const
{
	return mRenderers.GetNbEntries();
}

PintShapeRenderer** TestBase::GetRegisteredRenderers() const
{
	return (PintShapeRenderer**)mRenderers.GetEntries();
}

/////

void TestBase::CommonRelease()
{
//	ObjectsManager::Release();
	DELETESINGLE(mZB2Factory);

	if(mRepX)
	{
		ReleaseRepXContext(mRepX);
		mRepX = null;
	}

	mCurrentTime = 0.0f;
	mAABBs.Empty();
	ReleaseManagedSurfaces();
	mPlanarMeshHelper.Release();
	mRaycastData.Empty();
	mBoxSweepData.Empty();
	mSphereSweepData.Empty();
	mCapsuleSweepData.Empty();
	mConvexSweepData.Empty();
	mSphereOverlapData.Empty();
	mBoxOverlapData.Empty();
	mCapsuleOverlapData.Empty();
	mConvexOverlapData.Empty();
	mRenderers.Empty();
	mCameraManager.Release();

	DeleteOwnedObjects<TestData>(mTestData);
}

IceTabControl* TestBase::CreateOverrideTabControl(const char* name, udword extra_size)
{
	WindowDesc WD;
	WD.mParent	= null;
	WD.mX		= 50;
	WD.mY		= 50;
	WD.mWidth	= 300;
	WD.mHeight	= 200 + extra_size;
	WD.mLabel	= name;
	WD.mType	= WINDOW_DIALOG;
	IceWindow* UI = ICE_NEW(IceWindow)(WD);
	RegisterUIElement(UI);
	UI->SetVisible(true);

//	Widgets& UIElems = GetUIElements();

//	const sdword EditBoxWidth = 60;
	const sdword LabelWidth = 100;
//	const sdword OffsetX = LabelWidth + 10;
//	const sdword LabelOffsetY = 2;
	const sdword YStep = 20;
	sdword y = 0;
	y += YStep;
	AddResetButton(UI, 4, y, WD.mWidth - 16);

	IceTabControl* TabControl;
	{
		TabControlDesc TCD;
		TCD.mParent	= UI;
		TCD.mX		= 4;
		TCD.mY		= y + 30;
		TCD.mWidth	= WD.mWidth - 16;
		TCD.mHeight	= 120 + extra_size;
		TabControl = ICE_NEW(IceTabControl)(TCD);
		RegisterUIElement(TabControl);
	}
	return TabControl;
}

///////////////////////////////////////////////////////////////////////////////

void RegisterArrayOfRaycasts(TestBase& test, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_y, const Point& dir, float max_dist, const Point& offset)
{
	const float OneOverNbX = OneOverNb(nb_x);
	const float OneOverNbY = OneOverNb(nb_y);
	for(udword y=0;y<nb_y;y++)
	{
		const float CoeffY = 2.0f * ((float(y)*OneOverNbY) - 0.5f);
		for(udword x=0;x<nb_x;x++)
		{
			const float CoeffX = 2.0f * ((float(x)*OneOverNbX) - 0.5f);

			const Point Origin(CoeffX * scale_x, altitude, CoeffY * scale_y);

			test.RegisterRaycast(Origin + offset, dir, max_dist);
		}
	}
}

bool GenerateArrayOfVerticalRaycasts(TestBase& test, float scale, udword nb_x, udword nb_y, float max_dist)
{
	const float Altitude = 30.0f;
	const Point Dir(0.0f, -1.0f, 0.0f);
	const Point Offset(0.0f, 0.0f, 0.0f);
	RegisterArrayOfRaycasts(test, nb_x, nb_y, Altitude, scale, scale, Dir, max_dist, Offset);
	return true;
}

void RegisterArrayOfBoxSweeps(TestBase& test, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_y, const Point& dir, const Point& extents, const Point& offset, float max_dist)
{
	const float OneOverNbX = OneOverNb(nb_x);
	const float OneOverNbY = OneOverNb(nb_y);
	for(udword y=0;y<nb_y;y++)
	{
		const float CoeffY = 2.0f * ((float(y)*OneOverNbY) - 0.5f);
		for(udword x=0;x<nb_x;x++)
		{
			const float CoeffX = 2.0f * ((float(x)*OneOverNbX) - 0.5f);

			const Point Origin(CoeffX * scale_x, altitude, CoeffY * scale_y);

			const OBB Box(Origin + offset, extents, Get3x3IdentityMatrix());

			test.RegisterBoxSweep(Box, dir, max_dist);
		}
	}
}

bool GenerateArrayOfVerticalBoxSweeps(TestBase& test, float scale, udword nb_x, udword nb_y, float max_dist)
{
	const float Altitude = 30.0f;
	const Point Dir(0.0f, -1.0f, 0.0f);
	const Point Offset(0.0f, 0.0f, 0.0f);
	const Point Extents(1.2f, 0.5f, 0.5f);
	RegisterArrayOfBoxSweeps(test, nb_x, nb_y, Altitude, scale, scale, Dir, Extents, Offset, max_dist);
	return true;
}

void RegisterArrayOfSphereSweeps(TestBase& test, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_y, const Point& dir, float radius, const Point& offset, float max_dist)
{
	const float OneOverNbX = OneOverNb(nb_x);
	const float OneOverNbY = OneOverNb(nb_y);
	for(udword y=0;y<nb_y;y++)
	{
		const float CoeffY = 2.0f * ((float(y)*OneOverNbY) - 0.5f);
		for(udword x=0;x<nb_x;x++)
		{
			const float CoeffX = 2.0f * ((float(x)*OneOverNbX) - 0.5f);

			const Point Origin(CoeffX * scale_x, altitude, CoeffY * scale_y);

			test.RegisterSphereSweep(Sphere(Origin + offset, radius), dir, max_dist);
		}
	}
}

bool GenerateArrayOfVerticalSphereSweeps(TestBase& test, float scale, udword nb_x, udword nb_y, float max_dist)
{
	const float Altitude = 30.0f;
	const Point Dir(0.0f, -1.0f, 0.0f);
	const Point Offset(0.0f, 0.0f, 0.0f);
	const float SphereRadius = 0.75f;
	RegisterArrayOfSphereSweeps(test, nb_x, nb_y, Altitude, scale, scale, Dir, SphereRadius, Offset, max_dist);
	return true;
}

void RegisterArrayOfCapsuleSweeps(TestBase& test, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_y, const Point& dir, float radius, float half_height, const Point& offset, float max_dist)
{
	const float OneOverNbX = OneOverNb(nb_x);
	const float OneOverNbY = OneOverNb(nb_y);
	for(udword y=0;y<nb_y;y++)
	{
		const float CoeffY = 2.0f * ((float(y)*OneOverNbY) - 0.5f);
		for(udword x=0;x<nb_x;x++)
		{
			const float CoeffX = 2.0f * ((float(x)*OneOverNbX) - 0.5f);

			const Point Origin(offset.x + CoeffX * scale_x, offset.y + altitude, offset.z + CoeffY * scale_y);
			//const Point P0 = Origin + Point(half_height, 0.0f, 0.0f);
			//const Point P1 = Origin - Point(half_height, 0.0f, 0.0f);
			const Point P0 = Origin + Point(0.0f, half_height, 0.0f);
			const Point P1 = Origin - Point(0.0f, half_height, 0.0f);

			test.RegisterCapsuleSweep(LSS(Segment(P0, P1), radius), dir, max_dist);
		}
	}
}

bool GenerateArrayOfVerticalCapsuleSweeps(TestBase& test, float scale, udword nb_x, udword nb_y, float max_dist)
{
	const float Altitude = 30.0f;
	const Point Dir(0.0f, -1.0f, 0.0f);
	const Point Offset(0.0f, 0.0f, 0.0f);
//	const float CapsuleRadius = 0.4f;
//	const float HalfHeight = 1.8f;
	const float CapsuleRadius = 0.5f;
	const float HalfHeight = 0.5f;
	RegisterArrayOfCapsuleSweeps(test, nb_x, nb_y, Altitude, scale, scale, Dir, CapsuleRadius, HalfHeight, Offset, max_dist);
	return true;
}

void UpdateBoxSweeps(TestBase& test, float angle)
{
	//###TOFIX
//	Matrix3x3 Rot;
//	Rot.RotZ(angle);

		Matrix3x3 RotZ;
		RotZ.RotZ(0.3f);
	//	RotZ.RotZ(angle*0.1f);
		RotZ.RotZ(angle);

		Matrix3x3 RotX;
	//	RotX.RotX(angle);
		RotX.RotX(angle*0.77f);

	//	Matrix3x3 RotY;
	//	RotY.RotY(angle);

		Matrix3x3 Rot = RotZ * RotX;
	//	Matrix3x3 Rot = RotX * RotZ;
	//	Matrix3x3 Rot = RotY * RotZ;


	udword Nb = test.GetNbRegisteredBoxSweeps();
	PintBoxSweepData* Data = test.GetRegisteredBoxSweeps();
	while(Nb--)
	{
		Data->mBox.mRot = Rot;
		Data++;
	}
}

void UpdateCapsuleSweeps(TestBase& test, float angle)
{
	Matrix3x3 Rot;
	Rot.RotZ(angle);

	udword Nb = test.GetNbRegisteredCapsuleSweeps();
	PintCapsuleSweepData* Data = test.GetRegisteredCapsuleSweeps();
	while(Nb--)
	{
		const Point Center = (Data->mCapsule.mP0 + Data->mCapsule.mP1)*0.5f;
		const float HalfHeight = Data->mCapsule.mP0.Distance(Data->mCapsule.mP1)*0.5f;
//		const Point Local0 = Data->mCapsule.mP0 - Center;
//		const Point Local1 = Data->mCapsule.mP1 - Center;
		const Point Local0(HalfHeight, 0.0f, 0.0f);
		const Point Local1(-HalfHeight, 0.0f, 0.0f);
		const Point NewLocal0 = Local0 * Rot;
		const Point NewLocal1 = Local1 * Rot;
		Data->mCapsule.mP0 = NewLocal0 + Center;
		Data->mCapsule.mP1 = NewLocal1 + Center;
		Data++;
	}
}

void UpdateConvexSweeps(TestBase& test, float angle)
{
		Matrix3x3 RotZ;
		RotZ.RotZ(0.3f);
	//	RotZ.RotZ(angle*0.1f);
		RotZ.RotZ(angle);

		Matrix3x3 RotX;
	//	RotX.RotX(angle);
		RotX.RotX(angle*0.77f);

	//	Matrix3x3 RotY;
	//	RotY.RotY(angle);

		Matrix3x3 Rot = RotZ * RotX;
	//	Matrix3x3 Rot = RotX * RotZ;
	//	Matrix3x3 Rot = RotY * RotZ;


	udword Nb = test.GetNbRegisteredConvexSweeps();
	PintConvexSweepData* Data = test.GetRegisteredConvexSweeps();
	while(Nb--)
	{
		Data->mTransform.mRot = Rot;
		Data++;
	}
}

udword DoBatchRaycasts(TestBase& test, Pint& pint)
{
	const udword Nb = test.GetNbRegisteredRaycasts();
	const PintRaycastData* Data = test.GetRegisteredRaycasts();

	udword NbHits=0;
//	if(gRaycastClosest)
	if(gRaycastMode==0)
	{
		// Raycast closest
		PintRaycastHit* Dest = pint.mSQHelper->PrepareRaycastQuery(Nb, Data);

		NbHits = pint.BatchRaycasts(pint.mSQHelper->GetThreadContext(), Nb, Dest, Data);

		if(0)	// Save results
		{
			static bool firstTime = true;
			if(firstTime)
			{
				firstTime = false;
				FILE* fp = fopen("c:\\results.bin", "wb");
				if(fp)
				{
					fwrite(Dest, sizeof(PintRaycastHit), Nb, fp);
					fclose(fp);
				}
			}
		}
	}
	else if(gRaycastMode==1)
	{
		// Raycast any
		PintBooleanHit* Dest = pint.mSQHelper->PrepareRaycastAnyQuery(Nb, Data);

		NbHits = pint.BatchRaycastAny(pint.mSQHelper->GetThreadContext(), Nb, Dest, Data);
	}
	else if(gRaycastMode==2)
	{
		// Raycast all
		PintMultipleHits* Dest = pint.mSQHelper->PrepareRaycastAllQuery(Nb, Data);

		NbHits = pint.BatchRaycastAll(pint.mSQHelper->GetThreadContext(), Nb, Dest, pint.mSQHelper->GetRaycastAllStream(), Data);
	}
	return NbHits;
}

udword DoBatchBoxSweeps(TestBase& test, Pint& pint)
{
	const udword Nb = test.GetNbRegisteredBoxSweeps();
	const PintBoxSweepData* Data = test.GetRegisteredBoxSweeps();

	PintRaycastHit* Dest = pint.mSQHelper->PrepareBoxSweepQuery(Nb, Data);

	return pint.BatchBoxSweeps(pint.mSQHelper->GetThreadContext(), Nb, Dest, Data);
}

udword DoBatchSphereSweeps(TestBase& test, Pint& pint)
{
	const udword Nb = test.GetNbRegisteredSphereSweeps();
	const PintSphereSweepData* Data = test.GetRegisteredSphereSweeps();

	PintRaycastHit* Dest = pint.mSQHelper->PrepareSphereSweepQuery(Nb, Data);

	return pint.BatchSphereSweeps(pint.mSQHelper->GetThreadContext(), Nb, Dest, Data);
}

udword DoBatchCapsuleSweeps(TestBase& test, Pint& pint)
{
	const udword Nb = test.GetNbRegisteredCapsuleSweeps();
	const PintCapsuleSweepData* Data = test.GetRegisteredCapsuleSweeps();

	PintRaycastHit* Dest = pint.mSQHelper->PrepareCapsuleSweepQuery(Nb, Data);

	return pint.BatchCapsuleSweeps(pint.mSQHelper->GetThreadContext(), Nb, Dest, Data);
}

udword DoBatchConvexSweeps(TestBase& test, Pint& pint)
{
	const udword Nb = test.GetNbRegisteredConvexSweeps();
	const PintConvexSweepData* Data = test.GetRegisteredConvexSweeps();

	PintRaycastHit* Dest = pint.mSQHelper->PrepareConvexSweepQuery(Nb, Data);

	return pint.BatchConvexSweeps(pint.mSQHelper->GetThreadContext(), Nb, Dest, Data);
}

udword DoBatchSphereOverlaps(TestBase& test, Pint& pint, BatchOverlapMode mode)
{
	const udword Nb = test.GetNbRegisteredSphereOverlaps();
	const PintSphereOverlapData* Data = test.GetRegisteredSphereOverlaps();

	if(mode==OVERLAP_ANY)
	{
		PintBooleanHit* Dest = pint.mSQHelper->PrepareSphereOverlapAnyQuery(Nb, Data);
		return pint.BatchSphereOverlapAny(pint.mSQHelper->GetThreadContext(), Nb, Dest, Data);
	}
	else if(mode==OVERLAP_OBJECTS)
	{
		PintMultipleHits* Dest = pint.mSQHelper->PrepareSphereOverlapObjectsQuery(Nb, Data);
		return pint.BatchSphereOverlapObjects(pint.mSQHelper->GetThreadContext(), Nb, Dest, pint.mSQHelper->GetSphereOverlapObjectsStream(), Data);
	}
	return 0;
}

udword DoBatchBoxOverlaps(TestBase& test, Pint& pint, BatchOverlapMode mode)
{
	const udword Nb = test.GetNbRegisteredBoxOverlaps();
	const PintBoxOverlapData* Data = test.GetRegisteredBoxOverlaps();

	if(mode==OVERLAP_ANY)
	{
		PintBooleanHit* Dest = pint.mSQHelper->PrepareBoxOverlapAnyQuery(Nb, Data);
		return pint.BatchBoxOverlapAny(pint.mSQHelper->GetThreadContext(), Nb, Dest, Data);
	}
	else if(mode==OVERLAP_OBJECTS)
	{
		PintMultipleHits* Dest = pint.mSQHelper->PrepareBoxOverlapObjectsQuery(Nb, Data);
		return pint.BatchBoxOverlapObjects(pint.mSQHelper->GetThreadContext(), Nb, Dest, pint.mSQHelper->GetBoxOverlapObjectsStream(), Data);
	}
	return 0;
}

udword DoBatchCapsuleOverlaps(TestBase& test, Pint& pint, BatchOverlapMode mode)
{
	const udword Nb = test.GetNbRegisteredCapsuleOverlaps();
	const PintCapsuleOverlapData* Data = test.GetRegisteredCapsuleOverlaps();

	if(mode==OVERLAP_ANY)
	{
		PintBooleanHit* Dest = pint.mSQHelper->PrepareCapsuleOverlapAnyQuery(Nb, Data);
		return pint.BatchCapsuleOverlapAny(pint.mSQHelper->GetThreadContext(), Nb, Dest, Data);
	}
	else if(mode==OVERLAP_OBJECTS)
	{
		PintMultipleHits* Dest = pint.mSQHelper->PrepareCapsuleOverlapObjectsQuery(Nb, Data);
		return pint.BatchCapsuleOverlapObjects(pint.mSQHelper->GetThreadContext(), Nb, Dest, pint.mSQHelper->GetCapsuleOverlapObjectsStream(), Data);
	}
	return 0;
}

udword DoBatchConvexOverlaps(TestBase& test, Pint& pint, BatchOverlapMode mode)
{
	const udword Nb = test.GetNbRegisteredConvexOverlaps();
	const PintConvexOverlapData* Data = test.GetRegisteredConvexOverlaps();

	if(mode==OVERLAP_ANY)
	{
		PintBooleanHit* Dest = pint.mSQHelper->PrepareConvexOverlapAnyQuery(Nb, Data);
		return pint.BatchConvexOverlapAny(pint.mSQHelper->GetThreadContext(), Nb, Dest, Data);
	}
	else if(mode==OVERLAP_OBJECTS)
	{
		PintMultipleHits* Dest = pint.mSQHelper->PrepareConvexOverlapObjectsQuery(Nb, Data);
		return pint.BatchConvexOverlapObjects(pint.mSQHelper->GetThreadContext(), Nb, Dest, pint.mSQHelper->GetConvexOverlapObjectsStream(), Data);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static bool LoadRays(const char* filename, TestBase& test, bool only_rays, bool no_processing)
{
	IceFile BinFile(filename);
	if(!BinFile.IsValid())
		return false;

	const udword NbRays = BinFile.LoadDword();

	const float Scale = gValveScale;
	udword NbIsRays = 0;
	udword NbIsSwept = 0;
	udword NbAligned = 0;
	udword NbAlignedUp = 0;
	udword NbAlignedDown = 0;
//	FILE* fp = fopen("d:\\rays.txt", "w");

	const Matrix3x3 Rot(Idt);

#ifdef VALVE_ROTATE45
	Matrix3x3 Rot;
	Rot.RotX(45.0f * DEGTORAD);
#endif

	if(no_processing)
	{
		for(udword i=0;i<NbRays;i++)
		{
			Source1_Ray_t RayData;
			BinFile.LoadBuffer(&RayData, sizeof(Source1_Ray_t));

			const Point Origin(	RayData.m_Start.x+RayData.m_StartOffset.x,
								RayData.m_Start.y+RayData.m_StartOffset.y,
								RayData.m_Start.z+RayData.m_StartOffset.z);
			Point Dir(RayData.m_Delta.x, RayData.m_Delta.y, RayData.m_Delta.z);

			const float MaxDist = Dir.Magnitude();
			if(MaxDist!=0.0f)
			{
	//			fprintf(fp, "%f\n", MaxDist*Scale);
				Dir/=MaxDist;

				if(only_rays || RayData.m_IsRay)
				{
					test.RegisterRaycast(Origin, Dir, MaxDist);
				}
				else
				{
					const Point Extents(RayData.m_Extents.x, RayData.m_Extents.y, RayData.m_Extents.z);
					const OBB Box(Origin, Extents, Idt);
					test.RegisterBoxSweep(Box, Dir, MaxDist*Scale);
				}
			}
		}
	}
	else
	{
		for(udword i=0;i<NbRays;i++)
		{
			Source1_Ray_t RayData;
			BinFile.LoadBuffer(&RayData, sizeof(Source1_Ray_t));

			if(RayData.m_IsRay)
				NbIsRays++;
			if(RayData.m_IsSwept)
				NbIsSwept++;

			const Point Origin(	(RayData.m_Start.x+RayData.m_StartOffset.x)*Scale,
								(RayData.m_Start.z+RayData.m_StartOffset.z)*Scale,
								(RayData.m_Start.y+RayData.m_StartOffset.y)*Scale);
			Point Dir(RayData.m_Delta.x, RayData.m_Delta.z, RayData.m_Delta.y);

			const float MaxDist = Dir.Magnitude();
			if(MaxDist!=0.0f)
			{
	//			fprintf(fp, "%f\n", MaxDist*Scale);
				Dir/=MaxDist;

				if(Dir.x==0.0f && Dir.z==0.0f)
				{
					NbAligned++;
					if(Dir.y>0.0f)
						NbAlignedUp++;
					else
						NbAlignedDown++;
				}

				if(only_rays || RayData.m_IsRay)
				{
#ifdef VALVE_ROTATE45
					test.RegisterRaycast(Origin*Rot, Dir*Rot, MaxDist*Scale);
#else
					test.RegisterRaycast(Origin, Dir, MaxDist*Scale);
#endif
				}
				else
				{
					const Point Extents(RayData.m_Extents.x*Scale, RayData.m_Extents.z*Scale, RayData.m_Extents.y*Scale);
					const OBB Box(Origin, Extents, Idt);
					test.RegisterBoxSweep(Box, Dir, MaxDist*Scale);
				}
			}
		}
	}
//	fclose(fp);
	return true;
}

void LoadRaysFile(TestBase& test, const char* filename, bool only_rays, bool no_processing)
{
	ASSERT(filename);

	const char* File = FindPEELFile(filename);
	if(!File || !LoadRays(File, test, only_rays, no_processing))
		printf(_F("Failed to load '%s'\n", filename));

//	if(!LoadRays(_F("../build/%s", filename), test, only_rays, no_processing))
//		if(!LoadRays(_F("./%s", filename), test, only_rays, no_processing))
//			printf(_F("Failed to load '%s'\n", filename));
}

///////////////////////////////////////////////////////////////////////////////
