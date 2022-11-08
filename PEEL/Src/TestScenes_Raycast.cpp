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
#include "PintObjectsManager.h"
#include "Loader_RepX.h"
#include "Loader_Bin.h"
#include "GUI_Helpers.h"
#include "MyConvex.h"

///////////////////////////////////////////////////////////////////////////////

#define START_SQ_RAYCAST_TEST_VS_MESH(name, category, desc)												\
	class name : public TestBase																		\
	{																									\
		public:																							\
								name()						{						}					\
		virtual					~name()						{						}					\
		virtual	const char*		GetName()			const	{ return #name;			}					\
		virtual	const char*		GetDescription()	const	{ return desc;			}					\
		virtual	TestCategory	GetCategory()		const	{ return category;		}					\
		virtual	udword			GetProfilingFlags()	const	{ return PROFILING_TEST_UPDATE;			}	\
		virtual	udword			Update(Pint& pint, float dt){ return DoBatchRaycasts(*this, pint);	}	\
		virtual	bool			Setup(Pint& pint, const PintCaps& caps)									\
		{																								\
			if(!caps.mSupportRaycasts)																	\
				return false;																			\
			return CreateMeshesFromRegisteredSurfaces(pint, caps);										\
		}

///////////////////////////////////////////////////////////////////////////////

	class SceneRaycastVsShapes : public TestBase
	{
		public:
								SceneRaycastVsShapes()			{ mShapeType = PINT_SHAPE_UNDEFINED;	mConvexIndex = CONVEX_INDEX_4;	mSize = 32;	mDynamic = false;	}
		virtual					~SceneRaycastVsShapes()			{										}
		virtual	TestCategory	GetCategory()			const	{ return CATEGORY_RAYCAST;				}
		virtual	udword			GetProfilingFlags()		const	{ return PROFILING_TEST_UPDATE;			}
		virtual	udword			Update(Pint& pint, float dt)	{ return DoBatchRaycasts(*this, pint);	}

		virtual void			GetSceneParams(PINT_WORLD_CREATE& desc)
		{
			TestBase::GetSceneParams(desc);
			desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.59f, -0.55f, -0.59f));
			desc.mCamera[1] = PintCameraPose(Point(37.45f, 11.64f, 32.89f), Point(-0.81f, -0.40f, 0.44f));
			SetDefEnv(desc, false);
		}

		virtual bool			CommonSetup()
		{
			TestBase::CommonSetup();
			return GenerateArrayOfVerticalRaycasts(*this, float(mSize)+3.0f, mSize, mSize, gSQMaxDist);
		}

		virtual bool			Setup(Pint& pint, const PintCaps& caps)
		{
			if(!caps.mSupportRaycasts)
				return false;

			if(mShapeType==PINT_SHAPE_CYLINDER || mShapeType==PINT_SHAPE_MESH)
				return false;

			const udword NbX = mSize;
			const udword NbY = mSize;
			const float Altitude = 10.0f;
			const float Scale = float(mSize)+3.0f;
			const float Mass = mDynamic ? 1.0f : 0.0f;

			return GenerateArrayOfObjects(pint, caps, mShapeType, mConvexIndex, NbX, NbY, Altitude, Scale, Mass);
		}
				PintShape		mShapeType;
				ConvexIndex		mConvexIndex;
				udword			mSize;
				bool			mDynamic;
	};

static const char* gDesc_SceneRaycastVsShapes = "A grid of raycasts against a grid of shapes. Select undefined shape to create an empty scene and measure the operating overhead of raycasts.";

class ConfigurableSceneRaycastVsShapes : public SceneRaycastVsShapes
{
			ComboBoxPtr		mComboBox_ConvexIndex;
			ComboBoxPtr		mComboBox_ShapeType;
			EditBoxPtr		mEditBox_Size;
			CheckBoxPtr		mCheckBox_Dynamic;
	public:
							ConfigurableSceneRaycastVsShapes()		{							}
	virtual					~ConfigurableSceneRaycastVsShapes()		{							}
	virtual	const char*		GetName()			const	{ return "SceneRaycastVsShapes";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_SceneRaycastVsShapes;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 200;
		WD.mLabel	= "SceneRaycastVsShapes";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 100;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			mCheckBox_Dynamic = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Dynamic shapes", &UIElems, false, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Grid size:", &UIElems);
			mEditBox_Size = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "32", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Shape:", &UIElems);
			mComboBox_ShapeType = CreateShapeTypeComboBox(UI, 4+OffsetX, y, true, SSM_UNDEFINED|SSM_SPHERE|SSM_CAPSULE|SSM_BOX|SSM_CONVEX);
			RegisterUIElement(mComboBox_ShapeType);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Convex:", &UIElems);
			mComboBox_ConvexIndex = CreateConvexObjectComboBox(UI, 4+OffsetX, y, true);
			RegisterUIElement(mComboBox_ConvexIndex);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

		return null;
	}

	virtual bool	CommonSetup()
	{
		mDynamic = mCheckBox_Dynamic ? mCheckBox_Dynamic->IsChecked() : false;

		mSize = GetInt(mSize, mEditBox_Size);

		if(mComboBox_ShapeType)
			mShapeType = PintShape(mComboBox_ShapeType->GetSelectedIndex());

		if(mComboBox_ConvexIndex)
			mConvexIndex = ConvexIndex(mComboBox_ConvexIndex->GetSelectedIndex());

		return SceneRaycastVsShapes::CommonSetup();
	}

}ConfigurableSceneRaycastVsShapes;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_PotPourri_StaticRaycasts = "4096 random raycasts against 4096 random static objects (sphere, box or capsule).";

START_SQ_RAYCAST_TEST(PotPourri_StaticRaycasts, CATEGORY_RAYCAST, gDesc_PotPourri_StaticRaycasts)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		return Setup_PotPourri_Raycasts(*this, 4096, 100.0f);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return Setup_PotPourri_Raycasts(pint, caps, 0.0f, 16, 16, 16);
	}

END_TEST(PotPourri_StaticRaycasts)

