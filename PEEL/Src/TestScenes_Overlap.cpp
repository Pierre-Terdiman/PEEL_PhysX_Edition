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
#include "Loader_RepX.h"
#include "Loader_Bin.h"
#include "PintSQ.h"
#include "GUI_Helpers.h"

///////////////////////////////////////////////////////////////////////////////

static bool SetupSphereOverlaps(TestBase& test, udword nb_x, udword nb_y, float scale_x, float scale_y, float altitude, float sphere_radius)
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

			test.RegisterSphereOverlap(Sphere(Origin, sphere_radius));
		}
	}
	return true;
}

static bool SetupBoxOverlaps(TestBase& test, udword nb_x, udword nb_y, float scale_x, float scale_y, float altitude, float box_radius)
{
	BasicRandom Rnd(42);

	const Point Extents(box_radius, box_radius, box_radius*2.0f);

	const float OneOverNbX = OneOverNb(nb_x);
	const float OneOverNbY = OneOverNb(nb_y);
	for(udword y=0;y<nb_y;y++)
	{
		const float CoeffY = 2.0f * ((float(y)*OneOverNbY) - 0.5f);
		for(udword x=0;x<nb_x;x++)
		{
			const float CoeffX = 2.0f * ((float(x)*OneOverNbX) - 0.5f);

			const Point Origin(CoeffX * scale_x, altitude, CoeffY * scale_y);

			Quat Q;
			UnitRandomQuat(Q, Rnd);
			const Matrix3x3 Rot = Q;

			test.RegisterBoxOverlap(OBB(Origin, Extents, Rot));
		}
	}
	return true;
}

