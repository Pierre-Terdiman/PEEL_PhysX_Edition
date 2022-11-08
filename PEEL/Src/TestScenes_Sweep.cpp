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
#include "Loader_RepX.h"
#include "Loader_Bin.h"
#include "GUI_Helpers.h"
#include "GLRenderStates.h"

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ZigZagBug = "Zig-zag bug repro.";

START_SQ_TEST(ZigZagBug, CATEGORY_SWEEP, gDesc_ZigZagBug)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		SetDefEnv(desc, false);

		//const char* Filename = FindPEELFile("ZigZagyThing_triangulated.zb2");
		const char* Filename = FindPEELFile("ZigZagBug.zb2");
		if(Filename)
		{
			bool status = ImportZB2File(desc, Filename);
		}
		desc.mCamera[0] = PintCameraPose(Point(-0.24f, 4.00f, 2.79f), Point(0.05f, -0.77f, -0.64f));
	}

	virtual bool	CommonSetup()
	{
		const Point Dir(0.0f, -1.0f, 0.0f);
		const float CapsuleRadius = 0.5f;
		//const float HalfHeight = 0.5f;
		const float HalfHeight = 0.0f;

		//const Point Pos(22.76f, 4.433f, -21.43f);
		//const Point Pos(22.7555f, 4.433f, -21.43088f);

		const Point Pos(0.0f, 4.433f, 0.0f);
		const Point P0 = Pos + Point(0.0f, -HalfHeight, 0.0f);
		const Point P1 = Pos + Point(0.0f, HalfHeight, 0.0f);
		RegisterCapsuleSweep(LSS(Segment(P0, P1), CapsuleRadius), Dir, gSQMaxDist);

		return TestBase::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCapsuleSweeps || !mZB2Factory)
			return false;

		return CreateZB2Scene(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchCapsuleSweeps(*this, pint);
	}

END_TEST(ZigZagBug)

START_SQ_TEST(ZigZagBug2, CATEGORY_SWEEP, gDesc_ZigZagBug)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		SetDefEnv(desc, false);

		desc.mCamera[0] = PintCameraPose(Point(-0.24f, 4.00f, 2.79f), Point(0.05f, -0.77f, -0.64f));
	}

	virtual bool	CommonSetup()
	{
		const Point Dir(0.0f, -1.0f, 0.0f);
		const float CapsuleRadius = 0.5f;
		//const float HalfHeight = 0.5f;
		const float HalfHeight = 0.0f;

		//const Point Pos(22.76f, 4.433f, -21.43f);
		//const Point Pos(22.7555f, 4.433f, -21.43088f);

		const Point Pos(0.0f, 4.433f, 0.0f);
		const Point P0 = Pos + Point(0.0f, -HalfHeight, 0.0f);
		const Point P1 = Pos + Point(0.0f, HalfHeight, 0.0f);
		RegisterCapsuleSweep(LSS(Segment(P0, P1), CapsuleRadius), Dir, gSQMaxDist);

		return TestBase::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCapsuleSweeps)
			return false;

		const float Verts[] = 
		{
			8, 0, 0, 8, 0, -4,
			8, 0.25, 0, 8, 0.25, -4,
			0, 0, -4, 0, 0, 0,
			0, 0.25, -4, 0, 0.25, 0,
			0.5, 0, 0, 0.5, 0.75, 0,
			1, 0, 0, 1, 0.25, 0,
			0.5, 0.75, -4, 0.5, 0, -4,
			1, 0.25, -4, 1, 0, -4,
			7, 0, 0, 7.5, 0, 0,
			7.5, 0.75, 0, 7, 0.25, 0,
			7.5, 0.75, -4, 7, 0, -4,
			7, 0.25, -4, 7.5, 0, -4,
			1.5, 0, 0, 1.5, 0.75, 0,
			2, 0, 0, 2, 0.25, 0,
			1.5, 0.75, -4, 1.5, 0, -4,
			2, 0.25, -4, 2, 0, -4,
			3, 0, 0, 3.5, 0, 0,
			3.5, 0.75, 0, 3, 0.25, 0,
			4, 0, 0, 4, 0.25, 0,
			3.5, 0.75, -4, 3, 0, -4,
			3, 0.25, -4, 3.5, 0, -4,
			4, 0.25, -4, 4, 0, -4,
			5, 0, 0, 5.5, 0, 0,
			5.5, 0.75, 0, 5, 0.25, 0,
			6, 0, 0, 6, 0.25, 0,
			5.5, 0.75, -4, 5, 0, -4,
			5, 0.25, -4, 5.5, 0, -4,
			6, 0.25, -4, 6, 0, -4,
			6.5, 0, 0, 6.5, 0.75, 0,
			6.5, 0.75, -4, 6.5, 0, -4,
			4.5, 0, 0, 4.5, 0.75, 0,
			4.5, 0.75, -4, 4.5, 0, -4,
			2.5, 0, 0, 2.5, 0.75, 0,
			2.5, 0.75, -4, 2.5, 0, -4
		};
		const udword Indices[] = { 22, 58, 57 };

		const udword NbVerts = sizeof(Verts)/(sizeof(Verts[0])*3);

		const SurfaceInterface SI(NbVerts, (const Point*)Verts, 1, Indices, null);

		PINT_MESH_CREATE MeshDesc;
		MeshDesc.SetSurfaceData(SI);
		MeshDesc.mRenderer	= CreateMeshRenderer(MeshDesc.GetSurface());

		PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
		ObjectDesc.mPosition	= Point(-6.827755f, 0.0f, 4.382126f);
		ObjectDesc.mMass		= 0.0f;
		CreatePintObject(pint, ObjectDesc);
		return true;
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchCapsuleSweeps(*this, pint);
	}

END_TEST(ZigZagBug2)

///////////////////////////////////////////////////////////////////////////////

class SceneSweepVsShapes : public TestBase
{
	public:
							SceneSweepVsShapes() : mQueryShapeType(PINT_SHAPE_UNDEFINED), mSceneShapeType(PINT_SHAPE_UNDEFINED), mTypeData(4), mSize(32), mDynamic(false), mRotate(false)	{}
	virtual					~SceneSweepVsShapes()			{}
	virtual	TestCategory	GetCategory()			const	{ return CATEGORY_SWEEP;		}
	virtual	udword			GetProfilingFlags()		const	{ return PROFILING_TEST_UPDATE;	}

	virtual void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.59f, -0.55f, -0.59f));
		desc.mCamera[1] = PintCameraPose(Point(52.39f, 11.74f, 47.06f), Point(-0.69f, -0.17f, 0.70f));
		SetDefEnv(desc, false);
	}

	virtual bool			CommonSetup()
	{
		TestBase::CommonSetup();
		if(mQueryShapeType==PINT_SHAPE_BOX)
			return GenerateArrayOfVerticalBoxSweeps(*this, 50.0f, mSize, mSize, gSQMaxDist);
		if(mQueryShapeType==PINT_SHAPE_SPHERE)
			return GenerateArrayOfVerticalSphereSweeps(*this, 50.0f, mSize, mSize, gSQMaxDist);
		if(mQueryShapeType==PINT_SHAPE_CAPSULE)
			return GenerateArrayOfVerticalCapsuleSweeps(*this, 50.0f, mSize, mSize, gSQMaxDist);
		return true;
	}

	virtual bool			Setup(Pint& pint, const PintCaps& caps)
	{
		if(mQueryShapeType==PINT_SHAPE_BOX && !caps.mSupportBoxSweeps)
			return false;
		if(mQueryShapeType==PINT_SHAPE_SPHERE && !caps.mSupportSphereSweeps)
			return false;
		if(mQueryShapeType==PINT_SHAPE_CAPSULE && !caps.mSupportCapsuleSweeps)
			return false;

		const float Altitude = 10.0f;
		const float Scale = 50.0f;
		const float Mass = mDynamic ? 1.0f : 0.0f;
		return GenerateArrayOfObjects(pint, caps, mSceneShapeType, mTypeData, mSize, mSize, Altitude, Scale, Mass);
	}

	virtual void			CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		if(mRotate)
		{
			if(mQueryShapeType==PINT_SHAPE_BOX)
				UpdateBoxSweeps(*this, mCurrentTime);
//			if(mQueryShapeType==PINT_SHAPE_SPHERE)
//				UpdateSphereSweeps(*this, mCurrentTime);
			if(mQueryShapeType==PINT_SHAPE_CAPSULE)
				UpdateCapsuleSweeps(*this, mCurrentTime);
		}
	}

	virtual	udword			Update(Pint& pint, float dt)
	{
		if(mQueryShapeType==PINT_SHAPE_BOX)
			return DoBatchBoxSweeps(*this, pint);
		if(mQueryShapeType==PINT_SHAPE_SPHERE)
			return DoBatchSphereSweeps(*this, pint);
		if(mQueryShapeType==PINT_SHAPE_CAPSULE)
			return DoBatchCapsuleSweeps(*this, pint);
		return 0;
	}

			PintShape		mQueryShapeType;
			PintShape		mSceneShapeType;
			udword			mTypeData;
			udword			mSize;
			bool			mDynamic;
			bool			mRotate;
};

static const char* gDesc_SceneSweepsVsShapes = "A grid of shape-sweeps against a grid of static or dynamic shapes. Select undefined scene-shape to create an empty scene and measure the operating overhead of shape-sweeps.";

class ConfigurableSceneSweepVsShapes : public SceneSweepVsShapes
{
			ComboBoxPtr		mComboBox_ConvexIndex;
			ComboBoxPtr		mComboBox_QueryShapeType;
			ComboBoxPtr		mComboBox_SceneShapeType;
			EditBoxPtr		mEditBox_Size;
			CheckBoxPtr		mCheckBox_Dynamic;
			CheckBoxPtr		mCheckBox_Rotate;
	public:
							ConfigurableSceneSweepVsShapes()	{							}
	virtual					~ConfigurableSceneSweepVsShapes()	{							}
	virtual	const char*		GetName()			const	{ return "SceneSweepVsShapes";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_SceneSweepsVsShapes;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 200;
		WD.mLabel	= "SceneSweepVsShapes";
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
			mCheckBox_Dynamic = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Dynamic scene shapes", &UIElems, false, null, null);
			y += YStep;

			mCheckBox_Rotate = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Rotate swept shapes", &UIElems, true, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Grid size:", &UIElems);
			mEditBox_Size = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "32", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Query shape:", &UIElems);
			mComboBox_QueryShapeType = CreateShapeTypeComboBox(UI, 4+OffsetX, y, true, SSM_UNDEFINED|SSM_SPHERE|SSM_CAPSULE|SSM_BOX);
			RegisterUIElement(mComboBox_QueryShapeType);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Scene shape:", &UIElems);
			mComboBox_SceneShapeType = CreateShapeTypeComboBox(UI, 4+OffsetX, y, true, SSM_UNDEFINED|SSM_SPHERE|SSM_CAPSULE|SSM_BOX|SSM_CONVEX);
			RegisterUIElement(mComboBox_SceneShapeType);
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
		mRotate = mCheckBox_Rotate ? mCheckBox_Rotate->IsChecked() : false;

		mSize = GetInt(mSize, mEditBox_Size);

