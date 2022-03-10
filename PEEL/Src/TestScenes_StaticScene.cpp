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
#include "PintShapeRenderer.h"
#include "Loader_Bin.h"
#include "Loader_RepX.h"

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SeaOfStaticConvexes = "64*64 static convexes. Used to measure memory usage. (Check or uncheck the per-engine options in the UI to share convexes). \
This also exposes issues in some engines that take a significant amount of time to simulate a scene where everything is static.";

START_TEST(SeaOfStaticConvexes, CATEGORY_STATIC_SCENE, gDesc_SeaOfStaticConvexes)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.64f, -0.21f, -0.74f));
		desc.mCamera[1] = PintCameraPose(Point(125.30f, 216.41f, 122.06f), Point(-0.45f, -0.78f, -0.44f));
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return CreateSeaOfStaticConvexes(pint, caps, 64, 64, 40.0f);
	}

END_TEST(SeaOfStaticConvexes)

///////////////////////////////////////////////////////////////////////////////

/*static const char* gDesc_MovingSeaOfStaticConvexes = "Cost of moving static convexes each frame.";
#include "RenderModel.h"
#include "MyConvex.h"
START_TEST(MovingSeaOfStaticConvexes, CATEGORY_STATIC_SCENE, gDesc_MovingSeaOfStaticConvexes)

	Container	mTmp;

	virtual void	CommonRelease()
	{
		mTmp.Empty();
		TestBase::CommonRelease();
	}

	virtual	udword	GetProfilingFlags()	const	{ return PROFILING_TEST_UPDATE;	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.64f, -0.21f, -0.74f));
		desc.mCamera[1] = PintCameraPose(Point(125.30f, 216.41f, 122.06f), Point(-0.45f, -0.78f, -0.44f));
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		mCreateDefaultEnvironment = false;
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
//		return CreateSeaOfStaticConvexes(pint, caps, 64, 64, 40.0f);

		udword nb_x = 128;
		udword nb_y = 128;
		float altitude = 40.0f;

		if(!caps.mSupportConvexes)
			return false;

		MyConvex C;
	//	udword i=2;	// Small convexes
		udword i=4;	// 'Big' convexes
	//	udword i=7;
	//	udword i=13;
		C.LoadFile(i);

		PINT_CONVEX_CREATE ConvexCreate(C.mNbVerts, C.mVerts);
		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

		const float Scale = 3.0f;
		for(udword y=0;y<nb_y;y++)
		{
			for(udword x=0;x<nb_x;x++)
			{
				const float xf = (float(x)-float(nb_x)*0.5f)*Scale;
				const float yf = (float(y)-float(nb_y)*0.5f)*Scale;

				const Point pos = Point(xf, altitude, yf);

				PintActorHandle Handle = CreateStaticObject(pint, &ConvexCreate, pos);
				ASSERT(Handle);
				mTmp.Add(udword(Handle));
			}
		}
		return true;
	}

	virtual	udword					Update(Pint& pint, float dt)
	{
		static float mX = 0.0f;
		PR Pose;
		Pose.Identity();
		Pose.mPos.x = mX;
		mX += dt;

		const udword Nb = mTmp.GetNbEntries();
		for(udword i=0;i<Nb;i++)
		{
			PintActorHandle Handle = (PintActorHandle)mTmp.GetEntry(i);
			pint.SetWorldTransform(Handle, Pose);
		}

		return TestBase::Update(pint, dt);
	}

END_TEST(MovingSeaOfStaticConvexes)*/

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SeaOfStaticBoxes = "128*128 static boxes. Used to measure memory usage. \
This also exposes issues in some engines that take a significant amount of time to simulate a scene where everything is static.";

START_TEST(SeaOfStaticBoxes, CATEGORY_STATIC_SCENE, gDesc_SeaOfStaticBoxes)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.48f, -0.73f, -0.48f));
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return CreateSeaOfStaticBoxes(pint, 40.0f, 128, 128, 0.0f);
	}

END_TEST(SeaOfStaticBoxes)

static const char* gDesc_SeaOfStaticBoxes2 = "128*128 static boxes. Used to measure memory usage. \
This also exposes issues in some engines that take a significant amount of time to simulate a scene where everything is static.";