static bool SetupCapsuleOverlaps(TestBase& test, udword nb_x, udword nb_y, float scale_x, float scale_y, float altitude, float capsule_radius, float capsule_height)
{
	BasicRandom Rnd(42);

	const float OneOverNbX = OneOverNb(nb_x);
	const float OneOverNbY = OneOverNb(nb_y);
	for(udword y=0;y<nb_y;y++)
	{
		const float CoeffY = 2.0f * ((float(y)*OneOverNbY) - 0.5f);
		for(udword x=0;x<nb_x;x++)
		{
			const float CoeffX = 2.0f * ((float(x)*OneOverNbX) - 0.5f);

			const Point Origin(CoeffX * scale_x, altitude, CoeffY * scale_y);

			Quat Q;
			UnitRandomQuat(Q, Rnd);
			const Matrix3x3 Rot = Q;

			const Point D = Rot[1] * capsule_height;

			test.RegisterCapsuleOverlap(LSS(Segment(Origin-D, Origin+D), capsule_radius));
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapShapesVsShapes = "Overlaps N*N query shapes vs M*M static shapes. Overlap mode is either 'any' or 'all'. \
In 'any' mode the overlap query returns a 'boolean' yes/no answer (i.e. whether the shape touches something or not. In 'all' mode the overlap query returns all objects \
touching the input shape. Obviously, the 'any' mode is faster since the query can early exit as soon as it finds a single hit.";

class OverlapShapesVsShapes : public TestBase
{
			ComboBoxPtr		mComboBox_OverlapQueryType;
			ComboBoxPtr		mComboBox_QueryShapeType;
			ComboBoxPtr		mComboBox_StaticShapeType;
			EditBoxPtr		mEditBox_QueryShapeSize;
//			EditBoxPtr		mEditBox_StaticShapeSize;
			EditBoxPtr		mEditBox_QueryShapeGridSize;
			EditBoxPtr		mEditBox_StaticShapeGridSize;
			PintShape		mQueryShapeType;

	public:
							OverlapShapesVsShapes() :
								mQueryShapeType				(PINT_SHAPE_UNDEFINED)
								{}
	virtual					~OverlapShapesVsShapes()	{										}
	virtual	const char*		GetName()			const	{ return "OverlapShapesVsShapes";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_OverlapShapesVsShapes;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_OVERLAP;				}
	virtual	udword			GetProfilingFlags()	const	{ return PROFILING_TEST_UPDATE;			}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 240;
		WD.mLabel	= "OverlapShapesVsShapes";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 110;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Overlap query type:", &UIElems);
			{
				IceComboBox* CB = CreateComboBox<IceComboBox>(UI, 0, 4+OffsetX, y, 170, 20, "Overlap query type", null, null);
				CB->Add("Overlap any (returns 1st hit)");
				CB->Add("Overlap multiple (returns all hits)");
				CB->Select(0);
				mComboBox_OverlapQueryType = CB;
			}
			RegisterUIElement(mComboBox_OverlapQueryType);
			y += YStep;
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Query shape grid size:", &UIElems);
			mEditBox_QueryShapeGridSize = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "32", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			mEditBox_QueryShapeGridSize->SetEnabled(true);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Query shape size:", &UIElems);
			mEditBox_QueryShapeSize = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_QueryShapeSize->SetEnabled(true);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Query shape type:", &UIElems);
			mComboBox_QueryShapeType = CreateShapeTypeComboBox(UI, 4+OffsetX, y, true, SSM_UNDEFINED|SSM_SPHERE|SSM_BOX);
			RegisterUIElement(mComboBox_QueryShapeType);
			mComboBox_QueryShapeType->Select(1);
			y += YStep;
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Static shape grid size:", &UIElems);
			mEditBox_StaticShapeGridSize = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "64", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			mEditBox_StaticShapeGridSize->SetEnabled(true);
			y += YStep;

/*			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Static shape size:", &UIElems);
			mEditBox_StaticShapeSize = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_StaticShapeSize->SetEnabled(true);
			y += YStep;*/

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Static shape type:", &UIElems);
			mComboBox_StaticShapeType = CreateShapeTypeComboBox(UI, 4+OffsetX, y, true, SSM_UNDEFINED|SSM_SPHERE|SSM_CAPSULE|SSM_BOX|SSM_CONVEX);
			RegisterUIElement(mComboBox_StaticShapeType);
			mComboBox_StaticShapeType->Select(1);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

		return null;
	}

	virtual void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(251.56f, 290.07f, 258.05f), Point(-0.49f, -0.73f, -0.47f));
		desc.mCamera[1] = PintCameraPose(Point(121.41f, 39.53f, 123.18f), Point(-0.66f, -0.46f, -0.60f));
		SetDefEnv(desc, false);
	}

	virtual bool			CommonSetup()
	{
		TestBase::CommonSetup();

		const udword GridSize = GetInt(32, mEditBox_QueryShapeGridSize);
		const float ShapeSize = GetFloat(1.0f, mEditBox_QueryShapeSize);
		const float Altitude = 0.0f;

		mQueryShapeType = PINT_SHAPE_UNDEFINED;
		if(mComboBox_QueryShapeType)
			mQueryShapeType = PintShape(mComboBox_QueryShapeType->GetSelectedIndex());

		if(mQueryShapeType==PINT_SHAPE_SPHERE)
			return SetupSphereOverlaps(*this, GridSize, GridSize, 200.0f, 200.0f, Altitude, ShapeSize);
		else if(mQueryShapeType==PINT_SHAPE_BOX)
			return SetupBoxOverlaps(*this, GridSize, GridSize, 200.0f, 200.0f, Altitude, ShapeSize);
		else
			return false;
	}

	virtual bool			Setup(Pint& pint, const PintCaps& caps)
	{
		if(mQueryShapeType==PINT_SHAPE_SPHERE && !caps.mSupportSphereOverlaps)
			return false;
		if(mQueryShapeType==PINT_SHAPE_BOX && !caps.mSupportBoxOverlaps)
			return false;
		if(mQueryShapeType==PINT_SHAPE_CAPSULE && !caps.mSupportCapsuleOverlaps)
			return false;
		if(mQueryShapeType==PINT_SHAPE_CONVEX && !caps.mSupportConvexOverlaps)
			return false;

		// We only support these ones for now
		if(mQueryShapeType!=PINT_SHAPE_UNDEFINED && mQueryShapeType!=PINT_SHAPE_SPHERE && mQueryShapeType!=PINT_SHAPE_BOX)
			return false;

		PintShape StaticShapeType = PINT_SHAPE_UNDEFINED;
		if(mComboBox_StaticShapeType)
			StaticShapeType = PintShape(mComboBox_StaticShapeType->GetSelectedIndex());
		if(StaticShapeType==PINT_SHAPE_UNDEFINED)
			return true;

		const udword GridSize = GetInt(32, mEditBox_StaticShapeGridSize);
//		const float ShapeSize = GetFloat(1.0f, mEditBox_StaticShapeSize, 0.0f, MAX_FLOAT);
		const float Altitude = 0.0f;
		const float Amplitude = 200.0f;

		if(StaticShapeType==PINT_SHAPE_SPHERE)
			return CreateSeaOfStaticSpheres(pint, Amplitude, GridSize, GridSize, Altitude);
		if(StaticShapeType==PINT_SHAPE_CAPSULE)
			return CreateSeaOfStaticCapsules(pint, Amplitude, GridSize, GridSize, Altitude);
		if(StaticShapeType==PINT_SHAPE_BOX)
			return CreateSeaOfStaticBoxes(pint, Amplitude, GridSize, GridSize, Altitude);
		if(StaticShapeType==PINT_SHAPE_CONVEX)
			return CreateSeaOfStaticConvexes(pint, caps, GridSize, GridSize, Altitude);

		return false;
	}

	virtual udword			Update(Pint& pint, float dt)
	{
		BatchOverlapMode OverlapQueryType = OVERLAP_ANY;
		if(mComboBox_OverlapQueryType && mComboBox_OverlapQueryType->GetSelectedIndex()==1)
			OverlapQueryType = OVERLAP_OBJECTS;

		if(mQueryShapeType==PINT_SHAPE_SPHERE)
			return DoBatchSphereOverlaps(*this, pint, OverlapQueryType);
		else if(mQueryShapeType==PINT_SHAPE_BOX)
			return DoBatchBoxOverlaps(*this, pint, OverlapQueryType);
		return 0;
	}

}OverlapShapesVsShapes;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapObjects_DynamicSphere = "OVERLAP_OBJECTS: single dynamic sphere overlap against 128*128 static boxes. \
The goal is to report all boxes touched by the sphere.";