		if(mComboBox_QueryShapeType)
			mQueryShapeType = PintShape(mComboBox_QueryShapeType->GetSelectedIndex());

		if(mComboBox_SceneShapeType)
			mSceneShapeType = PintShape(mComboBox_SceneShapeType->GetSelectedIndex());

		if(mComboBox_ConvexIndex)
			mTypeData = ConvexIndex(mComboBox_ConvexIndex->GetSelectedIndex());

		return SceneSweepVsShapes::CommonSetup();
	}

}ConfigurableSceneSweepVsShapes;

///////////////////////////////////////////////////////////////////////////////

static PintShapeRenderer* CommonSetup_SceneConvexSweep(MyConvex& convex, TestBase& test, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_y, const Point& dir, const Point& offset, float max_dist, udword convex_id)
{
	convex.LoadFile(convex_id);

	PintShapeRenderer* renderer = CreateConvexRenderer(convex.mNbVerts, convex.mVerts);

	const Matrix3x3 Rot(Idt);

	const float OneOverNbX = OneOverNb(nb_x);
	const float OneOverNbY = OneOverNb(nb_y);
	for(udword y=0;y<nb_y;y++)
	{
		const float CoeffY = 2.0f * ((float(y)*OneOverNbY) - 0.5f);
		for(udword x=0;x<nb_x;x++)
		{
			const float CoeffX = 2.0f * ((float(x)*OneOverNbX) - 0.5f);

			const Point Origin(offset.x + CoeffX * scale_x, offset.y + altitude, offset.z + CoeffY * scale_y);

			test.RegisterConvexSweep(0, renderer, PR(Origin, Rot), dir, max_dist);
		}
	}
	return renderer;
}

static const char* gDesc_SceneConvexSweepVsStaticConvexes = "A grid of convex-sweeps against a grid of static convexes.";

class SceneConvexSweepVsStaticConvexes : public SceneSweepVsShapes
{
			MyConvex			mConvex;
			PintShapeRenderer*	mRenderer;

			ComboBoxPtr			mComboBox_ConvexIndex;
			EditBoxPtr			mEditBox_Size;
			sdword				mGridSize;
	public:
								SceneConvexSweepVsStaticConvexes() :
									mRenderer	(null),
									mGridSize	(0)
									{
										mSceneShapeType = PINT_SHAPE_CONVEX;
										mDynamic = false;
										mTypeData = CONVEX_INDEX_0;
									}

	virtual	const char*		GetName()			const	{ return "SceneConvexSweepVsStaticConvexes";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_SceneConvexSweepVsStaticConvexes;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 256;
		WD.mHeight	= 160;
		WD.mLabel	= "SceneConvexSweepVsStaticConvexes";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword OffsetX = 70;
		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 60;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			y += YStep;
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Convex:", &UIElems);
			mComboBox_ConvexIndex = CreateConvexObjectComboBox(UI, 4+OffsetX, y, true);
			RegisterUIElement(mComboBox_ConvexIndex);
			y += YStep;
		}

		{
			y += YStep;
			{
				helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Grid size:", &UIElems);
				mEditBox_Size = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "32", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
				y += YStep;
			}
		}

		y += YStep;
		AddResetButton(UI, 4, y, 256-16);

		return null;
	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual void				CommonRelease()
	{
		mConvex.Release();
		SceneSweepVsShapes::CommonRelease();
	}

	virtual bool				CommonSetup()
	{
		SceneSweepVsShapes::CommonSetup();

		ConvexIndex Index = CONVEX_INDEX_0;
		if(mComboBox_ConvexIndex)
			Index = ConvexIndex(mComboBox_ConvexIndex->GetSelectedIndex());
		mTypeData = Index;

		const udword Size = GetInt(32, mEditBox_Size);
		mGridSize = Size;

		const float Altitude = 30.0f;
		const Point Dir(0.0f, -1.0f, 0.0f);
		const Point Offset(0.0f, 0.0f, 0.0f);
		mRenderer = CommonSetup_SceneConvexSweep(mConvex, *this, Size, Size, Altitude, 50.0f, 50.0f, Dir, Offset, gSQMaxDist, mTypeData);
		return mRenderer!=null;
	}

	virtual bool				Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexSweeps)
			return false;

		PINT_CONVEX_DATA_CREATE Desc(mConvex.mNbVerts, mConvex.mVerts);
		udword h;
		const PintConvexHandle Handle = pint.CreateConvexObject(Desc, &h);
		ASSERT(Handle);
		ASSERT(h==0);

		return GenerateArrayOfConvexes(pint, caps, true, 50.0f, mGridSize, mGridSize, mTypeData);
	}

	virtual void				CommonUpdate(float dt)
	{
		SceneSweepVsShapes::CommonUpdate(dt);
		UpdateConvexSweeps(*this, mCurrentTime);
	}

	virtual udword				Update(Pint& pint, float dt)
	{
		return DoBatchConvexSweeps(*this, pint);
	}
}SceneConvexSweepVsStaticConvexes;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SweepAccuracy = "Tests the accuracy of linear sweep tests. Ideally all engines should return the same impact distance.";

START_SQ_TEST(SweepAccuracy, CATEGORY_SWEEP, gDesc_SweepAccuracy)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(8.55f, -0.99f, 0.74f), Point(-0.93f, 0.37f, -0.07f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		const Point Dir(0.0f, -1.0f, 0.0f);
		const float CapsuleRadius = 1.4f;
		const float HalfHeight = 1.8f;

		const Point P0(HalfHeight, 10.0f, HalfHeight);
		const Point P1(-HalfHeight, 10.0f, -HalfHeight);
		RegisterCapsuleSweep(LSS(Segment(P0, P1), CapsuleRadius), Dir, gSQMaxDist);

		return TestBase::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCapsuleSweeps)
			return false;

			const float Radius = 0.5f;
			const float HalfHeight_ = 1.0f;
	/*
			PINT_CAPSULE_CREATE ShapeDesc(Radius, HalfHeight_);
			ShapeDesc.mRenderer		= CreateCapsuleRenderer(Radius, HalfHeight_*2.0f);
	*/

	/*
			PINT_SPHERE_CREATE ShapeDesc(Radius);
			ShapeDesc.mRenderer		= CreateSphereRenderer(Radius);
	*/
			PINT_BOX_CREATE ShapeDesc(Radius, Radius, Radius);
			ShapeDesc.mRenderer		= CreateBoxRenderer(ShapeDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&ShapeDesc);
			ObjectDesc.mMass			= 0.0f;
			ObjectDesc.mPosition.Zero();
			ObjectDesc.mRotation = Quat(1.1f, 2.2f, 3.3f, 4.4f);
			ObjectDesc.mRotation.Normalize();
			CreatePintObject(pint, ObjectDesc);

		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);

		const float Offset = sinf(mCurrentTime)*50.0f;

		const float Altitude = 100.0f;
		const float HalfHeight = 1.8f;
		udword Nb = GetNbRegisteredCapsuleSweeps();
		PintCapsuleSweepData* Data = GetRegisteredCapsuleSweeps();
		while(Nb--)
		{
			const Point P0(1.3f + HalfHeight, Altitude+Offset, HalfHeight);
			const Point P1(1.3f + -HalfHeight, Altitude+Offset, -HalfHeight);

			Data->mCapsule.mP0 = P0;
			Data->mCapsule.mP1 = P1;
			Data++;
		}
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchCapsuleSweeps(*this, pint);
	}

END_TEST(SweepAccuracy)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SweepAccuracy2 = "Tests the accuracy of linear sweep tests. Ideally all engines should return the same impact distance.";

START_SQ_TEST(SweepAccuracy2, CATEGORY_SWEEP, gDesc_SweepAccuracy2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(10.42f, 0.02f, 5.22f), Point(-0.82f, 0.15f, -0.55f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		CreateSingleTriangleMesh(*this, 5000.0f);

		const Point Dir(0.0f, -1.0f, 0.0f);
		const float CapsuleRadius = 1.4f;
		const float HalfHeight = 1.8f;

		const Point P0(HalfHeight, 10.0f, HalfHeight);
		const Point P1(-HalfHeight, 10.0f, -HalfHeight);
		RegisterCapsuleSweep(LSS(Segment(P0, P1), CapsuleRadius), Dir, gSQMaxDist);

		return TestBase::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes || !caps.mSupportCapsuleSweeps)
			return false;

		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

	/*		const float Radius = 0.5f;
			const float HalfHeight_ = 1.0f;
			PINT_BOX_CREATE ShapeDesc(Radius, Radius, Radius);
			ShapeDesc.mRenderer		= CreateBoxRenderer(ShapeDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&ShapeDesc);
			ObjectDesc.mMass			= 0.0f;
			ObjectDesc.mPosition.Zero();
			ObjectDesc.mRotation = Quat(1.1f, 2.2f, 3.3f, 4.4f);
			ObjectDesc.mRotation.Normalize();
			CreatePintObject(pint, ObjectDesc);
	*/

		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);

		const float Offset = sinf(mCurrentTime)*50.0f;

		const float Altitude = 100.0f;
		const float HalfHeight = 1.8f;
		udword Nb = GetNbRegisteredCapsuleSweeps();
		PintCapsuleSweepData* Data = GetRegisteredCapsuleSweeps();
		while(Nb--)
		{
			const Point P0(1.3f + HalfHeight, Altitude+Offset, HalfHeight);
			const Point P1(1.3f + -HalfHeight, Altitude+Offset, -HalfHeight);

			Data->mCapsule.mP0 = P0;
			Data->mCapsule.mP1 = P1;
			Data++;
		}
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchCapsuleSweeps(*this, pint);
	}