START_TEST(SeaOfStaticBoxes2, CATEGORY_STATIC_SCENE, gDesc_SeaOfStaticBoxes2)

	virtual	float	GetRenderData(Point& center)	const	{ return 500.0f;	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(111.63f, 147.75f, 112.85f), Point(-0.43f, -0.80f, -0.42f));
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return CreateSeaOfStaticBoxes(pint, 100.0f, 128, 128, 0.0f);
	}

END_TEST(SeaOfStaticBoxes2)

static const char* gDesc_SeaOfStaticBoxes3 = "255*255 static boxes. Used to measure memory usage. \
This also exposes issues in some engines that take a significant amount of time to simulate a scene where everything is static.";

START_TEST(SeaOfStaticBoxes3, CATEGORY_STATIC_SCENE, gDesc_SeaOfStaticBoxes3)

	virtual	float	GetRenderData(Point& center)	const	{ return 1000.0f;	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(159.01f, 148.44f, 126.04f), Point(-0.46f, -0.84f, -0.28f));
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return CreateSeaOfStaticBoxes(pint, 200.0f, 255, 255, 0.0f);
	}

END_TEST(SeaOfStaticBoxes3)

static const char* gDesc_SeaOfStaticBoxes4 = "Bounds captured from the Unity repro scene.";

START_TEST(SeaOfStaticBoxes4, CATEGORY_STATIC_SCENE, gDesc_SeaOfStaticBoxes4)

	virtual	float	GetRenderData(Point& center)	const	{ return 1000.0f;	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(159.01f, 148.44f, 126.04f), Point(-0.46f, -0.84f, -0.28f));
		SetDefEnv(desc, false);
	}

	virtual	udword	GetProfilingFlags()	const	{ return PROFILING_TEST_UPDATE;	}
	virtual	udword	Update(Pint& pint, float dt){ return DoBatchRaycasts(*this, pint);	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadRaysFile(*this, "SeaOfStaticBoxes4_rays.bin", true, true);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRaycasts)
			return false;

		const char* Filename = FindPEELFile("bounds.data");
		if(!Filename)
			return false;

		PINT_BOX_CREATE BoxCreate;

		FILE* fp = fopen(Filename, "rb");
		if(fp)
		{
			udword NbBoxes;
			fread(&NbBoxes, 1, sizeof(udword), fp);
			printf("Creating %d boxes\n", NbBoxes);
			for(udword i=0;i<NbBoxes;i++)
			{
				AABB Box;
				fread(&Box, 1, sizeof(AABB), fp);

				Point Center, Extents;
				Box.GetCenter(Center);
				Box.GetExtents(Extents);

				BoxCreate.mExtents	= Extents;
				BoxCreate.mRenderer	= null;//CreateBoxRenderer(Extents);

				PINT_OBJECT_CREATE ObjectDesc(&BoxCreate);
				ObjectDesc.mMass		= 0.0f;
				ObjectDesc.mPosition	= Center;
				CreatePintObject(pint, ObjectDesc);
			}
			fclose(fp);
		}
		return true;
	}

END_TEST(SeaOfStaticBoxes4)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SingleTriangle = "Single triangle. Used to check edge cases of static collision structures.";

START_TEST(SingleTriangle, CATEGORY_STATIC_SCENE, gDesc_SingleTriangle)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		CreateSingleTriangleMesh(*this, 100.0f);
		return TestBase::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

END_TEST(SingleTriangle)

///////////////////////////////////////////////////////////////////////////////

namespace
{
enum MeshIndex
{
	MESH_VENUS,
	MESH_TESSVENUS,
	MESH_VENUSMINI100,
	MESH_VENUSMINI1000,
	MESH_VENUSMAXI100,
	MESH_VENUSMAXI1000,
	MESH_TESTZONE,
	MESH_ARCHIPELAGO,
	MESH_TESSBUNNY,
	MESH_TESSBUNNY2,
	MESH_TERRAIN,
	MESH_TESSTERRAIN,
	MESH_TERRAIN_SQUASHED,
	MESH_TESSTERRAIN_SQUASHED,
	MESH_KONOKOPAYNE,
	MESH_RANRAN,
	MESH_TESSRANRAN,
	MESH_MECHA,
	MESH_MONSTERTRUCK,
	MESH_UNWRAP,
	MESH_COW_KNOT,
	MESH_BUDDHA,
//	MESH_MAZE,
};

struct MeshInfo
{
	const char*	mFilename;
	udword		mTess;
	Point		mScale;
	bool		mCreateDefaultEnv;
	bool		mResetPivots;
};
}