static const char* gDesc_PotPourri_StaticRaycasts2 = "20000 random raycasts against 4096 random static objects (sphere, box or capsule).";

START_SQ_RAYCAST_TEST(PotPourri_StaticRaycasts2, CATEGORY_RAYCAST, gDesc_PotPourri_StaticRaycasts2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		return Setup_PotPourri_Raycasts(*this, 20000, 100.0f);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return Setup_PotPourri_Raycasts(pint, caps, 0.0f, 16, 16, 16);
	}

END_TEST(PotPourri_StaticRaycasts2)

static const char* gDesc_PotPourri_DynamicRaycasts = "4096 random raycasts against 4096 random dynamic objects (sphere, box or capsule).";

START_SQ_RAYCAST_TEST(PotPourri_DynamicRaycasts, CATEGORY_RAYCAST, gDesc_PotPourri_DynamicRaycasts)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		return Setup_PotPourri_Raycasts(*this, 4096, 1.0f);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return Setup_PotPourri_Raycasts(pint, caps, 1.0f, 16, 16, 16);
	}

END_TEST(PotPourri_DynamicRaycasts)

static const char* gDesc_PotPourri_DynamicRaycasts2 = "20000 random raycasts against 4096 random dynamic objects (sphere, box or capsule).";

START_SQ_RAYCAST_TEST(PotPourri_DynamicRaycasts2, CATEGORY_RAYCAST, gDesc_PotPourri_DynamicRaycasts2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		return Setup_PotPourri_Raycasts(*this, 20000, 1.0f);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return Setup_PotPourri_Raycasts(pint, caps, 1.0f, 16, 16, 16);
	}

END_TEST(PotPourri_DynamicRaycasts2)

///////////////////////////////////////////////////////////////////////////////

static bool LoadMeshSurfaceRays(const char* filename, TestBase& test, Vertices& origins)
{
	IceFile BinFile(filename);
	if(!BinFile.IsValid())
		return false;

	udword Length;
	const Point* Data = (const Point*)BinFile.Load(Length);
	for(udword i=0;i<Length/sizeof(Point);i++)
	{
		origins.AddVertex(Data[i]);
	}
	return true;
}

static void LoadMeshSurfaceRays(TestBase& test, const char* filename, Vertices& origins)
{
	ASSERT(filename);

	const char* File = FindPEELFile(filename);
	if(!File || !LoadMeshSurfaceRays(File, test, origins))
		printf(_F("Failed to load '%s'\n", filename));

//	if(!LoadMeshSurfaceRays(_F("../build/%s", filename), test, origins))
//		if(!LoadMeshSurfaceRays(_F("./%s", filename), test, origins))
//			printf(_F("Failed to load '%s'\n", filename));
}

static const char* gDesc_SceneRaycastVsStaticMeshes_MeshSurface = "Mesh surface Visual Test. 42500 rays.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_MeshSurface, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_MeshSurface)

	Vertices	mSurfaceRayOrigins;
	float		mAlpha;
	float		mPhi;
	float		mAlphaStep;
	float		mPhiStep;

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual	void	CommonRelease()
	{
		mSurfaceRayOrigins.Empty();
		TestBase::CommonRelease();
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "testzone.bin");
//		LoadMeshesFromFile_(*this, "testzone.bin", null, false, 1, TESS_POLYHEDRAL);

		mAlpha = mPhi = 0.0f;
		mAlphaStep = 0.001f;
		mPhiStep = 0.002f;

		LoadMeshSurfaceRays(*this, "SurfaceOrigins.bin", mSurfaceRayOrigins);

		const udword nRays = mSurfaceRayOrigins.GetNbVertices();
		//printf("%d rays\n", nRays);
		const Point* Origins = mSurfaceRayOrigins.GetVertices();
		BasicRandom Rnd(42);
		for(uword i=0;i<nRays;i++)
		{
			if(0)
			{
				RegisterRaycast(Origins[i], Point(0.0f, 0.0f, 0.0f), gSQMaxDist);
			}
			else
			{
				for(udword j=0;j<100;j++)
				{
					Point p; UnitRandomPt(p, Rnd); p*=0.1f;
					RegisterRaycast(Origins[i]+p, Point(0.0f, 0.0f, 0.0f), gSQMaxDist);
				}
			}
		}
		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);

		mAlpha += mAlphaStep;
		if (mAlpha > PI*2)
			mAlpha -= PI*2;
		mPhi += mPhiStep;
		if (mPhi > PI/4 || mPhi < -PI/4)
			mPhiStep = -mPhiStep;

		PintRaycastData* Rays = GetRegisteredRaycasts();
//		const udword nRays = mSurfaceRayOrigins.GetNbVertices();
		const udword nRays = GetNbRegisteredRaycasts();
		for(udword i=0; i<nRays; i++)
		{
			Point dir = Point(sinf(mAlpha), sinf(mPhi), cosf(mAlpha));
			dir.Normalize();
			Rays[i].mDir = dir;
		}
	}

END_TEST(SceneRaycastVsStaticMeshes_MeshSurface)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_TestZone_ShortRays = "TestZone. Short rays.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_TestZone_ShortRays, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_TestZone_ShortRays)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "testzone.bin");

//		return Setup_PotPourri_Raycasts(*this, 4096, 10.0f);

		if(!GetNbRegisteredSurfaces())
			return false;
		const SurfaceManager::SurfaceData* SD = GetSurfaceData(0);
		const IndexedSurface* IS = SD->mSurface;

		const Point* V = IS->GetVerts();
		const float Length = 0.1f;
		for(udword i=0;i<IS->GetNbFaces();i++)
		{
			const IndexedTriangle* T = IS->GetFace(i);

			Point Center, Normal;
			T->Center(V, Center);
			T->Normal(V, Normal);
			RegisterRaycast(Center+Normal*Length, -Normal, Length*2.0f);
		}
		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_TestZone_ShortRays)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_TestZone_LongRays = "TestZone. Long rays.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_TestZone_LongRays, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_TestZone_LongRays)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "testzone.bin");