END_TEST(SweepAccuracy2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneLongSweepVsSeaOfStatics = "Long diagonal shape-sweep against a sea of static boxes.";
class SceneLongSweepVsSeaOfStatics : public TestBase
{
			PintShape		mQueryShapeType;
			ComboBoxPtr		mComboBox_QueryShapeType;
	public:
							SceneLongSweepVsSeaOfStatics() :
								mQueryShapeType	(PINT_SHAPE_UNDEFINED)
																{}
	virtual					~SceneLongSweepVsSeaOfStatics()		{									}
	virtual	const char*		GetName()			const	{ return "SceneLongSweepVsSeaOfStatics";	}
	virtual	const char*		GetDescription()	const	{ return gDesc_SceneLongSweepVsSeaOfStatics;}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_SWEEP;					}
	virtual	udword			GetProfilingFlags()	const	{ return PROFILING_TEST_UPDATE;				}

	virtual void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(40.53f, 3.74f, 42.89f), Point(0.15f, -0.89f, -0.43f));
		desc.mCamera[1] = PintCameraPose(Point(54.50f, 10.80f, 72.83f), Point(-0.59f, -0.11f, -0.80f));
		SetDefEnv(desc, false);
	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 100;
		WD.mLabel	= "SceneLongSweepVsSeaOfStatics";
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
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Query shape:", &UIElems);
			mComboBox_QueryShapeType = CreateShapeTypeComboBox(UI, 4+OffsetX, y, true, SSM_UNDEFINED|SSM_SPHERE|SSM_CAPSULE|SSM_BOX);
			RegisterUIElement(mComboBox_QueryShapeType);
			mComboBox_QueryShapeType->Select(PINT_SHAPE_BOX);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

		return null;
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		if(mComboBox_QueryShapeType)
			mQueryShapeType = PintShape(mComboBox_QueryShapeType->GetSelectedIndex());

		if(mQueryShapeType==PINT_SHAPE_SPHERE)
		{
			Point Dir(1.f, 0.0f, 1.0f);
			Dir.Normalize();

			const Point Origin(50.0f, 0.0f, 50.0f);

			RegisterSphereSweep(Sphere(Origin, 1.0f), -Dir, 2000.0f);
			RegisterSphereSweep(Sphere(Origin + Point(0.0f, 10.0f, 0.0f), 1.0f), -Dir, 2000.0f);
		}
		else if(mQueryShapeType==PINT_SHAPE_CAPSULE)
		{
			Point Dir(1.f, 0.0f, 1.0f);
			Dir.Normalize();

			const Point Origin(50.0f, 0.0f, 50.0f);

			const Point Ext(2.0f, 0.0f, 0.0f);

			RegisterCapsuleSweep(LSS(Segment(Origin-Ext, Origin+Ext), 1.0f), -Dir, 2000.0f);

			const Point C = Origin + Point(0.0f, 10.0f, 0.0f);
			RegisterCapsuleSweep(LSS(Segment(C-Ext, C+Ext), 1.0f), -Dir, 2000.0f);
		}
		else if(mQueryShapeType==PINT_SHAPE_BOX)
		{
			Point Dir(1.f, 0.0f, 1.0f);
			Dir.Normalize();

			const Point Origin(50.0f, 0.0f, 50.0f);

			OBB Box(Origin, Point(1.0f, 1.0f, 1.0f), Get3x3IdentityMatrix());
			RegisterBoxSweep(Box, -Dir, 2000.0f);

			Box.mCenter		= Origin + Point(0.0f, 10.0f, 0.0f);
			RegisterBoxSweep(Box, -Dir, 2000.0f);
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(mQueryShapeType==PINT_SHAPE_SPHERE && !caps.mSupportSphereSweeps)
			return false;
		if(mQueryShapeType==PINT_SHAPE_CAPSULE && !caps.mSupportCapsuleSweeps)
			return false;
		if(mQueryShapeType==PINT_SHAPE_BOX && !caps.mSupportBoxSweeps)
			return false;

		return CreateSeaOfStaticBoxes(pint, 40.0f, 128, 128, 0.0f);
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		if(mQueryShapeType==PINT_SHAPE_CAPSULE)
			UpdateCapsuleSweeps(*this, mCurrentTime);
		else if(mQueryShapeType==PINT_SHAPE_BOX)
			UpdateBoxSweeps(*this, mCurrentTime);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		if(mQueryShapeType==PINT_SHAPE_SPHERE)
			return DoBatchSphereSweeps(*this, pint);
		if(mQueryShapeType==PINT_SHAPE_CAPSULE)
			return DoBatchCapsuleSweeps(*this, pint);
		if(mQueryShapeType==PINT_SHAPE_BOX)
			return DoBatchBoxSweeps(*this, pint);
		return 0;
	}

}SceneLongSweepVsSeaOfStatics;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneShapeSweepVsSingleTriangle = "One shape-sweep against a single triangle, various configurations.";
class ConfigurableSceneSweepVsSingleTriangle : public TestBase
{
	enum TestMode
	{
		TEST_VERTICAL_SWEEP,
		TEST_PARALLEL_SWEEP,
		TEST_INITIAL_OVERLAP,
	};

			MyConvex			mConvex;
			PintShapeRenderer*	mRenderer;

			Triangle			mTriangle;
			PintShape			mQueryShapeType;
			TestMode			mTestMode;
			ComboBoxPtr			mComboBox_QueryShapeType;
			ComboBoxPtr			mComboBox_TestMode;
			CheckBoxPtr			mCheckBox_DoubleSided;
			bool				mDoubleSided;
	public:
							ConfigurableSceneSweepVsSingleTriangle() :
										mRenderer		(null),
										mQueryShapeType	(PINT_SHAPE_UNDEFINED),
										mTestMode		(TEST_VERTICAL_SWEEP),
										mDoubleSided	(false)
																		{								}
	virtual					~ConfigurableSceneSweepVsSingleTriangle()	{								}
	virtual	const char*		GetName()			const	{ return "SceneSweepVsSingleTriangle";			}
	virtual	const char*		GetDescription()	const	{ return gDesc_SceneShapeSweepVsSingleTriangle;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_SWEEP;						}
	virtual	udword			GetProfilingFlags()	const	{ return PROFILING_TEST_UPDATE;					}