START_SQ_TEST(OverlapObjects_DynamicSphere, CATEGORY_OVERLAP, gDesc_OverlapObjects_DynamicSphere)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(68.28f, 170.77f, -131.67f), Point(-0.30f, -0.73f, 0.61f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		RegisterSphereOverlap(Sphere(Point(0.0f, 0.0f, 0.0f), 1.0f));
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphereOverlaps)
			return false;

		return CreateSeaOfStaticBoxes(pint, 200.0f, 128, 128, 0.0f);
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		PintSphereOverlapData* Data = GetRegisteredSphereOverlaps();
		Data->mSphere.mRadius = (2.0f+sinf(mCurrentTime*1.0f))*10.0f;
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchSphereOverlaps(*this, pint, OVERLAP_OBJECTS);
	}

END_TEST(OverlapObjects_DynamicSphere)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapObjects_DynamicSpheres = "OVERLAP_OBJECTS: 32*32 (dynamic) spheres against 128*128 static boxes. \
The goal is to report all boxes touched by the spheres.";

START_SQ_TEST(OverlapObjects_DynamicSpheres, CATEGORY_OVERLAP, gDesc_OverlapObjects_DynamicSpheres)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(130.81f, 79.15f, 0.13f), Point(-0.79f, -0.62f, 0.03f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		return SetupSphereOverlaps(*this, 32, 32, 200.0f, 200.0f, 0.0f, 20.0f);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphereOverlaps)
			return false;

		return CreateSeaOfStaticBoxes(pint, 200.0f, 128, 128, 0.0f);
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		udword Nb = GetNbRegisteredSphereOverlaps();
		PintSphereOverlapData* Data = GetRegisteredSphereOverlaps();
		const float Radius = (2.0f+sinf(mCurrentTime*1.0f))*5.0f;
		while(Nb--)
		{
			Data->mSphere.mRadius = Radius;
			Data++;
		}
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchSphereOverlaps(*this, pint, OVERLAP_OBJECTS);
	}

END_TEST(OverlapObjects_DynamicSpheres)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapAny_SphereVsTessBunny = "OVERLAP_ANY: 1 sphere against the tessellated bunny. This is really just to investigate an issue in PhysX.";

START_SQ_TEST(OverlapAny_SphereVsTessBunny, CATEGORY_OVERLAP, gDesc_OverlapAny_SphereVsTessBunny)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(207.55f, 226.44f, 129.57f), Point(-0.62f, -0.65f, -0.43f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadMeshesFromFile_(*this, "bunny.bin", null, false, 3);
		RegisterSphereOverlap(Sphere(Point(0.0f, 0.0f, 0.0f), 100.0f));
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphereOverlaps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchSphereOverlaps(*this, pint, OVERLAP_ANY);
	}

END_TEST(OverlapAny_SphereVsTessBunny)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapObjects_SphereVsTessBunny = "OVERLAP_OBJECTS: 1 sphere against the tessellated bunny. This is really just to investigate an issue in Havok.";

START_SQ_TEST(OverlapObjects_SphereVsTessBunny, CATEGORY_OVERLAP, gDesc_OverlapObjects_SphereVsTessBunny)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(207.55f, 226.44f, 129.57f), Point(-0.62f, -0.65f, -0.43f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadMeshesFromFile_(*this, "bunny.bin", null, false, 3);
		RegisterSphereOverlap(Sphere(Point(0.0f, 0.0f, 0.0f), 100.0f));
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphereOverlaps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchSphereOverlaps(*this, pint, OVERLAP_OBJECTS);
	}

END_TEST(OverlapObjects_SphereVsTessBunny)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapAny_SpheresVsTessBunny = "OVERLAP_ANY: 16*16*16 spheres against the tessellated bunny. \
This reports how many spheres touched the mesh.";

START_SQ_TEST(OverlapAny_SpheresVsTessBunny, CATEGORY_OVERLAP, gDesc_OverlapAny_SpheresVsTessBunny)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(22.92f, 27.83f, 23.32f), Point(-0.61f, -0.53f, -0.59f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadMeshesFromFile_(*this, "bunny.bin", null, false, 3);
		for(udword i=0;i<16;i++)
			SetupSphereOverlaps(*this, 16, 16, 10.0f, 10.0f, float(i), 0.5f);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphereOverlaps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		udword Nb = GetNbRegisteredSphereOverlaps();
		PintSphereOverlapData* Data = GetRegisteredSphereOverlaps();
		const float Radius = 0.1f+(1.0f + sinf(mCurrentTime*1.0f))*1.0f;
		while(Nb--)
		{
			Data->mSphere.mRadius = Radius;
			Data++;
		}
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchSphereOverlaps(*this, pint, OVERLAP_ANY);
	}