//		return Setup_PotPourri_Raycasts(*this, 4096, 10.0f);

		if(!GetNbRegisteredSurfaces())
			return false;
		const SurfaceManager::SurfaceData* SD = GetSurfaceData(0);
		const IndexedSurface* IS = SD->mSurface;

		const Point* V = IS->GetVerts();
		const float Length = 0.1f;
		for(udword i=0;i<IS->GetNbFaces();i++)
		{
			const IndexedTriangle* T = IS->GetFace(i);

			Point Center, Normal;
			T->Center(V, Center);
			T->Normal(V, Normal);
//			RegisterRaycast(Center+Normal*Length, -Normal, Length*2.0f);
			RegisterRaycast(Center+Normal*Length, -Normal, 10000.0f);
//			RegisterRaycast(Center+Normal*Length, -Normal, MAX_FLOAT);
		}
		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_TestZone_LongRays)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_TestZone_VerticalRays = "TestZone. Vertical rays.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_TestZone_VerticalRays, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_TestZone_VerticalRays)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "testzone.bin");

//		return Setup_PotPourri_Raycasts(*this, 4096, 10.0f);

		Point Offset, Extents;
		GetGlobalBounds(Offset, Extents);

	//	const udword NbX = 32;
	//	const udword NbY = 32;
		const udword NbX = 128;
		const udword NbY = 128;
		const float Altitude = 30.0f;
		const float ScaleX = Extents.x - 1.0f;
		const float ScaleY = Extents.z - 1.0f;
		const Point Dir(0.0f, -1.0f, 0.0f);
		RegisterArrayOfRaycasts(*this, NbX, NbY, Altitude, ScaleX, ScaleY, Dir, gSQMaxDist, Offset);

		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_TestZone_VerticalRays)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_Archipelago1024 = "32*32 raycasts against the Archipelago mesh level.";

START_SQ_RAYCAST_TEST(SceneRaycastVsStaticMeshes_Archipelago1024, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_Archipelago1024)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "Archipelago.bin");

		Point Offset, Extents;
		GetGlobalBounds(Offset, Extents);

		const udword NbX = 32;
		const udword NbY = 32;
	//	const udword NbX = 128;
	//	const udword NbY = 128;
		const float Altitude = 30.0f;
		const float ScaleX = Extents.x - 1.0f;
		const float ScaleY = Extents.z - 1.0f;
		const Point Dir(0.0f, -1.0f, 0.0f);
		RegisterArrayOfRaycasts(*this, NbX, NbY, Altitude, ScaleX, ScaleY, Dir, gSQMaxDist, Offset);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRaycasts)
			return false;

		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_Archipelago1024)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_Archipelago16384 = "128*128 raycasts against the Archipelago mesh level.";