static MeshInfo gMeshInfo[] = 
{
	{"Venus.bin",			0, Point(1.0f, 1.0f, 1.0f), false, false},
	{"Venus.bin",			1, Point(1.0f, 1.0f, 1.0f), false, false},
	{"Venus.bin",			0, Point(1.0f/100.0f, 1.0f/100.0f, 1.0f/100.0f), false, false},
	{"Venus.bin",			0, Point(1.0f/1000.0f, 1.0f/1000.0f, 1.0f/1000.0f), false, false},
	{"Venus.bin",			0, Point(100.0f, 100.0f, 100.0f), false, false},
	{"Venus.bin",			0, Point(1000.0f, 1000.0f, 1000.0f), false, false},
	{"TestZone.bin",		0, Point(1.0f, 1.0f, 1.0f), false, false},
	{"Archipelago.bin",		0, Point(1.0f, 1.0f, 1.0f), false, false},
	{"Bunny.bin",			3, Point(1.0f, 1.0f, 1.0f), false, false},
	{"Bunny.bin",			4, Point(1.0f, 1.0f, 1.0f), false, false},
	{"Terrain.bin",			0, Point(1.0f, 1.0f, 1.0f), false, false},
	{"Terrain.bin",			1, Point(1.0f, 1.0f, 1.0f), false, false},
	{"Terrain.bin",			0, Point(1.0f, 0.2f, 1.0f), false, false},
	{"Terrain.bin",			1, Point(1.0f, 0.2f, 1.0f), false, false},
	{"kp.bin",				0, Point(1.0f, 1.0f, 1.0f), false, false},
	{"RanRan.bin",			0, Point(1.0f, 1.0f, 1.0f), false, false},
	{"RanRan.bin",			1, Point(1.0f, 1.0f, 1.0f), false, false},
	{"Mecha.bin",			0, Point(1.0f, 1.0f, 1.0f), false, false},
	{"NovodeX_Truck.bin",	0, Point(0.1f, 0.1f, 0.1f), true, true},
	{"Unwrap.bin",			0, Point(0.1f, 0.1f, 0.1f), false, true},
//	{"NewDome.bin",			0, Point(0.1f, 0.1f, 0.1f), false, false},
//	{"Dome.bin",			0, Point(1.0f, 1.0f, 1.0f), false, false},
	{"Cow_Knot.bin",		0, Point(1.0f, 1.0f, 1.0f), false, false},
	{"Buddha.bin",			0, Point(1.0f, 1.0f, 1.0f), false, false},
//	{"Maze.bin",			0, Point(1.0f, 1.0f, 1.0f), false, false},
};

static const char* gDesc_StaticMeshes = "Static mesh. Used to check memory usage and for raytracing tests.";

class StaticMesh : public TestBase
{
			ComboBoxPtr		mComboBox_Filename;
	public:
							StaticMesh()				{								}
	virtual					~StaticMesh()				{								}
	virtual	const char*		GetName()			const	{ return "StaticMesh";			}
	virtual	const char*		GetDescription()	const	{ return gDesc_StaticMeshes;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_STATIC_SCENE;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 150;
		WD.mLabel	= "Static mesh";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

//		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 30;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			y += YStep;
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "File:", &UIElems);
			{
				IceComboBox* CB = CreateComboBox<IceComboBox>(UI, 0, 4+OffsetX, y, 170, 200, "Filename", null, null);
				CB->Add("Venus");
				CB->Add("Tessellated Venus");
				CB->Add("Venus scaled by 1/100");
				CB->Add("Venus scaled by 1/1000");
				CB->Add("Venus scaled by 100");
				CB->Add("Venus scaled by 1000");
				CB->Add("Test Zone");
				CB->Add("Archipelago");
				CB->Add("Tessellated Bunny");
				CB->Add("Tessellated Bunny (2)");
				CB->Add("Terrain");
				CB->Add("Tessellated Terrain");
				CB->Add("Terrain (squashed)");
				CB->Add("Tessellated Terrain (squashed)");
				CB->Add("Konoko Payne");
				CB->Add("RanRan (Poser mesh)");
				CB->Add("Tessellated RanRan");
				CB->Add("Mecha");
				CB->Add("NovodeX Monster Truck");
				CB->Add("Unwrap scene");
//				CB->Add("NewDome scene");
//				CB->Add("Dome scene");
				CB->Add("Cow & Knot");
				CB->Add("Buddha");
//				CB->Add("Maze");
				CB->Select(0);
				mComboBox_Filename = CB;
			}
			RegisterUIElement(mComboBox_Filename);
			y += YStep;
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