	virtual void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(0.90f, 1.15f, 6.40f), Point(-0.13f, -0.03f, -0.99f));
		SetDefEnv(desc, false);
	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 160;
		WD.mLabel	= "SceneSweepVsSingleTriangle";
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
		sdword y = 10;
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Configuration:", &UIElems);
			{
				IceComboBox* CB = CreateComboBox<IceComboBox>(UI, 0, 4+OffsetX, y, 150, 20, "Configuration", &UIElems, null);
				CB->Add("Vertical sweep");
				CB->Add("Parallel sweep");
				CB->Add("Initial overlap");
				CB->Select(TEST_VERTICAL_SWEEP);
				mComboBox_TestMode = CB;
			}
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Query shape:", &UIElems);
			mComboBox_QueryShapeType = CreateShapeTypeComboBox(UI, 4+OffsetX, y, true, SSM_UNDEFINED|SSM_SPHERE|SSM_CAPSULE|SSM_BOX|SSM_CONVEX);
			RegisterUIElement(mComboBox_QueryShapeType);
			mComboBox_QueryShapeType->Select(PINT_SHAPE_CAPSULE);
			y += YStep;

			mCheckBox_DoubleSided = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Flip triangle winding", &UIElems, false, null, null);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

		return null;
	}

	virtual void				CommonRelease()
	{
		mConvex.Release();
		TestBase::CommonRelease();
	}

	virtual bool			CommonSetup()
	{
		mDoubleSided = mCheckBox_DoubleSided ? mCheckBox_DoubleSided->IsChecked() : false;

		CreateSingleTriangleMesh(*this, 4.0f, &mTriangle, mDoubleSided);

		if(mComboBox_QueryShapeType)
			mQueryShapeType = PintShape(mComboBox_QueryShapeType->GetSelectedIndex());

		if(mComboBox_TestMode)
			mTestMode = TestMode(mComboBox_TestMode->GetSelectedIndex());

		if(mQueryShapeType==PINT_SHAPE_BOX)
		{
			const Matrix3x3 Rot(Idt);
			Point Dir, P;
			if(mTestMode==TEST_VERTICAL_SWEEP)
			{
				Dir = Point(0.0f, -1.0f, 0.0f);
				P = Point(0.0f, 10.0f, 0.0f);
			}
			else if(mTestMode==TEST_PARALLEL_SWEEP)
			{
				Dir = Point(1.0f, 0.0f, 0.0f);
				P = Point(-10.0f, 0.1f, 0.0f);
			}
			else if(mTestMode==TEST_INITIAL_OVERLAP)
			{
				Dir = Point(0.0f, -1.0f, 0.0f);
				P = Point(0.0f, 0.1f, 0.0f);
			}
			RegisterBoxSweep(OBB(P, Point(0.4f, 2.0f, 0.4f), Rot), Dir, gSQMaxDist);
		}
		else if(mQueryShapeType==PINT_SHAPE_SPHERE)
		{
			if(0)
			{
				const Triangle& T = mTriangle;

				Point TriCenter;
				T.Center(TriCenter);

				const Point D0 = (T.mVerts[0] - TriCenter).Normalize();
				const Point D1 = (T.mVerts[1] - TriCenter).Normalize();
				const Point D2 = (T.mVerts[2] - TriCenter).Normalize();

				const Point E01 = (T.mVerts[0] + T.mVerts[1])*0.5f;
				const Point E12 = (T.mVerts[1] + T.mVerts[2])*0.5f;
				const Point E20 = (T.mVerts[2] + T.mVerts[0])*0.5f;

				const Point D01 = (E01 - TriCenter).Normalize();
				const Point D12 = (E12 - TriCenter).Normalize();
				const Point D20 = (E20 - TriCenter).Normalize();
			}

			const float Radius = 0.4f;
			if(mTestMode==TEST_VERTICAL_SWEEP)
			{
				const Point Dir(0.0f, -1.0f, 0.0f);
				const Point P(0.0f, 10.0f, 0.0f);
				RegisterSphereSweep(Sphere(P, Radius), Dir, gSQMaxDist);
			}
			else if(mTestMode==TEST_PARALLEL_SWEEP)
			{
				const Point Dir(1.0f, 0.0f, 0.0f);
				const Point P(-10.0f, 0.1f, 0.0f);
				RegisterSphereSweep(Sphere(P, Radius), Dir, gSQMaxDist);
			}
			else if(mTestMode==TEST_INITIAL_OVERLAP)
			{
		//		const Point Dir(0.0f, -1.0f, 0.0f);
				Point Dir(-1.0f, -1.0f, -1.0f);
				Dir.Normalize();

				const Point P(0.0f, 0.1f, 0.0f);
		//		const Point P(0.0f, -0.1f, 0.0f);
		//		const Point P(0.0f, -Radius+0.01f, 0.0f);
		//		const Point P(0.0f, -Radius-0.1f, 0.0f);
		//		const Point P = (T.mVerts[0] + D0*0.2f) + Point(0.0f, 0.1f, 0.0f);
		//		const Point P = (T.mVerts[1] + D1*0.2f) + Point(0.0f, 0.1f, 0.0f);
		//		const Point P = (E01 + D01*0.2f) + Point(0.0f, 0.1f, 0.0f);
				RegisterSphereSweep(Sphere(P, Radius), Dir, gSQMaxDist);

				BasicRandom Rnd(42);
				for(udword i=0;i<4095;i++)
				{
					UnitRandomPt(Dir, Rnd);
					Dir.y = -fabsf(Dir.y);
					RegisterSphereSweep(Sphere(P, Radius), Dir, gSQMaxDist);
				}
			}
		}
		else if(mQueryShapeType==PINT_SHAPE_CAPSULE)
		{
			Point Dir, P0, P1;
//			const float CapsuleRadius = 1.4f;
			const float CapsuleRadius = 0.4f;
			const float HalfHeight = 1.8f;
			const float CapsuleAltitude = 0.1f;
//			const float CapsuleAltitude = 1.5f;
			if(mTestMode==TEST_VERTICAL_SWEEP)
			{
				Dir = Point(0.0f, -1.0f, 0.0f);
				P0 = Point(HalfHeight, 10.0f, HalfHeight);
				P1 = Point(-HalfHeight, 10.0f, -HalfHeight);
			}
			else if(mTestMode==TEST_PARALLEL_SWEEP)
			{
				Dir = Point(1.0f, 0.0f, 0.0f);
				P0 = Point(HalfHeight-10.0f, CapsuleAltitude, HalfHeight);
				P1 = Point(-HalfHeight-10.0f, CapsuleAltitude, -HalfHeight);
			}
			else if(mTestMode==TEST_INITIAL_OVERLAP)
			{
				Dir = Point(0.0f, -1.0f, 0.0f);
				P0 = Point(HalfHeight, CapsuleAltitude, HalfHeight);
				P1 = Point(-HalfHeight, CapsuleAltitude, -HalfHeight);
			}
			RegisterCapsuleSweep(LSS(Segment(P0, P1), CapsuleRadius), Dir, gSQMaxDist);
		}
		else if(mQueryShapeType==PINT_SHAPE_CONVEX)
		{
			const Matrix3x3 Rot(Idt);
			Point Dir, P;
			if(mTestMode==TEST_VERTICAL_SWEEP)
			{
				Dir = Point(0.0f, -1.0f, 0.0f);
				P = Point(0.0f, 10.0f, 0.0f);
			}
			else if(mTestMode==TEST_PARALLEL_SWEEP)
			{
				Dir = Point(1.0f, 0.0f, 0.0f);
				P = Point(-10.0f, 0.1f, 0.0f);
			}
			else if(mTestMode==TEST_INITIAL_OVERLAP)
			{
				Dir = Point(0.0f, -1.0f, 0.0f);
				P = Point(0.0f, 0.1f, 0.0f);
			}

		//	udword i=2;	// Small convexes
			udword i=4;	// 'Big' convexes
		//	udword i=7;
		//	udword i=13;
			mConvex.LoadFile(i);

			// There's a design issue for convex sweeps. The sweeps now need data from each Pint (the convex mesh), which
			// was not the case before. Creating things in CommonSetup() doesn't work since there's no Pint pointer. Creating
			// things in Setup() doesn't work since we UnregisterAllConvexSweeps() and the previous Pint-provided objects are
			// lost. We end up calling all Pint plugins with the convex data from the last Pint. Which happens to work just
			// fine between PhysX 3.3 and 3.4, which is mental (we send a PxConvexMesh created in the 3.3 plugin to the 3.4 SDK
			// and things just work. Madness!)
			//
			// The workaround is to create things in CommonSetup while using the future convex indices created later in Setup.
			// For example here we use "0", and we expect "pint.CreateConvexObject()" to later return 0 for each PINT plugin.
			mRenderer = CreateConvexRenderer(mConvex.mNbVerts, mConvex.mVerts);
			RegisterConvexSweep(0, mRenderer, PR(P, Rot), Dir, gSQMaxDist);
		}
		return TestBase::CommonSetup();
	}

	virtual bool			Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

		if(mQueryShapeType==PINT_SHAPE_BOX)
			return caps.mSupportBoxSweeps;

		if(mQueryShapeType==PINT_SHAPE_SPHERE)
			return caps.mSupportSphereSweeps;

		if(mQueryShapeType==PINT_SHAPE_CAPSULE)
			return caps.mSupportCapsuleSweeps;

		if(mQueryShapeType==PINT_SHAPE_CONVEX)
		{
			if(!caps.mSupportConvexSweeps)
				return false;

			PINT_CONVEX_DATA_CREATE Desc(mConvex.mNbVerts, mConvex.mVerts);
			udword h;
			const PintConvexHandle Handle = pint.CreateConvexObject(Desc, &h);
			ASSERT(Handle);
			ASSERT(h==0);
			return true;
		}

		return false;
	}

	virtual void			CommonUpdate(float dt)
	{
		mCurrentTime += 0.5f/60.0f;
//		mCurrentTime = 20.0f/60.0f;		// Enable this for the static version corresponding to the debug render code
		if(mQueryShapeType==PINT_SHAPE_BOX)
			UpdateBoxSweeps(*this, mCurrentTime);
//		else if(mQueryShapeType==PINT_SHAPE_SPHERE)
//			UpdateSphereSweeps(*this, mCurrentTime);
		else if(mQueryShapeType==PINT_SHAPE_CAPSULE)
			UpdateCapsuleSweeps(*this, mCurrentTime);
		else if(mQueryShapeType==PINT_SHAPE_CONVEX)
			UpdateConvexSweeps(*this, mCurrentTime);
	}

	virtual udword			Update(Pint& pint, float dt)
	{
		if(mQueryShapeType==PINT_SHAPE_BOX)
			return DoBatchBoxSweeps(*this, pint);
		if(mQueryShapeType==PINT_SHAPE_SPHERE)
			return DoBatchSphereSweeps(*this, pint);
		if(mQueryShapeType==PINT_SHAPE_CAPSULE)
			return DoBatchCapsuleSweeps(*this, pint);
		if(mQueryShapeType==PINT_SHAPE_CONVEX)
			return DoBatchConvexSweeps(*this, pint);
		return 0;
	}

	virtual	void			CommonDebugRender(PintRender& renderer)
	{
		const Point Color(1.0f, 1.0f, 1.0f);
		renderer.DrawLine(mTriangle.mVerts[0], mTriangle.mVerts[1], Color);
		renderer.DrawLine(mTriangle.mVerts[1], mTriangle.mVerts[2], Color);
		renderer.DrawLine(mTriangle.mVerts[2], mTriangle.mVerts[0], Color);
		if(mDoubleSided)
//			renderer.DrawTriangle(mTriangle.mVerts[0], mTriangle.mVerts[1], mTriangle.mVerts[2], Color);
			renderer.DrawTriangle(mTriangle.mVerts[0], mTriangle.mVerts[2], mTriangle.mVerts[1], Color);

		if(0)	// Debug render code drawing extruded triangle for the capsule case / TEST_INITIAL_OVERLAP
		{
			const Point v0(-2.666666f, 0.0f, -1.333333f);
			const Point v1(1.333333f, 0.0f, 2.666666f);
			const Point v2(1.333333f, 0.0f, -1.333333f);

//			const Point ExtrusionDir(2.4054673f, 0.83290195f, 0.0f);

			PintCapsuleSweepData* Data = GetRegisteredCapsuleSweeps();
			const Point ExtrusionDir = (Data->mCapsule.mP0 - Data->mCapsule.mP1)*0.5f;
			const Point Center = (Data->mCapsule.mP0 + Data->mCapsule.mP1)*0.5f;
			renderer.DrawLine(Center, Center+Point(10.0f, 0.0f, 0.0f), Point(0.5f, 0.5f, 0.5f));
			renderer.DrawLine(Center, Center+Point(0.0f, 10.0f, 0.0f), Point(0.5f, 0.5f, 0.5f));
			renderer.DrawLine(Center, Center+Point(0.0f, 0.0f, 10.0f), Point(0.5f, 0.5f, 0.5f));

			const Point p0 = v0 - ExtrusionDir;
			const Point p1 = v1 - ExtrusionDir;
			const Point p2 = v2 - ExtrusionDir;

			const Point p0b = v0 + ExtrusionDir;
			const Point p1b = v1 + ExtrusionDir;
			const Point p2b = v2 + ExtrusionDir;

			renderer.DrawTriangle(v0, v1, v2, Point(0.0f, 1.0f, 0.0f));
			renderer.DrawTriangle(p0, p1, p2, Point(1.0f, 0.0f, 0.0f));
			renderer.DrawTriangle(p0b, p1b, p2b, Point(0.0f, 0.0f, 1.0f));

			renderer.DrawTriangle(p1, p1b, p2b, Point(0.0f, 1.0f, 1.0f));
			renderer.DrawTriangle(p1, p2b, p2, Point(0.0f, 1.0f, 1.0f));

			renderer.DrawTriangle(p0, p2, p2b, Point(1.0f, 1.0f, 0.0f));
			renderer.DrawTriangle(p0, p2b, p0b, Point(1.0f, 1.0f, 0.0f));

			renderer.DrawTriangle(p0b, p1b, p1, Point(1.0f, 0.0f, 1.0f));
			renderer.DrawTriangle(p0b, p1, p0, Point(1.0f, 0.0f, 1.0f));
		}

		if(0)	// Debug render code drawing extruded triangle for the capsule case / TEST_PARALLEL_SWEEP
		{
			const Point v0(-2.666666f, 0.0f, -1.333333f);
			const Point v1(1.333333f, 0.0f, 2.666666f);
			const Point v2(1.333333f, 0.0f, -1.333333f);

//			const Point ExtrusionDir(2.4054673f, 0.83290195f, 0.0f);

			PintCapsuleSweepData* Data = GetRegisteredCapsuleSweeps();
			const Point ExtrusionDir = (Data->mCapsule.mP0 - Data->mCapsule.mP1)*0.5f;
			const Point Center = (Data->mCapsule.mP0 + Data->mCapsule.mP1)*0.5f;
			renderer.DrawLine(Center, Center+Point(10.0f, 0.0f, 0.0f), Point(0.5f, 0.5f, 0.5f));
			renderer.DrawLine(Center, Center+Point(0.0f, 10.0f, 0.0f), Point(0.5f, 0.5f, 0.5f));
			renderer.DrawLine(Center, Center+Point(0.0f, 0.0f, 10.0f), Point(0.5f, 0.5f, 0.5f));

			const Point p0 = v0 - ExtrusionDir;
			const Point p1 = v1 - ExtrusionDir;
			const Point p2 = v2 - ExtrusionDir;

			const Point p0b = v0 + ExtrusionDir;
			const Point p1b = v1 + ExtrusionDir;
			const Point p2b = v2 + ExtrusionDir;

			renderer.DrawTriangle(v0, v1, v2, Point(0.0f, 1.0f, 0.0f));
			renderer.DrawTriangle(p0, p1, p2, Point(1.0f, 0.0f, 0.0f));
			renderer.DrawTriangle(p0b, p1b, p2b, Point(0.0f, 0.0f, 1.0f));

			renderer.DrawTriangle(p1, p1b, p2b, Point(0.0f, 1.0f, 1.0f));
			renderer.DrawTriangle(p1, p2b, p2, Point(0.0f, 1.0f, 1.0f));

			renderer.DrawTriangle(p0, p2, p2b, Point(1.0f, 1.0f, 0.0f));
			renderer.DrawTriangle(p0, p2b, p0b, Point(1.0f, 1.0f, 0.0f));

			renderer.DrawTriangle(p0b, p1b, p1, Point(1.0f, 0.0f, 1.0f));
			renderer.DrawTriangle(p0b, p1, p0, Point(1.0f, 0.0f, 1.0f));
		}
	}

}ConfigurableSceneSweepVsSingleTriangle;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneBoxSweepVsStaticMeshes_Archipelago = "32*32 box-sweeps against the Archipelago mesh level.";

START_SQ_TEST(SceneBoxSweepVsStaticMeshes_Archipelago, CATEGORY_SWEEP, gDesc_SceneBoxSweepVsStaticMeshes_Archipelago)

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
		const float ScaleX = Extents.x - 1.0f;
		const float ScaleY = Extents.z - 1.0f;
		const float Altitude = 30.0f;
		const Point Dir(0.0f, -1.0f, 0.0f);
		const Point BoxExtents(1.2f, 0.5f, 0.5f);
		RegisterArrayOfBoxSweeps(*this, NbX, NbY, Altitude, ScaleX, ScaleY, Dir, BoxExtents, Offset, gSQMaxDist);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportBoxSweeps)
			return false;

		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		UpdateBoxSweeps(*this, mCurrentTime);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchBoxSweeps(*this, pint);
	}

END_TEST(SceneBoxSweepVsStaticMeshes_Archipelago)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneBoxSweepVsStaticMeshes_KP = "32*32 box-sweeps against the Konoko Payne mesh level.";