START_SQ_RAYCAST_TEST(SceneRaycastVsStaticMeshes_Archipelago16384, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_Archipelago16384)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "Archipelago.bin");

		Point Offset, Extents;
		GetGlobalBounds(Offset, Extents);

		const udword NbX = 128;
		const udword NbY = 128;
		const float Altitude = 30.0f;
		const float ScaleX = Extents.x - 1.0f;
		const float ScaleY = Extents.z - 1.0f;
		const Point Dir(0.0f, -1.0f, 0.0f);
		RegisterArrayOfRaycasts(*this, NbX, NbY, Altitude, ScaleX, ScaleY, Dir, gSQMaxDist, Offset);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRaycasts)
			return false;

		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_Archipelago16384)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_KP1024 = "1024 raycasts against the Konoko Payne mesh level.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_KP1024, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_KP1024)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "kp.bin");

		const Point Origin(50.0f, 50.0f, 50.0f);

		BasicRandom Rnd(42);
		for(udword i=0;i<1024;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);
			RegisterRaycast(Origin, Dir, gSQMaxDist);
		}
		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_KP1024)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_KP16384 = "16384 raycasts against the Konoko Payne mesh level.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_KP16384, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_KP16384)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "kp.bin");

		const Point Origin(50.0f, 50.0f, 50.0f);

		BasicRandom Rnd(42);
		for(udword i=0;i<16384;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);
			RegisterRaycast(Origin, Dir, gSQMaxDist);
		}
		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_KP16384)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_KP16384_2 = "16384 raycasts against the Konoko Payne mesh level (2).";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_KP16384_2, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_KP16384_2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "kp.bin");

		const Point Origin(0.0f, 1000.0f, 0.0f);

		BasicRandom Rnd(42);
		for(udword i=0;i<16384;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);
			RegisterRaycast(Origin, Dir, gSQMaxDist);
		}
		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_KP16384_2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_KP16384_NoHit = "16384 very short 'sensor' raycasts against the Konoko Payne mesh level. No hits.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_KP16384_NoHit, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_KP16384_NoHit)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "kp.bin");

		const float MaxDist = 10.0f;

		const Point Origin(50.0f, 50.0f, 50.0f);

		BasicRandom Rnd(42);
		for(udword i=0;i<16384;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);
			RegisterRaycast(Origin, Dir, MaxDist);
		}
		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_KP16384_NoHit)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_KP_Single = "Single long raycasts against the Konoko Payne mesh level.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_KP_Single, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_KP_Single)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		const Point Scale(0.1f, 0.1f, 0.1f);	// Should be similar to BenchMark_RaycastKP in PhysX
		LoadMeshesFromFile_(*this, "kp.bin", &Scale);

		BasicRandom Rnd(42);
		const Point Origin(34.7202f, 6.0501f, -2.9846f);
		for(udword i=0;i<1;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);
	//		Dir.x = fabsf(Dir.x);
	//		Dir.y = fabsf(Dir.y);
	//		Dir.z = fabsf(Dir.z);
			RegisterRaycast(Origin, Dir, 5000.0f);
		}
		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_KP_Single)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_KP2_Long = "2048 long raycasts against the Konoko Payne mesh level.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_KP2_Long, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_KP2_Long)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		const Point Scale(0.1f, 0.1f, 0.1f);	// Should be similar to BenchMark_RaycastKP in PhysX
		LoadMeshesFromFile_(*this, "kp.bin", &Scale);

		BasicRandom Rnd(42);
		const Point Origin(34.7202f, 6.0501f, -2.9846f);
		for(udword i=0;i<2048;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);
	//		Dir.x = fabsf(Dir.x);
	//		Dir.y = fabsf(Dir.y);
	//		Dir.z = fabsf(Dir.z);
			RegisterRaycast(Origin, Dir, 5000.0f);
		}
		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_KP2_Long)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_KP2_Short = "2048 short raycasts against the Konoko Payne mesh level.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_KP2_Short, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_KP2_Short)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		const Point Scale(0.1f, 0.1f, 0.1f);	// Should be similar to BenchMark_RaycastKP in PhysX
		LoadMeshesFromFile_(*this, "kp.bin", &Scale);

		BasicRandom Rnd(42);

		// This version uses very short raycasts from just before the impact point
		{
			udword FileSize;
			const char* Filename = GetFile("SceneRaycastVsStaticMeshes_KP2.bin", FileSize);
			if(FileSize)
			{
				ubyte* Buffer = (ubyte*)ICE_ALLOC_TMP(FileSize);

				FILE* fp = fopen(Filename, "rb");
				if(fp)
				{
					fread(Buffer, FileSize, 1, fp);
					fclose(fp);

					udword NbRays = FileSize/sizeof(PintRaycastHit);
					const PintRaycastHit* Hits = (const PintRaycastHit*)Buffer;
					for(udword i=0;i<NbRays;i++)
					{
						Point Dir;
						UnitRandomPt(Dir, Rnd);

						if(Hits[i].mTouchedActor)
							RegisterRaycast(Hits[i].mImpact - Dir*0.1f, Dir, 0.2f);
					}
				}
				ICE_FREE(Buffer);
			}
		}
		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_KP2_Short)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_Terrain_Long = "128*128 long raycasts against the tessellated terrain.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_Terrain_Long, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_Terrain_Long)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

	//	LoadMeshesFromFile_(*this, "Terrain.bin");
		LoadMeshesFromFile_(*this, "Terrain.bin", null, false, 1);

		const Point Dir(0.0f, -1.0f, 0.0f);
		// This version uses long raycasts from high above
		{
			Point Offset, Extents;
			GetGlobalBounds(Offset, Extents);

			const udword NbX = 128;
			const udword NbY = 128;
			const float Altitude = 500.0f;
			const float ScaleX = Extents.x - 1.0f;
			const float ScaleY = Extents.z - 1.0f;
			RegisterArrayOfRaycasts(*this, NbX, NbY, Altitude, ScaleX, ScaleY, Dir, gSQMaxDist, Offset);
		}
		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_Terrain_Long)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_Terrain_Short = "32*32 short raycasts against the tessellated terrain.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_Terrain_Short, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_Terrain_Short)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

	//	LoadMeshesFromFile_(*this, "Terrain.bin");
		LoadMeshesFromFile_(*this, "Terrain.bin", null, false, 1);

		const Point Dir(0.0f, -1.0f, 0.0f);
		// This version uses very short raycasts from just above the impact point
		{
			udword FileSize;
			const char* Filename = GetFile("SceneRaycastVsStaticMeshes_Terrain.bin", FileSize);
			if(FileSize)
			{
				ubyte* Buffer = (ubyte*)ICE_ALLOC_TMP(FileSize);

				FILE* fp = fopen(Filename, "rb");
				if(fp)
				{
					fread(Buffer, FileSize, 1, fp);
					fclose(fp);

					udword NbRays = FileSize/sizeof(PintRaycastHit);
					const PintRaycastHit* Hits = (const PintRaycastHit*)Buffer;
					for(udword i=0;i<NbRays;i++)
					{
						if(Hits[i].mTouchedActor)
							RegisterRaycast(Hits[i].mImpact - Dir, Dir, 2.0f);
					}
				}
				ICE_FREE(Buffer);
			}
		}
		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_Terrain_Short)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_TessBunny_NodeSorting = "Node sorting test. 2048 raycasts are fired from one of the 8 canonical directions in sequence. In theory the \
cost for each of the 8 directions should be roughly the same. In practice, engines without node sorting show a very high variation here. The direction changes every 32 frames.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_TessBunny_NodeSorting, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_TessBunny_NodeSorting)

	udword	mNbFrames;
	udword	mIndex;

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(24.69f, 19.88f, -5.46f), Point(-0.89f, -0.42f, 0.20f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		mNbFrames = 0;
		mIndex = 0;

		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "Bunny.bin", null, false, 3);

		for(udword i=0;i<2048;i++)
			RegisterRaycast(Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), 2000.0f);
		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		if(mNbFrames==0)
		{
			const udword Index = mIndex;
			Point Dir;
			if(Index==0)
				Dir = Point(1.0f, 1.0f, 1.0f);
			else if(Index==1)
				Dir = Point(1.0f, 1.0f, -1.0f);
			else if(Index==2)
				Dir = Point(1.0f, -1.0f, 1.0f);
			else if(Index==3)
				Dir = Point(1.0f, -1.0f, -1.0f);
			else if(Index==4)
				Dir = Point(-1.0f, 1.0f, 1.0f);
			else if(Index==5)
				Dir = Point(-1.0f, 1.0f, -1.0f);
			else if(Index==6)
				Dir = Point(-1.0f, -1.0f, 1.0f);
			else if(Index==7)
				Dir = Point(-1.0f, -1.0f, -1.0f);

			Dir.Normalize();

			const float Distance = 20.0f;

			Point Center, Extents;
			GetGlobalBounds(Center, Extents);

			udword Nb = GetNbRegisteredRaycasts();
			PintRaycastData* Data = GetRegisteredRaycasts();
			while(Nb--)
			{
				Data->mOrigin = Center+Dir*Distance;
				Data->mDir = -Dir;
				Data++;
			}

			mIndex++;
			if(mIndex==8)
				mIndex=0;
		}
		mNbFrames++;
		if(mNbFrames==32)
			mNbFrames=0;
	}

