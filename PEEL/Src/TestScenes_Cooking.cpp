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
#include "PintObjectsManager.h"

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ConvexHullFail = "Repro for misc convex hull cooking failures.";

START_TEST(MiscConvexHullFail, CATEGORY_COOKING, gDesc_ConvexHullFail)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-6.42f, 7.49f, 4.86f), Point(0.63f, -0.63f, -0.46f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		return TestBase::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes)
			return false;

		{
			const float Altitude = 0.0f;
			PINT_BOX_CREATE BoxDesc(100.0f, 0.05f, 100.0f);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mPosition.x	= 0.0f;
			ObjectDesc.mPosition.y	= Altitude;
			ObjectDesc.mPosition.z	= 0.0f;
			ObjectDesc.mMass		= 0.0f;
			CreatePintObject(pint, ObjectDesc);
		}

		{
			const char* filename = FindPEELFile("fail.bin");
			if(filename)
			{
				IceFile fp(filename);
				udword Length;
				const Point* pts = (const Point*)fp.Load(Length);
				const udword NbVerts = Length/sizeof(Point);

				PINT_CONVEX_CREATE Create(NbVerts, pts);
				Create.mRenderer	= CreateConvexRenderer(NbVerts, pts);

				CreateStaticObject(pint, &Create, Point(0.0f, 1.0f, 0.0f), null, "Convex");
			}
		}

		return true;
	}

END_TEST(MiscConvexHullFail)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ConvexCookingPerf = "Convex cooking perf test.";

START_TEST(ConvexCookingBenchmark, CATEGORY_COOKING, gDesc_ConvexCookingPerf)

	Vertices			mVerts;
	PintShapeRenderer*	mRenderer;

	virtual	udword	GetProfilingFlags()	const	{ return PROFILING_TEST_UPDATE;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-6.42f, 7.49f, 4.86f), Point(0.63f, -0.63f, -0.46f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		const char* filename = FindPEELFile("fail.bin");
		if(filename)
		{
			IceFile fp(filename);
			udword Length;
			const Point* pts = (const Point*)fp.Load(Length);
			const udword NbVerts = Length/sizeof(Point);
			mVerts.Add(reinterpret_cast<const udword*>(pts), NbVerts*3);
			mRenderer = CreateConvexRenderer(NbVerts, pts);
		}

		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		mVerts.Empty();

		TestBase::CommonRelease();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;
		return true;
	}

	virtual	udword	Update(Pint& pint, float dt)
	{
		const udword NbVerts = mVerts.GetNbVertices();
		if(NbVerts)
		{
			const Point* pts = mVerts.GetVertices();

			PINT_CONVEX_CREATE Create(NbVerts, pts);
			Create.mRenderer	= null;//mRenderer;
			Create.mSharing		= SHAPE_SHARING_NO;
			PintActorHandle h = CreateStaticObject(pint, &Create, Point(0.0f, 0.0f, 0.0f));

			pint._ReleaseObject(h);
		}
		return TestBase::Update(pint, dt);
	}

END_TEST(ConvexCookingBenchmark)

///////////////////////////////////////////////////////////////////////////////