END_TEST(OverlapAny_SpheresVsTessBunny)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapAny_BoxesVsTessBunny = "OVERLAP_ANY: 16*16*16 boxes against the tessellated bunny. \
This reports how many boxes touched the mesh.";

START_SQ_TEST(OverlapAny_BoxesVsTessBunny, CATEGORY_OVERLAP, gDesc_OverlapAny_BoxesVsTessBunny)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(22.92f, 27.83f, 23.32f), Point(-0.61f, -0.53f, -0.59f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadMeshesFromFile_(*this, "bunny.bin", null, false, 3);
		for(udword i=0;i<16;i++)
			SetupBoxOverlaps(*this, 16, 16, 10.0f, 10.0f, float(i), 0.5f);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportBoxOverlaps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchBoxOverlaps(*this, pint, OVERLAP_ANY);
	}

END_TEST(OverlapAny_BoxesVsTessBunny)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapAny_CapsulesVsTessBunny = "OVERLAP_ANY: 16*16*16 capsules against the tessellated bunny. \
This reports how many capsules touched the mesh.";

START_SQ_TEST(OverlapAny_CapsulesVsTessBunny, CATEGORY_OVERLAP, gDesc_OverlapAny_CapsulesVsTessBunny)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(22.92f, 27.83f, 23.32f), Point(-0.61f, -0.53f, -0.59f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadMeshesFromFile_(*this, "bunny.bin", null, false, 3);
		for(udword i=0;i<16;i++)
			SetupCapsuleOverlaps(*this, 16, 16, 10.0f, 10.0f, float(i), 0.5f, 10.0f);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCapsuleOverlaps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchCapsuleOverlaps(*this, pint, OVERLAP_ANY);
	}

END_TEST(OverlapAny_CapsulesVsTessBunny)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapAny_SpheresVsTerrain = "OVERLAP_ANY: 4096 spheres against the terrain mesh";

START_SQ_TEST(OverlapAny_SpheresVsTerrain, CATEGORY_OVERLAP, gDesc_OverlapAny_SpheresVsTerrain)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-480.68f, 1639.54f, -443.50f), Point(0.59f, -0.60f, 0.54f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadMeshesFromFile_(*this, "terrain.bin");

		const Point Min(-128.0f, -432.117371f, -128.0f);
		const Point Max(3968.0f, 710.848145f, 3968.0f);
		const Point Center = (Max + Min)*0.5f;
		const Point Extents = (Max - Min)*0.5f;

		BasicRandom Rnd(42);
		for(udword i=0;i<4096;i++)
		{
			Point p;
			UnitRandomPt(p, Rnd);
			Point Pos = Center + Extents*p;
			Pos.y = 0.0f;
			RegisterSphereOverlap(Sphere(Pos, 20.0f));
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphereOverlaps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchSphereOverlaps(*this, pint, OVERLAP_ANY);
	}

END_TEST(OverlapAny_SpheresVsTerrain)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapAny_BoxesVsTerrain = "OVERLAP_ANY: 4096 boxes against the terrain mesh";

START_SQ_TEST(OverlapAny_BoxesVsTerrain, CATEGORY_OVERLAP, gDesc_OverlapAny_BoxesVsTerrain)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-480.68f, 1639.54f, -443.50f), Point(0.59f, -0.60f, 0.54f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadMeshesFromFile_(*this, "terrain.bin");

		const Point Min(-128.0f, -432.117371f, -128.0f);
		const Point Max(3968.0f, 710.848145f, 3968.0f);
		const Point Center = (Max + Min)*0.5f;
		const Point Extents = (Max - Min)*0.5f;

		BasicRandom Rnd(42);
		Matrix3x3 Rot;
		Rot.RotX(45.0f * DEGTORAD);
		for(udword i=0;i<4096;i++)
		{
			Point p;
			UnitRandomPt(p, Rnd);
			Point Pos = Center + Extents*p;
			Pos.y = 0.0f;
			RegisterBoxOverlap(OBB(Pos, Point(20.0f, 20.0f, 20.0f), Rot));
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportBoxOverlaps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
//		return DoBatchBoxOverlaps(*this, pint, OVERLAP_OBJECTS);
		return DoBatchBoxOverlaps(*this, pint, OVERLAP_ANY);
	}

END_TEST(OverlapAny_BoxesVsTerrain)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapAny_SpheresVsPlanetSide = "OVERLAP_ANY: 4096 spheres against the PlanetSide level";

START_SQ_TEST(OverlapAny_SpheresVsPlanetSide, CATEGORY_OVERLAP, gDesc_OverlapAny_SpheresVsPlanetSide)

	virtual bool	IsPrivate()	const
	{
		return true;
	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-480.68f, 1639.54f, -443.50f), Point(0.59f, -0.60f, 0.54f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		mRepX = CreateRepXContext("Planetside_Statics.repx", 1.0f, false);

		const Point Min(0.0f, -20.f, 0.0f);
		const Point Max(1000.0f, 20.0f, 1000.0f);
		const Point Center = (Max + Min)*0.5f;
		const Point Extents = (Max - Min)*0.5f;

		BasicRandom Rnd(42);
		for(udword i=0;i<4096;i++)
		{
			Point p;
			UnitRandomPt(p, Rnd);
			Point Pos = Center + Extents*p;
//			Pos.y = 0.0f;
//			RegisterSphereOverlap(Sphere(Pos, 2.0f));
//			RegisterSphereOverlap(Sphere(Pos, 20.0f));
			RegisterSphereOverlap(Sphere(Pos, 10.0f));
//			RegisterSphereOverlap(Sphere(Pos, 1.0f));
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphereOverlaps || !caps.mSupportMeshes)
			return false;

		return AddToPint(pint, mRepX);
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		udword Nb = GetNbRegisteredSphereOverlaps();
		PintSphereOverlapData* Data = GetRegisteredSphereOverlaps();
		const float Radius = 1.0f+(1.0f + sinf(mCurrentTime*1.0f))*5.0f;
		while(Nb--)
		{
			Data->mSphere.mRadius = Radius;
			Data++;
		}
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchSphereOverlaps(*this, pint, OVERLAP_ANY);
	}

END_TEST(OverlapAny_SpheresVsPlanetSide)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapAny_BoxesVsPlanetSide = "OVERLAP_ANY: 4096 boxes against the PlanetSide level";

START_SQ_TEST(OverlapAny_BoxesVsPlanetSide, CATEGORY_OVERLAP, gDesc_OverlapAny_BoxesVsPlanetSide)

	virtual bool	IsPrivate()	const
	{
		return true;
	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-480.68f, 1639.54f, -443.50f), Point(0.59f, -0.60f, 0.54f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		mRepX = CreateRepXContext("Planetside_Statics.repx", 1.0f, false);

		const Point Min(0.0f, -20.f, 0.0f);
		const Point Max(1000.0f, 20.0f, 1000.0f);
		const Point Center = (Max + Min)*0.5f;
		const Point Extents = (Max - Min)*0.5f;

		BasicRandom Rnd(42);
		Matrix3x3 Rot;
		Rot.RotX(45.0f * DEGTORAD);
		for(udword i=0;i<4096;i++)
		{
			Point p;
			UnitRandomPt(p, Rnd);
			Point Pos = Center + Extents*p;
//			Pos.y = 0.0f;
			RegisterBoxOverlap(OBB(Pos, Point(10.0f, 10.0f, 10.0f), Rot));
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportBoxOverlaps || !caps.mSupportMeshes)
			return false;

		return AddToPint(pint, mRepX);
	}

/*	virtual void	CommonUpdate(float dt)
	{
		mCurrentTime += dt;
		TestBase::CommonUpdate(dt);
		udword Nb = GetNbRegisteredSphereOverlaps();
		PintSphereOverlapData* Data = GetRegisteredSphereOverlaps();
		const float Radius = 1.0f+(1.0f + sinf(mCurrentTime*1.0f))*5.0f;
		while(Nb--)
		{
			Data->mSphere.mRadius = Radius;
			Data++;
		}
	}*/

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchBoxOverlaps(*this, pint, OVERLAP_ANY);
	}