END_TEST(SceneRaycastVsStaticMeshes_TessBunny_NodeSorting)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_TessBunny16384 = "16384 radial raycasts against the tessellated bunny.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_TessBunny16384, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_TessBunny16384)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "Bunny.bin", null, false, 3);

		BasicRandom Rnd(42);

		Point Center, Extents;
		GetGlobalBounds(Center, Extents);

		for(udword i=0;i<16384;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);

			const Point Origin = Center + Dir * 20.0f;

			RegisterRaycast(Origin, -Dir, 20.0f);
		}
		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_TessBunny16384)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_TessBunny16384_2 = "16384 radial raycasts against the tessellated bunny. Same as before but with a larger ray length. In theory the \
ray length should not have an impact on performance. In practice however, some engines become much slower when the ray becomes larger.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_TessBunny16384_2, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_TessBunny16384_2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "Bunny.bin", null, false, 3);

		BasicRandom Rnd(42);

		Point Center, Extents;
		GetGlobalBounds(Center, Extents);

		for(udword i=0;i<16384;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);

			const Point Origin = Center + Dir * 20.0f;

			RegisterRaycast(Origin, -Dir, 20000.0f);
		}
		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_TessBunny16384_2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneRaycastVsStaticMeshes_TessBunnyShort = "Short raycasts against the tessellated bunny. There is one raycast per triangle. \
Each ray starts a little bit above the triangle. Ray direction is the opposite of the triangle normal.";

START_SQ_RAYCAST_TEST_VS_MESH(SceneRaycastVsStaticMeshes_TessBunnyShort, CATEGORY_RAYCAST, gDesc_SceneRaycastVsStaticMeshes_TessBunnyShort)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(19.98f, 16.48f, -0.94f), Point(-0.93f, -0.36f, 0.05f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "Bunny.bin", null, false, 2);

		if(!GetNbRegisteredSurfaces())
			return false;
		const SurfaceManager::SurfaceData* SD = GetSurfaceData(0);
		const IndexedSurface* IS = SD->mSurface;

		const Point* V = IS->GetVerts();
		const float Length = 0.1f;
		for(udword i=0;i<IS->GetNbFaces();i++)
		{
			const IndexedTriangle* T = IS->GetFace(i);

			Point Center, Normal;
			T->Center(V, Center);
			T->Normal(V, Normal);
			RegisterRaycast(Center+Normal*Length, -Normal, Length*2.0f);
		}
		return true;
	}

END_TEST(SceneRaycastVsStaticMeshes_TessBunnyShort)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_KP_RT = "Konoko Payne mesh level. Raytracing test.";

START_SQ_RAYCAST_TEST_VS_MESH(KP_RT, CATEGORY_RAYCAST, gDesc_KP_RT)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadRaysFile(*this, "default_peel_view_rays.bin", true, true);
		LoadMeshesFromFile_(*this, "kp.bin");
//		LoadMeshesFromFile_(*this, "kp.bin", null, false, 1, TESS_POLYHEDRAL);
		return true;
	}

END_TEST(KP_RT)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Bunny_RT = "Bunny. Raytracing test.";

START_SQ_RAYCAST_TEST_VS_MESH(Bunny_RT, CATEGORY_RAYCAST, gDesc_Bunny_RT)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadRaysFile(*this, "bunny_rays.bin", true, true);
//		LoadMeshesFromFile_(*this, "Bunny.bin", null, false, 2);
		LoadMeshesFromFile_(*this, "Bunny.bin");
		return true;
	}

END_TEST(Bunny_RT)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Terrain_RT = "Terrain tiles. Raytracing test.";

START_SQ_RAYCAST_TEST_VS_MESH(Terrain_RT, CATEGORY_RAYCAST, gDesc_Terrain_RT)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadRaysFile(*this, "default_peel_view_rays.bin", true, true);
		LoadMeshesFromFile_(*this, "Terrain.bin");
		return true;
	}

END_TEST(Terrain_RT)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_TestZone_RT = "TestZone (tess). Raytracing test.";

START_SQ_RAYCAST_TEST_VS_MESH(TestZone_RT, CATEGORY_RAYCAST, gDesc_TestZone_RT)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadRaysFile(*this, "testzone_rays_5.bin", true, true);
//		LoadMeshesFromFile_(*this, "testzone.bin");
		LoadMeshesFromFile_(*this, "testzone.bin", null, false, 1, TESS_POLYHEDRAL);
		return true;
	}

END_TEST(TestZone_RT)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_TestZone_RT2 = "TestZone (tess). Raytracing test 2.";

START_SQ_RAYCAST_TEST_VS_MESH(TestZone_RT2, CATEGORY_RAYCAST, gDesc_TestZone_RT2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadRaysFile(*this, "testzone_rays_4.bin", true, true);
//		LoadMeshesFromFile_(*this, "testzone.bin");
		LoadMeshesFromFile_(*this, "testzone.bin", null, false, 1, TESS_POLYHEDRAL);
		return true;
	}

END_TEST(TestZone_RT2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_TestZone_RT3 = "TestZone. Raytracing ride test.";

START_SQ_RAYCAST_TEST_VS_MESH(TestZone_RT3, CATEGORY_RAYCAST, gDesc_TestZone_RT3)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		mCameraManager.LoadCameraData("testzone_camera_data.bin");

//		LoadRaysFile(*this, "testzone_rays_2.bin", true, true);
		LoadMeshesFromFile_(*this, "testzone.bin");
//		LoadMeshesFromFile_(*this, "testzone.bin", null, false, 1, TESS_POLYHEDRAL);

		for(udword i=0;i<128*128;i++)
			RegisterRaycast(Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), 0.0f);

		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		mCameraManager.UpdateCameraPose();
		mCameraManager.GenerateRays(GetRegisteredRaycasts(), 128, 1000.0f);
	}

END_TEST(TestZone_RT3)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Terrain_RT2 = "Terrain. Raytracing ride test.";