		return null;
	}

	virtual	float	GetRenderData(Point& center)	const
	{
		Point Extents;
		GetGlobalBounds(center, Extents);
		return Extents.Magnitude()*3.0f;
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		if(mComboBox_Filename)
		{
			const udword Index = mComboBox_Filename->GetSelectedIndex();

			const Point& Scale = gMeshInfo[Index].mScale;
			const bool HasScale = Scale.x!=1.0f || Scale.y!=1.0f || Scale.z!=1.0f;
			//LoadMeshesFromFile_(*this, gMeshInfo[Index].mFilename, HasScale ? &Scale : null, false, gMeshInfo[Index].mTess);
			BinLoaderSettings BLS;
			if(HasScale)
				BLS.mScale = Scale;
			BLS.mMergeMeshes	= false;
			BLS.mResetPivot		= gMeshInfo[Index].mResetPivots;
			BLS.mTessellation	= gMeshInfo[Index].mTess;
			LoadBinMeshesFromFile(*this, gMeshInfo[Index].mFilename, BLS);
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);

		if(mComboBox_Filename)
		{
			const udword Index = mComboBox_Filename->GetSelectedIndex();

			if(Index==MESH_VENUS || Index==MESH_TESSVENUS)
			{
				desc.mCamera[0] = PintCameraPose(Point(14.38f, 17.68f, -17.02f), Point(-0.73f, -0.36f, 0.58f));
			}
			else if(Index==MESH_VENUSMINI100)
			{
				desc.mCamera[0] = PintCameraPose(Point(0.1438f, 0.1768f, -0.1702f), Point(-0.73f, -0.36f, 0.58f));
			}
			else if(Index==MESH_VENUSMINI1000)
			{
				desc.mCamera[0] = PintCameraPose(Point(0.01438f, 0.01768f, -0.01702f), Point(-0.73f, -0.36f, 0.58f));
			}
			else if(Index==MESH_VENUSMAXI100)
			{
				desc.mCamera[0] = PintCameraPose(Point(1438.0f, 1768.0f, -1702.0f), Point(-0.73f, -0.36f, 0.58f));
			}
			else if(Index==MESH_VENUSMAXI1000)
			{
				desc.mCamera[0] = PintCameraPose(Point(14380.0f, 17680.0f, -17020.0f), Point(-0.73f, -0.36f, 0.58f));
			}
			else if(Index==MESH_TESTZONE)
			{
				desc.mCamera[0] = PintCameraPose(Point(10.43f, 5.68f, -31.10f), Point(0.90f, 0.05f, -0.43f));
				desc.mCamera[1] = PintCameraPose(Point(-39.54f, 13.74f, -92.63f), Point(0.76f, -0.16f, 0.64f));
				desc.mCamera[2] = PintCameraPose(Point(-22.40f, 10.29f, -39.07f), Point(0.42f, -0.28f, -0.86f));
				desc.mCamera[3] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.45f, -0.61f, -0.66f));
			}
			else if(Index==MESH_ARCHIPELAGO)
			{
				desc.mCamera[0] = PintCameraPose(Point(-44.80f, 96.74f, -6.67f), Point(0.71f, -0.52f, 0.47f));
			}
			else if(Index==MESH_TESSBUNNY||Index==MESH_TESSBUNNY2)
			{
				desc.mCamera[0] = PintCameraPose(Point(11.67f, 16.70f, 11.07f), Point(-0.67f, -0.53f, -0.52f));
			}
			else if(Index>=MESH_TERRAIN && Index<=MESH_TESSTERRAIN_SQUASHED)
			{
				desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.60f, -0.20f, -0.70f));
				desc.mCamera[1] = PintCameraPose(Point(-19.04f, 158.54f, -117.30f), Point(0.63f, -0.19f, 0.75f));
				desc.mCamera[2] = PintCameraPose(Point(3326.92f, 116.30f, 2212.80f), Point(-0.93f, -0.29f, -0.22f));
				desc.mCamera[3] = PintCameraPose(Point(3444.69f, 3034.93f, 117.91f), Point(-0.35f, -0.85f, 0.39f));
			}
			else if(Index==MESH_KONOKOPAYNE)
			{
				desc.mCamera[0] = PintCameraPose(Point(988.78f, 690.59f, -239.36f), Point(0.01f, -0.16f, -0.99f));
				desc.mCamera[1] = PintCameraPose(Point(1565.97f, 73.77f, -919.54f), Point(-0.65f, 0.41f, -0.63f));	// Zoom on a crack
				desc.mCamera[2] = PintCameraPose(Point(-135.20f, 714.83f, 182.81f), Point(-0.96f, -0.10f, 0.24f));
				desc.mCamera[3] = PintCameraPose(Point(132.35f, 10.32f, 29.94f), Point(-0.45f, 0.70f, -0.56f));
				desc.mCamera[4] = PintCameraPose(Point(288.28f, 33.02f, 118.42f), Point(-0.78f, -0.33f, 0.54f));
				desc.mCamera[5] = PintCameraPose(Point(666.93f, 127.14f, 137.84f), Point(-0.77f, -0.29f, -0.57f));
				desc.mCamera[6] = PintCameraPose(Point(347.43f, 83.30f, 149.97f), Point(0.64f, -0.18f, -0.75f));
				desc.mCamera[7] = PintCameraPose(Point(367.75f, 91.14f, 119.23f), Point(-0.77f, -0.45f, 0.45f));
				desc.mCamera[8] = PintCameraPose(Point(318.61f, -88.88f, -995.75f), Point(-0.78f, 0.41f, 0.48f));
				desc.mCamera[9] = PintCameraPose(Point(1597.49f, 38.58f, -876.76f), Point(0.23f, 0.15f, -0.96f));
				desc.mCamera[10] = PintCameraPose(Point(561.67f, 190.55f, -1401.69f), Point(-0.41f, -0.12f, 0.90f));
				desc.mCamera[11] = PintCameraPose(Point(58.80f, 15.29f, 531.64f), Point(-0.81f, 0.11f, 0.57f));
				desc.mCamera[12] = PintCameraPose(Point(223.86f, 20.62f, 445.55f), Point(-0.42f, -0.40f, -0.81f));
				desc.mCamera[13] = PintCameraPose(Point(237.01f, 182.25f, 392.53f), Point(-0.79f, -0.26f, 0.55f));
			}
			else if(Index==MESH_RANRAN || Index==MESH_TESSRANRAN)
			{
				desc.mCamera[0] = PintCameraPose(Point(-18.72f, 57.64f, -14.98f), Point(0.78f, 0.04f, 0.62f));
			}
			else if(Index==MESH_MECHA)
			{
				desc.mCamera[0] = PintCameraPose(Point(168.62f, 302.79f, -196.47f), Point(-0.40f, -0.74f, 0.55f));
			}
			else if(Index==MESH_MONSTERTRUCK)
			{
				desc.mCamera[0] = PintCameraPose(Point(22.78f, 11.72f, -18.13f), Point(-0.74f, -0.25f, 0.62f));
				desc.mCamera[1] = PintCameraPose(Point(-26.56f, 9.11f, -0.64f), Point(0.99f, -0.12f, 0.02f));
			}
			else if(Index==MESH_UNWRAP)
			{
				desc.mCamera[0] = PintCameraPose(Point(22.78f, 11.72f, -18.13f), Point(-0.74f, -0.25f, 0.62f));
			}
			else if(Index==MESH_COW_KNOT)
			{
				desc.mCamera[0] = PintCameraPose(Point(3.67f, 1.96f, -2.97f), Point(-0.89f, -0.27f, 0.37f));
			}
			else if(Index==MESH_BUDDHA)
			{
				desc.mCamera[0] = PintCameraPose(Point(1018.40f, 326.78f, -39.56f), Point(-0.97f, -0.25f, 0.02f));
			}

			SetDefEnv(desc, gMeshInfo[Index].mCreateDefaultEnv);
		}
	}