END_TEST(OverlapAny_BoxesVsPlanetSide)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapObjects_SpheresVsKP = "OVERLAP_OBJECTS: 4096 spheres against the KP level";

START_SQ_TEST(OverlapObjects_SpheresVsKP, CATEGORY_OVERLAP, gDesc_OverlapObjects_SpheresVsKP)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(1085.22f, 3963.36f, -9239.41f), Point(-0.59f, -0.59f, 0.56f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadMeshesFromFile_(*this, "kp.bin");

		const Point Min(-9560.175781f, -1093.885132f, -9461.288086f);
		const Point Max(9538.423828f, 4906.125488f, 9637.304688f);
		const Point Center = (Max + Min)*0.5f;
		const Point Extents = (Max - Min)*0.5f;

		BasicRandom Rnd(42);
		for(udword i=0;i<4096;i++)
		{
			Point p;
			UnitRandomPt(p, Rnd);
			Point Pos = Center + Extents*p;
//			Pos.y = 0.0f;
			Pos.y *= 0.1f;
			RegisterSphereOverlap(Sphere(Pos, 200.0f));
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphereOverlaps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchSphereOverlaps(*this, pint, OVERLAP_OBJECTS);
	}

END_TEST(OverlapObjects_SpheresVsKP)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapObjects_GiantSphereVsKP = "OVERLAP_OBJECTS: one single giant sphere against the KP level. Goal is to retrieve all meshes touched by the sphere. It's one of these rare tests where Opcode 1.3 still beats everybody, thanks to its 'containment tests' during traversal.";

START_SQ_TEST(OverlapObjects_GiantSphereVsKP, CATEGORY_OVERLAP, gDesc_OverlapObjects_GiantSphereVsKP)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(926.05f, 933.78f, 523.28f), Point(-0.03f, -0.00f, -1.00f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadMeshesFromFile_(*this, "kp.bin");

		const Point Min(-9560.175781f, -1093.885132f, -9461.288086f);
		const Point Max(9538.423828f, 4906.125488f, 9637.304688f);
		const Point Center = (Max + Min)*0.5f;
		const Point Extents = (Max - Min)*0.5f;

//		RegisterSphereOverlap(Sphere(Center, Extents.Magnitude()*0.25f));
		RegisterSphereOverlap(Sphere(Center, 2500.0f));
		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		udword Nb = GetNbRegisteredSphereOverlaps();
		PintSphereOverlapData* Data = GetRegisteredSphereOverlaps();
		const float Radius = 0.1f+(1.0f + sinf(mCurrentTime*1.0f))*1250.0f;
		while(Nb--)
		{
			Data->mSphere.mRadius = Radius;
			Data++;
		}
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphereOverlaps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchSphereOverlaps(*this, pint, OVERLAP_OBJECTS);
	}

END_TEST(OverlapObjects_GiantSphereVsKP)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapObjects_GiantBoxVsKP = "OVERLAP_OBJECTS: one single giant box against the KP level. Goal is to retrieve all meshes touched by the box.";

START_SQ_TEST(OverlapObjects_GiantBoxVsKP, CATEGORY_OVERLAP, gDesc_OverlapObjects_GiantBoxVsKP)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(926.05f, 933.78f, 523.28f), Point(-0.03f, -0.00f, -1.00f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadMeshesFromFile_(*this, "kp.bin");

		const Point Min(-9560.175781f, -1093.885132f, -9461.288086f);
		const Point Max(9538.423828f, 4906.125488f, 9637.304688f);
		const Point Center = (Max + Min)*0.5f;
		const Point Extents = (Max - Min)*0.5f;

		const Matrix3x3 Rot(Idt);
//		Rot.RotYX(0.1f, 0.2f);
		RegisterBoxOverlap(OBB(Center, Point(2500.0f, 2500.0f, 2500.0f), Rot));
		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		udword Nb = GetNbRegisteredBoxOverlaps();
		PintBoxOverlapData* Data = GetRegisteredBoxOverlaps();
		const float Radius = 0.1f+(1.0f + sinf(mCurrentTime*1.0f))*1250.0f;
		while(Nb--)
		{
			Data->mBox.mExtents = Point(Radius*0.75f, Radius*0.75f, Radius*0.75f);
			Data++;
		}
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportBoxOverlaps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchBoxOverlaps(*this, pint, OVERLAP_OBJECTS);
	}

END_TEST(OverlapObjects_GiantBoxVsKP)

///////////////////////////////////////////////////////////////////////////////

#define START_SQ_OVERLAP_TESTZONE_TEST(name, category, desc)											\
	START_SQ_TEST(name, category, desc)																	\
																										\
	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)												\
	{																									\
		TestBase::GetSceneParams(desc);																	\
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.44f, -0.57f, -0.69f));	\
		SetDefEnv(desc, false);																			\
	}

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SphereOverlapAny_TestZone = "TestZone. Sphere overlap any.";