START_SQ_TEST(SceneBoxSweepVsStaticMeshes_KP, CATEGORY_SWEEP, gDesc_SceneBoxSweepVsStaticMeshes_KP)

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

		const OBB Box(Point(50.0f, 50.0f, 50.0f), Point(1.2f, 0.5f, 0.5f), Get3x3IdentityMatrix());

		BasicRandom Rnd(42);
		for(udword i=0;i<1024;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);

			RegisterBoxSweep(Box, Dir, gSQMaxDist);
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportBoxSweeps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		UpdateBoxSweeps(*this, mCurrentTime);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchBoxSweeps(*this, pint);
	}

END_TEST(SceneBoxSweepVsStaticMeshes_KP)

///////////////////////////////////////////////////////////////////////////////

static bool CommonSetup_SceneBoxSweepVsStaticMeshes_TessBunny(TestBase& test, const Point& extents, udword nb_sweeps, const Matrix3x3& rot)
{
	GLRenderStates::SetDefaultCullMode(CULL_NONE);

	LoadMeshesFromFile_(test, "Bunny.bin", null, false, 3);

	BasicRandom Rnd(42);

	Point Center, Extents;
	test.GetGlobalBounds(Center, Extents);

	for(udword i=0;i<nb_sweeps;i++)
	{
		Point Dir;
		UnitRandomPt(Dir, Rnd);

		const Point Origin = Center + Dir * 20.0f;
		const OBB Box(Origin, extents, rot);
		test.RegisterBoxSweep(Box, -Dir, 200.0f);
//		if(i>=12 && i<15)
//		if(i==13)
//		test.RegisterBoxSweep(Box, -Dir, 20.0f);	//###MEGADEBUG
	}
	return true;
}

static bool Setup_SceneBoxSweepVsStaticMeshes_TessBunny(TestBase& test, Pint& pint, const PintCaps& caps/*, const Point& extents, udword nb_sweeps, const Matrix3x3& rot*/)
{
	if(!caps.mSupportBoxSweeps)
		return false;

	return test.CreateMeshesFromRegisteredSurfaces(pint, caps);
}

static const char* gDesc_SceneBoxSweepVsStaticMeshes_TessBunny_Test1 = "64 radial box-sweeps against the tessellated bunny. Each box is a (1, 1, 1) cube. Box orientation is identity.";

START_SQ_TEST(SceneBoxSweepVsStaticMeshes_TessBunny_Test1, CATEGORY_SWEEP, gDesc_SceneBoxSweepVsStaticMeshes_TessBunny_Test1)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		const Matrix3x3 Rot(Idt);
		return CommonSetup_SceneBoxSweepVsStaticMeshes_TessBunny(*this, Point(1.0f, 1.0f, 1.0f), 64, Rot);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return Setup_SceneBoxSweepVsStaticMeshes_TessBunny(*this, pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchBoxSweeps(*this, pint);
	}

END_TEST(SceneBoxSweepVsStaticMeshes_TessBunny_Test1)

static const char* gDesc_SceneBoxSweepVsStaticMeshes_TessBunny_Test1b = "4096 radial box-sweeps against the tessellated bunny. Each box is a (1, 1, 1) cube. Box orientation is identity.";

START_SQ_TEST(SceneBoxSweepVsStaticMeshes_TessBunny_Test1b, CATEGORY_SWEEP, gDesc_SceneBoxSweepVsStaticMeshes_TessBunny_Test1b)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		const Matrix3x3 Rot(Idt);
		return CommonSetup_SceneBoxSweepVsStaticMeshes_TessBunny(*this, Point(1.0f, 1.0f, 1.0f), 4096, Rot);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return Setup_SceneBoxSweepVsStaticMeshes_TessBunny(*this, pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchBoxSweeps(*this, pint);
	}

END_TEST(SceneBoxSweepVsStaticMeshes_TessBunny_Test1b)

static const char* gDesc_SceneBoxSweepVsStaticMeshes_TessBunny_Test2 = "256 radial box-sweeps against the tessellated bunny. Each box is a (1, 1, 1) cube. Box orientation is non-identity.";

START_SQ_TEST(SceneBoxSweepVsStaticMeshes_TessBunny_Test2, CATEGORY_SWEEP, gDesc_SceneBoxSweepVsStaticMeshes_TessBunny_Test2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		Matrix3x3 RotX;
		RotX.RotX(degToRad(45.0f));

		Matrix3x3 RotY;
		RotY.RotY(degToRad(45.0f));

		RotX *= RotY;

		return CommonSetup_SceneBoxSweepVsStaticMeshes_TessBunny(*this, Point(1.0f, 1.0f, 1.0f), 256, RotX);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return Setup_SceneBoxSweepVsStaticMeshes_TessBunny(*this, pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchBoxSweeps(*this, pint);
	}

END_TEST(SceneBoxSweepVsStaticMeshes_TessBunny_Test2)

static const char* gDesc_SceneBoxSweepVsStaticMeshes_TessBunny_Test3 = "A single radial box-sweep against the tessellated bunny. Box is a (1, 1, 1) cube. Box orientation is non-identity.";

START_SQ_TEST(SceneBoxSweepVsStaticMeshes_TessBunny_Test3, CATEGORY_SWEEP, gDesc_SceneBoxSweepVsStaticMeshes_TessBunny_Test3)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		Matrix3x3 RotX;
		RotX.RotX(degToRad(45.0f));

		Matrix3x3 RotY;
		RotY.RotY(degToRad(45.0f));

		RotX *= RotY;

		return CommonSetup_SceneBoxSweepVsStaticMeshes_TessBunny(*this, Point(1.0f, 1.0f, 1.0f), 1, RotX);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return Setup_SceneBoxSweepVsStaticMeshes_TessBunny(*this, pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchBoxSweeps(*this, pint);
	}

END_TEST(SceneBoxSweepVsStaticMeshes_TessBunny_Test3)

static const char* gDesc_SceneBoxSweepVsStaticMeshes_TessBunny2_Test4 = "256 radial box-sweeps against the tessellated bunny. Each box is a (0.1, 0.1, 0.1) cube. Box orientation is identity.";

START_SQ_TEST(SceneBoxSweepVsStaticMeshes_TessBunny_Test4, CATEGORY_SWEEP, gDesc_SceneBoxSweepVsStaticMeshes_TessBunny2_Test4)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.43f, -0.58f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		const Matrix3x3 Rot(Idt);
		return CommonSetup_SceneBoxSweepVsStaticMeshes_TessBunny(*this, Point(0.1f, 0.1f, 0.1f), 256, Rot);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return Setup_SceneBoxSweepVsStaticMeshes_TessBunny(*this, pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchBoxSweeps(*this, pint);
	}

END_TEST(SceneBoxSweepVsStaticMeshes_TessBunny_Test4)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneSphereSweepVsStaticMeshes_Archipelago = "32*32 sphere-sweeps against the Archipelago mesh level.";

START_SQ_TEST(SceneSphereSweepVsStaticMeshes_Archipelago, CATEGORY_SWEEP, gDesc_SceneSphereSweepVsStaticMeshes_Archipelago)

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
		const float ScaleX = Extents.x - 1.0f;
		const float ScaleY = Extents.z - 1.0f;
		const float Altitude = 30.0f;
		const Point Dir(0.0f, -1.0f, 0.0f);
		const float Radius = 0.75f;
		RegisterArrayOfSphereSweeps(*this, NbX, NbY, Altitude, ScaleX, ScaleY, Dir, Radius, Offset, gSQMaxDist);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphereSweeps)
			return false;

		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

		return true;
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchSphereSweeps(*this, pint);
	}

END_TEST(SceneSphereSweepVsStaticMeshes_Archipelago)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneSphereSweepVsStaticMeshes_KP = "32*32 sphere-sweeps against the Konoko Payne mesh level.";

START_SQ_TEST(SceneSphereSweepVsStaticMeshes_KP, CATEGORY_SWEEP, gDesc_SceneSphereSweepVsStaticMeshes_KP)

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

		const float Radius = 0.75f;
		const Sphere S(Point(50.0f, 50.0f, 50.0f), Radius);

		BasicRandom Rnd(42);
		for(udword i=0;i<1024;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);

			RegisterSphereSweep(S, Dir, gSQMaxDist);
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphereSweeps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchSphereSweeps(*this, pint);
	}

END_TEST(SceneSphereSweepVsStaticMeshes_KP)

///////////////////////////////////////////////////////////////////////////////

class SceneSphereSweepVsStaticMeshes_TessBunny_Base : public TestBase
{
	public:
							SceneSphereSweepVsStaticMeshes_TessBunny_Base() : mCurrentStartDistance(0.0f), mCurrentSphereRadius(0.0f)	{}
	virtual					~SceneSphereSweepVsStaticMeshes_TessBunny_Base()															{}
	virtual	TestCategory	GetCategory()								const	{ return CATEGORY_SWEEP;		}
	virtual	udword			GetProfilingFlags()							const	{ return PROFILING_TEST_UPDATE;	}