START_SQ_RAYCAST_TEST_VS_MESH(Terrain_RT2, CATEGORY_RAYCAST, gDesc_Terrain_RT2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		mCameraManager.LoadCameraData("terrain_camera_data.bin");

		LoadMeshesFromFile_(*this, "terrain.bin");

		for(udword i=0;i<128*128;i++)
			RegisterRaycast(Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), 0.0f);

		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		mCameraManager.UpdateCameraPose();
		mCameraManager.GenerateRays(GetRegisteredRaycasts(), 128, 10000.0f);
	}

END_TEST(Terrain_RT2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_KP_RT2 = "KP. Raytracing ride test.";

START_SQ_RAYCAST_TEST_VS_MESH(KP_RT2, CATEGORY_RAYCAST, gDesc_KP_RT2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		mCameraManager.LoadCameraData("kp_camera_data.bin");

		LoadMeshesFromFile_(*this, "kp.bin");

		for(udword i=0;i<128*128;i++)
			RegisterRaycast(Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), 0.0f);

		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		mCameraManager.UpdateCameraPose();
		mCameraManager.GenerateRays(GetRegisteredRaycasts(), 128, 10000.0f);
	}

END_TEST(KP_RT2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_InsideRays_TestZone = "TestZone. Inside rays.";

START_SQ_RAYCAST_TEST_VS_MESH(InsideRays_TestZone, CATEGORY_RAYCAST, gDesc_InsideRays_TestZone)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "testzone.bin");

//		return Setup_PotPourri_Raycasts(*this, 4096, 10.0f);

		{
			// Shooting rays from inside the tower
			BasicRandom Rnd(42);
			for(udword i=0;i<4096;i++)
			{
				const float Coeff = TWOPI*float(i)/4096.0f;
				const float x = sinf(Coeff);
				const float z = cosf(Coeff);

//				Point D(x, 0.0f, z);
//				D.Normalize();
				Point D;
				UnitRandomPt(D, Rnd);

//				RegisterRaycast(Point(41.78f, 5.64f, -69.18f), D, 1000.0f);
				RegisterRaycast(Point(-15.08f, 5.20f, -58.01f), D, 1000.0f);
//				RegisterRaycast(Point(0.0f, 100.0f, 0.0f), Point(0.0f, 1.0f, 0.0f), 1000.0f);
			}
		}
		return true;
	}

END_TEST(InsideRays_TestZone)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OutsideRays_TestZone = "TestZone. Outside rays.";

START_SQ_RAYCAST_TEST_VS_MESH(OutsideRays_TestZone, CATEGORY_RAYCAST, gDesc_OutsideRays_TestZone)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "testzone.bin");

//		return Setup_PotPourri_Raycasts(*this, 4096, 10.0f);

		{
			// Radial rays from the center, roughly
			for(udword i=0;i<4096;i++)
			{
				const float Coeff = TWOPI*float(i)/4096.0f;
				const float x = sinf(Coeff);
				const float z = cosf(Coeff);

				Point D(x, 0.0f, z);
				D.Normalize();

				RegisterRaycast(Point(8.36f, -4.22f, -28.18f), D, 1000.0f);
//				RegisterRaycast(Point(8.36f, -4.22f, -28.18f), D, 2.0f);
			}
		}
		return true;
	}

END_TEST(OutsideRays_TestZone)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SpecularBulletRays = "Specular level. Bullet rays.";

START_SQ_RAYCAST_TEST(SpecularBulletRays, CATEGORY_RAYCAST, gDesc_SpecularBulletRays)

	virtual bool	IsPrivate()	const
	{
		return true;
	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
//		mRepX = CreateRepXContext("specular_raycast_meshes.repx", 1.0f, false);
		mRepX = CreateRepXContext("specular_raycast_meshes_statics.repx", 1.0f, false);

		if(1)
		{
			for(udword i=0;i<4096;i++)
			{
				const float Coeff = TWOPI*float(i)/4096.0f;
				const float x = sinf(Coeff);
				const float z = cosf(Coeff);

				Point D(x, 0.0f, z);
				D.Normalize();

				RegisterRaycast(Point(0.0f, 4.0f, 0.0f), D, 10000.0f);
			}
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRaycasts || !caps.mSupportMeshes)
			return false;
		return AddToPint(pint, mRepX);
	}

END_TEST(SpecularBulletRays)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ValveRaycasts = "Valve raycasts.";
class ValveRaycasts : public TestBase
{
			IceComboBox*	mComboBox_SceneIndex;
	public:
							ValveRaycasts() : mComboBox_SceneIndex(null)			{			}
	virtual					~ValveRaycasts()			{										}
	virtual	const char*		GetName()			const	{ return "ValveRaycasts";				}
	virtual	const char*		GetDescription()	const	{ return gDesc_ValveRaycasts;			}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_RAYCAST;				}
	virtual	udword			GetProfilingFlags()	const	{ return PROFILING_TEST_UPDATE;			}
	virtual bool			IsPrivate()			const	{ return true;							}
	virtual	udword			Update(Pint& pint, float dt){ return DoBatchRaycasts(*this, pint);	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 120;
		WD.mLabel	= "ValveRaycasts";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword OffsetX = 70;
		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 30;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			y += YStep;
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "File", &UIElems);
			{
				IceComboBox* CB = CreateComboBox<IceComboBox>(UI, 0, 4+OffsetX, y, 200, 20, "Scene index", &GetUIElements(), null);
				CB->Add("Valve level, raycasts 'only rays'.");
				CB->Add("Valve level, raycasts 'MidGame1'.");
				CB->Add("Valve level, raycasts 'MidGame2'.");
				CB->Add("Valve level, raycasts 'MidGame3'.");
				CB->Add("Valve level, raycasts 'MidGame4'.");
				CB->Select(0);
				mComboBox_SceneIndex = CB;
			}
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

		return null;
	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool			CommonSetup()
	{
		TestBase::CommonSetup();

		udword Index = 0;
		if(mComboBox_SceneIndex)
			Index = udword(mComboBox_SceneIndex->GetSelectedIndex());

		const char* Filename = null;
		if(Index==0)
			Filename = "rays(only rays).bin";
		else if(Index==1)
			Filename = "l4d2_rays(midgame1).bin";
		else if(Index==2)
			Filename = "l4d2_rays(midgame2).bin";
		else if(Index==3)
			Filename = "l4d2_rays(midgame3).bin";
		else if(Index==4)
			Filename = "l4d2_rays(midgame4).bin";

		LoadRaysFile(*this, Filename, true);
		mRepX = CreateRepXContext("c5m4_quarter2_Statics.repx", gValveScale, true);
//		LoadRepXFile_Obsolete(*this, "c5m4_quarter2_Statics.repx", gValveScale, true);
		return true;
	}

	virtual bool			Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes || !caps.mSupportRaycasts)
			return false;
		return AddToPint(pint, mRepX);
	}

}ValveRaycasts;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_PlanetsideBulletRays = "512*4 bullet rays against the Planetside level.";