START_SQ_OVERLAP_TESTZONE_TEST(SphereOverlapAny_TestZone, CATEGORY_OVERLAP, gDesc_SphereOverlapAny_TestZone)

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
			Point Center;
			T->Center(V, Center);
			RegisterSphereOverlap(Sphere(Center, 10.0f));
//			RegisterSphereOverlap(Sphere(Center, 1.0f));
//			RegisterSphereOverlap(Sphere(Center, 0.1f));
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphereOverlaps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchSphereOverlaps(*this, pint, OVERLAP_ANY);
	}

END_TEST(SphereOverlapAny_TestZone)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_BoxOverlapAny_TestZone = "TestZone. Box overlap any.";

START_SQ_OVERLAP_TESTZONE_TEST(BoxOverlapAny_TestZone, CATEGORY_OVERLAP, gDesc_BoxOverlapAny_TestZone)

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
		BasicRandom Rnd(42);
		for(udword i=0;i<IS->GetNbFaces();i++)
		{
			const IndexedTriangle* T = IS->GetFace(i);
			Point Center;
			T->Center(V, Center);

//			Center.y += 10.0f;

			if(0)
			{
				Point N;
				T->Normal(V, N);
				Center += N*0.4f;
//				Center += N*10.0f;
			}

			Quat Q;
			UnitRandomQuat(Q, Rnd);

//			RegisterBoxOverlap(OBB(Center, Point(0.1f, 0.1f, 0.1f), Q));
//			RegisterBoxOverlap(OBB(Center, Point(10.1f, 10.1f, 10.1f), Q));
			RegisterBoxOverlap(OBB(Center, Point(1.0f, 1.0f, 1.0f), Q));
//				RegisterBoxOverlap(OBB(Center, Point(0.01f, 0.01f, 0.01f), Q));
//			RegisterBoxOverlap(OBB(Center, Point(0.1f, 1.0f, 0.1f), Q));
//			RegisterBoxOverlap(OBB(Center, Point(10.0f, 100.0f, 10.0f), Q));
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportBoxOverlaps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchBoxOverlaps(*this, pint, OVERLAP_ANY);
	}

END_TEST(BoxOverlapAny_TestZone)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CapsuleOverlapAny_TestZone = "TestZone. Capsule overlap any.";