			EditBoxPtr		mEditBox_SweepStartDistance;
			EditBoxPtr		mEditBox_SphereRadius;
			float			mCurrentStartDistance;
			float			mCurrentSphereRadius;

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)	override
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 350;
		WD.mHeight	= 150;
		WD.mLabel	= "SceneSphereSweepVsStaticMeshes_TessBunny";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 100;
		const sdword LabelWidth = 120;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;

		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Sweep start distance:", &UIElems);
			mEditBox_SweepStartDistance = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "20", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Sphere radius:", &UIElems);
			mEditBox_SphereRadius = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, _F("%.2f", mCurrentSphereRadius), &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;
		}

		{
			y += YStep;
			AddResetButton(UI, 4, y, WD.mWidth-4*2*2);
		}
		return null;
	}

	bool CommonSetupBase(float radius)
	{
		GLRenderStates::SetDefaultCullMode(CULL_NONE);

		LoadMeshesFromFile_(*this, "Bunny.bin", null, false, 3);
		//LoadMeshesFromFile_(*this, "Bunny.bin", null, false, 0);

		BasicRandom Rnd(42);

		Point Center, Extents;
		GetGlobalBounds(Center, Extents);

		const float StartDistance = GetFloat(20.0f, mEditBox_SweepStartDistance);
		mCurrentStartDistance = StartDistance;
		mCurrentSphereRadius = radius;
		mEditBox_SphereRadius->SetText(_F("%.2f", radius));

		for(udword i=0;i<64;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);

			const Point Origin = Center + Dir * StartDistance;

			RegisterSphereSweep(Sphere(Origin, radius), -Dir, StartDistance);
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)	override
	{
		if(!caps.mSupportSphereSweeps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	void UpdateSphereSweeps()
	{
		const float StartDistance = GetFloat(20.0f, mEditBox_SweepStartDistance);
		const float SphereRadius = GetFloat(1.0f, mEditBox_SphereRadius);
		if(StartDistance==mCurrentStartDistance && SphereRadius==mCurrentSphereRadius)
			return;
		mCurrentStartDistance = StartDistance;
		mCurrentSphereRadius = SphereRadius;

		Point Center, Extents;
		GetGlobalBounds(Center, Extents);

		udword Nb = GetNbRegisteredSphereSweeps();
		PintSphereSweepData* Data = GetRegisteredSphereSweeps();
		while(Nb--)
		{
			Data->mSphere.mCenter = Center - Data->mDir * StartDistance;
			Data->mSphere.mRadius = SphereRadius;
			Data->mMaxDist = StartDistance;
			Data++;
		}
	}
};

static const char* gDesc_SceneSphereSweepVsStaticMeshes_TessBunny = "64 radial sphere-sweeps against the tessellated bunny. Radius of the spheres is 1.0.";

class SceneSphereSweepVsStaticMeshes_TessBunny : public SceneSphereSweepVsStaticMeshes_TessBunny_Base
{
	public:
							SceneSphereSweepVsStaticMeshes_TessBunny()	{}
	virtual					~SceneSphereSweepVsStaticMeshes_TessBunny()	{}
	virtual	const char*		GetName()			const	{ return "SceneSphereSweepVsStaticMeshes_TessBunny";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_SceneSphereSweepVsStaticMeshes_TessBunny;	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)	override
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(26.99f, 29.04f, 25.00f), Point(-0.63f, -0.49f, -0.60f));
		desc.mCamera[1] = PintCameraPose(Point(-2.53f, 4.53f, 2.63f), Point(-0.86f, -0.45f, -0.26f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()	override
	{
		return CommonSetupBase(1.0f);
	}

	virtual udword	Update(Pint& pint, float dt)	override
	{
		UpdateSphereSweeps();
		return DoBatchSphereSweeps(*this, pint);
	}

END_TEST(SceneSphereSweepVsStaticMeshes_TessBunny)

static const char* gDesc_SceneSphereSweepVsStaticMeshes_TessBunny2 = "64 radial sphere-sweeps against the tessellated bunny. Radius of the spheres is 0.1.";

class SceneSphereSweepVsStaticMeshes_TessBunny2 : public SceneSphereSweepVsStaticMeshes_TessBunny_Base
{
	public:
							SceneSphereSweepVsStaticMeshes_TessBunny2()		{}
	virtual					~SceneSphereSweepVsStaticMeshes_TessBunny2()	{}
	virtual	const char*		GetName()			const	{ return "SceneSphereSweepVsStaticMeshes_TessBunny2";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_SceneSphereSweepVsStaticMeshes_TessBunny2;	}


	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)	override
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(24.68f, 28.58f, 24.72f), Point(-0.61f, -0.51f, -0.61f));
		desc.mCamera[1] = PintCameraPose(Point(-2.74f, 5.06f, 2.11f), Point(-0.84f, -0.11f, -0.53f));
		desc.mCamera[2] = PintCameraPose(Point(2.59f, 8.21f, -0.28f), Point(0.46f, 0.45f, -0.77f));
		desc.mCamera[3] = PintCameraPose(Point(-1.71f, 3.58f, -2.00f), Point(-0.55f, -0.55f, -0.63f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()	override
	{
		return CommonSetupBase(0.1f);
	}

	virtual udword	Update(Pint& pint, float dt)	override
	{
		UpdateSphereSweeps();
		return DoBatchSphereSweeps(*this, pint);
	}

END_TEST(SceneSphereSweepVsStaticMeshes_TessBunny2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneCapsuleSweepVsStaticMeshes_Archipelago = "32*32 capsule-sweeps against the Archipelago mesh level.";

START_SQ_TEST(SceneCapsuleSweepVsStaticMeshes_Archipelago, CATEGORY_SWEEP, gDesc_SceneCapsuleSweepVsStaticMeshes_Archipelago)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(71.21f, 4.04f, 83.04f), Point(0.55f, -0.46f, 0.70f));
		desc.mCamera[1] = PintCameraPose(Point(414.28f, 2.60f, 228.18f), Point(0.46f, -0.12f, 0.88f));
		desc.mCamera[2] = PintCameraPose(Point(75.14f, 0.70f, 95.50f), Point(-0.14f, -0.34f, 0.93f));
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
		const float ScaleX = Extents.x - 1.0f;
		const float ScaleY = Extents.z - 1.0f;
		const float Altitude = 30.0f;
		const Point Dir(0.0f, -1.0f, 0.0f);
		const float CapsuleRadius = 0.4f;
		const float HalfHeight = 1.8f;
		RegisterArrayOfCapsuleSweeps(*this, NbX, NbY, Altitude, ScaleX, ScaleY, Dir, CapsuleRadius, HalfHeight, Offset, gSQMaxDist);

		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCapsuleSweeps)
			return false;

		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		UpdateCapsuleSweeps(*this, mCurrentTime);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchCapsuleSweeps(*this, pint);
	}

END_TEST(SceneCapsuleSweepVsStaticMeshes_Archipelago)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneCapsuleSweepVsStaticMeshes_Archipelago2 = "Capsule sweep debug.";

START_SQ_TEST(SceneCapsuleSweepVsStaticMeshes_Archipelago2, CATEGORY_SWEEP, gDesc_SceneCapsuleSweepVsStaticMeshes_Archipelago2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(73.75f, 2.61f, 90.15f), Point(0.23f, -0.42f, 0.88f));
		SetDefEnv(desc, false);

		//const char* Filename = "C:/Projects/#PEEL/PEEL_Timestamp56/Media/Debug/CapsuleSweepDebug.zb2";
		const char* Filename = FindPEELFile("CapsuleSweepDebug.zb2");
		if(Filename)
		{
			bool status = ImportZB2File(desc, Filename);
			(void)status;

			desc.mCamera[0] = PintCameraPose(Point(73.75f, 2.61f, 90.15f), Point(0.23f, -0.42f, 0.88f));
		}

	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();


//		Point Offset(75.14f, 0.70f, 95.50f);
//		Point Offset(75.0f, 0.70f, 95.0f);
		Point Offset(76.0f, 0.70f, 95.0f);
		const udword NbX = 1;
		const udword NbY = 1;
		const float ScaleX = 1.0f;
		const float ScaleY = 1.0f;
		const float Altitude = 30.0f;
		const Point Dir(0.0f, -1.0f, 0.0f);
		const float CapsuleRadius = 0.4f;
		const float HalfHeight = 1.8f;
		RegisterArrayOfCapsuleSweeps(*this, NbX, NbY, Altitude, ScaleX, ScaleY, Dir, CapsuleRadius, HalfHeight, Offset, gSQMaxDist);

		//mCurrentTime = 91.0f/60.0f;
		UpdateCapsuleSweeps(*this, 91.0f/60.0f);

		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCapsuleSweeps)
			return false;

		if(!mZB2Factory)
			return false;

		return CreateZB2Scene(pint, caps);
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
//		UpdateCapsuleSweeps(*this, mCurrentTime);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchCapsuleSweeps(*this, pint);
	}

END_TEST(SceneCapsuleSweepVsStaticMeshes_Archipelago2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneCapsuleSweepVsStaticMeshes_KP = "32*32 capsule-sweeps against the Konoko Payne mesh level.";

START_SQ_TEST(SceneCapsuleSweepVsStaticMeshes_KP, CATEGORY_SWEEP, gDesc_SceneCapsuleSweepVsStaticMeshes_KP)

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

		const float CapsuleRadius = 0.4f;
		const float HalfHeight = 1.8f;

		LSS Capsule;
		Capsule.mP0 = Point(50.0f-HalfHeight, 50.0f, 50.0f);
		Capsule.mP1 = Point(50.0f+HalfHeight, 50.0f, 50.0f);
		Capsule.mRadius	= CapsuleRadius;

		BasicRandom Rnd(42);
		for(udword i=0;i<1024;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);

			RegisterCapsuleSweep(Capsule, Dir, gSQMaxDist);
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCapsuleSweeps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		UpdateCapsuleSweeps(*this, mCurrentTime);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchCapsuleSweeps(*this, pint);
	}

END_TEST(SceneCapsuleSweepVsStaticMeshes_KP)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneCapsuleSweepVsStaticMeshes_TessBunny = "64 radial capsule-sweeps against the tessellated bunny.";

START_SQ_TEST(SceneCapsuleSweepVsStaticMeshes_TessBunny, CATEGORY_SWEEP, gDesc_SceneCapsuleSweepVsStaticMeshes_TessBunny)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(24.47f, 28.62f, 24.55f), Point(-0.59f, -0.51f, -0.62f));
		desc.mCamera[1] = PintCameraPose(Point(0.02f, 10.59f, -0.01f), Point(-0.82f, 0.15f, -0.55f));
		desc.mCamera[2] = PintCameraPose(Point(-2.02f, 9.48f, 0.98f), Point(-0.90f, 0.42f, -0.06f));
		desc.mCamera[3] = PintCameraPose(Point(-2.39f, 8.45f, 5.80f), Point(-0.72f, -0.19f, 0.67f));
		desc.mCamera[4] = PintCameraPose(Point(3.06f, 6.90f, 4.47f), Point(0.99f, -0.12f, 0.12f));
		desc.mCamera[5] = PintCameraPose(Point(2.06f, 5.60f, -2.30f), Point(0.89f, -0.37f, -0.27f));
		desc.mCamera[6] = PintCameraPose(Point(1.35f, 8.42f, -2.10f), Point(0.59f, 0.73f, -0.35f));
		desc.mCamera[7] = PintCameraPose(Point(-0.17f, 9.85f, -7.00f), Point(-0.10f, 0.42f, -0.90f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		GLRenderStates::SetDefaultCullMode(CULL_NONE);

		LoadMeshesFromFile_(*this, "Bunny.bin", null, false, 3);

		BasicRandom Rnd(42);

		Point Center, Extents;
		GetGlobalBounds(Center, Extents);

		for(udword i=0;i<64;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);

			const Point Origin = Center + Dir * 20.0f;

			LSS Capsule;
			Capsule.mP0		= Origin + Point(1.0f, 1.0f, 1.0f);
			Capsule.mP1		= Origin - Point(1.0f, 1.0f, 1.0f);
			Capsule.mRadius	= 0.1f;

			RegisterCapsuleSweep(Capsule, -Dir, 20.0f);
		}
/*
//		for(udword i=0;i<1024;i++)
		for(udword i=0;i<16;i++)
		{
//			UnitRandomPt(mRays[i].mDir, Rnd);
//			mRays[i].mOrig = mRays[i].mDir * 5000.0f;
//			mRays[i].mDir = -mRays[i].mDir;

			Point Dir;
			UnitRandomPt(Dir, Rnd);

			const Point Origin = Dir * 5000.0f;

			LSS Capsule;
			Capsule.mP0		= Origin + Point(0.5f, 0.0f, 0.0f);
			Capsule.mP1		= Origin - Point(0.5f, 0.0f, 0.0f);
			Capsule.mRadius	= 1.0f;

//			if(i==0)	// Very interesting one, gives very different hit results for each engine
//			if(i==1)	// Clear issue with bv4, impact is way too early
//			if(i==2)	// Gives a hit in bv4 (probably wrong), no hit for other engines... clear bug => now fixed
			RegisterCapsuleSweep(Capsule, -Dir, 5000.0f);
		}*/

		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCapsuleSweeps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchCapsuleSweeps(*this, pint);
	}