START_SQ_RAYCAST_TEST(PlanetsideBulletRays, CATEGORY_RAYCAST, gDesc_PlanetsideBulletRays)

	virtual bool	IsPrivate()	const
	{
		return true;
	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(157.59f, 21.66f, 770.35f), Point(0.66f, -0.54f, 0.52f));
		desc.mCamera[1] = PintCameraPose(Point(248.93f, 12.60f, 676.81f), Point(-0.04f, -0.23f, 0.97f));
		desc.mCamera[2] = PintCameraPose(Point(275.19f, 32.54f, 703.23f), Point(-1.00f, -0.08f, 0.05f));
		desc.mCamera[3] = PintCameraPose(Point(295.76f, 4.08f, 845.31f), Point(-0.32f, 0.11f, -0.94f));
		SetDefEnv(desc, false);

		SRand(42);
		const Point Orig(241.09f, 6.29f, 730.26f);
		const Point Orig2(164.74f, 1.67f, 783.85f);
		const Point Orig3(275.19f, 32.54f, 703.23f);
		const Point Orig4(295.76f, 4.08f, 845.31f);
		for(udword j=0;j<512;j++)
		{
			const float angle = (float(j)/512.0f)*3.14159f*2.0f;
			const float s = sinf(angle);
			const float c = cosf(angle);
			Point Dir(s, 0.1f*(UnitRandomFloat()-0.5f), c);
			Dir.Normalize();
			RegisterRaycast(Orig, Dir, 5000.0f);
			RegisterRaycast(Orig2, Dir, 5000.0f);
			RegisterRaycast(Orig3, Dir, 5000.0f);
			RegisterRaycast(Orig4, Dir, 5000.0f);
		}
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		mRepX = CreateRepXContext("Planetside_Statics.repx", 1.0f, false);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes || !caps.mSupportRaycasts)
			return false;
		return AddToPint(pint, mRepX);
	}

/*	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		udword Nb = GetNbRegisteredRaycasts();
		PintRaycastData* Data = GetRegisteredRaycasts();
		while(Nb--)
		{
			Ray R(Data->mOrigin, Data->mDir);
			R.Perturb(10.0f);
			Data->mDir = R.mDir;
			Data++;
		}
	}*/

END_TEST(PlanetsideBulletRays)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_BlackRays = "Black rays (debug).";

START_SQ_RAYCAST_TEST_VS_MESH(BlackRays, CATEGORY_RAYCAST, gDesc_BlackRays)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadRaysFile(*this, "black_rays.bin", true, true);
		LoadMeshesFromFile_(*this, "Venus.bin", null, false, 0);
		return true;
	}

END_TEST(BlackRays)

///////////////////////////////////////////////////////////////////////////////

#include "TestScenes_Heightfield.h"

static const char* gDesc_SceneRaycastVsStaticHeightfield = "raycasts against a heightfield.";

class SceneRaycastVsStaticHeightfield : public HeightfieldTest
{
	public:
								SceneRaycastVsStaticHeightfield() : HeightfieldTest(64, 32)	{												}
	virtual						~SceneRaycastVsStaticHeightfield()							{												}
	virtual	TestCategory		GetCategory()										const	{ return CATEGORY_RAYCAST;						}
	virtual	udword				GetProfilingFlags()									const	{ return PROFILING_TEST_UPDATE;					}
	virtual	const char*			GetName()											const	{ return "SceneRaycastVsStaticHeightfield";		}
	virtual	const char*			GetDescription()									const	{ return gDesc_SceneRaycastVsStaticHeightfield;	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-0.33f, 20.30f, 89.37f), Point(0.00f, -0.13f, -0.99f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		const float SizeX = 80.0f;
		const float SizeZ = 60.0f;

		mHH.Init(SizeX, SizeZ, 4.0f);

		const Point Offset(0.0f, 0.0f, 0.0f);
		const Point Extents(SizeZ*0.5f, 0.0f, SizeX*0.5f);

		const udword NbX = 64;
		const udword NbY = 64;
		const float ScaleX = Extents.x - 1.0f;
		const float ScaleY = Extents.z - 1.0f;
		const float Altitude = 30.0f;
		const Point Dir(0.0f, -1.0f, 0.0f);
		GenerateArrayOfVerticalRaycasts(*this, 40.0f, NbX, NbY, gSQMaxDist);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRaycasts)
			return false;

		return HeightfieldTest::Init(pint, caps, Point(0.0f, 0.0f, 0.0f))!=null;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchRaycasts(*this, pint);
	}

END_TEST(SceneRaycastVsStaticHeightfield)

///////////////////////////////////////////////////////////////////////////////

#include "PintShapeRenderer.h"

// https://github.com/NVIDIAGameWorks/PhysX/issues/550

static const char* gDesc_RaycastBugGithub550 = "Repro for raycast bug reported on Github.";

class RaycastBugGithub550 : public TestBase
{
	PtrContainer	mObjects;