START_SQ_OVERLAP_TESTZONE_TEST(CapsuleOverlapAny_TestZone, CATEGORY_OVERLAP, gDesc_CapsuleOverlapAny_TestZone)

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
		BasicRandom Rnd(42);
		const float R = 1.0f;
//		const float R = 0.1f;
//		const float R = 10.0f;
		for(udword i=0;i<IS->GetNbFaces();i++)
		{
			const IndexedTriangle* T = IS->GetFace(i);
			Point Center;
			T->Center(V, Center);

//			Center.y += 10.0f;

			Quat Q;
			UnitRandomQuat(Q, Rnd);

			Matrix3x3 M = Q;
			const Point D = M[1]*4.0f;
//			const Point D = M[1]*2.0f;

			RegisterCapsuleOverlap(LSS(Segment(Center-D, Center+D), R));	// 1.95
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCapsuleOverlaps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchCapsuleOverlaps(*this, pint, OVERLAP_ANY);
	}

END_TEST(CapsuleOverlapAny_TestZone)

///////////////////////////////////////////////////////////////////////////////

#include "TestScenes_Heightfield.h"
#include "MyConvex.h"
#include "PintShapeRenderer.h"

class SceneOverlapAnyVsStaticHeightfield : public HeightfieldTest
{
	public:
								SceneOverlapAnyVsStaticHeightfield(udword scenario) : HeightfieldTest(64, 32), mRenderer(null), mScenario(scenario)	{}
	virtual						~SceneOverlapAnyVsStaticHeightfield()				{								}
	virtual	TestCategory		GetCategory()								const	{ return CATEGORY_OVERLAP;		}
	virtual	udword				GetProfilingFlags()							const	{ return PROFILING_TEST_UPDATE;	}

			MyConvex			mConvex;
			PintShapeRenderer*	mRenderer;
			udword				mScenario;

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

		//mHH.Init(SizeX, SizeZ, 4.0f);
		mHH.Init(SizeX, SizeZ, 0.0f);

		const Point Offset(0.0f, 0.0f, 0.0f);
		const Point Extents(SizeZ*0.5f, 0.0f, SizeX*0.5f);

		if(mScenario==0)
		{
			SetupBoxOverlaps(*this, 80, 80, Extents.x, Extents.z, 0.2f, 2.0f);
		}
		else if(mScenario==1)
		{
			SetupSphereOverlaps(*this, 80, 80, Extents.x, Extents.z, 0.2f, 2.0f);
		}
		else if(mScenario==2)
		{
			SetupCapsuleOverlaps(*this, 48, 48, Extents.x, Extents.z, 0.2f, 0.5f, 0.5f);
		}
		else if(mScenario==3)
		{
		//	udword i=2;	// Small convexes
			udword i=4;	// 'Big' convexes
		//	udword i=7;
		//	udword i=13;
			mConvex.LoadFile(i);

			PintShapeRenderer* renderer = CreateConvexRenderer(mConvex.mNbVerts, mConvex.mVerts);

			const udword nb_x = 48;
			const udword nb_y = 48;

			BasicRandom Rnd(42);

			const float OneOverNbX = OneOverNb(nb_x);
			const float OneOverNbY = OneOverNb(nb_y);
			for(udword y=0;y<nb_y;y++)
			{
				const float CoeffY = 2.0f * ((float(y)*OneOverNbY) - 0.5f);
				for(udword x=0;x<nb_x;x++)
				{
					const float CoeffX = 2.0f * ((float(x)*OneOverNbX) - 0.5f);

					const Point Origin(CoeffX * Extents.x, 0.2f, CoeffY * Extents.z);

					Quat Q;
					UnitRandomQuat(Q, Rnd);
					const Matrix3x3 Rot = Q;

					// Beware: we rely on the indices returned by pint.CreateConvexObject() here,
					// even though the functions haven't been called yet.
					RegisterConvexOverlap(0, renderer, PR(Origin, Rot));
				}
			}
		}
		return true;
	}

	virtual	void	CommonRelease()
	{
		HeightfieldTest::CommonRelease();
		mConvex.Release();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(mScenario==0)
		{
			if(!caps.mSupportBoxOverlaps)
				return false;
		}
		else if(mScenario==1)
		{
			if(!caps.mSupportSphereOverlaps)
				return false;
		}
		else if(mScenario==2)
		{
			if(!caps.mSupportCapsuleOverlaps)
				return false;
		}
		else if(mScenario==3)
		{
			if(!caps.mSupportConvexOverlaps)
				return false;

			PINT_CONVEX_DATA_CREATE Desc(mConvex.mNbVerts, mConvex.mVerts);
			udword h;
			const PintConvexHandle Handle = pint.CreateConvexObject(Desc, &h);
			ASSERT(Handle);
			ASSERT(h==0);
		}

		return mHH.Setup(pint, caps, Point(0.0f, 0.0f, 0.0f))!=null;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);

		if(mScenario==0)
		{
			udword Nb = GetNbRegisteredBoxOverlaps();
			PintBoxOverlapData* Data = GetRegisteredBoxOverlaps();
			const float Radius = 0.1f+(1.0f + sinf(mCurrentTime*1.0f))*0.5f;
			while(Nb--)
			{
				Data->mBox.mExtents = Point(Radius, Radius, Radius);
				Data++;
			}
		}
		else if(mScenario==1)
		{
			udword Nb = GetNbRegisteredSphereOverlaps();
			PintSphereOverlapData* Data = GetRegisteredSphereOverlaps();
			const float Radius = 0.1f+(1.0f + sinf(mCurrentTime*1.0f))*0.5f;
			while(Nb--)
			{
				Data->mSphere.mRadius = Radius;
				Data++;
			}
		}
		else if(mScenario==2)
		{
			udword Nb = GetNbRegisteredCapsuleOverlaps();
			PintCapsuleOverlapData* Data = GetRegisteredCapsuleOverlaps();
			const float Radius = 0.1f+(1.0f + sinf(mCurrentTime*1.0f))*0.5f;
			while(Nb--)
			{
				Data->mCapsule.mRadius = Radius;
				Data++;
			}
		}
		else if(mScenario==3)
		{
//#error todo
/*			udword Nb = GetNbRegisteredCapsuleOverlaps();
			PintCapsuleOverlapData* Data = GetRegisteredCapsuleOverlaps();
			const float Radius = 0.1f+(1.0f + sinf(mCurrentTime*1.0f))*0.5f;
			while(Nb--)
			{
				Data->mCapsule.mRadius = Radius;
				Data++;
			}*/

			/*{
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
			}*/

		}
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		if(mScenario==0)
			return DoBatchBoxOverlaps(*this, pint, OVERLAP_ANY);
		else if(mScenario==1)
			return DoBatchSphereOverlaps(*this, pint, OVERLAP_ANY);
		else if(mScenario==2)
			return DoBatchCapsuleOverlaps(*this, pint, OVERLAP_ANY);
		else if(mScenario==3)
			return DoBatchConvexOverlaps(*this, pint, OVERLAP_ANY);

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_OverlapAny_BoxesVsHeightfield = "OVERLAP_ANY: 4096 boxes against a heightfield";