END_TEST(SceneCapsuleSweepVsStaticMeshes_TessBunny)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneCapsuleSweepVsStaticMeshes_TessBunny2 = "64 radial capsule-sweeps against the tessellated bunny.";

START_SQ_TEST(SceneCapsuleSweepVsStaticMeshes_TessBunny2, CATEGORY_SWEEP, gDesc_SceneCapsuleSweepVsStaticMeshes_TessBunny2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(24.47f, 28.62f, 24.55f), Point(-0.59f, -0.51f, -0.62f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		GLRenderStates::SetDefaultCullMode(CULL_NONE);

		LoadMeshesFromFile_(*this, "Bunny.bin", null, false, 3);

		BasicRandom Rnd(42);

		Point Center, Extents;
		GetGlobalBounds(Center, Extents);

		for(udword i=0;i<64;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);

			const Point Origin = Center + Dir * 20.0f;

			LSS Capsule;
			Capsule.mP0		= Origin + Point(1.0f, 1.0f, 1.0f);
			Capsule.mP1		= Origin - Point(1.0f, 1.0f, 1.0f);
			Capsule.mRadius	= 0.5f;

			RegisterCapsuleSweep(Capsule, -Dir, 20.0f);
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCapsuleSweeps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchCapsuleSweeps(*this, pint);
	}

END_TEST(SceneCapsuleSweepVsStaticMeshes_TessBunny2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneCapsuleSweepVsStaticMeshes_TessBunny3 = "64 radial capsule-sweeps against the tessellated bunny.";

START_SQ_TEST(SceneCapsuleSweepVsStaticMeshes_TessBunny3, CATEGORY_SWEEP, gDesc_SceneCapsuleSweepVsStaticMeshes_TessBunny3)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(24.47f, 28.62f, 24.55f), Point(-0.59f, -0.51f, -0.62f));
		desc.mCamera[1] = PintCameraPose(Point(2.28f, 5.47f, -3.05f), Point(0.91f, -0.41f, 0.05f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		GLRenderStates::SetDefaultCullMode(CULL_NONE);

		LoadMeshesFromFile_(*this, "Bunny.bin", null, false, 3);

		BasicRandom Rnd(42);

		Point Center, Extents;
		GetGlobalBounds(Center, Extents);

		for(udword i=0;i<64;i++)
		{
			Point Dir;
			UnitRandomPt(Dir, Rnd);

			const Point Origin = Center + Dir * 20.0f;

			LSS Capsule;
			Capsule.mP0		= Origin + Point(1.0f, 1.0f, 1.0f);
			Capsule.mP1		= Origin - Point(1.0f, 1.0f, 1.0f);
			Capsule.mRadius	= 0.01f;

			RegisterCapsuleSweep(Capsule, -Dir, 20.0f);
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCapsuleSweeps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchCapsuleSweeps(*this, pint);
	}

END_TEST(SceneCapsuleSweepVsStaticMeshes_TessBunny3)

///////////////////////////////////////////////////////////////////////////////

static PintShapeRenderer* CommonSetup_SceneConvexSweepVsStaticMeshes_TessBunny(MyConvex& convex, TestBase& test)
{
	GLRenderStates::SetDefaultCullMode(CULL_NONE);

//	LoadMeshesFromFile_(test, "Venus.bin", null, false, 0);
	LoadMeshesFromFile_(test, "Bunny.bin", null, false, 3);

	BasicRandom Rnd(42);

	Point Center, Extents;
	test.GetGlobalBounds(Center, Extents);

	udword i=2;	// Small convexes
//	udword i=4;	// 'Big' convexes
//	udword i=7;
//	udword i=13;
	convex.LoadFile(i);

	PintShapeRenderer* renderer = CreateConvexRenderer(convex.mNbVerts, convex.mVerts);

	const Point P(0.0f, 10.0f, 0.0f);
	const Matrix3x3 Rot(Idt);

	for(udword i=0;i<64;i++)
	{
		Point Dir;
		UnitRandomPt(Dir, Rnd);

		const Point Origin = Center + Dir * 20.0f;

		// Beware: we rely on the indices returned by pint.CreateConvexObject() here,
		// even though the functions haven't been called yet.
		test.RegisterConvexSweep(0, renderer, PR(Origin, Rot), -Dir, 20.0f);
	}
	return renderer;
}

static bool Setup_SceneConvexSweepVsStaticMeshes_TessBunny(PintShapeRenderer* /*renderer*/, MyConvex& convex, TestBase& test, Pint& pint, const PintCaps& caps)
{
	if(!caps.mSupportMeshes || !caps.mSupportConvexSweeps)
		return false;

	PINT_CONVEX_DATA_CREATE Desc(convex.mNbVerts, convex.mVerts);
	udword h;
	const PintConvexHandle Handle = pint.CreateConvexObject(Desc, &h);
	ASSERT(Handle);
	ASSERT(h==0);

	return test.CreateMeshesFromRegisteredSurfaces(pint, caps);
}

static const char* gDesc_SceneConvexSweepVsStaticMeshes_TessBunny = "64 radial convex-sweeps against the tessellated bunny.";

START_SQ_TEST(SceneConvexSweepVsStaticMeshes_TessBunny, CATEGORY_SWEEP, gDesc_SceneConvexSweepVsStaticMeshes_TessBunny)

	MyConvex			mConvex;
	PintShapeRenderer*	mRenderer;

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(27.58f, 29.80f, 27.00f), Point(-0.58f, -0.56f, -0.59f));
		SetDefEnv(desc, false);
	}

	virtual void	CommonRelease()
	{
		mConvex.Release();
		TestBase::CommonRelease();
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		mRenderer = CommonSetup_SceneConvexSweepVsStaticMeshes_TessBunny(mConvex, *this);
		return mRenderer!=null;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return Setup_SceneConvexSweepVsStaticMeshes_TessBunny(mRenderer, mConvex, *this, pint, caps);
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
		UpdateConvexSweeps(*this, mCurrentTime);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchConvexSweeps(*this, pint);
	}

END_TEST(SceneConvexSweepVsStaticMeshes_TessBunny)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_BoxSweep_TestZone = "TestZone. Box sweep.";

START_SQ_TEST(BoxSweep_TestZone, CATEGORY_SWEEP, gDesc_BoxSweep_TestZone)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(11.97f, 3.80f, -54.58f), Point(-0.33f, -0.50f, 0.80f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "testzone.bin");

//		return Setup_PotPourri_Raycasts(*this, 4096, 10.0f);

		if(1)
		{
			BasicRandom Rnd(42);
/*			for(udword i=0;i<1024;i++)
			{
				const float x = Rnd.randomFloat()*100.0f;
				const float z = Rnd.randomFloat()*100.0f;

				Point D(x, -100.0f, z);
				D.Normalize();

				Quat Q;
				//UnitRandomQuat(Q, Rnd);
				Q.Identity();
				RegisterBoxSweep(OBB(Point(x, 50.0f, z), Point(1.0f, 1.0f, 1.0f), Q), D, 1000.0f);
//				RegisterBoxSweep(OBB(Point(x, 50.0f, z), Point(1.0f, 1.0f, 1.0f), Q), Point(0.0f, -1.0f, 0.0f), 1000.0f);
			}*/

			Point Center, Extents;
			GetGlobalBounds(Center, Extents);
//desc.mCamera[0] = PintCameraPose(Point(8.36f, -4.22f, -28.18f), Point(0.14f, 0.21f, -0.97f));
			const udword Nb = 2048;
			for(udword i=0;i<Nb;i++)
			{
				const float Coeff = TWOPI*float(i)/float(Nb);
				const float x = sinf(Coeff);
				const float z = cosf(Coeff);

				Point D(x, 0.0f, z);
				D.Normalize();

				Quat Q;
//				UnitRandomQuat(Q, Rnd);
				Q.Identity();
//				RegisterBoxSweep(OBB(Point(Center.x, 2.0f, Center.z), Point(0.1f, 0.1f, 0.1f), Q), D, 1000.0f);
//				RegisterBoxSweep(OBB(Point(8.36f, -4.22f, -28.18f), Point(0.1f, 0.1f, 0.1f), Q), D, 1000.0f);
				RegisterBoxSweep(OBB(Point(8.36f, -4.22f, -28.18f), Point(0.5f, 0.5f, 0.5f), Q), D, 1000.0f);
			}
		}
		else
		{
			if(!GetNbRegisteredSurfaces())
				return false;
			const SurfaceManager::SurfaceData* SD = GetSurfaceData(0);
			const IndexedSurface* IS = SD->mSurface;

			const Point* V = IS->GetVerts();
			const float Length = 0.1f;
			BasicRandom Rnd(42);
			for(udword i=0;i<IS->GetNbFaces();i++)
			{
				const IndexedTriangle* T = IS->GetFace(i);
				Point Center, Normal;
				T->Center(V, Center);
				T->Normal(V, Normal);

				Quat Q;
				UnitRandomQuat(Q, Rnd);
				Q.Identity();

				RegisterBoxSweep(OBB(Center+Normal*2.0f, Point(1.0f, 1.0f, 1.0f), Q), -Normal, 10000.0f);
	//			RegisterBoxSweep(OBB(Center+Normal*2.0f, Point(0.1f, 0.1f, 0.1f), Q), -Normal, 10.0f);
			}
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportBoxSweeps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchBoxSweeps(*this, pint);
	}

END_TEST(BoxSweep_TestZone)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_BoxSweep_TestZone2 = "TestZone. Box sweep 2.";

START_SQ_TEST(BoxSweep2_TestZone, CATEGORY_SWEEP, gDesc_BoxSweep_TestZone2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-11.54f, 8.86f, -60.97f), Point(-0.60f, -0.57f, 0.57f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "testzone.bin");

//		return Setup_PotPourri_Raycasts(*this, 4096, 10.0f);

		if(1)
		{
			BasicRandom Rnd(42);
			Point Center, Extents;
			GetGlobalBounds(Center, Extents);
			const udword Nb=2048;
			for(udword i=0;i<Nb;i++)
			{
				const float Coeff = TWOPI*float(i)/float(Nb);
				const float x = sinf(Coeff);
				const float z = cosf(Coeff);

				Point D(x, 0.0f, z);
				D.Normalize();

				Quat Q;
//				UnitRandomQuat(Q, Rnd);
				Q.Identity();
				RegisterBoxSweep(OBB(Point(-15.08f, 5.20f, -58.01f), Point(0.5f, 0.5f, 0.5f), Q), D, 1000.0f);
			}

		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportBoxSweeps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchBoxSweeps(*this, pint);
	}

END_TEST(BoxSweep2_TestZone)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SphereSweep_TestZone = "TestZone. Sphere sweep.";

START_SQ_TEST(SphereSweep_TestZone, CATEGORY_SWEEP, gDesc_SphereSweep_TestZone)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-11.54f, 8.86f, -60.97f), Point(-0.60f, -0.57f, 0.57f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "testzone.bin");

//		return Setup_PotPourri_Raycasts(*this, 4096, 10.0f);

		if(1)
		{
			const Sphere S(Point(-15.08f, 5.20f, -58.01f), 0.5f);

			BasicRandom Rnd(42);
			Point Center, Extents;
			GetGlobalBounds(Center, Extents);
			for(udword i=0;i<1024;i++)
			{
				const float Coeff = TWOPI*float(i)/1024.0f;
				const float x = sinf(Coeff);
				const float z = cosf(Coeff);

				Point D(x, 0.0f, z);
				D.Normalize();

				RegisterSphereSweep(S, D, 1000.0f);
			}
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphereSweeps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchSphereSweeps(*this, pint);
	}

