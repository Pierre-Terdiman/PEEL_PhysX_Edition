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
#include "GUI_Helpers.h"
#include "Cylinder.h"

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_DominosConfigurable = "Dominos. Things to look for are whether the dominoes drift or not after they have all been knocked down. \
The speed at which dominoes are falling greatly depends on friction. You can try disabling VSYNC in the main UI to artificially make the scene run faster if it takes too long.";

#define NB_DOMINOS_PRESETS	4
static const float	gPreset_Friction[]		= { 1.0f,	0.1f,	0.5f,	0.1f	};
static const float	gPreset_CircleRadius[]	= { 10.0f,	10.0f,	20.0f,	40.0f	};
static const udword	gPreset_NbPerCircle[]	= { 128,	128,	128,	200		};
static const float	gPreset_ShrinkFactor[]	= { 0.0f,	0.0f,	0.1f,	0.05f	};
static const udword	gPreset_NbRounds[]		= { 1,		1,		6,		16		};
static const bool	gPreset_PushLine[]		= { false,	false,	false,	true	};

class DominosConfigurable : public TestBase
{
			ComboBoxPtr		mComboBox_Preset;
			EditBoxPtr		mEditBox_Friction;
			EditBoxPtr		mEditBox_CircleRadius;
			EditBoxPtr		mEditBox_NbPerCircle;
			EditBoxPtr		mEditBox_ShrinkFactor;
			EditBoxPtr		mEditBox_NbRounds;
			CheckBoxPtr		mCheckBox_PushLine;
	public:
							DominosConfigurable()		{									}
	virtual					~DominosConfigurable()		{									}
	virtual	const char*		GetName()			const	{ return "Dominos";					}
	virtual	const char*		GetDescription()	const	{ return gDesc_DominosConfigurable;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_BEHAVIOR;			}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 350;
		WD.mHeight	= 250;
		WD.mLabel	= "Dominos";
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
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Friction:", &UIElems);
			mEditBox_Friction = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_Friction->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Circle radius:", &UIElems);
			mEditBox_CircleRadius = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "10.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_CircleRadius->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Nb dominos per circle:", &UIElems);
			mEditBox_NbPerCircle = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "128", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			mEditBox_NbPerCircle->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Shrink factor:", &UIElems);
			mEditBox_ShrinkFactor = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT, null, null);
			mEditBox_ShrinkFactor->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Nb rounds:", &UIElems);
			mEditBox_NbRounds = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			mEditBox_NbRounds->SetEnabled(false);
			y += YStep;

			mCheckBox_PushLine = helper.CreateCheckBox(UI, 4, 4, y, 200, 20, "Push line", &UIElems, false, null, null);
			mCheckBox_PushLine->SetEnabled(false);
			y += YStep;
		}
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Presets:", &UIElems);

			class MyComboBox : public IceComboBox
			{
				DominosConfigurable&	mTest;
				public:
								MyComboBox(const ComboBoxDesc& desc, DominosConfigurable& test) : IceComboBox(desc), mTest(test)	{}
				virtual			~MyComboBox()																						{}

				virtual	void	OnComboBoxEvent(ComboBoxEvent event)
				{
					if(event==CBE_SELECTION_CHANGED)
					{
						const udword SelectedIndex = GetSelectedIndex();
						const bool Enabled = SelectedIndex==GetItemCount()-1;
						mTest.mEditBox_Friction->SetEnabled(Enabled);
						mTest.mEditBox_CircleRadius->SetEnabled(Enabled);
						mTest.mEditBox_NbPerCircle->SetEnabled(Enabled);
						mTest.mEditBox_ShrinkFactor->SetEnabled(Enabled);
						mTest.mEditBox_NbRounds->SetEnabled(Enabled);
						mTest.mCheckBox_PushLine->SetEnabled(Enabled);

						if(!Enabled && SelectedIndex<NB_DOMINOS_PRESETS)
						{
							mTest.mEditBox_Friction->SetText(_F("%.1f", gPreset_Friction[SelectedIndex]));
							mTest.mEditBox_CircleRadius->SetText(_F("%.1f", gPreset_CircleRadius[SelectedIndex]));
							mTest.mEditBox_NbPerCircle->SetText(_F("%d", gPreset_NbPerCircle[SelectedIndex]));
							mTest.mEditBox_ShrinkFactor->SetText(_F("%.3f", gPreset_ShrinkFactor[SelectedIndex]));
							mTest.mEditBox_NbRounds->SetText(_F("%d", gPreset_NbRounds[SelectedIndex]));
							mTest.mCheckBox_PushLine->SetChecked(gPreset_PushLine[SelectedIndex]);
						}

						mTest.mMustResetTest = true;
					}
				}
			};

			ComboBoxDesc CBBD;
			CBBD.mID		= 0;
			CBBD.mParent	= UI;
			CBBD.mX			= 4+OffsetX;
			CBBD.mY			= y;
			CBBD.mWidth		= 200;
			CBBD.mHeight	= 20;
			CBBD.mLabel		= "Presets";
			mComboBox_Preset = ICE_NEW(MyComboBox)(CBBD, *this);
			RegisterUIElement(mComboBox_Preset);
			mComboBox_Preset->Add("DominosHighFriction - PEEL 1.1 test");
			mComboBox_Preset->Add("DominosLowFriction - PEEL 1.1 test");
			mComboBox_Preset->Add("DominosEx - PEEL 1.1 test");
			mComboBox_Preset->Add("Dominos");
			mComboBox_Preset->Add("User-defined");
			mComboBox_Preset->Select(3);
			mComboBox_Preset->SetVisible(true);
			mComboBox_Preset->OnComboBoxEvent(CBE_SELECTION_CHANGED);
			y += YStep;

			y += YStep;
			AddResetButton(UI, 4, y, WD.mWidth-4*2*2);
		}
		return null;
	}

	virtual	const char*		GetSubName()	const
	{
		if(mComboBox_Preset)
		{
			const udword SelectedIndex = mComboBox_Preset->GetSelectedIndex();
			if(SelectedIndex==0)
				return "DominosHighFriction";
			else if(SelectedIndex==1)
				return "DominosLowFriction";
			else if(SelectedIndex==2)
				return "DominosEx";
			else if(SelectedIndex==3)
				return "Dominos";
		}
		return null;
	}

	virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);

		if(mComboBox_Preset)
		{
			const udword SelectedIndex = mComboBox_Preset->GetSelectedIndex();
			if(SelectedIndex==3)
			{
				desc.mCamera[0] = PintCameraPose(Point(49.77f, 10.49f, 1.77f), Point(-0.87f, -0.35f, 0.34f));
				desc.mCamera[1] = PintCameraPose(Point(-3.86f, 71.03f, 6.06f), Point(0.04f, -1.00f, -0.06f));
			}
			else if(SelectedIndex==2)
				desc.mCamera[0] = PintCameraPose(Point(24.95f, 21.33f, 24.20f), Point(-0.57f, -0.63f, -0.53f));
			else
				desc.mCamera[0] = PintCameraPose(Point(13.88f, 11.57f, 13.32f), Point(-0.60f, -0.57f, -0.57f));
		}
		SetDefEnv(desc, true);
	}

	virtual	bool			Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float Friction = GetFloat(0.5f, mEditBox_Friction);
		const float CircleRadius = GetFloat(10.0f, mEditBox_CircleRadius);
		const float ShrinkFactor = GetFloat(0.0f, mEditBox_ShrinkFactor);
		const udword NbRounds = GetInt(1, mEditBox_NbRounds);
		const udword NbPerCircle = GetInt(128, mEditBox_NbPerCircle);
		const bool PushLine = mCheckBox_PushLine ? mCheckBox_PushLine->IsChecked() : false;

		const PINT_MATERIAL_CREATE MatDesc(0.0f, Friction, 0.0f);

		const Point Extents(0.1f, 0.5f, 1.0f);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);
		BoxDesc.mMaterial	= &MatDesc;

		const udword Nb = NbPerCircle;
		Point* Pts = (Point*)StackAlloc(Nb*sizeof(Point));
		GenerateCirclePts(Nb, Pts, CircleRadius, 0.0f);

		const bool ApplyInitialImpulse = true;

		float Scale = 1.0f;
		for(udword j=0;j<NbRounds;j++)
		{
			for(udword i=0;i<Nb;i++)
			{
//				const float CoeffI = float(i)/float(Nb);

				PintArticHandle Articulation = null;
				if(0)
					Articulation = pint.CreateArticulation(PINT_ARTICULATION_CREATE());

				PintArticHandle RCArticulation = null;
				if(0)
					RCArticulation = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE());

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass		= 1.0f;
				ObjectDesc.mPosition	= Pts[i]*Scale + Point(0.0f, Extents.z, 0.0f);
				Scale -= ShrinkFactor*(1.0f/float(Nb));

				const Point Dir = (Pts[(i+1)%Nb] - Pts[i]).Normalize();
				Point Right, Up;
				ComputeBasis(Dir, Right, Up);

				Matrix3x3 Rot;
				Rot[0] = Dir;
				Rot[1] = Right;
				Rot[2] = Up;
				ObjectDesc.mRotation = Rot;

/*				if(i==0 && j==0)
				{
					ObjectDesc.mAngularVelocity = Point(10.0f, 0.0f, 0.0f);
				}*/

				if(1)
				{
					if(ApplyInitialImpulse && i==0)
					{
						if(PushLine || j==0)
						{
	//						pint.AddWorldImpulseAtWorldPos(h, Point(0.0f, 0.0f, 4.0f), ObjectDesc.mPosition + Point(0.0f, Extents.y, 0.0f));
							ObjectDesc.mLinearVelocity = Point(0.0f, 0.0f, 4.0f);
							ObjectDesc.mAngularVelocity = Point(4.0f, 0.0f, 0.0f);
						}
					}
				}

				PintActorHandle h;
				if(Articulation)
				{
					PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
					h = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);

					pint.AddArticulationToScene(Articulation);

				}
				else if(RCArticulation)
				{
					PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
					h = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCArticulation);

					pint.AddRCArticulationToScene(RCArticulation);
				}
				else
				{
					h = CreatePintObject(pint, ObjectDesc);
				}

				if(0)
				{
					if(ApplyInitialImpulse && i==0)
					{
						if(PushLine || j==0)
						{
							pint.AddWorldImpulseAtWorldPos(h, Point(0.0f, 0.0f, 4.0f), ObjectDesc.mPosition + Point(0.0f, Extents.y, 0.0f));
						}
					}
				}
			}
		}

		return true;
	}

}DominosConfigurable;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_DoubleDominoEffect = "Double domino effect.";