	public:
							RaycastBugGithub550()		{									}
	virtual					~RaycastBugGithub550()		{									}
	virtual	const char*		GetName()			const	{ return "RaycastBugGithub550";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_RaycastBugGithub550;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_RAYCAST;			}
	virtual	udword			GetProfilingFlags()	const	{ return PROFILING_TEST_UPDATE;		}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)	override
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(84.37f, 45.76f, -20.16f), Point(-0.93f, -0.29f, 0.21f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()	override
	{
		const Point Zero(0.0f, 0.0f, 0.0f);
		RegisterRaycast(Zero, Zero, 0.0f);
		return TestBase::CommonSetup();
	}

	virtual void	CommonRelease()	override
	{
		mObjects.Empty();
		TestBase::CommonRelease();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)	override
	{
		PINT_SPHERE_CREATE SphereDesc(1.0f);
		SphereDesc.mRenderer	= CreateSphereRenderer(SphereDesc.mRadius);

		PINT_OBJECT_CREATE ObjectDesc(&SphereDesc);
		ObjectDesc.mMass		= 0.0f;
		ObjectDesc.mPosition	= Point(0.0f, 10.0f, 0.0f);
		pint.mUserData = CreatePintObject(pint, ObjectDesc);

		//return Setup_PotPourri_Raycasts(pint, caps, 0.0f, 1, 16, 16);

		float mass = 0.0f;
		udword nb_layers = 1;
		udword nb_x = 32;
		udword nb_y = 32;
		//bool Setup_PotPourri_Raycasts(Pint& pint, const PintCaps& caps, float mass, udword nb_layers, udword nb_x, udword nb_y)
		{
			if(!caps.mSupportRaycasts)
				return false;

			const float BoxHeight = 4.0f;
			const float BoxSide = 1.0f;
			const float BoxDepth = 10.0f;

			const float SphereRadius = 0.5f;
			const float CapsuleRadius = 0.3f;
			const float HalfHeight = 0.5f;
			float yy = CapsuleRadius;
			BasicRandom Rnd(42);

			PINT_SPHERE_CREATE SphereDesc(SphereRadius);
			SphereDesc.mRenderer	= CreateSphereRenderer(SphereRadius);

			PINT_BOX_CREATE BoxDesc(CapsuleRadius, CapsuleRadius, CapsuleRadius);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_CAPSULE_CREATE CapsuleDesc(CapsuleRadius, HalfHeight);
			CapsuleDesc.mRenderer	= CreateCapsuleRenderer(CapsuleRadius, HalfHeight*2.0f);

			for(udword k=0;k<nb_layers;k++)
			{
				for(udword y=0;y<nb_y;y++)
				{
					const float CoeffY = float(nb_y) * 0.5f * ((float(y)/float(nb_y-1)) - 0.5f);
					for(udword x=0;x<nb_x;x++)
					{
						const float CoeffX = float(nb_x) * 0.5f * ((float(x)/float(nb_x-1)) - 0.5f);

						const float RandomX = 4.0f * Rnd.RandomFloat();
						const float RandomY = 4.0f * Rnd.RandomFloat();
						const float RandomZ = 4.0f * Rnd.RandomFloat();

						const udword Index = Rnd.Randomize() % 3;

						PINT_OBJECT_CREATE ObjectDesc;
						if(Index==0)
							ObjectDesc.SetShape(&SphereDesc);
						else if(Index==1)
							ObjectDesc.SetShape(&BoxDesc);
						else if(Index==2)
							ObjectDesc.SetShape(&CapsuleDesc);
						ObjectDesc.mMass		= mass;
						ObjectDesc.mPosition.x	= RandomX + CoeffX * (BoxDepth - SphereRadius - BoxSide*2.0f);
						ObjectDesc.mPosition.y	= RandomY + yy;
						ObjectDesc.mPosition.z	= RandomZ + CoeffY * (BoxDepth - SphereRadius - BoxSide*2.0f);

						UnitRandomQuat(ObjectDesc.mRotation, Rnd);

						PintActorHandle h = CreatePintObject(pint, ObjectDesc);
						mObjects.AddPtr(h);
					}
				}
				yy += HalfHeight*2.0f*4.0f;
			}
			return true;
		}

	}

//	virtual	void			CommonUpdate(float dt)						= 0;
	virtual	udword	Update(Pint& pint, float dt)
	{
		PintActorHandle h = PintActorHandle(pint.mUserData);
		if(!h)
			return 0;

		PR Pose(Idt);

		const float m = 20.0f;
		const float a = 20.0f;
		const float s = sinf(mCurrentTime*m) * a;
		const float c = cosf(mCurrentTime*m) * a;
		Pose.mPos = Point(s, 30.0f+c, 0.0f);
		pint.SetWorldTransform(h, Pose);

		{
			udword Nb = mObjects.GetNbEntries();
			for(udword i=0;i<Nb;i++)
			{
				PintActorHandle h2 = PintActorHandle(mObjects[i]);
				PR Pose2 = pint.GetWorldTransform(h2);

				const float Coeff = float(i)/float(Nb-1);
				const float m2 = 0.5f + Coeff*3.0f;
				const float a2 = 4.0f;
				const float s2 = sinf(mCurrentTime*m2) * a2;
				const float c2 = cosf(mCurrentTime*m2) * a2;

				Pose2.mPos += Point(s2*0.1f, 0.0f, c2*0.1f);
				pint.SetWorldTransform(h2, Pose2);
			}
		}

		PintRaycastData* Data = GetRegisteredRaycasts();
		ASSERT(Data);
		Data->mDir		= Point(0.0f, 0.0f, 1.0f);
		Data->mMaxDist	= 100.0f;
		Data->mOrigin	= Pose.mPos - Data->mDir * Data->mMaxDist*0.5f;

		udword NbHits = DoBatchRaycasts(*this, pint);
		ASSERT(NbHits==1);
		return NbHits;
	}

	virtual	udword	PostUpdate(Pint& pint, float dt)
	{
		udword NbHits = DoBatchRaycasts(*this, pint);
		ASSERT(NbHits==1);
		return NbHits;
	}


END_TEST(RaycastBugGithub550)