class OverlapAny_BoxesVsHeightfield : public SceneOverlapAnyVsStaticHeightfield
{
	public:
								OverlapAny_BoxesVsHeightfield() : SceneOverlapAnyVsStaticHeightfield(0)		{}
	virtual						~OverlapAny_BoxesVsHeightfield()											{}
	virtual	const char*			GetName()			const	{ return "OverlapAny_BoxesVsHeightfield";		}
	virtual	const char*			GetDescription()	const	{ return gDesc_OverlapAny_BoxesVsHeightfield;	}

END_TEST(OverlapAny_BoxesVsHeightfield)

static const char* gDesc_OverlapAny_SpheresVsHeightfield = "OVERLAP_ANY: 4096 spheres against a heightfield";

class OverlapAny_SpheresVsHeightfield : public SceneOverlapAnyVsStaticHeightfield
{
	public:
								OverlapAny_SpheresVsHeightfield() : SceneOverlapAnyVsStaticHeightfield(1)	{}
	virtual						~OverlapAny_SpheresVsHeightfield()											{}
	virtual	const char*			GetName()			const	{ return "OverlapAny_SpheresVsHeightfield";		}
	virtual	const char*			GetDescription()	const	{ return gDesc_OverlapAny_SpheresVsHeightfield;	}

END_TEST(OverlapAny_SpheresVsHeightfield)

static const char* gDesc_OverlapAny_CapsulesVsHeightfield = "OVERLAP_ANY: 4096 capsules against a heightfield";

class OverlapAny_CapsulesVsHeightfield : public SceneOverlapAnyVsStaticHeightfield
{
	public:
								OverlapAny_CapsulesVsHeightfield() : SceneOverlapAnyVsStaticHeightfield(2)		{}
	virtual						~OverlapAny_CapsulesVsHeightfield()												{}
	virtual	const char*			GetName()			const	{ return "OverlapAny_CapsulesVsHeightfield";		}
	virtual	const char*			GetDescription()	const	{ return gDesc_OverlapAny_CapsulesVsHeightfield;	}

END_TEST(OverlapAny_CapsulesVsHeightfield)

static const char* gDesc_OverlapAny_ConvexesVsHeightfield = "OVERLAP_ANY: 4096 convexes against a heightfield";

class OverlapAny_ConvexesVsHeightfield : public SceneOverlapAnyVsStaticHeightfield
{
	public:
								OverlapAny_ConvexesVsHeightfield() : SceneOverlapAnyVsStaticHeightfield(3)		{}
	virtual						~OverlapAny_ConvexesVsHeightfield()												{}
	virtual	const char*			GetName()			const	{ return "OverlapAny_ConvexesVsHeightfield";		}
	virtual	const char*			GetDescription()	const	{ return gDesc_OverlapAny_ConvexesVsHeightfield;	}

END_TEST(OverlapAny_ConvexesVsHeightfield)