END_TEST(SphereSweep_TestZone)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CapsuleSweep_TestZone = "TestZone. Capsule sweep.";

START_SQ_TEST(CapsuleSweep_TestZone, CATEGORY_SWEEP, gDesc_CapsuleSweep_TestZone)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-17.09f, 4.59f, -47.51f), Point(0.14f, 0.30f, -0.94f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "testzone.bin");

//		return Setup_PotPourri_Raycasts(*this, 4096, 10.0f);

		if(1)
		{
			const LSS Capsule(Segment(Point(-15.08f, 5.20f, -58.01f), Point(-15.08f, 7.20f, -58.01f)), 0.5f);

			BasicRandom Rnd(42);
			Point Center, Extents;
			GetGlobalBounds(Center, Extents);
//			for(udword i=0;i<1024;i++)
			for(udword i=0;i<32;i++)
			{
//				const float Coeff = TWOPI*float(i)/1024.0f;
				const float Coeff = TWOPI*float(i)/32.0f;
				const float x = sinf(Coeff);
				const float z = cosf(Coeff);

				Point D(x, 0.0f, z);
				D.Normalize();

				RegisterCapsuleSweep(Capsule, D, 1000.0f);
			}
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCapsuleSweeps)
			return false;

		return CreateMeshesFromRegisteredSurfaces(pint, caps);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchCapsuleSweeps(*this, pint);
	}

END_TEST(CapsuleSweep_TestZone)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ValveRaysAndSweeps = "Valve level, raycasts & sweeps.";

START_SQ_TEST(ValveRaysAndSweeps, CATEGORY_SWEEP, gDesc_ValveRaysAndSweeps)

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
		LoadRaysFile(*this, "rays(lotsof boxes).bin", false);
		mRepX = CreateRepXContext("c5m4_quarter2_Statics.repx", gValveScale, true);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRaycasts || !caps.mSupportMeshes || !caps.mSupportBoxSweeps)
			return false;
		return AddToPint(pint, mRepX);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		udword Nb1 = DoBatchRaycasts(*this, pint);
		udword Nb2 = DoBatchBoxSweeps(*this, pint);
		return Nb1+Nb2;
	}

END_TEST(ValveRaysAndSweeps)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_PlanetsideBoxSweeps = "512*4 box sweeps against the Planetside level.";

START_SQ_TEST(PlanetsideBoxSweeps, CATEGORY_SWEEP, gDesc_PlanetsideBoxSweeps)

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

		SRand(42);
		const Point Orig(241.09f, 6.29f, 730.26f);
		const Point Orig2(164.74f, 1.67f, 783.85f);
		const Point Orig3(275.19f, 32.54f, 703.23f);
		const Point Orig4(295.76f, 4.08f, 845.31f);
		const Matrix3x3 Rot(Idt);
		for(udword j=0;j<512;j++)
		{
			const float angle = (float(j)/512.0f)*3.14159f*2.0f;
			const float s = sinf(angle);
			const float c = cosf(angle);
			Point Dir(s, 0.1f*(UnitRandomFloat()-0.5f), c);
			Dir.Normalize();
			RegisterBoxSweep(OBB(Orig, Point(0.1f, 0.1f, 0.1f), Rot), Dir, 5000.0f);
			RegisterBoxSweep(OBB(Orig2, Point(0.1f, 0.1f, 0.1f), Rot), Dir, 5000.0f);
			RegisterBoxSweep(OBB(Orig3, Point(0.1f, 0.1f, 0.1f), Rot), Dir, 5000.0f);
			RegisterBoxSweep(OBB(Orig4, Point(0.1f, 0.1f, 0.1f), Rot), Dir, 5000.0f);
		}
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		mRepX = CreateRepXContext("Planetside_Statics.repx", 1.0f, false);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes || !caps.mSupportBoxSweeps)
			return false;
		return AddToPint(pint, mRepX);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchBoxSweeps(*this, pint);
	}

END_TEST(PlanetsideBoxSweeps)

///////////////////////////////////////////////////////////////////////////////

#include "TestScenes_Heightfield.h"

class SceneSweepVsStaticHeightfield : public HeightfieldTest
{
	public:
								SceneSweepVsStaticHeightfield(udword scenario) : HeightfieldTest(64, 32), mRenderer(null), mScenario(scenario)	{}
	virtual						~SceneSweepVsStaticHeightfield()				{								}
	virtual	TestCategory		GetCategory()							const	{ return CATEGORY_SWEEP;		}
	virtual	udword				GetProfilingFlags()						const	{ return PROFILING_TEST_UPDATE;	}

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

		mHH.Init(SizeX, SizeZ, 4.0f);

		const Point Offset(0.0f, 0.0f, 0.0f);
		const Point Extents(SizeZ*0.5f, 0.0f, SizeX*0.5f);

		const udword NbX = 16;
		const udword NbY = 16;
		const float ScaleX = Extents.x - 1.0f;
		const float ScaleY = Extents.z - 1.0f;
		const float Altitude = 30.0f;
		const Point Dir(0.0f, -1.0f, 0.0f);
		if(mScenario==0)
		{
			const Point BoxExtents(1.2f, 0.5f, 0.5f);
			RegisterArrayOfBoxSweeps(*this, NbX, NbY, Altitude, ScaleX, ScaleY, Dir, BoxExtents, Offset, gSQMaxDist);
		}
		else if(mScenario==1)
		{
			RegisterArrayOfSphereSweeps(*this, NbX, NbY, Altitude, ScaleX, ScaleY, Dir, 0.5f, Offset, gSQMaxDist);
		}
		else if(mScenario==2)
		{
			const float CapsuleRadius = 0.4f;
			const float HalfHeight = 1.8f;
			RegisterArrayOfCapsuleSweeps(*this, NbX, NbY, Altitude, ScaleX, ScaleY, Dir, CapsuleRadius, HalfHeight, Offset, gSQMaxDist);
		}
		else if(mScenario==3)
		{
			BasicRandom Rnd(42);

		//	udword i=2;	// Small convexes
			udword i=4;	// 'Big' convexes
		//	udword i=7;
		//	udword i=13;
			mConvex.LoadFile(i);

			PintShapeRenderer* renderer = CreateConvexRenderer(mConvex.mNbVerts, mConvex.mVerts);

			const Point P(0.0f, 10.0f, 0.0f);
			const Matrix3x3 Rot(Idt);

			const float OneOverNbX = OneOverNb(NbX);
			const float OneOverNbY = OneOverNb(NbY);
			for(udword y=0;y<NbY;y++)
			{
				const float CoeffY = 2.0f * ((float(y)*OneOverNbY) - 0.5f);
				for(udword x=0;x<NbX;x++)
				{
					const float CoeffX = 2.0f * ((float(x)*OneOverNbX) - 0.5f);

					const Point Origin(CoeffX * ScaleX, Altitude, CoeffY * ScaleY);

					// Beware: we rely on the indices returned by pint.CreateConvexObject() here,
					// even though the functions haven't been called yet.
					RegisterConvexSweep(0, renderer, PR(Origin, Rot), Dir, gSQMaxDist);
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
			if(!caps.mSupportBoxSweeps)
				return false;
		}
		else if(mScenario==1)
		{
			if(!caps.mSupportSphereSweeps)
				return false;
		}
		else if(mScenario==2)
		{
			if(!caps.mSupportCapsuleSweeps)
				return false;
		}
		else if(mScenario==3)
		{
			if(!caps.mSupportConvexSweeps)
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
			UpdateBoxSweeps(*this, mCurrentTime);
//		else if(mScenario==1)
//			UpdateSphereSweeps(*this, mCurrentTime);
		else if(mScenario==2)
			UpdateCapsuleSweeps(*this, mCurrentTime);
		else if(mScenario==3)
			UpdateConvexSweeps(*this, mCurrentTime);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		if(mScenario==0)
			return DoBatchBoxSweeps(*this, pint);
		else if(mScenario==1)
			return DoBatchSphereSweeps(*this, pint);
		else if(mScenario==2)
			return DoBatchCapsuleSweeps(*this, pint);
		else if(mScenario==3)
			return DoBatchConvexSweeps(*this, pint);

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SceneBoxSweepVsStaticHeightfield = "box-sweeps against a heightfield.";

class SceneBoxSweepVsStaticHeightfield : public SceneSweepVsStaticHeightfield
{
	public:
								SceneBoxSweepVsStaticHeightfield() : SceneSweepVsStaticHeightfield(0)	{}
	virtual						~SceneBoxSweepVsStaticHeightfield()										{}
	virtual	const char*			GetName()					const	{ return "SceneBoxSweepVsStaticHeightfield";		}
	virtual	const char*			GetDescription()			const	{ return gDesc_SceneBoxSweepVsStaticHeightfield;	}

END_TEST(SceneBoxSweepVsStaticHeightfield)

static const char* gDesc_SceneSphereSweepVsStaticHeightfield = "sphere-sweeps against a heightfield.";

class SceneSphereSweepVsStaticHeightfield : public SceneSweepVsStaticHeightfield
{
	public:
								SceneSphereSweepVsStaticHeightfield() : SceneSweepVsStaticHeightfield(1)	{}
	virtual						~SceneSphereSweepVsStaticHeightfield()										{}
	virtual	const char*			GetName()					const	{ return "SceneSphereSweepVsStaticHeightfield";		}
	virtual	const char*			GetDescription()			const	{ return gDesc_SceneSphereSweepVsStaticHeightfield;	}

END_TEST(SceneSphereSweepVsStaticHeightfield)

static const char* gDesc_SceneCapsuleSweepVsStaticHeightfield = "capsule-sweeps against a heightfield.";

class SceneCapsuleSweepVsStaticHeightfield : public SceneSweepVsStaticHeightfield
{
	public:
								SceneCapsuleSweepVsStaticHeightfield() : SceneSweepVsStaticHeightfield(2)	{}
	virtual						~SceneCapsuleSweepVsStaticHeightfield()										{}
	virtual	const char*			GetName()					const	{ return "SceneCapsuleSweepVsStaticHeightfield";		}
	virtual	const char*			GetDescription()			const	{ return gDesc_SceneCapsuleSweepVsStaticHeightfield;	}

END_TEST(SceneCapsuleSweepVsStaticHeightfield)

static const char* gDesc_SceneConvexSweepVsStaticHeightfield = "convex-sweeps against a heightfield.";

class SceneConvexSweepVsStaticHeightfield : public SceneSweepVsStaticHeightfield
{
	public:
								SceneConvexSweepVsStaticHeightfield() : SceneSweepVsStaticHeightfield(3)	{}
	virtual						~SceneConvexSweepVsStaticHeightfield()										{}
	virtual	const char*			GetName()					const	{ return "SceneConvexSweepVsStaticHeightfield";		}
	virtual	const char*			GetDescription()			const	{ return gDesc_SceneConvexSweepVsStaticHeightfield;	}

END_TEST(SceneConvexSweepVsStaticHeightfield)