START_TEST(DoubleDominoEffect, CATEGORY_BEHAVIOR, gDesc_DoubleDominoEffect)

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("DoubleDominoEffect", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(0.91f, 1.15f, 3.16f), Point(-0.50f, -0.69f, -0.52f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float Scale = 0.5f;
		const Point Extents(0.1f*Scale, 0.2f*Scale, 0.02f*Scale);

		PINT_BOX_CREATE Create(Extents);
		Create.mRenderer	= CreateBoxRenderer(Extents);

		const udword NbDominoes = 16;
		PintActorHandle Handles[NbDominoes];

		for(udword i=0;i<NbDominoes;i++)
			Handles[i] = CreateDynamicObject(pint, &Create, Point(0.0f, Extents.y, Extents.y*2.0f*float(i)));

		pint.AddWorldImpulseAtWorldPos(Handles[0], Point(0.0f, 0.0f, 0.1f), Point(0.0f, Extents.y*2.0f, 0.0f));

		return true;
	}

END_TEST(DoubleDominoEffect)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CapsuleStack = "The old capsule stack from the NovodeX SDK. Engines using 'adaptive force' tend to be stable here... until that feature is disabled.";

START_TEST(CapsuleStack, CATEGORY_BEHAVIOR, gDesc_CapsuleStack)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(16.41f, 15.60f, 14.42f), Point(-0.66f, -0.37f, -0.65f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float Radius = 0.5f;
	//	const float Radius = 1.0f;
	//	const float Radius = 2.0f;
		const float HalfHeight = 4.0f;
		const float Scale = 0.75f;

		PINT_CAPSULE_CREATE Create(Radius, HalfHeight);
		Create.mRenderer	= CreateCapsuleRenderer(Create.mRadius, Create.mHalfHeight*2.0f);

		const Quat q0 = ShortestRotation(Point(0.0f, 1.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
		const Quat q1 = ShortestRotation(Point(0.0f, 1.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

		float y = 0.0f;
	//	for(udword i=0;i<1;i++)
		for(udword i=0;i<8;i++)
	//	for(udword i=0;i<6;i++)
	//	for(udword i=0;i<4;i++)
	//	for(udword i=0;i<10;i++)
		{
			CreateDynamicObject(pint, &Create, Point(0.0f, y+Radius, HalfHeight*Scale), &q0);
			CreateDynamicObject(pint, &Create, Point(0.0f, y+Radius, -HalfHeight*Scale), &q0);

			CreateDynamicObject(pint, &Create, Point(HalfHeight*Scale, y+Radius*3.0f, 0.0f), &q1);
			CreateDynamicObject(pint, &Create, Point(-HalfHeight*Scale, y+Radius*3.0f, -0.0f), &q1);

			y += Radius*4.0f;
	//		y += Radius*5.0f;
		}
		return true;
	}

END_TEST(CapsuleStack)

static const char* gDesc_CapsuleStack2 = "Another capsule stack, simpler to simulate.";

START_TEST(CapsuleStack2, CATEGORY_BEHAVIOR, gDesc_CapsuleStack2)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(18.36f, 13.76f, 18.33f), Point(-0.68f, -0.27f, -0.69f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

	//	const float Radius = 0.5f;
		const float Radius = 1.0f;
	//	const float Radius = 2.0f;
		const float HalfHeight = 4.0f;
		const float Scale = 0.75f;

		PINT_CAPSULE_CREATE Create(Radius, HalfHeight);
		Create.mRenderer	= CreateCapsuleRenderer(Create.mRadius, Create.mHalfHeight*2.0f);

		const Quat q0 = ShortestRotation(Point(0.0f, 1.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
		const Quat q1 = ShortestRotation(Point(0.0f, 1.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

		float y = 0.0f;
//		for(udword i=0;i<2;i++)
//		for(udword i=0;i<8;i++)
	//	for(udword i=0;i<6;i++)
		for(udword i=0;i<4;i++)		// We only had 4 layers in the old NX scene
	//	for(udword i=0;i<10;i++)
		{
			CreateDynamicObject(pint, &Create, Point(0.0f, y+Radius, HalfHeight*Scale), &q0);
			CreateDynamicObject(pint, &Create, Point(0.0f, y+Radius, -HalfHeight*Scale), &q0);

			CreateDynamicObject(pint, &Create, Point(HalfHeight*Scale, y+Radius*3.0f, 0.0f), &q1);
			CreateDynamicObject(pint, &Create, Point(-HalfHeight*Scale, y+Radius*3.0f, -0.0f), &q1);

			y += Radius*4.0f;
	//		y += Radius*5.0f;
		}
		return true;
	}

END_TEST(CapsuleStack2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_BoxTower = "The 'capsule stack', but with boxes.";

START_TEST(BoxTower, CATEGORY_BEHAVIOR, gDesc_BoxTower)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(16.41f, 15.60f, 14.42f), Point(-0.66f, -0.37f, -0.65f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float Radius = 0.5f;
	//	const float Radius = 1.0f;
	//	const float Radius = 2.0f;
		const float HalfHeight = 4.0f;
		const float Scale = 0.75f;

		PINT_BOX_CREATE Create(Radius, HalfHeight, Radius);
		Create.mRenderer	= CreateBoxRenderer(Create.mExtents);

		const Quat q0 = ShortestRotation(Point(0.0f, 1.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
		const Quat q1 = ShortestRotation(Point(0.0f, 1.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

		float y = 0.0f;
	//	for(udword i=0;i<1;i++)
		for(udword i=0;i<8;i++)
	//	for(udword i=0;i<6;i++)
	//	for(udword i=0;i<4;i++)
	//	for(udword i=0;i<10;i++)
		{
			CreateDynamicObject(pint, &Create, Point(0.0f, y+Radius, HalfHeight*Scale), &q0);
			CreateDynamicObject(pint, &Create, Point(0.0f, y+Radius, -HalfHeight*Scale), &q0);

			CreateDynamicObject(pint, &Create, Point(HalfHeight*Scale, y+Radius*3.0f, 0.0f), &q1);
			CreateDynamicObject(pint, &Create, Point(-HalfHeight*Scale, y+Radius*3.0f, -0.0f), &q1);

			y += Radius*4.0f;
	//		y += Radius*5.0f;
		}
		return true;
	}

END_TEST(BoxTower)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_StackedSpheres = "Stacked spheres. Just to check if they collapse or not. What is more realistic here? \
It would not feel very natural in the real world if spheres would fall on top of each other and not collapse, would it?";

START_TEST(StackedSpheres, CATEGORY_BEHAVIOR, gDesc_StackedSpheres)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(19.12f, 19.17f, 19.28f), Point(-0.68f, -0.21f, -0.70f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float Radius = 1.0f;

		PINT_SPHERE_CREATE Create(Radius);
		Create.mRenderer = CreateSphereRenderer(Radius);

		for(udword i=0;i<10;i++)
			CreateDynamicObject(pint, &Create, Point(0.0f, Radius*2.0f + (Radius*2.0f+1.0f)*float(i), 0.0f));
		return true;
	}

END_TEST(StackedSpheres)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_StackedCapsules = "Stacked capsules. Mainly to check that the capsule parameters (height? half-height? from top to bottom, or \
from one sphere center to the other?) are properly interpreted by all engines.";

START_TEST(StackedCapsules, CATEGORY_BEHAVIOR, gDesc_StackedCapsules)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(26.89f, 27.29f, 29.30f), Point(-0.69f, -0.16f, -0.71f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float Radius = 1.0f;
		const float HalfHeight = 1.0f;

		PINT_CAPSULE_CREATE Create(Radius, HalfHeight);
		Create.mRenderer = CreateCapsuleRenderer(Radius, HalfHeight*2.0f);

		for(udword i=0;i<10;i++)
			CreateDynamicObject(pint, &Create, Point(0.0f, Radius+HalfHeight*2.0f + (Radius+HalfHeight*2.0f+1.0f)*float(i), 0.0f));
		return true;
	}

END_TEST(StackedCapsules)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_InitialVelocities = "Initial velocities. This test fails with the default max angular velocity value in old PhysX versions, \
but it works just fine if you increase it. After impact, the objects should all move back towards the left of the screen.";

START_TEST(InitialVelocities, CATEGORY_BEHAVIOR, gDesc_InitialVelocities)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-0.30f, 5.93f, 19.15f), Point(0.06f, -0.07f, -1.00f));
		desc.mCamera[1] = PintCameraPose(Point(11.05f, 8.77f, 16.60f), Point(-0.64f, -0.20f, -0.74f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float Radius = 1.0f;

		PINT_SPHERE_CREATE Create(Radius);
		Create.mRenderer	= CreateSphereRenderer(Radius);

		PINT_BOX_CREATE Create2(Radius, Radius, Radius);
		Create2.mRenderer	= CreateBoxRenderer(Create2.mExtents);

		const Point LinVel(0.0f, 10.0f, 0.0f);
		const Point LinVel2(4.0f, 10.0f, 0.0f);
		const Point AngVel(0.0f, 0.0f, 20.0f);

		CreateDynamicObject(pint, &Create, Point(0.0f, Radius*2.0f, 0.0f), null, &LinVel, &AngVel);
		CreateDynamicObject(pint, &Create, Point(0.0f, Radius*2.0f, 4.0f), null, &LinVel2, &AngVel);
		CreateDynamicObject(pint, &Create2, Point(0.0f, Radius*2.0f, 8.0f), null, &LinVel, &AngVel);
		return true;
	}

END_TEST(InitialVelocities)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_GyroscopicPrecession  = "Gyroscopic Precession (a.k.a. spinning tops from the old NovodeX SDK). This test fails \
with the default max angular velocity value in old PhysX versions, but it works just fine if you increase it. You can set angular damping to 0.0 in each engine's UI.";

START_TEST(GyroscopicPrecession, CATEGORY_BEHAVIOR, gDesc_GyroscopicPrecession)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(11.05f, 8.77f, 16.60f), Point(-0.64f, -0.20f, -0.74f));
		SetDefEnv(desc, true);

//		SetDefEnv(desc, false);
//		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportConvexes)
			return false;

//		const float Radius = 1.0f;

		const sdword numSegs = 7;
		const float r = 2.0f;
		const float h = 2.0f;
		Point hullPoints[numSegs+1];
		const float dphi = TWOPI / numSegs;
		for(sdword i=0; i<numSegs; i++)
			hullPoints[i] = Point(r * cosf(i*dphi), h, r * sinf(i*dphi));
		hullPoints[numSegs] = Point(0.0f, 0.0f, 0.0f);

		PINT_CONVEX_CREATE Create(numSegs+1, hullPoints);
		Create.mRenderer	= CreateConvexRenderer(Create.mNbVerts, Create.mVerts);

		const Quat rot (AngleAxis (15.0f * 3.1416f / 180.0f, Point (0.0f, 0.0f, 1.0f)));
		const Point LinVel(0.0f, 0.0f, 0.0f);
		Point AngVel(0.0f, 75.0f, 0.0f);
		AngVel.InvTransform(AngVel, rot, Point (0.0f, 0.0f, 0.0f));
		Point damp(0.0f, 0.0f, 0.0f);

		const sdword count = 8;
		const sdword separation = 6;
		for(sdword x=0; x<count; x++)
		{
			for(sdword z=0; z<count; z++)
			{
				Point posit (float((x - count/2) * separation), h, float((z - count/2) * separation));
				PintActorHandle Handle = CreateDynamicObject(pint, &Create, posit, &rot, &LinVel, &AngVel);
				ASSERT(Handle);
	//			pint.SetLinearAndAngularDamp(Handle, 0.0f, 0.0f);
			}
		}
		return true;
	}

END_TEST(GyroscopicPrecession)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_HighMassRatioCollisions = "High mass ratio collisions. A heavy box moves against a light box, and should easily push it away. \
A light box moves against a heavy box, and should be stopped by it.";

START_TEST(HighMassRatioCollisions, CATEGORY_BEHAVIOR, gDesc_HighMassRatioCollisions)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-13.34f, 8.59f, 20.95f), Point(0.52f, -0.38f, -0.77f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float BoxExtent = 1.0f;

		PINT_BOX_CREATE BoxDesc(BoxExtent, BoxExtent, BoxExtent);
		BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mPosition		= Point(0.0f, BoxExtent, 0.0f);
		ObjectDesc.mMass			= 1.0f;
		ObjectDesc.mLinearVelocity.Zero();
		PintActorHandle Handle		= CreatePintObject(pint, ObjectDesc);
		ASSERT(Handle);

		ObjectDesc.mPosition		= Point(10.0f, BoxExtent, 0.0f);
		ObjectDesc.mMass			= 100.0f;
		ObjectDesc.mLinearVelocity	= Point(-20.0f, 0.0f, 0.0f);
		PintActorHandle Handle2	= CreatePintObject(pint, ObjectDesc);
		ASSERT(Handle2);

		ObjectDesc.mPosition		= Point(0.0f, BoxExtent, 10.0f);
		ObjectDesc.mMass			= 100.0f;
		ObjectDesc.mLinearVelocity.Zero();
		PintActorHandle Handle3	= CreatePintObject(pint, ObjectDesc);
		ASSERT(Handle3);

		ObjectDesc.mPosition		= Point(10.0f, BoxExtent, 10.0f);
		ObjectDesc.mMass			= 1.0f;
		ObjectDesc.mLinearVelocity	= Point(-20.0f, 0.0f, 0.0f);
		PintActorHandle Handle4	= CreatePintObject(pint, ObjectDesc);
		ASSERT(Handle4);

		return true;
	}

END_TEST(HighMassRatioCollisions)

///////////////////////////////////////////////////////////////////////////////

/*static const char* gDesc_HeavyBoxOnLightBox = ".";

START_TEST(HeavyBoxOnLightBox, CATEGORY_BEHAVIOR, gDesc_HeavyBoxOnLightBox)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-13.34f, 8.59f, 20.95f), Point(0.52f, -0.38f, -0.77f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float BoxExtent = 1.0f;

		PINT_BOX_CREATE BoxDesc(BoxExtent, BoxExtent, BoxExtent);
		BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mPosition		= Point(0.0f, BoxExtent, 0.0f);
		ObjectDesc.mMass			= 15.0f;
		ObjectDesc.mMass			= 1000.0f;
		ObjectDesc.mLinearVelocity.Zero();
		PintActorHandle Handle		= CreatePintObject(pint, ObjectDesc);
		ASSERT(Handle);

		ObjectDesc.mPosition		= Point(0.0f, BoxExtent*4.0f, 0.0f);
		ObjectDesc.mMass			= 100.0f;
		ObjectDesc.mMass			= 28000.0f;
		PintActorHandle Handle2	= CreatePintObject(pint, ObjectDesc);
		ASSERT(Handle2);

		return true;
	}

END_TEST(HeavyBoxOnLightBox)*/

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_HighMassRatio = "Basic high mass ratio test. The top box in those box stacks is 100, 200 or 300 times heavier than the other boxes. \
Increase the number of solver iterations or select more accurate solvers in the UI to make this test behave better.";

START_TEST(HighMassRatio, CATEGORY_BEHAVIOR, gDesc_HighMassRatio)

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("High mass ratio settings", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(20.65f, 17.10f, 27.86f), Point(-0.69f, -0.31f, -0.65f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float BoxExtent = 1.0f;

		PINT_BOX_CREATE BoxDesc(BoxExtent, BoxExtent, BoxExtent);
		BoxDesc.mRenderer = CreateBoxRenderer(BoxDesc.mExtents);

		const udword NbStacks = 3;
		for(udword j=0;j<NbStacks;j++)
		{
			udword NbBoxes = 10;
			float BoxPosY = BoxExtent;
			while(NbBoxes)
			{
				for(udword i=0;i<NbBoxes;i++)
				{
					const float Coeff = float(i) - float(NbBoxes)*0.5f;

					PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
					ObjectDesc.mPosition.x	= Coeff * BoxExtent * 2.0f;
					ObjectDesc.mPosition.y	= BoxPosY;
					ObjectDesc.mPosition.z	= float(j) * BoxExtent * 8.0f;
					ObjectDesc.mMass		= NbBoxes == 1 ? float((j+1)*100): 1.0f;
					PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
					ASSERT(Handle);
				}

				NbBoxes--;
				BoxPosY += BoxExtent*2.0f;
			}
		}
		return true;
	}

END_TEST(HighMassRatio)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_InitialPenetration = "A test to check how engines deal with objects created in an initially overlapping state. Ideally this should be resolved \
gently, without objects exploding. In PhysX, this test shows the effect of the 'max depenetration velocity' parameter.";

START_TEST(InitialPenetration, CATEGORY_BEHAVIOR, gDesc_InitialPenetration)

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("Initial penetration settings", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(9.25f, 20.56f, 36.44f), Point(-0.15f, -0.23f, -0.96f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float Scale = 1.0f;
		const float BoxExtent = 3.0f*Scale;
		const float BoxPosY = BoxExtent;
		const udword NbBoxes = 4;
		PintActorHandle H;
		for(udword i=0;i<NbBoxes;i++)
		{
			const float Coeff = float(i);
			H = CreateDynamicBox(pint, BoxExtent, BoxExtent, BoxExtent, Point(0.0f, BoxPosY + Coeff*BoxExtent*1.5f, 0.0f));
	//		H = CreateDynamicBox(pint, BoxExtent, BoxExtent, BoxExtent, Point(0.0f, BoxPosY + Coeff*BoxExtent*5.0f, 0.0f));
//			ASSERT(H);
		}

		{
			H = CreateDynamicBox(pint, BoxExtent, BoxExtent, BoxExtent, Point(10.0f*Scale, BoxPosY*0.5f, 0.0f));
//			ASSERT(H);

			PINT_BOX_CREATE BoxDesc(BoxExtent, BoxExtent, BoxExtent);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
			H = CreateSimpleObject(pint, &BoxDesc, 100.0f, Point(20.0f*Scale, BoxPosY*0.5f, 0.0f));
//			ASSERT(H);
		}

		{
			PINT_SPHERE_CREATE SphereDesc(4.0f*Scale);
			SphereDesc.mRenderer	= CreateSphereRenderer(SphereDesc.mRadius);
			H = CreateDynamicObject(pint, &SphereDesc, Point(-15.0f*Scale, 0.0f, 0.0f));
//			ASSERT(H);
		}
		return true;
	}

END_TEST(InitialPenetration)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_FrictionRamp = "Classical friction ramp test. Static friction is 0.0. Dynamic friction coeffs range from 0.0 to 1.0. \
Note that some engines like PhysX support several friction models, and the default one is not the most realistic. Enable the other ones in the PhysX plugin UI.";

START_TEST(FrictionRamp, CATEGORY_BEHAVIOR, gDesc_FrictionRamp)

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("FrictionRamp", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.60f, -0.47f, -0.65f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float BoxExtent = 10.0f;
		const float BoxPosY = BoxExtent;
		const udword NbBoxes = 8;
		for(udword i=0;i<NbBoxes;i++)
		{
			const PINT_MATERIAL_CREATE MatDesc(0.0f, float(i)/float(NbBoxes-1), 0.0f);

			PINT_BOX_CREATE BoxDesc(BoxExtent*0.5f, 0.5f, BoxExtent);
			BoxDesc.mMaterial	= &MatDesc;
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			const float x = (float(i)-float(NbBoxes)*0.5f)*BoxExtent*1.1f;

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mPosition.x	= x;
			ObjectDesc.mPosition.y	= BoxPosY;
			ObjectDesc.mPosition.z	= 0.0f;

			Matrix3x3 m;
	//		m.RotX(degToRad(40.0f));
	//		m.RotX(degToRad(45.0f));
			m.RotX(degToRad(42.0f));
			ObjectDesc.mRotation = m;

			ObjectDesc.mMass		= 0.0f;
			PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
			ASSERT(Handle);

			PintActorHandle CubeHandle = CreateDynamicBox(pint, 1.0f, 1.0f, 1.0f, Point(x, 18.0f, -6.0f), &ObjectDesc.mRotation, &MatDesc);
			ASSERT(CubeHandle);
		}
		return true;
	}

END_TEST(FrictionRamp)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_FrictionRamp2 = "A variation on the classical friction ramp test. \
Note that some engines like PhysX support several friction models, and the default one is not the most realistic. Enable the other ones in the PhysX plugin UI.";

START_TEST(FrictionRamp2, CATEGORY_BEHAVIOR, gDesc_FrictionRamp2)

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("FrictionRamp2", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.60f, -0.47f, -0.65f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float BoxExtent = 10.0f;
		const float BoxPosY = BoxExtent;
		const udword NbBoxes = 8;
		for(udword i=0;i<NbBoxes;i++)
		{
			const PINT_MATERIAL_CREATE MatDesc(0.0f, float(i)/float(NbBoxes-1), 0.0f);

			PINT_BOX_CREATE BoxDesc(BoxExtent*0.5f, 0.5f, BoxExtent);
			BoxDesc.mMaterial	= &MatDesc;
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			const float x = (float(i)-float(NbBoxes)*0.5f)*BoxExtent*1.1f;

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mPosition.x	= x;
			ObjectDesc.mPosition.y	= BoxPosY;
			ObjectDesc.mPosition.z	= 0.0f;

			Matrix3x3 m;
	//		m.RotX(degToRad(40.0f));
	//		m.RotX(degToRad(45.0f));
			m.RotX(degToRad(42.0f));
			ObjectDesc.mRotation = m;

			ObjectDesc.mMass		= 0.0f;
			PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
			ASSERT(Handle);

			for(udword j=0;j<4;j++)
			{
				PintActorHandle BoxHandle = CreateDynamicBox(pint, 4.0f, 0.5f, 4.0f, Point(x, 18.0f + float(j)*2.0f, -6.0f), &ObjectDesc.mRotation, &MatDesc);
				ASSERT(BoxHandle);
			}
		}
		return true;
	}

END_TEST(FrictionRamp2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_FrictionRamp3 = "A variation on the classical friction ramp test. Each box has an initial angular velocity of 10. \
Note that some engines like PhysX support several friction models, and the default one is not the most realistic. Enable the other ones in the PhysX plugin UI.";

START_TEST(FrictionRamp3, CATEGORY_BEHAVIOR, gDesc_FrictionRamp3)

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("FrictionRamp3", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.60f, -0.47f, -0.65f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float BoxExtent = 10.0f;
		const float BoxPosY = BoxExtent;
		const udword NbBoxes = 8;
		for(udword i=0;i<NbBoxes;i++)
		{
			const PINT_MATERIAL_CREATE MatDesc(0.0f, float(i)/float(NbBoxes-1), 0.0f);

			PINT_BOX_CREATE BoxDesc(BoxExtent*0.5f, 0.5f, BoxExtent);
			BoxDesc.mMaterial	= &MatDesc;
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			const float x = (float(i)-float(NbBoxes)*0.5f)*BoxExtent*1.1f;

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mPosition.x	= x;
			ObjectDesc.mPosition.y	= BoxPosY;
			ObjectDesc.mPosition.z	= 0.0f;

			Matrix3x3 m;
			m.RotX(degToRad(30.0f));
			ObjectDesc.mRotation = m;

			ObjectDesc.mMass		= 0.0f;
			PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
			ASSERT(Handle);

	//		PintActorHandle BoxHandle = CreateDynamicBox(pint, 4.0f, 0.5f, 4.0f, Point(x, 13.5f, -4.0f), &ObjectDesc.mRotation, &MatDesc);
	//		ASSERT(BoxHandle);
			{
				PINT_BOX_CREATE BoxDesc(2.0f, 0.5f, 2.0f);
				BoxDesc.mMaterial	= &MatDesc;
				BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= 1.0f;
				ObjectDesc.mPosition		= Point(x, 13.5f, -4.0f);
				ObjectDesc.mRotation		= m;
				ObjectDesc.mAngularVelocity	= Point(0.0f, 10.0f, 0.0f);
				PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
				ASSERT(Handle);
			}
		}
		return true;
	}

END_TEST(FrictionRamp3)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Restitution = "Classical restitution test. Coeffs of restitution range from 0.0 to 1.0. Ideally when restitution is 1.0, the falling object should \
bounce as high as the position it started from, but not higher. If it does, the physics engine is adding energy into the system (which is wrong). (Disable linear damping \
to check if an engine is correct or not, as damping usually hides the issue).";

START_TEST(Restitution, CATEGORY_BEHAVIOR, gDesc_Restitution)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-2.57f, 23.39f, 45.13f), Point(0.01f, -0.36f, -0.93f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float BoxExtent = 3.0f;
		const float BoxPosY = BoxExtent;
		const udword NbBoxes = 8;
		const float Radius = 1.0f;

		PintShapeRenderer* RenderObject = CreateSphereRenderer(Radius);

		for(udword i=0;i<NbBoxes;i++)
		{
			const PINT_MATERIAL_CREATE MatDesc(0.5f, 0.5f, float(i)/float(NbBoxes-1));

			PINT_BOX_CREATE BoxDesc(BoxExtent*0.5f, 0.5f, BoxExtent);
			BoxDesc.mMaterial	= &MatDesc;
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			const float x = (float(i)-float(NbBoxes)*0.5f)*BoxExtent*1.1f;

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mPosition.x	= x;
			ObjectDesc.mPosition.y	= BoxPosY;
			ObjectDesc.mPosition.z	= 0.0f;

			ObjectDesc.mMass		= 0.0f;
			PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
			ASSERT(Handle);

			PINT_SPHERE_CREATE SphereDesc(Radius);
			SphereDesc.mMaterial	= &MatDesc;
			SphereDesc.mRenderer	= RenderObject;
			PintActorHandle SphereHandle = CreateDynamicObject(pint, &SphereDesc, Point(x, 20.0f, 0.0f));
			ASSERT(SphereHandle);
		}
		return true;
	}

END_TEST(Restitution)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_GravityRace = "A dynamic box is dropped 1000 units away from the origin, and falls towards it. In theory boxes simulated with different engines \
should all touch the ground at the same time... In practice the different implementations of 'linear damping' make boxes arrive at different times (i.e. even if they use the \
same numerical value for linear damping). Wait until frame 1000 or so, for the boxes to reach the ground.";

START_TEST(GravityRace, CATEGORY_BEHAVIOR, gDesc_GravityRace)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const Point Extents(1.0f, 1.0f, 1.0f);

		PINT_BOX_CREATE Desc(Extents);
		Desc.mRenderer	= CreateBoxRenderer(Extents);
		PintActorHandle Handle = CreateDynamicObject(pint, &Desc, Point(0.0f, 1000.0f, 0.0f));
		ASSERT(Handle);
		return true;
	}

END_TEST(GravityRace)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ConvexCompoundsChain = "A chain made of 20 torus object. Each torus is a compound of 32 convexes. This test shows the artefacts caused \
by the 'adaptive force' hack. The chain will appear to fall more slowly with engines using 'adaptive force'. Pick & drag objects to test each engine's stability. \
This is also a CCD torture test (all engines suffer). This test also exposes issues when the contact offset and/or the max depenetration velocities are too small \
in PhysX (the chain breaks).";

START_TEST(ConvexCompoundsChain, CATEGORY_BEHAVIOR, gDesc_ConvexCompoundsChain)

	virtual	float	GetRenderData(Point& center)	const	{ return 400.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(54.56f, 47.52f, 68.45f), Point(-0.48f, -0.05f, -0.87f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation || !caps.mSupportCompounds)
			return false;

		// Generate torus
		const float BigRadius = 3.0f;
	//	const float SmallRadius = 1.0f;
		const float SmallRadius = 0.75f;
	//	const udword NbPtsSmallCircle = 8;
	//	const udword NbSlices = 16;
		const udword NbPtsSmallCircle = 16;
		const udword NbSlices = 32;

		Point Pts[NbPtsSmallCircle];
		GeneratePolygon(NbPtsSmallCircle, Pts, sizeof(Point), ORIENTATION_XY, SmallRadius);

		udword TotalNbVerts = NbPtsSmallCircle * NbSlices;
		Point* Verts = ICE_NEW(Point)[TotalNbVerts];

		udword Index = 0;
		for(udword j=0;j<NbSlices;j++)
		{
			const float Coeff = float(j)/float(NbSlices);

			Matrix3x3 Rot;
			Rot.RotX(Coeff * TWOPI);

			const Point Trans = Rot[1]*BigRadius;
			for(udword i=0;i<NbPtsSmallCircle;i++)
				Verts[Index++] = Trans + Pts[i]*Rot; 
		}
		ASSERT(Index==TotalNbVerts);

		PINT_CONVEX_CREATE ConvexCreate[NbSlices];
		Point ConvexPts[NbSlices*NbPtsSmallCircle*2];
		udword Offset = 0;

		for(udword s=0;s<NbSlices;s++)
		{
			const udword SliceIndex0 = s;
			const udword SliceIndex1 = (s+1)%NbSlices;
			const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
			const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

	//		Point ConvexPts[NbPtsSmallCircle*2];
			for(udword i=0;i<NbPtsSmallCircle;i++)
			{
	//			ConvexPts[i] = V0[i];
	//			ConvexPts[i+NbPtsSmallCircle] = V1[i];
				ConvexPts[Offset+i] = V0[i];
				ConvexPts[Offset+i+NbPtsSmallCircle] = V1[i];
			}

			if(1)	// Recenter vertices
			{
				Point Center(0.0f, 0.0f, 0.0f);
				const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
				for(udword i=0;i<NbPtsSmallCircle*2;i++)
					Center += ConvexPts[Offset+i] * Coeff;

				for(udword i=0;i<NbPtsSmallCircle*2;i++)
					ConvexPts[Offset+i] -= Center;

				ConvexCreate[s].mLocalPos = Center;// * 1.1f;
			}

			ConvexCreate[s].mNbVerts	= NbPtsSmallCircle*2;
			ConvexCreate[s].mVerts		= &ConvexPts[Offset];
			ConvexCreate[s].mRenderer	= CreateConvexRenderer(ConvexCreate[s].mNbVerts, ConvexCreate[s].mVerts);
			if(s!=NbSlices-1)
				ConvexCreate[s].SetNext(&ConvexCreate[s+1]);
			Offset += NbPtsSmallCircle*2;

	/*
			PINT_CONVEX_CREATE ConvexCreate;
			ConvexCreate.mNbVerts	= NbPtsSmallCircle*2;
			ConvexCreate.mVerts		= ConvexPts;
			ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexPts);

			const Point pos(0.0f, 10.0f, 0.0f);
			PintActorHandle Handle = CreateDynamicObject(pint, &ConvexCreate, pos);
			ASSERT(Handle);*/
		}
		DELETEARRAY(Verts);

		const udword NbObjects = 20;
		Matrix3x3 RotX;	RotX.RotZ(HALFPI);
		Matrix3x3 RotY;	RotY.RotY(HALFPI);
		const Quat QX = RotX;
		const Quat QY = RotY;
		for(udword i=0;i<NbObjects;i++)
		{
	//		const Quat* Rot0 = i&1 ? &Q : null;
			const Quat* Rot0 = i&1 ? &QX : &QY;

	//		const Point pos(0.0f, 30.0f + float(i)*4.0f, 0.0f);
			const Point pos(float(i)*4.0f, 10.0f + float(NbObjects)*4.0f, 0.0f);
	//		if(i==NbObjects-1)
			if(i==0)
			{
				PintActorHandle Handle = CreateStaticObject(pint, &ConvexCreate[0], pos, Rot0);
				ASSERT(Handle);
			}
			else
			{
				PintActorHandle Handle = CreateDynamicObject(pint, &ConvexCreate[0], pos, Rot0);
				ASSERT(Handle);
			}
		}
		return true;
	}

END_TEST(ConvexCompoundsChain)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_BoxCompoundsChain = "A chain made of 20 torus object. Each torus is a compound of 32 boxes. This is a variation on previous test.";

START_TEST(BoxCompoundsChain, CATEGORY_BEHAVIOR, gDesc_BoxCompoundsChain)

	virtual	float	GetRenderData(Point& center)	const	{ return 400.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(63.41f, 50.44f, 84.89f), Point(-0.47f, -0.07f, -0.88f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportCompounds)
			return false;

		// Generate torus
		const float BigRadius = 3.0f;
	//	const float SmallRadius = 1.0f;
		const float SmallRadius = 0.75f;
	//	const udword NbPtsSmallCircle = 8;
	//	const udword NbSlices = 16;
		const udword NbPtsSmallCircle = 16;
		const udword NbSlices = 32;

		Point Pts[NbPtsSmallCircle];
		GeneratePolygon(NbPtsSmallCircle, Pts, sizeof(Point), ORIENTATION_XY, SmallRadius);

		udword TotalNbVerts = NbPtsSmallCircle * NbSlices;
		Point* Verts = ICE_NEW(Point)[TotalNbVerts];

		udword Index = 0;
		for(udword j=0;j<NbSlices;j++)
		{
			const float Coeff = float(j)/float(NbSlices);

			Matrix3x3 Rot;
			Rot.RotX(Coeff * TWOPI);

			const Point Trans = Rot[1]*BigRadius;
			for(udword i=0;i<NbPtsSmallCircle;i++)
				Verts[Index++] = Trans + Pts[i]*Rot; 
		}
		ASSERT(Index==TotalNbVerts);

		PINT_BOX_CREATE BoxCreate[NbSlices];
		Point ConvexPts[NbSlices*NbPtsSmallCircle*2];
		udword Offset = 0;

		for(udword s=0;s<NbSlices;s++)
		{
			const udword SliceIndex0 = s;
			const udword SliceIndex1 = (s+1)%NbSlices;
			const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
			const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

	//		Point ConvexPts[NbPtsSmallCircle*2];
			for(udword i=0;i<NbPtsSmallCircle;i++)
			{
	//			ConvexPts[i] = V0[i];
	//			ConvexPts[i+NbPtsSmallCircle] = V1[i];
				ConvexPts[Offset+i] = V0[i];
				ConvexPts[Offset+i+NbPtsSmallCircle] = V1[i];
			}

			if(1)	// Recenter vertices
			{
				Point Center(0.0f, 0.0f, 0.0f);
				const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
				for(udword i=0;i<NbPtsSmallCircle*2;i++)
					Center += ConvexPts[Offset+i] * Coeff;

				for(udword i=0;i<NbPtsSmallCircle*2;i++)
					ConvexPts[Offset+i] -= Center;

				BoxCreate[s].mLocalPos = Center;// * 1.1f;
			}

//			BoxCreate[s].mNbVerts	= NbPtsSmallCircle*2;
//			BoxCreate[s].mVerts		= &ConvexPts[Offset];
			BoxCreate[s].mExtents	= Point(1.0f, 0.5f, 0.5f);
			BoxCreate[s].mRenderer	= CreateBoxRenderer(BoxCreate[s].mExtents);
			if(s!=NbSlices-1)
				BoxCreate[s].SetNext(&BoxCreate[s+1]);
			Offset += NbPtsSmallCircle*2;
		}
		DELETEARRAY(Verts);

		const udword NbObjects = 20;
		Matrix3x3 RotX;	RotX.RotZ(HALFPI);
		Matrix3x3 RotY;	RotY.RotY(HALFPI);
		const Quat QX = RotX;
		const Quat QY = RotY;
		for(udword i=0;i<NbObjects;i++)
		{
	//		const Quat* Rot0 = i&1 ? &Q : null;
			const Quat* Rot0 = i&1 ? &QX : &QY;

	//		const Point pos(0.0f, 30.0f + float(i)*4.0f, 0.0f);
			const Point pos(float(i)*4.0f, 10.0f + float(NbObjects)*4.0f, 0.0f);
	//		if(i==NbObjects-1)
			if(i==0)
			{
				PintActorHandle Handle = CreateStaticObject(pint, &BoxCreate[0], pos, Rot0);
				ASSERT(Handle);
			}
			else
			{
				PintActorHandle Handle = CreateDynamicObject(pint, &BoxCreate[0], pos, Rot0);
				ASSERT(Handle);
			}
		}
		return true;
	}

END_TEST(BoxCompoundsChain)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CompoundTowerTweaked = "Compound tower. Can you tweak the settings so that the tower doesn't collapse?";

START_TEST(CompoundTowerTweaked, CATEGORY_BEHAVIOR, gDesc_CompoundTowerTweaked)

	virtual	float	GetRenderData(Point& center)	const	{ return 400.0f;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("CompoundTowerTweaked", 20);
	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(7.54f, 70.68f, 8.23f), Point(-0.42f, -0.81f, -0.42f));
		desc.mCamera[1] = PintCameraPose(Point(50.08f, 0.78f, 45.65f), Point(-0.64f, 0.46f, -0.62f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportCompounds)
			return false;

		const Point Extents(1.5f, 0.5f, 0.5f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		const Point Extents2(0.5f, 0.5f, 1.5f);
		PINT_BOX_CREATE BoxDesc2(Extents2);
		BoxDesc2.mRenderer	= CreateBoxRenderer(Extents2);
		BoxDesc2.SetNext(&BoxDesc);

		for(udword i=0;i<64;i++)
		{
			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc2);
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mPosition	= Point(0.0f, float(i)*2.0f, 0.0f);
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(CompoundTowerTweaked)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_BoxOnRotatingPlanarMesh = "Unity bug report: a (potentially sleeping) box on a rotating planar mesh. Although the mesh is static, its rotation is \
modified directly via a 'setGlobalPose' kind of call. Depending on whether the box is sleeping or not when it happens, and whether they are prepared for this scenario, \
various engines show various issues with this test. This is technically illegal since the mesh is static, but users do that kind of things all the time anyway.";

START_TEST(BoxOnRotatingPlanarMesh, CATEGORY_BEHAVIOR, gDesc_BoxOnRotatingPlanarMesh)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-34.96f, 0.71f, -8.55f), Point(0.99f, -0.04f, 0.12f));
		SetDefEnv(desc, false);
	}

	virtual	bool		CommonSetup()
	{
		mPlanarMeshHelper.Generate(8, 0.1f);
		return TestBase::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes || !caps.mSupportRigidBodySimulation)
			return false;

		const float Altitude = 0.0f;
		const PintActorHandle h = mPlanarMeshHelper.CreatePlanarMesh(pint, Altitude, null);
		pint.mUserData = h;

		const float Radius = 0.5f;
		const Point Extents(Radius, Radius, Radius);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass		= 1.0f;
		ObjectDesc.mPosition	= Point(0.0f, Radius, -10.0f);
		CreatePintObject(pint, ObjectDesc);
		return true;
	}

	virtual	void	OnObjectReleased(Pint& pint, PintActorHandle removed_object)
	{
		if(pint.mUserData==removed_object)
			pint.mUserData = null;
	}

	virtual	udword	Update(Pint& pint, float dt)
	{
		PintActorHandle h = PintActorHandle(pint.mUserData);
		if(h)
		{
			if(mCurrentTime>2.0f)
			{
				PR Pose = pint.GetWorldTransform(h);

				Matrix3x3 Rot;
//				Rot.RotX(-PI/4.0f);
				Rot.RotX(-PI/4.0f - (mCurrentTime-2.0f)*0.04f);

				Pose.mRot = Rot;
				pint.SetWorldTransform(h, Pose);
			}
		}
		return TestBase::Update(pint, dt);
	}

END_TEST(BoxOnRotatingPlanarMesh)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CardHouse = "Testing stacking of light thin objects";

START_TEST(CardHouse, CATEGORY_BEHAVIOR, gDesc_CardHouse)

	virtual	float	GetRenderData(Point& center)	const	{ return 20.0f;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("CardHouse", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(-2.25f, 0.32f, -0.11f), Point(0.90f, -0.05f, 0.43f));
		//desc.mCamera[0] = PintCameraPose(Point(-1.81f, 1.09f, 2.20f), Point(0.80f, -0.15f, -0.58f));
		desc.mCamera[0] = PintCameraPose(Point(-1.78f, 1.05f, 1.57f), Point(0.92f, -0.11f, -0.37f));
		SetDefEnv(desc, true);
	}

/*	virtual	bool		CommonSetup()
	{
		mPMH.Generate(*this, 8, 0.1f);
		return TestBase::CommonSetup();
	}*/

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float CardWidth = 0.1f;
		const float CardHeight = 0.2f;
		const float CardThickness = 0.001f;

		Matrix3x3 Rot0;	Rot0.RotX(25.0f*DEGTORAD);
		Matrix3x3 Rot1;	Rot1.RotX(-25.0f*DEGTORAD);
		Matrix3x3 Rot2;	Rot2.RotX(90.0f*DEGTORAD);

		const Point Extents(CardWidth, CardHeight, CardThickness);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass	= 0.01f;

		udword Nb = 5;
		float z0 = 0.0f;
		float y = CardHeight - 0.02f;
		while(Nb)
		{
			float z = z0;
			for(udword i=0;i<Nb;i++)
			{
				if(i!=Nb-1)
				{
					ObjectDesc.mPosition	= Point(0.0f, y+CardHeight-0.015f, z + 0.25f);
					ObjectDesc.mRotation	= Rot2;
					CreatePintObject(pint, ObjectDesc);
				}
				ObjectDesc.mPosition	= Point(0.0f, y, z);
				ObjectDesc.mRotation	= Rot0;
				CreatePintObject(pint, ObjectDesc);

				z += 0.175f;

				ObjectDesc.mPosition	= Point(0.0f, y, z);
				ObjectDesc.mRotation	= Rot1;
				CreatePintObject(pint, ObjectDesc);

				z += 0.175f;
			}
			y += CardHeight*2.0f - 0.03f;
			z0 += 0.175f;
			Nb--;
		}
		return true;
	}

/*	virtual	udword	Update(Pint& pint, float dt)
	{
		PintActorHandle h = (PintActorHandle)pint.mUserData;
		if(h)
		{
			if(mCurrentTime>2.0f)
			{
				PR Pose = pint.GetWorldTransform(h);

				Matrix3x3 Rot;
//				Rot.RotX(-PI/4.0f);
				Rot.RotX(-PI/4.0f - (mCurrentTime-2.0f)*0.04f);

				Pose.mRot = Rot;
				pint.SetWorldTransform(h, Pose);
			}
		}
		return TestBase::Update(pint, dt);
	}*/

END_TEST(CardHouse)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Dzhanibekov = "Dzhanibekov effect. In the PhysX family only PhysX5 handles it correctly, after enabling gyroscopic forces.";

START_TEST(Dzhanibekov, CATEGORY_BEHAVIOR, gDesc_Dzhanibekov)

	virtual	float	GetRenderData(Point& center)	const	{ return 20.0f;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("Dzhanibekov", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(0.65f, 0.56f, 1.63f), Point(-0.36f, -0.19f, -0.91f));
		SetDefEnv(desc, false);
		desc.mGravity.Zero();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const Point Extents0(0.05f, 0.5f, 0.05f);
		PINT_BOX_CREATE BoxDesc(Extents0);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents0);

		const Point Extents1(0.1f, 0.05f, 0.05f);
		PINT_BOX_CREATE BoxDesc2(Extents1);
		BoxDesc2.mLocalPos = Point(0.1f, 0.0f, 0.0f);
		BoxDesc2.mRenderer	= CreateBoxRenderer(Extents1);

		BoxDesc.SetNext(&BoxDesc2);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass	= 0.1f;
		ObjectDesc.mAngularVelocity	= Point(30.f, 20.1f, 0.0f)*0.25f;
		CreatePintObject(pint, ObjectDesc);
		return true;
	}

END_TEST(Dzhanibekov)

///////////////////////////////////////////////////////////////////////////////
