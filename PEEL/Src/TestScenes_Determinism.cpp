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

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Determinism = "Determinism with different islands. Check if the presence of another island (even one far from the action) affects determinism. \
Run the scene with and without the extra stack, and check if the final object positions are the same. This is what 'enhanced determinism' is about in PhysX.";

START_TEST(DeterminismWithDifferentIslands, CATEGORY_DETERMINISM, gDesc_Determinism)

	CheckBoxPtr		mCheckBox_AddExtraStack;

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)	override
	{
		const sdword Width = 300;
		IceWindow* UI = CreateTestWindow(Width, 400);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 60;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 10;
		{
			mCheckBox_AddExtraStack = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Add extra stack", &UIElems, false, null, null);
			y += YStep;
		}

		y += YStep;

		return CreateTestTabControlAndResetButton(UI, Width, y, 0);
	}

	virtual	float	GetRenderData(Point& center)	const	{ return 400.0f;	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)	override
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(62.24f, 43.39f, -48.98f), Point(-0.69f, -0.28f, 0.67f));
		desc.mCamera[1] = PintCameraPose(Point(42.98f, 49.70f, 101.45f), Point(-0.56f, -0.50f, -0.66f));
		desc.mCamera[2] = PintCameraPose(Point(81.11f, 59.81f, 14.95f), Point(-0.87f, -0.45f, -0.20f));
		desc.mNbSimulateCallsPerFrame = 8;	// Make time pass 8x faster, we're interested in the final poses
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)	override
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		CreateBoxStack(pint, caps, 1, 20);
		const Point Offset(0.0f, 0.0f, -4.0f);
		CreateBoxStack(pint, caps, 1, 20, &Offset);

		const Point Extents(3.0f, 3.0f, 3.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateRenderer(BoxDesc);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass			= 10.0f;
		ObjectDesc.mPosition		= Point(0.0f, 10.0f, 10.0f);
		ObjectDesc.mLinearVelocity	= Point(0.0f, 0.0f, -100.0f);
		CreatePintObject(pint, ObjectDesc);

		if(mCheckBox_AddExtraStack && mCheckBox_AddExtraStack->IsChecked())
		{
			const Point Offset(0.0f, 0.0f, 80.0f);
			CreateBoxStack(pint, caps, 1, 20, &Offset);
		}

		return true;
	}

END_TEST(DeterminismWithDifferentIslands)

///////////////////////////////////////////////////////////////////////////////