END_TEST(StaticMesh)

///////////////////////////////////////////////////////////////////////////////

namespace
{
enum CustomerMeshIndex
{
	MESH_EMPTY,
	MESH_PLANETSIDE,
	MESH_VALVE,
	MESH_SPECULAR,
	MESH_UNITY,
};

struct CustomerMeshInfo
{
	const char*	mFilename;
	float		mScale;
	bool		mZIsUp;
	bool		mRepX;
};
}

static CustomerMeshInfo gCustomerMeshInfo[] = 
{
	{null,										1.0f, false, false},
	{"Planetside_Statics.repx",					1.0f, false, true},
	{"c5m4_quarter2_Statics.repx",				gValveScale, true, true},
//	{"specular_raycast_meshes.repx",			1.0f, false, true},
	{"specular_raycast_meshes_statics.repx",	1.0f, false, true},
	{"hadley.zb2",								1.0f, false, false},
};

static const char* gDesc_CustomerStaticMeshes = "Customer static mesh. Used to check memory usage and for raytracing tests.";

class CustomerMesh : public TestBase
{
			ComboBoxPtr		mComboBox_Filename;
	public:
							CustomerMesh()				{										}
	virtual					~CustomerMesh()				{										}
	virtual	const char*		GetName()			const	{ return "CustomerStaticMesh";			}
	virtual	const char*		GetDescription()	const	{ return gDesc_CustomerStaticMeshes;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_STATIC_SCENE;			}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 150;
		WD.mLabel	= "Customer static mesh";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

//		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 30;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			y += YStep;
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "File:", &UIElems);
			{
				IceComboBox* CB = CreateComboBox<IceComboBox>(UI, 0, 4+OffsetX, y, 170, 20, "Filename", null, null);
				CB->Add("Empty scene");
				CB->Add("PlanetSide");
				CB->Add("Valve");
				CB->Add("Specular");
				CB->Add("Unity");
				CB->Select(0);
				mComboBox_Filename = CB;
			}
			RegisterUIElement(mComboBox_Filename);
			y += YStep;
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

		return null;
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		if(mComboBox_Filename)
		{
			const udword Index = mComboBox_Filename->GetSelectedIndex();
			if(gCustomerMeshInfo[Index].mFilename)
			{
				if(gCustomerMeshInfo[Index].mRepX)
					mRepX = CreateRepXContext(gCustomerMeshInfo[Index].mFilename, gCustomerMeshInfo[Index].mScale, gCustomerMeshInfo[Index].mZIsUp);
//				else
//					LoadMeshesFromFile_(*this, gCustomerMeshInfo[Index].mFilename);
//					ImportZB2File(desc, gCustomerMeshInfo[Index].mFilename);
			}
		}
		return true;
	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		SetDefEnv(desc, false);

		if(mComboBox_Filename)
		{
			const udword Index = mComboBox_Filename->GetSelectedIndex();
			if(Index==MESH_PLANETSIDE)
			{
				desc.mCamera[0] = PintCameraPose(Point(93.89f, 19.72f, 689.68f), Point(0.38f, -0.37f, 0.85f));
				desc.mCamera[1] = PintCameraPose(Point(60.21f, 49.41f, 618.84f), Point(0.72f, -0.25f, 0.65f));
				desc.mCamera[2] = PintCameraPose(Point(229.98f, 5.80f, 704.46f), Point(0.51f, -0.12f, 0.85f));
				desc.mCamera[3] = PintCameraPose(Point(163.14f, 1.22f, 776.86f), Point(-0.05f, -0.06f, -1.00f));
				desc.mCamera[4] = PintCameraPose(Point(232.90f, 5.12f, 774.70f), Point(0.39f, -0.13f, -0.91f));
				desc.mCamera[5] = PintCameraPose(Point(409.69f, 23.02f, 712.23f), Point(-0.99f, -0.12f, 0.10f));
				desc.mCamera[6] = PintCameraPose(Point(131.35f, 69.80f, 700.85f), Point(-0.50f, -0.82f, 0.28f));
				desc.mCamera[7] = PintCameraPose(Point(227.48f, 24.30f, 662.48f), Point(-0.23f, -0.11f, 0.97f));
			}
			else if(Index==MESH_VALVE)
			{
				desc.mCamera[0] = PintCameraPose(Point(-1327.62f, 259.49f, 384.52f), Point(0.91f, 0.03f, -0.42f));
			}
			else if(Index==MESH_UNITY)
			{
				if(mComboBox_Filename)
				{
					const udword Index = mComboBox_Filename->GetSelectedIndex();
					if(gCustomerMeshInfo[Index].mFilename)
					{
						const char* Filename = FindPEELFile(gCustomerMeshInfo[Index].mFilename);
						if(Filename)
							ImportZB2File(desc, Filename);
					}
				}
			}
		}
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes)
			return false;

		if(mRepX)
			return AddToPint(pint, mRepX);

		return CreateZB2Scene(pint, caps);
	}

	virtual bool	IsPrivate()	const
	{
		return true;
	}

END_TEST(CustomerMesh)

///////////////////////////////////////////////////////////////////////////////
