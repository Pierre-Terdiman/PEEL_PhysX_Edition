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
#include "MyConvex.h"
#include "Loader_Bin.h"

///////////////////////////////////////////////////////////////////////////////

#define START_CCD_TEST(name, desc)														\
	class name : public TestBase														\
	{																					\
		public:																			\
								name()						{						}	\
		virtual					~name()						{						}	\
		virtual	const char*		GetName()			const	{ return #name;			}	\
		virtual	const char*		GetDescription()	const	{ return desc;			}	\
		virtual	TestCategory	GetCategory()		const	{ return CATEGORY_CCD;	}	\
		virtual	IceTabControl*	InitUI(PintGUIHelper& helper)							\
		{																				\
			return CreateOverrideTabControl("CCD Test", 200);							\
		}

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCDTest_DynamicBoxesVsStaticContainer = "CCD: 10 dynamic boxes (1.0; 1.0; 1.0) moving with linear velocity 400 against thin static boxes.";

START_CCD_TEST(CCDTest_DynamicBoxesVsStaticContainer, gDesc_CCDTest_DynamicBoxesVsStaticContainer)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(22.80f, 27.34f, 23.90f), Point(-0.61f, -0.51f, -0.61f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float BoxHeight = 4.0f;
		const float BoxSide = 0.01f;
		const float BoxDepth = 10.0f;

		CreateBoxContainer(pint, BoxHeight, BoxSide, BoxDepth);

		const Point Extents(1.0f, 1.0f, 1.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		udword NbFastBoxes = 10;
		for(udword i=0;i<NbFastBoxes;i++)
		{
			const float Angle = float(i)*TWOPI/float(NbFastBoxes);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 1.0f;
			ObjectDesc.mPosition		= Point(cosf(Angle)*5.0f, 5.0f, sinf(Angle)*5.0f);
			ObjectDesc.mLinearVelocity	= 400.0f * Point(ObjectDesc.mPosition.x, 0.0f, ObjectDesc.mPosition.z).Normalize();
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(CCDTest_DynamicBoxesVsStaticContainer)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCDTest_DynamicBoxesVsStaticContainer2 = "CCD: 30 dynamic boxes (0.4; 0.4; 0.4) moving with linear velocity 200 against thin static boxes.";

START_CCD_TEST(CCDTest_DynamicBoxesVsStaticContainer2, gDesc_CCDTest_DynamicBoxesVsStaticContainer2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(20.37f, 25.77f, 19.78f), Point(-0.51f, -0.69f, -0.51f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float BoxHeight = 4.0f;
		const float BoxSide = 0.01f;
		const float BoxDepth = 10.0f;

		CreateBoxContainer(pint, BoxHeight, BoxSide, BoxDepth);

		const Point Extents(0.4f, 0.4f, 0.4f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		udword NbFastBoxes = 30;
		for(udword i=0;i<NbFastBoxes;i++)
		{
			const float Angle = float(i)*TWOPI/float(NbFastBoxes);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 1.0f;
			ObjectDesc.mPosition		= Point(cosf(Angle)*5.0f, 5.0f, sinf(Angle)*5.0f);
			ObjectDesc.mLinearVelocity	= 200.0f * Point(ObjectDesc.mPosition.x, 0.0f, ObjectDesc.mPosition.z).Normalize();
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(CCDTest_DynamicBoxesVsStaticContainer2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCDTest_ThinRods = "CCD: 40 thin dynamic rods (0.04; 4.0; 0.04) falling with linear velocity 4 against a thin static planar mesh. The size and shape of these objects make the scene challenging even without CCD. \
In the PhysX family only PhysX5 handles it correctly, after enabling gyroscopic forces. Otherwise the rods don't go through the plane but their motion remains unnatural.";

START_CCD_TEST(CCDTest_ThinRods, gDesc_CCDTest_ThinRods)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(25.15f, 0.10f, 23.19f), Point(-0.73f, -0.12f, -0.67f));
		desc.mCamera[1] = PintCameraPose(Point(20.96f, 11.22f, 22.69f), Point(-0.65f, -0.31f, -0.70f));
		SetDefEnv(desc, false);
	}

	virtual	bool		CommonSetup()
	{
		mPlanarMeshHelper.Generate(32, 0.1f);
		return TestBase::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes || !caps.mSupportRigidBodySimulation)
			return false;

		mPlanarMeshHelper.CreatePlanarMesh(pint, 0.0f, null);

	//	const Point Extents(0.01f, 4.0f, 0.01f);
		const Point Extents(0.04f, 4.0f, 0.04f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		udword NbFastBoxes = 40;
		for(udword i=0;i<NbFastBoxes;i++)
		{
			const float Angle = float(i)*TWOPI/float(NbFastBoxes);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 1.0f;
			ObjectDesc.mPosition		= Point(cosf(Angle)*5.0f, 10.0f, sinf(Angle)*5.0f);
			ObjectDesc.mLinearVelocity	= 4.0f * Point(ObjectDesc.mPosition.x, 0.0f, ObjectDesc.mPosition.z).Normalize();
			CreatePintObject(pint, ObjectDesc);
		}

		return true;
	}

END_TEST(CCDTest_ThinRods)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCDTest_GameLevel = "CCD: 32*32 dynamic convexes falling with linear velocity 100 against a static mesh level.";

START_CCD_TEST(CCDTest_GameLevelStressTest, gDesc_CCDTest_GameLevel)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(42.22f, 50.00f, 42.77f), Point(0.66f, -0.50f, 0.55f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "Archipelago.bin");
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

		Point Offset, Extents;
		GetGlobalBounds(Offset, Extents);

		MyConvex C;
	//	udword i=2;	// Small convexes
		udword i=4;	// 'Big' convexes
	//	udword i=7;
	//	udword i=13;
		C.LoadFile(i);

		PINT_CONVEX_CREATE ConvexCreate(C.mNbVerts, C.mVerts);
		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

		const udword NbX = 32;
		const udword NbY = 32;
		const float AltitudeC = 10.0f;
		const Point LinVel(0.0f, -100.0f, 0.0f);
		return CreateArrayOfDynamicConvexes(pint, ConvexCreate, NbX, NbY, AltitudeC, Extents.x-8.0f, Extents.z-8.0f, &Offset, &LinVel);
	}

END_TEST(CCDTest_GameLevelStressTest)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCDTest_SmallConvexes = "CCD: 32*32 dynamic convexes (of low complexity) moving down with linear velocity 100 against on a tessellated planar mesh.";

START_CCD_TEST(CCDTest_SmallConvexes, gDesc_CCDTest_SmallConvexes)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(87.10f, 15.01f, 23.17f), Point(-0.93f, -0.29f, -0.24f));
		SetDefEnv(desc, false);
	}

	virtual	bool		CommonSetup()
	{
		mPlanarMeshHelper.Generate(32, 0.1f);
		return TestBase::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes || !caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		const float Altitude = 1.0f;
		mPlanarMeshHelper.CreatePlanarMesh(pint, Altitude, null);

		MyConvex C;
		udword i=2;	// Small convexes
	//	udword i=4;	// 'Big' convexes
	//	udword i=7;
	//	udword i=13;
		C.LoadFile(i);

		for(int i=0;i<C.mNbVerts;i++)
			C.mVerts[i] *= 0.1f;

		PINT_CONVEX_CREATE ConvexCreate(C.mNbVerts, C.mVerts);
		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

		const udword NbX = 32;
		const udword NbY = 32;
		const float AltitudeC = 20.0f;
		const float Scale = 35.0f;
		const Point LinVel(0.0f, -100.0f, 0.0f);
		return CreateArrayOfDynamicConvexes(pint, ConvexCreate, NbX, NbY, AltitudeC, Scale, Scale, null, &LinVel);
	}

END_TEST(CCDTest_SmallConvexes)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCDTest_BehaviorAfterImpact = "CCD: tests behavior of object after a CCD impact.";

START_CCD_TEST(CCDTest_BehaviorAfterImpact, gDesc_CCDTest_BehaviorAfterImpact)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-26.01f, 8.50f, 25.06f), Point(0.45f, -0.14f, -0.88f));
		SetDefEnv(desc, false);
	}

	virtual	bool		CommonSetup()
	{
		mPlanarMeshHelper.Generate(32, 0.1f);
		return TestBase::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes || !caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		const float Altitude = 1.0f;
		mPlanarMeshHelper.CreatePlanarMesh(pint, Altitude, null);

		MyConvex C;
		udword i=2;	// Small convexes
	//	udword i=4;	// 'Big' convexes
	//	udword i=7;
	//	udword i=13;
		C.LoadFile(i);

		for(int i=0;i<C.mNbVerts;i++)
			C.mVerts[i] *= 0.1f;

		PINT_CONVEX_CREATE ConvexCreate(C.mNbVerts, C.mVerts);
		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

		const udword NbX = 2;
		const udword NbY = 2;
		const float AltitudeC = 20.0f;
		const float Scale = 3.0f;
		const Point LinVel(-100.0f, -200.0f, 0.0f);
		return CreateArrayOfDynamicConvexes(pint, ConvexCreate, NbX, NbY, AltitudeC, Scale, Scale, null, &LinVel);
	}

END_TEST(CCDTest_BehaviorAfterImpact)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCDTest_BoxVsStack = "CCD: dynamic vs dynamic. A fast moving dynamic box is thrown against a stack of dynamic boxes.";

START_CCD_TEST(CCDTest_BoxVsStack, gDesc_CCDTest_BoxVsStack)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(34.25f, 19.94f, 51.78f), Point(-0.48f, 0.00f, -0.88f));
		desc.mCamera[1] = PintCameraPose(Point(-0.50f, 11.52f, 14.95f), Point(-0.01f, -0.04f, -1.00f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		CreateBoxStack(pint, caps, 1, 20);

		const Point Extents(1.0f, 1.0f, 1.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass			= 1.0f;
		ObjectDesc.mPosition		= Point(0.0f, 10.0f, 110.0f);
		ObjectDesc.mLinearVelocity	= Point(0.0f, 0.0f, -1500.0f);
		CreatePintObject(pint, ObjectDesc);
		return true;
	}

END_TEST(CCDTest_BoxVsStack)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCDTest_BoxVsBox = "CCD: dynamic boxes moving quickly towards each other.";

START_CCD_TEST(CCDTest_BoxVsBox, gDesc_CCDTest_BoxVsBox)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-19.86f, 15.54f, 0.77f), Point(0.96f, -0.27f, -0.05f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const Point Extents(1.0f, 1.0f, 1.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass			= 1.0f;

		for(udword i=0;i<100;i++)
		{
			ObjectDesc.mPosition		= Point(float(i)*2.5f, 10.0f, 110.0f+float(i)*0.5f);
			ObjectDesc.mLinearVelocity	= Point(0.0f, 0.0f, -1500.0f);
			CreatePintObject(pint, ObjectDesc);

			ObjectDesc.mPosition		= Point(float(i)*2.5f, 10.0f, -110.0f-float(i)*0.5f);
			ObjectDesc.mLinearVelocity	= Point(0.0f, 0.0f, 1500.0f);
			CreatePintObject(pint, ObjectDesc);
		}

		return true;
	}

END_TEST(CCDTest_BoxVsBox)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCDTest_AngularCCD = "CCD: dynamic vs dynamic. Test for angular CCD. This test shows the limits & side-effects of various \
CCD implementations in various engines. Support for 'angular CCD' (a.k.a. speculative contacts) is needed to make this test work properly.";

START_CCD_TEST(CCDTest_AngularCCD, gDesc_CCDTest_AngularCCD)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		desc.mCamera[0] = PintCameraPose(Point(5.70f, 13.69f, 24.39f), Point(-0.19f, -0.58f, -0.80f));
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		{
			const Point Extents(10.0f, 1.0f, 0.1f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer = CreateBoxRenderer(Extents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 1.0f;
			ObjectDesc.mPosition		= Point(0.0f, 0.0f, 0.0f);
			ObjectDesc.mAngularVelocity	= Point(0.0f, 10.0f, 0.0f);
			CreatePintObject(pint, ObjectDesc);
		}

		{
			const Point Extents(0.1f, 1.0f, 1.0f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer = CreateBoxRenderer(Extents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 1.0f;
			ObjectDesc.mPosition		= Point(0.0f, 0.0f, 10.0f);
			CreatePintObject(pint, ObjectDesc);
		}

		return true;
	}

END_TEST(CCDTest_AngularCCD)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCDTest_GhostContacts = "CCD: this test shows the limitation of speculative contacts. With regular CCD or raycast CCD \
the box is stopped by the static obstacle and does not interact with the objects behind it. With speculative contacts the box is still stopped by the wall, \
but with some implementations it still interacts with the objects behind.";

START_CCD_TEST(CCDTest_GhostContacts, gDesc_CCDTest_GhostContacts)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(61.91f, 20.29f, 0.49f), Point(-1.00f, -0.04f, 0.07f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		CreateBoxStack(pint, caps, 1, 20);

		// Create static obstacle between the fast moving box and the stack
		{
			const Point Extents(20.0f, 20.0f, 0.1f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mPosition	= Point(0.0f, Extents.y, 2.0f);
			CreatePintObject(pint, ObjectDesc);
		}

		// Create a fast moving box in such a way that it will land on the box stack without CCD
		{
			const Point Extents(1.0f, 1.0f, 1.0f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 1.0f;
			ObjectDesc.mPosition		= Point(0.0f, 10.0f, 110.0f+16.0f);
			ObjectDesc.mLinearVelocity	= Point(0.0f, 0.0f, -1500.0f);
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(CCDTest_GhostContacts)

///////////////////////////////////////////////////////////////////////////////

static bool CreateBoxStackWithHole(Pint& pint, const PintCaps& caps, const udword nb_stacks, udword nb_base_boxes)
{
	if(!caps.mSupportRigidBodySimulation)
		return false;

	const float BoxExtent = 1.0f;

	PINT_BOX_CREATE BoxDesc;
	BoxDesc.mExtents	= Point(BoxExtent, BoxExtent, BoxExtent);
	BoxDesc.mRenderer = CreateBoxRenderer(BoxDesc.mExtents);

	PINT_OBJECT_CREATE ObjectDesc;
	ObjectDesc.SetShape(&BoxDesc);
	ObjectDesc.mMass	= 1.0f;
//	ObjectDesc.mMass	= 0.5f;

	udword ID = 0;
	const udword NbStacks = nb_stacks;
	for(udword j=0;j<NbStacks;j++)
	{
		const float CoeffZ = NbStacks==1 ? 0.0f : float(j) - float(NbStacks)*0.5f;
		udword NbBoxes = nb_base_boxes;
		float BoxPosY = BoxExtent;
		while(NbBoxes)
		{
			for(udword i=0;i<NbBoxes;i++)
			{
		//		const float CoeffX = float(i)/float(NbBoxes-1);
				const float CoeffX = float(i) - float(NbBoxes)*0.5f;

		//		ObjectDesc.mPosition.x	= (CoeffX-0.5f) * BoxExtent * 2.0f;
				ObjectDesc.mPosition.x	= CoeffX * BoxExtent * 2.0f;
				ObjectDesc.mPosition.y	= BoxPosY;
				ObjectDesc.mPosition.z	= CoeffZ * BoxExtent * 4.0f;

				//ObjectDesc.mPosition.z += (UnitRandomFloat()-0.5f)*0.01f;

				bool CreateObject = true;
				if(ID==112 || ID==97 || ID==98 || ID==81 || ID==82|| ID==83)
					CreateObject = false;

				ObjectDesc.mName	= _F("%d", ID++);
				
				if(CreateObject)
				{
					const PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
					ASSERT(Handle);
				}
			}

			NbBoxes--;
			BoxPosY += BoxExtent*2.0f;
		}
	}
	return true;
}

static const char* gDesc_CCDTest_GhostContacts2 = "CCD: this test shows the limitation of speculative contacts. With regular CCD or raycast CCD \
the box goes through the hole in the stack. With speculative contacts the box can still interact with objects it should not touch, and the stack \
explodes.";

START_CCD_TEST(CCDTest_GhostContacts2, gDesc_CCDTest_GhostContacts2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(34.33f, 19.74f, 37.69f), Point(-0.65f, -0.09f, -0.76f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;
		
		CreateBoxStackWithHole(pint, caps, 1, 20);

		// Create a fast moving box that goes through the hole in the stack
		{
			const Point Extents(1.0f, 1.0f, 1.0f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 1.0f;
			ObjectDesc.mPosition		= Point(0.0f, 10.0f, 110.0f+16.0f);
			ObjectDesc.mLinearVelocity	= Point(0.0f, 0.0f, -1500.0f);
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(CCDTest_GhostContacts2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCDTest_ObjectVsComplexMesh = "CCD: a single dynamic convex thrown with linear velocity 1500 against a complex static mesh. This shows \
potential performance issues with speculative contacts when large velocities and complex triangle meshes are involved.";

static const udword gMaxCount = 4;
//static const udword gMaxCount = 2;
//static const udword gMaxCount = 1;

START_CCD_TEST(CCDTest_ObjectVsComplexMesh, gDesc_CCDTest_ObjectVsComplexMesh)

	udword	mCount;

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(45.24f, 27.75f, 55.62f), Point(-0.57f, -0.10f, -0.82f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "Bunny.bin", null, false, 3);

		mCount = gMaxCount;
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

		const Point Extents(1.0f, 1.0f, 1.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass			= 1.0f;
		ObjectDesc.mPosition		= Point(0.0f, 10.0f, 50.0f);
		ObjectDesc.mLinearVelocity	= Point(0.0f, 0.0f, -1500.0f);
		pint.mUserData = CreatePintObject(pint, ObjectDesc);
		return true;
	}

	virtual	void	CommonUpdate(float dt)
	{
		mCount--;
		if(!mCount)
			mCount = gMaxCount;
	}

	virtual	udword		Update(Pint& pint, float dt)
	{
		if(pint.mUserData && mCount==gMaxCount)
		{
			PR Pose(Idt);
			Pose.mPos	= Point(0.0f, 10.0f, 15.0f);

			const Point LinVel(0.0f, 0.0f, -1500.0f);

			const PintActorHandle handle = PintActorHandle(pint.mUserData);
			pint.SetWorldTransform(handle, Pose);
			pint.SetLinearVelocity(handle, LinVel);
		}

		return 0;
	}

END_TEST(CCDTest_ObjectVsComplexMesh)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCDTest_ConvexCascade = "CCD: dynamic vs dynamic stress test. Multiple convexes & thin boxes falling down from the sky. Try to pick up & manipulate pieces after the fall...";

START_CCD_TEST(CCDTest_ConvexCascade, gDesc_CCDTest_ConvexCascade)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportConvexes)
			return false;

		MyConvex C;
		C.LoadFile(1);

		PINT_CONVEX_CREATE ConvexCreate(C.mNbVerts, C.mVerts);
		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

		const Point Extents(10.0f, 0.1f, 10.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		for(udword i=0;i<100;i++)
		{
			PINT_OBJECT_CREATE ObjectDesc;
			if(i&1)
				ObjectDesc.SetShape(&ConvexCreate);
			else
				ObjectDesc.SetShape(&BoxDesc);
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mPosition	= Point(0.0f, float(i)*2.0f, 0.0f);
	//		if(!(i&1))
	//			ObjectDesc.mLinearVelocity	= Point(0.0f, -1000.0f, 0.0f);
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(CCDTest_ConvexCascade)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCDTest_ThinPyramidLayers = "CCD: dynamic vs dynamic. Pile of thin layers forming a pyramid. Try to pick up & manipulate pieces after the fall...";

START_CCD_TEST(CCDTest_ThinPyramidLayers, gDesc_CCDTest_ThinPyramidLayers)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(15.41f, 8.11f, 16.26f), Point(-0.64f, -0.42f, -0.65f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float SizeY = 0.01f;
		for(udword i=0;i<64;i++)
		{
			const float Size = 10.0f - float(i)*0.15f;
			const Point Extents(Size, SizeY, Size);

			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mPosition	= Point(0.0f, float(i)*SizeY*2.0f, 0.0f);
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(CCDTest_ThinPyramidLayers)

///////////////////////////////////////////////////////////////////////////////

/*static const char* gDesc_CCDTest_DynamicDynamic_PileOfThinBoxes2 = "CCD: dynamic vs dynamic. Pile of thin boxes. Try to pick up & manipulate pieces after the fall...";

START_CCD_TEST(CCDTest_DynamicDynamic_PileOfThinBoxes2, gDesc_CCDTest_DynamicDynamic_PileOfThinBoxes2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-1.12f, 1.52f, -4.10f), Point(0.28f, -0.31f, 0.91f));
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float BoxHeight = 0.01f;
		float y = BoxHeight;

		for(udword i=0;i<64;i++)
		{
			const float Coeff = 1.0f-(0.75f*float(i)/63.0f);

			const Point Extents(1.0f*Coeff, BoxHeight, 1.0f*Coeff);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer = CreateBoxRenderer(Extents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mPosition	= Point(0.0f, y, 0.0f);
			CreatePintObject(pint, ObjectDesc);

			y += BoxHeight*2.0f;
		}
		return true;
	}

END_TEST(CCDTest_DynamicDynamic_PileOfThinBoxes2)*/

///////////////////////////////////////////////////////////////////////////////
