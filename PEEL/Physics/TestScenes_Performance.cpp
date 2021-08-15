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
#include "GUI_Helpers.h"
//#include "Camera.h"
//#include "PintSQ.h"

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_BoxStackConfigurable = "(Configurable test) - Box stacks. Things to look for are the time it takes to simulate the stacks, \
but also whether they eventually collapse or not. Selecting multiple stacks creates multiple simulation islands that stress engines in different ways.";

#define NB_BOX_STACKS_PRESETS	5
static const udword	gPreset_NbStacks[]		= { 1,   1,  1, 10, 30 };
static const udword	gPreset_NbBaseBoxes[]	= { 20, 30, 50, 20, 10 };

class BoxStackConfigurable : public TestBase//, public PintContactNotifyCallback
{
			IceComboBox*	mComboBox_Preset;
			IceEditBox*		mEditBox_NbBaseBoxes;
			IceEditBox*		mEditBox_NbStacks;
			IceCheckBox*	mCheckBox_UseConvexes;
	public:
							BoxStackConfigurable() :
								mComboBox_Preset		(null),
								mEditBox_NbBaseBoxes	(null),
								mEditBox_NbStacks		(null),
								mCheckBox_UseConvexes	(null)	{}
	virtual					~BoxStackConfigurable()		{										}
	virtual	const char*		GetName()			const	{ return "BoxStacks";					}
	virtual	const char*		GetDescription()	const	{ return gDesc_BoxStackConfigurable;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_PERFORMANCE;			}

	virtual	float			GetRenderData(Point& center)	const	{ return 400.0f;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 300;
		WD.mLabel	= "Box stacks";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 100;
		const sdword LabelWidth = 80;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Nb base boxes:", &UIElems);
			mEditBox_NbBaseBoxes = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			mEditBox_NbBaseBoxes->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Nb stacks:", &UIElems);
			mEditBox_NbStacks = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			mEditBox_NbStacks->SetEnabled(false);
			y += YStep;

			mCheckBox_UseConvexes = helper.CreateCheckBox(UI, 0, 4, y, 200, 20, "Use convexes", &UIElems, false, null, null);
			y += YStep;
		}
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Presets:", &UIElems);

			class MyComboBox : public IceComboBox
			{
				BoxStackConfigurable&	mTest;
				public:
								MyComboBox(const ComboBoxDesc& desc, BoxStackConfigurable& test) : IceComboBox(desc), mTest(test)	{}
				virtual			~MyComboBox()																						{}

				virtual	void	OnComboBoxEvent(ComboBoxEvent event)
				{
					if(event==CBE_SELECTION_CHANGED)
					{
						mTest.mMustResetTest = true;
						const udword SelectedIndex = GetSelectedIndex();
						const bool Enabled = SelectedIndex==GetItemCount()-1;
						mTest.mEditBox_NbBaseBoxes->SetEnabled(Enabled);
						mTest.mEditBox_NbStacks->SetEnabled(Enabled);
						mTest.mCheckBox_UseConvexes->SetEnabled(Enabled);

						if(!Enabled && SelectedIndex<NB_BOX_STACKS_PRESETS)
						{
							mTest.mEditBox_NbBaseBoxes->SetText(_F("%d", gPreset_NbBaseBoxes[SelectedIndex]));
							mTest.mEditBox_NbStacks->SetText(_F("%d", gPreset_NbStacks[SelectedIndex]));
						}
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
			mComboBox_Preset->Add("MediumBoxStack20 - PEEL 1.1 test");
			mComboBox_Preset->Add("LargeBoxStack30 - PEEL 1.1 test");
			mComboBox_Preset->Add("LargeBoxStack50 - PEEL 1.1 test");
			mComboBox_Preset->Add("MediumBoxStacks20 - PEEL 1.1 test");
			mComboBox_Preset->Add("ManySmallBoxStacks10 - PEEL 1.1 test");			
			mComboBox_Preset->Add("User-defined");
			mComboBox_Preset->Select(2);
			mComboBox_Preset->SetVisible(true);
			mComboBox_Preset->OnComboBoxEvent(CBE_SELECTION_CHANGED);
			y += YStep;

			y += YStep;
			AddResetButton(UI, 4, y, WD.mWidth-4*2*2);
		}

		IceTabControl* TabControl;
		{
			TabControlDesc TCD;
			TCD.mParent	= UI;
			TCD.mX		= 4;
			TCD.mY		= y + 30;
			TCD.mWidth	= WD.mWidth - 16;
			TCD.mHeight	= 140;
			TabControl = ICE_NEW(IceTabControl)(TCD);
			RegisterUIElement(TabControl);
		}
		return TabControl;
	}

	virtual	const char*		GetSubName()	const
	{
		if(mComboBox_Preset)
		{
			const udword SelectedIndex = mComboBox_Preset->GetSelectedIndex();
			if(SelectedIndex==0)
				return "MediumBoxStack20";
			else if(SelectedIndex==1)
				return "LargeBoxStack30";
			else if(SelectedIndex==2)
				return "LargeBoxStack50";
			else if(SelectedIndex==3)
				return "MediumBoxStacks20";
			else if(SelectedIndex==4)
				return "ManySmallBoxStacks10";
		}
		return null;
	}

	virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);

		if(mComboBox_Preset)
		{
			const udword SelectedIndex = mComboBox_Preset->GetSelectedIndex();
			if(SelectedIndex==0)
			{
				desc.mCamera[0] = PintCameraPose(Point(26.29f, 36.32f, 35.39f), Point(-0.51f, -0.35f, -0.79f));
			}
			else if(SelectedIndex==1)
			{
				desc.mCamera[0] = PintCameraPose(Point(47.56f, 48.75f, 47.08f), Point(-0.61f, -0.31f, -0.73f));
			}
			else if(SelectedIndex==2)
			{
				desc.mCamera[0] = PintCameraPose(Point(69.17f, 51.73f, 80.45f), Point(-0.57f, -0.08f, -0.82f));
				desc.mCamera[1] = PintCameraPose(Point(1.25f, 99.61f, 9.47f), Point(-0.28f, -0.16f, -0.95f));
			}
			else if(SelectedIndex==3)
			{
				desc.mCamera[0] = PintCameraPose(Point(45.93f, 35.48f, 36.77f), Point(-0.74f, -0.33f, -0.58f));
			}
			else if(SelectedIndex==4)
			{
				desc.mCamera[0] = PintCameraPose(Point(21.99f, 20.23f, 73.95f), Point(-0.50f, -0.35f, -0.79f));
			}
		}
		SetDefEnv(desc, true);
		//desc.mContactNotifyCallback = this;
	}

	virtual	bool			Setup(Pint& pint, const PintCaps& caps)
	{
		const bool UseConvexes = mCheckBox_UseConvexes ? mCheckBox_UseConvexes->IsChecked() : false;
		const udword NbBaseBoxes = GetInt(1, mEditBox_NbBaseBoxes);
		const udword NbStacks = GetInt(1, mEditBox_NbStacks);
		return CreateBoxStack(pint, caps, NbStacks, NbBaseBoxes, UseConvexes);
	}

/*	virtual	bool	BufferContacts()		const	{ return false;			}
	virtual	udword	GetContactFlags()		const	{ return CONTACT_ALL;	}
	virtual	float	GetContactThreshold()	const	{ return FLT_MAX;		}

	virtual	void	OnContact(Pint& pint, udword nb_contacts, const PintContactData* contacts)
	{
		printf("OnContact: %d\n", nb_contacts);

		while(nb_contacts--)
		{
			const PintContactData& Current = *contacts++;
			if(0)
				printf("%f %f %f\n", Current.mNormal.x, Current.mNormal.y, Current.mNormal.z);
		}
	}*/

}BoxStackConfigurable;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_BoxContainerConfigurable = "(Configurable test) - A static box container filled with various dynamic objects.";

#define NB_BOX_CONTAINER_PRESETS	5
static const float	gPreset_SphereRadius[]	= { 0.5f, 0.0f, 0.0f, 0.5f, 0.5f };
static const float	gPreset_CapsuleRadius[]	= { 0.0f, 0.0f, 0.3f, 0.3f, 0.3f };
static const float	gPreset_HalfHeight[]	= { 0.0f, 0.0f, 0.5f, 0.5f, 0.5f };
static const float	gPreset_BoxSize[]		= { 0.0f, 0.3f, 0.0f, 0.3f, 0.3f };
static const float	gPreset_ConvexSize[]	= { 0.0f, 0.0f, 0.0f, 0.0f, 0.4f };

class BoxContainerConfigurable : public TestBase
{
			IceComboBox*	mComboBox_Preset;
			IceEditBox*		mEditBox_NbX;
			IceEditBox*		mEditBox_NbY;
			IceEditBox*		mEditBox_NbZ;
			IceEditBox*		mEditBox_SphereRadius;
			IceEditBox*		mEditBox_CapsuleRadius;
			IceEditBox*		mEditBox_HalfHeight;
			IceEditBox*		mEditBox_BoxSize;
			IceEditBox*		mEditBox_ConvexSize;
	public:
							BoxContainerConfigurable() :
								mComboBox_Preset		(null),
								mEditBox_NbX			(null),
								mEditBox_NbY			(null),
								mEditBox_NbZ			(null),
								mEditBox_SphereRadius	(null),
								mEditBox_CapsuleRadius	(null),
								mEditBox_HalfHeight		(null),
								mEditBox_BoxSize		(null),
								mEditBox_ConvexSize		(null)
								{}
	virtual					~BoxContainerConfigurable()	{										}
	virtual	const char*		GetName()			const	{ return "Box container";				}
	virtual	const char*		GetDescription()	const	{ return gDesc_BoxContainerConfigurable;}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_PERFORMANCE;			}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 400;
		WD.mHeight	= 250;
		WD.mLabel	= "Box container";
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
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Sphere radius:", &UIElems);
			mEditBox_SphereRadius = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_SphereRadius->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Capsule radius:", &UIElems);
			mEditBox_CapsuleRadius = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_CapsuleRadius->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Capsule half height:", &UIElems);
			mEditBox_HalfHeight = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_HalfHeight->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Box size:", &UIElems);
			mEditBox_BoxSize = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.3", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_BoxSize->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Convex size:", &UIElems);
			mEditBox_ConvexSize = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.2", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_ConvexSize->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "NbX:", &UIElems);
			mEditBox_NbX = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "16", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "NbY:", &UIElems);
			mEditBox_NbY = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "16", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "NbZ:", &UIElems);
			mEditBox_NbZ = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "4", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;
		}
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Presets:", &UIElems);

			class MyComboBox : public IceComboBox
			{
				BoxContainerConfigurable&	mTest;
				public:
								MyComboBox(const ComboBoxDesc& desc, BoxContainerConfigurable& test) : IceComboBox(desc), mTest(test)	{}
				virtual			~MyComboBox()																							{}

				virtual	void	OnComboBoxEvent(ComboBoxEvent event)
				{
					if(event==CBE_SELECTION_CHANGED)
					{
						mTest.mMustResetTest = true;
						const udword SelectedIndex = GetSelectedIndex();
						const bool Enabled = SelectedIndex==GetItemCount()-1;
						mTest.mEditBox_SphereRadius->SetEnabled(Enabled);
						mTest.mEditBox_CapsuleRadius->SetEnabled(Enabled);
						mTest.mEditBox_HalfHeight->SetEnabled(Enabled);
						mTest.mEditBox_BoxSize->SetEnabled(Enabled);
						mTest.mEditBox_ConvexSize->SetEnabled(Enabled);

						if(!Enabled && SelectedIndex<NB_BOX_STACKS_PRESETS)
						{
							mTest.mEditBox_SphereRadius->SetText(_F("%.2f", gPreset_SphereRadius[SelectedIndex]));
							mTest.mEditBox_CapsuleRadius->SetText(_F("%.2f", gPreset_CapsuleRadius[SelectedIndex]));
							mTest.mEditBox_HalfHeight->SetText(_F("%.2f", gPreset_HalfHeight[SelectedIndex]));
							mTest.mEditBox_BoxSize->SetText(_F("%.2f", gPreset_BoxSize[SelectedIndex]));
							mTest.mEditBox_ConvexSize->SetText(_F("%.2f", gPreset_ConvexSize[SelectedIndex]));
						}
					}
				}
			};

			ComboBoxDesc CBBD;
			CBBD.mID		= 0;
			CBBD.mParent	= UI;
			CBBD.mX			= 4+OffsetX;
			CBBD.mY			= y;
			CBBD.mWidth		= 250;
			CBBD.mHeight	= 20;
			CBBD.mLabel		= "Presets";
			mComboBox_Preset = ICE_NEW(MyComboBox)(CBBD, *this);
			RegisterUIElement(mComboBox_Preset);
			mComboBox_Preset->Add("BoxContainerAndSpheres - PEEL 1.1 test");
			mComboBox_Preset->Add("BoxContainerAndBoxes - PEEL 1.1 test");
			mComboBox_Preset->Add("BoxContainerAndCapsules - PEEL 1.1 test");
			mComboBox_Preset->Add("PotPourri_Box - PEEL 1.1 test");
			mComboBox_Preset->Add("PotPourri_Box_WithConvexes - PEEL 1.1 test");
			mComboBox_Preset->Add("User-defined");
			mComboBox_Preset->Select(4);
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
				return "BoxContainerAndSpheres";
			else if(SelectedIndex==1)
				return "BoxContainerAndBoxes";
			else if(SelectedIndex==2)
				return "BoxContainerAndCapsules";
			else if(SelectedIndex==3)
				return "PotPourri_Box";			
			else if(SelectedIndex==4)
				return "PotPourri_Box_WithConvexes";			
		}
		return null;
	}

	virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(10.85f, 14.54f, 10.42f), Point(-0.52f, -0.69f, -0.50f));
		SetDefEnv(desc, true);
	}

	virtual	bool			Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float SphereRadius = GetFloat(0.0f, mEditBox_SphereRadius);
		const float CapsuleRadius = GetFloat(0.0f, mEditBox_CapsuleRadius);
		const float HalfHeight = GetFloat(0.0f, mEditBox_HalfHeight);
		const float BoxSize = GetFloat(0.0f, mEditBox_BoxSize);
		const float ConvexSize = GetFloat(0.0f, mEditBox_ConvexSize);

		BasicRandom Rnd(42);

		const bool ValidSphere = SphereRadius!=0.0f;
		const bool ValidBox = BoxSize!=0.0f;
		const bool ValidCapsule = CapsuleRadius!=0.0f && HalfHeight!=0.0f;
		const bool ValidConvex = ConvexSize!=0.0f;
		if(ValidConvex && !caps.mSupportConvexes)
			return false;

		udword NbOptions = 0;
		if(ValidSphere)
			NbOptions++;
		if(ValidBox)
			NbOptions++;
		if(ValidCapsule)
			NbOptions++;
		if(ValidConvex)
			NbOptions++;
		if(!NbOptions)
			return true;

		const float BoxHeight = 4.0f;
		const float BoxSide = 1.0f;
		const float BoxDepth = 10.0f;
		CreateBoxContainer(pint, BoxHeight, BoxSide, BoxDepth);

		PINT_SPHERE_CREATE SphereDesc(SphereRadius);
		SphereDesc.mRenderer = ValidSphere ? CreateSphereRenderer(SphereRadius) : null;

		PINT_BOX_CREATE BoxDesc(BoxSize, BoxSize, BoxSize);
		BoxDesc.mRenderer = ValidBox ? CreateBoxRenderer(BoxDesc.mExtents) : null;

		PINT_CAPSULE_CREATE CapsuleDesc(CapsuleRadius, HalfHeight);
		CapsuleDesc.mRenderer = ValidCapsule ? CreateCapsuleRenderer(CapsuleRadius, HalfHeight*2.0f) : null;

		PINT_CONVEX_CREATE ConvexDesc;
		Point Pts[1024];
		if(ValidConvex)
		{
		//	const udword NbInsideCirclePts = 4;
		//	const udword NbOutsideCirclePts = 4;
			const udword NbInsideCirclePts = 8;
			const udword NbOutsideCirclePts = 8;
		//	const udword NbInsideCirclePts = 32;
		//	const udword NbOutsideCirclePts = 32;

			const udword TotalNbVerts = NbInsideCirclePts + NbOutsideCirclePts;
			ASSERT(TotalNbVerts<1024);

			const udword NbPts = GenerateConvex(Pts, NbInsideCirclePts, NbOutsideCirclePts, ConvexSize, ConvexSize*1.5f, ConvexSize);
			ASSERT(NbPts==TotalNbVerts);

			ConvexDesc.mNbVerts		= TotalNbVerts;
			ConvexDesc.mVerts		= Pts;
			ConvexDesc.mRenderer	= CreateConvexRenderer(TotalNbVerts, Pts);
		}

		PINT_SHAPE_CREATE* Shapes[8];
		float PosRadius[8];
		bool RandomRot[8];
		udword Offset = 0;
		float LayerInc = 0.0f;
		if(ValidSphere)
		{
			LayerInc = MAX(LayerInc, SphereRadius);
			RandomRot[Offset] = false;
			PosRadius[Offset] = SphereRadius;
			Shapes[Offset++] = &SphereDesc;
		}
		if(ValidCapsule)
		{
			LayerInc = MAX(LayerInc, HalfHeight);
			RandomRot[Offset] = true;
			PosRadius[Offset] = CapsuleRadius;
			Shapes[Offset++] = &CapsuleDesc;
		}
		if(ValidBox)
		{
			LayerInc = MAX(LayerInc, BoxSize);
			RandomRot[Offset] = true;
			PosRadius[Offset] = BoxSize;
			Shapes[Offset++] = &BoxDesc;
		}
		if(ValidConvex)
		{
			LayerInc = MAX(LayerInc, ConvexSize);
			RandomRot[Offset] = true;
			PosRadius[Offset] = ConvexSize;
			Shapes[Offset++] = &ConvexDesc;
		}
		float yy = LayerInc;
		const udword NbLayers = GetInt(4, mEditBox_NbZ);
		const udword NbX = GetInt(4, mEditBox_NbX);
		const udword NbY = GetInt(4, mEditBox_NbY);

		for(udword k=0;k<NbLayers;k++)
		{
			for(udword y=0;y<NbY;y++)
			{
				const float CoeffY = 2.0f * ((float(y)/float(NbY-1)) - 0.5f);
				for(udword x=0;x<NbX;x++)
				{
					const float CoeffX = 2.0f * ((float(x)/float(NbX-1)) - 0.5f);

					const float RandomX = 0.1f * Rnd.RandomFloat();
					const float RandomY = 0.1f * Rnd.RandomFloat();

					const udword Index = NbOptions ? (Rnd.Randomize()>>8) % NbOptions : 0;
//					const udword Index = NbOptions ? Rnd.Randomize() % NbOptions : 0;

					PINT_OBJECT_CREATE ObjectDesc;
					ObjectDesc.mShapes		= Shapes[Index];
					ObjectDesc.mMass		= 1.0f;
					ObjectDesc.mPosition.x	= RandomX + CoeffX * (BoxDepth - PosRadius[Index] - BoxSide*2.0f);
					ObjectDesc.mPosition.y	= yy;
					ObjectDesc.mPosition.z	= RandomY + CoeffY * (BoxDepth - PosRadius[Index] - BoxSide*2.0f);

					if(RandomRot[Index])
						UnitRandomQuat(ObjectDesc.mRotation, Rnd);

					CreatePintObject(pint, ObjectDesc);
				}
			}
			yy += LayerInc * 2.0f;
		}
		return true;
	}

}BoxContainerConfigurable;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Compounds320NotTouching = "The notorious compounds from the PhysX samples. Each compound is made of 320 boxes. In this test they do not touch, so \
we should not measure the impact of compound-vs-compound interaction. Engines using a single broadphase entry for each compound should perform a lot better than \
engines using a broadphase entry for each sub-shape.";

START_TEST(Compounds320NotTouching, CATEGORY_PERFORMANCE, gDesc_Compounds320NotTouching)

	virtual	float	GetRenderData(Point& center)	const	{ return 800.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(102.41f, 11.54f, 98.98f), Point(-0.74f, -0.07f, -0.67f));
		desc.mCamera[1] = PintCameraPose(Point(104.05f, 43.87f, 100.62f), Point(-0.63f, -0.48f, -0.61f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCompounds || !caps.mSupportRigidBodySimulation)
			return false;

		const udword NbBoxes = 320;
//		const udword NbBoxes = 32;
		const float BoxExtent = 0.4f;
	//	const float BoxExtent = 2.0f;
		const Point Extents(BoxExtent, BoxExtent, BoxExtent);
		PintShapeRenderer* RenderObject = CreateBoxRenderer(Extents);

		const float SphereRadius = 10.0f;
		const float BoxPosY = SphereRadius + BoxExtent;

		PINT_BOX_CREATE BoxDesc[NbBoxes];
		BasicRandom Rnd(42);
		for(udword i=0;i<NbBoxes;i++)
		{
			UnitRandomQuat(BoxDesc[i].mLocalRot, Rnd);
			UnitRandomPt(BoxDesc[i].mLocalPos, Rnd);
			BoxDesc[i].mLocalPos *= SphereRadius;

			BoxDesc[i].mExtents	= Extents;
			BoxDesc[i].mRenderer = RenderObject;
			if(i!=NbBoxes-1)
				BoxDesc[i].mNext = &BoxDesc[i+1];
		}

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes		= BoxDesc;
		ObjectDesc.mPosition.y	= BoxPosY;
		ObjectDesc.mMass		= 1.0f;

		const udword Nb = 4;
//		const udword Nb = 1;
		for(udword y=0;y<Nb;y++)
		{
			const float YCoeff = float(y) - 1.0f;
			for(udword x=0;x<Nb;x++)
			{
				const float XCoeff = float(x) - 1.0f;

				ObjectDesc.mPosition.x	= XCoeff * SphereRadius * 4.0f;
				ObjectDesc.mPosition.z	= YCoeff * SphereRadius * 4.0f;

				PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
				ASSERT(Handle);
			}
		}
		return true;
	}

END_TEST(Compounds320NotTouching)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Compounds320Touching = "The notorious compounds from the PhysX samples. Each compound is made of 320 boxes. In this test they do touch, so \
we should measure the impact of compound-vs-compound interaction. Engines using a single broadphase entry for each compound may perform a lot worse than \
engines using a broadphase entry for each sub-shape, if the compound-compound code is O(n^2).";

START_TEST(Compounds320Touching, CATEGORY_PERFORMANCE, gDesc_Compounds320Touching)

	virtual	float	GetRenderData(Point& center)	const	{ return 2000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(16.62f, 195.63f, 11.85f), Point(-0.33f, -0.93f, -0.17f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCompounds|| !caps.mSupportRigidBodySimulation)
			return false;

		const float BoxHeight = 80.0f;
		const float BoxSide = 1.0f;
		const float BoxDepth = 15.0f;
		CreateBoxContainer(pint, BoxHeight, BoxSide, BoxDepth);

		const float SphereRadius = 10.0f;
		const float BoxExtent = 0.4f;
		const float BoxPosY = SphereRadius + BoxExtent;
		const Point Extents(BoxExtent, BoxExtent, BoxExtent);
		PintShapeRenderer* RenderObject = CreateBoxRenderer(Extents);

		const udword NbBoxes = 320;
		PINT_BOX_CREATE BoxDesc[NbBoxes];
		BasicRandom Rnd(42);
		for(udword i=0;i<NbBoxes;i++)
		{
			UnitRandomQuat(BoxDesc[i].mLocalRot, Rnd);
			UnitRandomPt(BoxDesc[i].mLocalPos, Rnd);
			BoxDesc[i].mLocalPos *= SphereRadius;

			BoxDesc[i].mExtents		= Extents;
			BoxDesc[i].mRenderer	= RenderObject;
			if(i!=NbBoxes-1)
				BoxDesc[i].mNext = &BoxDesc[i+1];
		}

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes		= BoxDesc;
		ObjectDesc.mPosition.x	= 0.0f;
		ObjectDesc.mPosition.z	= 0.0f;
		ObjectDesc.mPosition.y	= BoxPosY;
		ObjectDesc.mMass		= 1.0f;

		for(udword n=0;n<8;n++)
		{
			ObjectDesc.mPosition.y	= BoxPosY + n * SphereRadius * 2.5f;

			PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
			ASSERT(Handle);
		}
		return true;
	}

END_TEST(Compounds320Touching)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_StackOfSmallCompounds = "8*8*8 compound objects stacked in a grid fashion. Each compound has 3 sub-shapes.";

START_TEST(StackOfSmallCompounds, CATEGORY_PERFORMANCE, gDesc_StackOfSmallCompounds)

	virtual	float	GetRenderData(Point& center)	const	{ return 1000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(111.02f, 69.80f, 121.30f), Point(-0.63f, -0.30f, -0.72f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCompounds || !caps.mSupportRigidBodySimulation)
			return false;

		const float LargeBoxExtent = 5.0f;
		const float SmallBoxExtent = 1.0f;

		PINT_BOX_CREATE BoxDesc[3];
		BoxDesc[0].mExtents	= Point(LargeBoxExtent, SmallBoxExtent, SmallBoxExtent);
		BoxDesc[1].mExtents	= Point(SmallBoxExtent, LargeBoxExtent, SmallBoxExtent);
		BoxDesc[2].mExtents	= Point(SmallBoxExtent, SmallBoxExtent, LargeBoxExtent);
		BoxDesc[0].mRenderer	= CreateBoxRenderer(BoxDesc[0].mExtents);
		BoxDesc[1].mRenderer	= CreateBoxRenderer(BoxDesc[1].mExtents);
		BoxDesc[2].mRenderer	= CreateBoxRenderer(BoxDesc[2].mExtents);
		BoxDesc[0].mNext = &BoxDesc[1];
		BoxDesc[1].mNext = &BoxDesc[2];

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes		= BoxDesc;
		ObjectDesc.mMass		= 1.0f;

		const udword NbX = 8;
		const udword NbY = 8;
		const udword NbZ = 8;

		const float Spacing = LargeBoxExtent*2.0f;

		for(udword y=0;y<NbY;y++)
		{
			const float CoeffY = float(y);
			for(udword z=0;z<NbZ;z++)
			{
				const float CoeffZ = float(z);
				for(udword x=0;x<NbX;x++)
				{
					const float CoeffX = float(x);

					ObjectDesc.mPosition	= Point(CoeffX*Spacing, LargeBoxExtent+CoeffY*Spacing, CoeffZ*Spacing);
					PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
					ASSERT(Handle);
				}
			}
		}
		return true;
	}

END_TEST(StackOfSmallCompounds)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_PileOfSmallCompounds = "A pile of 8*8*8 compound objects. Each compound has 3 sub-shapes.";

START_TEST(PileOfSmallCompounds, CATEGORY_PERFORMANCE, gDesc_PileOfSmallCompounds)

	virtual	float	GetRenderData(Point& center)	const	{ return 1000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(112.68f, 74.16f, 113.66f), Point(-0.61f, -0.51f, -0.61f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCompounds || !caps.mSupportRigidBodySimulation)
			return false;

		const float LargeBoxExtent = 5.0f;
		const float SmallBoxExtent = 1.0f;

		PINT_BOX_CREATE BoxDesc[3];
		BoxDesc[0].mExtents	= Point(LargeBoxExtent, SmallBoxExtent, SmallBoxExtent);
		BoxDesc[1].mExtents	= Point(SmallBoxExtent, LargeBoxExtent, SmallBoxExtent);
		BoxDesc[2].mExtents	= Point(SmallBoxExtent, SmallBoxExtent, LargeBoxExtent);
		BoxDesc[0].mRenderer	= CreateBoxRenderer(BoxDesc[0].mExtents);
		BoxDesc[1].mRenderer	= CreateBoxRenderer(BoxDesc[1].mExtents);
		BoxDesc[2].mRenderer	= CreateBoxRenderer(BoxDesc[2].mExtents);
		BoxDesc[0].mNext = &BoxDesc[1];
		BoxDesc[1].mNext = &BoxDesc[2];

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes		= BoxDesc;
		ObjectDesc.mMass		= 1.0f;

		const udword NbX = 8;
		const udword NbY = 8;
		const udword NbZ = 8;

		const float Spacing = LargeBoxExtent*2.1f;

		BasicRandom Rnd(42);

		for(udword y=0;y<NbY;y++)
		{
			const float CoeffY = float(y);
			for(udword z=0;z<NbZ;z++)
			{
				const float CoeffZ = float(z);
				for(udword x=0;x<NbX;x++)
				{
					const float CoeffX = float(x);

					UnitRandomQuat(ObjectDesc.mRotation, Rnd);

					ObjectDesc.mPosition	= Point(CoeffX*Spacing, LargeBoxExtent+CoeffY*Spacing, CoeffZ*Spacing);
					PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
					ASSERT(Handle);
				}
			}
		}
		return true;
	}

END_TEST(PileOfSmallCompounds)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ConvexStack = "Convex stack scene from the PhysX SDK. There are 4 layers of 16*16 convexes, i.e. 1024 convexes in total. Each convex has 16 vertices.";

START_TEST(ConvexStack, CATEGORY_PERFORMANCE, gDesc_ConvexStack)

	virtual	float	GetRenderData(Point& center)	const	{ return 1000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(81.67f, 27.75f, 68.78f), Point(-0.75f, -0.38f, -0.54f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		const udword NbX = 16;
		const udword NbY = 16;
		const udword NbLayers = 4;

	//	const udword NbInsideCirclePts = 4;
	//	const udword NbOutsideCirclePts = 4;
		const udword NbInsideCirclePts = 8;
		const udword NbOutsideCirclePts = 8;
	//	const udword NbInsideCirclePts = 32;
	//	const udword NbOutsideCirclePts = 32;

		return CreateTestScene_ConvexStack_Generic(pint, NbX, NbY, NbLayers, NbInsideCirclePts, NbOutsideCirclePts);
	}

END_TEST(ConvexStack)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ConvexStack2 = "Convex stack scene from the PhysX SDK. Each convex has 16 vertices.";

START_TEST(ConvexStack2, CATEGORY_PERFORMANCE, gDesc_ConvexStack2)

	virtual	float	GetRenderData(Point& center)	const	{ return 1000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(43.75f, 14.01f, 42.92f), Point(-0.70f, -0.24f, -0.67f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		const udword NbX = 12;
		const udword NbY = 12;

		Point Pts[16];
		udword NbPts = GenerateConvex(Pts, 8, 8, 2.0f, 3.0f, 2.0f);
		ASSERT(NbPts==16);

		PINT_CONVEX_CREATE ConvexCreate(16, Pts);
		ConvexCreate.mRenderer	= CreateConvexRenderer(16, Pts);

		const udword NbLayers = 12;
		for(udword j=0;j<NbLayers;j++)
		{
			const float Scale = 6.0f;
			for(udword y=0;y<NbY-j;y++)
			{
				for(udword x=0;x<NbX-j;x++)
				{
					const float xf = (float(j)*0.5f + float(x) - float(NbX)*0.5f)*Scale;
					const float yf = (float(j)*0.5f + float(y) - float(NbY)*0.5f)*Scale;

					const Point pos(xf, 0.0f + 2.0f * float(j), yf);

					PintActorHandle Handle = CreateDynamicObject(pint, &ConvexCreate, pos);
					ASSERT(Handle);
				}
			}
		}
		return true;
	}

END_TEST(ConvexStack2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ConvexStack3 = "Convex stack scene from the PhysX SDK. There are 8 layers of 16*16 convexes, i.e. 2048 convexes in total. Each convex has 56 vertices.";

START_TEST(ConvexStack3, CATEGORY_PERFORMANCE, gDesc_ConvexStack3)

	virtual	float	GetRenderData(Point& center)	const	{ return 1000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(75.47f, 20.40f, 73.86f), Point(-0.69f, -0.25f, -0.68f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		const udword NbX = 16;
		const udword NbY = 16;
		const udword NbLayers = 8;
		const udword NbInsideCirclePts = 28;
		const udword NbOutsideCirclePts = 28;

		return CreateTestScene_ConvexStack_Generic(pint, NbX, NbY, NbLayers, NbInsideCirclePts, NbOutsideCirclePts);
	}

END_TEST(ConvexStack3)

///////////////////////////////////////////////////////////////////////////////

static float gPileOfConvexesScale = 0.1f;

static const char* gDesc_PileOfLargeConvexes = "Pile of large convexes. The convex is the hull computed around 32 randomly generated vertices.";

START_TEST(PileOfLargeConvexes, CATEGORY_PERFORMANCE, gDesc_PileOfLargeConvexes)

	virtual	float	GetRenderData(Point& center)	const	{ return 1000.0f*gPileOfConvexesScale;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(gPileOfConvexesScale*Point(88.12f, 62.03f, 92.27f), Point(-0.66f, -0.26f, -0.71f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		const udword NbX = 5;
		const udword NbY = 5;
		const udword NbLayers = 20;
		const float Amplitude = 4.0f*gPileOfConvexesScale;
		const udword NbRandomPts = 32;
		GenerateConvexPile(pint, NbX, NbY, NbLayers, Amplitude, NbRandomPts, Amplitude*2.0f);
		return true;
	}

END_TEST(PileOfLargeConvexes)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_HugePileOfLargeConvexes = "Huge pile of large convexes. The convex is the hull computed around 32 randomly generated vertices.";

START_TEST(HugePileOfLargeConvexes, CATEGORY_PERFORMANCE, gDesc_HugePileOfLargeConvexes)

	virtual	float	GetRenderData(Point& center)	const	{ return 1000.0f*gPileOfConvexesScale;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(gPileOfConvexesScale*Point(164.90f, 67.88f, 164.53f), Point(-0.70f, 0.07f, -0.71f));
		desc.mCamera[1] = PintCameraPose(gPileOfConvexesScale*Point(38.54f, 276.43f, -144.63f), Point(-0.12f, -0.93f, 0.35f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		const udword NbX = 8;
		const udword NbY = 8;
		const udword NbLayers = 80;
		const float Amplitude = 4.0f*gPileOfConvexesScale;
		const udword NbRandomPts = 32;
		GenerateConvexPile(pint, NbX, NbY, NbLayers, Amplitude, NbRandomPts, Amplitude*2.0f);
		return true;
	}

END_TEST(HugePileOfLargeConvexes)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_PileOfMediumConvexes = "Pile of medium convexes. The convex is the hull computed around 16 randomly generated vertices.";

START_TEST(PileOfMediumConvexes, CATEGORY_PERFORMANCE, gDesc_PileOfMediumConvexes)

	virtual	float	GetRenderData(Point& center)	const	{ return 1000.0f*gPileOfConvexesScale;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(gPileOfConvexesScale*Point(50.00f, 50.00f, 50.00f), Point(-0.59f, -0.45f, -0.68f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		const udword NbX = 5;
		const udword NbY = 5;
		const udword NbLayers = 20;
		const float Amplitude = 2.0f*gPileOfConvexesScale;
		const udword NbRandomPts = 16;
		GenerateConvexPile(pint, NbX, NbY, NbLayers, Amplitude, NbRandomPts, Amplitude*4.0f);
		return true;
	}

END_TEST(PileOfMediumConvexes)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_PileOfSmallConvexes = "Pile of small convexes. The convex is the hull computed around 5 randomly generated vertices.";

START_TEST(PileOfSmallConvexes, CATEGORY_PERFORMANCE, gDesc_PileOfSmallConvexes)

	virtual	float	GetRenderData(Point& center)	const	{ return 1000.0f*gPileOfConvexesScale;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(gPileOfConvexesScale*Point(49.77f, 26.35f, 48.67f), Point(-0.68f, -0.35f, -0.64f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		const udword NbX = 8;
		const udword NbY = 8;
		const udword NbLayers = 20;
		const float Amplitude = 2.0f*gPileOfConvexesScale;
		const udword NbRandomPts = 5;
		GenerateConvexPile(pint, NbX, NbY, NbLayers, Amplitude, NbRandomPts, Amplitude*4.0f);
		return true;
	}

END_TEST(PileOfSmallConvexes)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ConvexGalore = "12 layers of 12*12 convexes. Each convex is randomly chosen in a set of 14 predefined convex objects, of various complexities.";

START_TEST(ConvexGalore, CATEGORY_PERFORMANCE, gDesc_ConvexGalore)

	virtual	float	GetRenderData(Point& center)	const	{ return 400.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(45.18f, 21.90f, 43.90f), Point(-0.69f, -0.21f, -0.69f));
		desc.mCamera[1] = PintCameraPose(Point(4.23f, 18.80f, 21.53f), Point(-0.18f, -0.70f, -0.69f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		PINT_CONVEX_CREATE ConvexCreate[14];
		MyConvex C[14];
		for(udword i=0;i<14;i++)
		{
			C[i].LoadFile(i);
			ConvexCreate[i].mNbVerts	= C[i].mNbVerts;
			ConvexCreate[i].mVerts		= C[i].mVerts;
			ConvexCreate[i].mRenderer	= CreateConvexRenderer(ConvexCreate[i].mNbVerts, ConvexCreate[i].mVerts);
		}

		const float Amplitude = 1.5f;
		const udword NbLayers = 12;
		const udword NbX = 12;
		const udword NbY = 12;
		BasicRandom Rnd(42);
		for(udword j=0;j<NbLayers;j++)
		{
			const float Scale = 4.0f;
			for(udword y=0;y<NbY;y++)
			{
				for(udword x=0;x<NbX;x++)
				{
					const float xf = (float(x)-float(NbX)*0.5f)*Scale;
					const float yf = (float(y)-float(NbY)*0.5f)*Scale;

					const Point pos = Point(xf, Amplitude + (Amplitude * 2.0f * float(j)), yf);

					const udword Index = Rnd.Randomize() % 14;

					PintActorHandle Handle = CreateDynamicObject(pint, &ConvexCreate[Index], pos);
					ASSERT(Handle);
				}
			}
		}
		return true;
	}

END_TEST(ConvexGalore)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ConvexGalore2 = "36 layers of 12*12 convexes. Each convex is randomly chosen in a set of 14 predefined convex objects, of various complexities.";

START_TEST(ConvexGalore2, CATEGORY_PERFORMANCE, gDesc_ConvexGalore2)

	virtual	float	GetRenderData(Point& center)	const	{ return 400.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(37.22f, 13.73f, 33.16f), Point(-0.76f, -0.15f, -0.63f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		PINT_CONVEX_CREATE ConvexCreate[14];
		MyConvex C[14];
		for(udword i=0;i<14;i++)
		{
			C[i].LoadFile(i);
			ConvexCreate[i].mNbVerts	= C[i].mNbVerts;
			ConvexCreate[i].mVerts		= C[i].mVerts;
			ConvexCreate[i].mRenderer	= CreateConvexRenderer(ConvexCreate[i].mNbVerts, ConvexCreate[i].mVerts);
		}

		const float Amplitude = 1.5f;
		const udword NbLayers = 36;
		const udword NbX = 12;
		const udword NbY = 12;
		BasicRandom Rnd(42);
		for(udword j=0;j<NbLayers;j++)
		{
			const float Scale = 4.0f;
			for(udword y=0;y<NbY;y++)
			{
				for(udword x=0;x<NbX;x++)
				{
					const float xf = (float(x)-float(NbX)*0.5f)*Scale;
					const float yf = (float(y)-float(NbY)*0.5f)*Scale;

					const Point pos = Point(xf, Amplitude + (Amplitude * 2.0f * float(j)), yf);

					const udword Index = Rnd.Randomize() % 14;

					PintActorHandle Handle = CreateDynamicObject(pint, &ConvexCreate[Index], pos);
					ASSERT(Handle);
				}
			}
		}
		return true;
	}

END_TEST(ConvexGalore2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ConvexGalore3 = "40 layers of 16*16 random objects. This is a test for 10000+ dynamic objects.";

START_TEST(ConvexGalore3, CATEGORY_PERFORMANCE, gDesc_ConvexGalore3)

	virtual	float	GetRenderData(Point& center)	const	{ return 400.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(37.22f, 13.73f, 33.16f), Point(-0.76f, -0.15f, -0.63f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		PINT_CONVEX_CREATE ConvexCreate[14];
		MyConvex C[14];
		for(udword i=0;i<14;i++)
		{
			C[i].LoadFile(i);
			ConvexCreate[i].mNbVerts	= C[i].mNbVerts;
			ConvexCreate[i].mVerts		= C[i].mVerts;
			ConvexCreate[i].mRenderer	= CreateConvexRenderer(ConvexCreate[i].mNbVerts, ConvexCreate[i].mVerts);
		}

		const float Amplitude = 1.5f;
		const udword NbLayers = 40;
		const udword NbX = 16;
		const udword NbY = 16;
		BasicRandom Rnd(42);
		for(udword j=0;j<NbLayers;j++)
		{
			const float Scale = 4.0f;
			for(udword y=0;y<NbY;y++)
			{
				for(udword x=0;x<NbX;x++)
				{
					const float xf = (float(x)-float(NbX)*0.5f)*Scale;
					const float yf = (float(y)-float(NbY)*0.5f)*Scale;

					const Point pos = Point(xf, Rnd.RandomFloat()*2.0f + Amplitude + (Amplitude * 2.0f * float(j)), yf);

//					const udword Index = Rnd.Randomize() % 16;
					const udword Index = Rnd.Randomize() % 14;

					if(Index<14)
					{
						PintActorHandle Handle = CreateDynamicObject(pint, &ConvexCreate[Index], pos);
						ASSERT(Handle);
					}
					else if(Index==14)
					{
						const float SphereRadius = 1.0f + Rnd.RandomFloat();
						PINT_SPHERE_CREATE SphereDesc(SphereRadius);
						SphereDesc.mRenderer	= CreateSphereRenderer(SphereRadius);

						PintActorHandle Handle = CreateDynamicObject(pint, &SphereDesc, pos);
						ASSERT(Handle);
					}
					else if(Index==15)
					{
						PINT_BOX_CREATE BoxDesc;
						UnitRandomPt(BoxDesc.mExtents, Rnd);
						BoxDesc.mExtents += Point(0.2f, 0.2f, 0.2f);
						BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

						PintActorHandle Handle = CreateDynamicObject(pint, &BoxDesc, pos);
						ASSERT(Handle);
					}
				}
			}
		}
		return true;
	}

END_TEST(ConvexGalore3)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Convex12K = "12K convex objects.";

START_TEST(Convex12K, CATEGORY_PERFORMANCE, gDesc_Convex12K)

	virtual	float	GetRenderData(Point& center)	const	{ return 1000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(86.47f, 29.19f, 69.14f), Point(-0.75f, -0.25f, -0.61f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		PINT_CONVEX_CREATE ConvexCreate;
		MyConvex C;
		const udword i=2;
		C.LoadFile(i);
		ConvexCreate.mNbVerts	= C.mNbVerts;
		ConvexCreate.mVerts		= C.mVerts;
		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

		const float Amplitude = 1.5f;
		const udword NbLayers = 44;
		const udword NbX = 16;
		const udword NbY = 17;
		BasicRandom Rnd(42);
		for(udword j=0;j<NbLayers;j++)
		{
			const float Scale = 4.0f;
			for(udword y=0;y<NbY;y++)
			{
				for(udword x=0;x<NbX;x++)
				{
					const float xf = (float(x)-float(NbX)*0.5f)*Scale;
					const float yf = (float(y)-float(NbY)*0.5f)*Scale;

					const Point pos = Point(xf, Rnd.RandomFloat()*2.0f + Amplitude + (Amplitude * 2.0f * float(j)), yf);

					PintActorHandle Handle = CreateDynamicObject(pint, &ConvexCreate, pos);
					ASSERT(Handle);
				}
			}
		}
		return true;
	}

END_TEST(Convex12K)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_10000_Spheres = "10000+ dynamic spheres.";

START_TEST(TenThousandsSpheres, CATEGORY_PERFORMANCE, gDesc_10000_Spheres)

	virtual	float	GetRenderData(Point& center)	const	{ return 1000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(66.98f, 10.23f, 58.22f), Point(-0.71f, 0.31f, -0.63f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float Amplitude = 1.5f;
		const udword NbLayers = 40;
		const udword NbX = 16;
		const udword NbY = 16;
		BasicRandom Rnd(42);
		for(udword j=0;j<NbLayers;j++)
		{
			const float Scale = 4.0f;
			for(udword y=0;y<NbY;y++)
			{
				for(udword x=0;x<NbX;x++)
				{
					const float xf = (float(x)-float(NbX)*0.5f)*Scale;
					const float yf = (float(y)-float(NbY)*0.5f)*Scale;

					const Point pos = Point(xf, Rnd.RandomFloat()*2.0f + Amplitude + (Amplitude * 2.0f * float(j)), yf);

					{
						const float SphereRadius = 1.0f + Rnd.RandomFloat();
						PINT_SPHERE_CREATE SphereDesc(SphereRadius);
						SphereDesc.mRenderer	= CreateSphereRenderer(SphereRadius);

						PintActorHandle Handle = CreateDynamicObject(pint, &SphereDesc, pos);
						ASSERT(Handle);
					}
				}
			}
		}
		return true;
	}

END_TEST(TenThousandsSpheres)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_10000_Boxes = "10000+ dynamic boxes.";

START_TEST(TenThousandsBoxes, CATEGORY_PERFORMANCE, gDesc_10000_Boxes)

	virtual	float	GetRenderData(Point& center)	const	{ return 1000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(53.65f, 2.36f, 43.87f), Point(-0.72f, 0.31f, -0.62f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float Amplitude = 1.5f;
		const udword NbLayers = 40;
		const udword NbX = 16;
		const udword NbY = 16;
		BasicRandom Rnd(42);
		for(udword j=0;j<NbLayers;j++)
		{
			const float Scale = 4.0f;
			for(udword y=0;y<NbY;y++)
			{
				for(udword x=0;x<NbX;x++)
				{
					const float xf = (float(x)-float(NbX)*0.5f)*Scale;
					const float yf = (float(y)-float(NbY)*0.5f)*Scale;

					const Point pos = Point(xf, Rnd.RandomFloat()*2.0f + Amplitude + (Amplitude * 2.0f * float(j)), yf);

					{
						PINT_BOX_CREATE BoxDesc;
						UnitRandomPt(BoxDesc.mExtents, Rnd);
						BoxDesc.mExtents.x = fabsf(BoxDesc.mExtents.x);
						BoxDesc.mExtents.y = fabsf(BoxDesc.mExtents.y);
						BoxDesc.mExtents.z = fabsf(BoxDesc.mExtents.z);
						BoxDesc.mExtents += Point(0.2f, 0.2f, 0.2f);
						BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

						PintActorHandle Handle = CreateDynamicObject(pint, &BoxDesc, pos);
						ASSERT(Handle);
					}
				}
			}
		}
		return true;
	}

END_TEST(TenThousandsBoxes)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ConvexClash = "(Configurable test) - Dynamic convexes are falling towards static convexes. \
This is a stress test for both convex contact generation and the broad phase.";

class ConvexClash : public TestBase
{
			IceComboBox*	mComboBox_ConvexIndex;
			IceEditBox*		mEditBox_Size;
	public:
							ConvexClash() : mComboBox_ConvexIndex(null), mEditBox_Size(null)	{}
	virtual					~ConvexClash()				{								}
	virtual	const char*		GetName()			const	{ return "ConvexClash";			}
	virtual	const char*		GetDescription()	const	{ return gDesc_ConvexClash;		}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_PERFORMANCE;	}

	virtual	float			GetRenderData(Point& center)	const	{ return 1000.0f;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 256;
		WD.mHeight	= 160;
		WD.mLabel	= "ConvexClash";
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

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		SetDefEnv(desc, true);
	}

	virtual	bool			Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		ConvexIndex Index = CONVEX_INDEX_0;
		if(mComboBox_ConvexIndex)
			Index = ConvexIndex(mComboBox_ConvexIndex->GetSelectedIndex());

		const udword Size = GetInt(32, mEditBox_Size);

		const udword i = Index;
		const udword nb_x = Size;
		const udword nb_y = Size;
		{
			const float Altitude = 10.0f;

			MyConvex C;
			C.LoadFile(i);

			PINT_CONVEX_CREATE ConvexCreate(C.mNbVerts, C.mVerts);
			ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

			const float Amplitude = 1.5f;
			const udword NbX = nb_x;
			const udword NbY = nb_y;

			const float Scale = 2.5f;
			for(udword y=0;y<NbY;y++)
			{
				for(udword x=0;x<NbX;x++)
				{
					const float xf = (float(x)-float(NbX)*0.5f)*Scale;
					const float yf = (float(y)-float(NbY)*0.5f)*Scale;

		//			const Point pos = Point(xf, Altitude + 20.0f, yf);
					const Point pos = Point(xf, Altitude + 30.0f, yf);

					PintActorHandle Handle = CreateStaticObject(pint, &ConvexCreate, pos);
					ASSERT(Handle);
				}
			}

			for(udword y=0;y<NbY;y++)
			{
				for(udword x=0;x<NbX;x++)
				{
					const float xf = (float(x)-float(NbX)*0.5f)*Scale;
					const float yf = (float(y)-float(NbY)*0.5f)*Scale;

		//			const Point pos = Point(xf, Altitude + 20.0f, yf);
					const Point pos = Point(xf, Altitude + 50.0f, yf);

					PintActorHandle Handle = CreateDynamicObject(pint, &ConvexCreate, pos);
					ASSERT(Handle);
				}
			}
		}
		return true;
	}

}ConvexClash;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SpheresOnLargeBox = "32*32 dynamic spheres on a large static box.";

START_TEST(SpheresOnLargeBox, CATEGORY_PERFORMANCE, gDesc_SpheresOnLargeBox)

	virtual	float	GetRenderData(Point& center)	const	{ return 400.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-32.59f, 5.48f, 31.90f), Point(0.63f, -0.42f, -0.65f));
		desc.mCamera[1] = PintCameraPose(Point(52.18f, 26.82f, 52.73f), Point(-0.65f, -0.40f, -0.65f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float Altitude = 1.0f;
		const float Radius = 0.5f;
		const udword NbX = 32;
		const udword NbY = 32;
		return GenerateArrayOfSpheres(pint, Radius, NbX, NbY, Altitude + Radius*2.0f, 30.0f, 30.0f);
	}

END_TEST(SpheresOnLargeBox)

///////////////////////////////////////////////////////////////////////////////

// TODO:
// - expose other meshes
// - use tessellation in plane scene
// - name/describe convexes?
// - random shape?
static const char* gDesc_DynamicsOnMeshLevel = "(Configurable test) - dynamic objects falling on a mesh level.";

enum MeshLevel
{
	MESH_LEVEL_ARCHIPELAGO	= 0,
	MESH_LEVEL_TESTZONE		= 1,
	MESH_LEVEL_FLAT			= 2,
};

#define NB_PRESETS	9
static const udword			gPreset_GridSize[NB_PRESETS] = { 32, 32, 32, 32, 64, 64, 32, 32, 64 };
static const float			gPreset_GridScale[NB_PRESETS] = { 30.0f, 30.0f, 35.0f, 35.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
static const udword			gPreset_Layers[NB_PRESETS] = { 1, 1, 1, 1, 1, 1, 1, 4, 1 };
static const float			gPreset_Altitude[NB_PRESETS] = { 2.0f, 2.0f, 3.0f, 3.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f };
static const float			gPreset_ShapeSize[NB_PRESETS] = { 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
static const PintShape		gPreset_ShapeType[NB_PRESETS] = { PINT_SHAPE_SPHERE, PINT_SHAPE_BOX, PINT_SHAPE_CONVEX, PINT_SHAPE_CONVEX, PINT_SHAPE_SPHERE, PINT_SHAPE_CAPSULE, PINT_SHAPE_CONVEX, PINT_SHAPE_CONVEX, PINT_SHAPE_CONVEX };
static const ConvexIndex	gPreset_ConvexIndex[NB_PRESETS] = { CONVEX_INDEX_0, CONVEX_INDEX_0, CONVEX_INDEX_2, CONVEX_INDEX_4, CONVEX_INDEX_0, CONVEX_INDEX_0, CONVEX_INDEX_2, CONVEX_INDEX_2, CONVEX_INDEX_13 };
static const MeshLevel		gPreset_MeshLevel[NB_PRESETS] = { MESH_LEVEL_FLAT, MESH_LEVEL_FLAT, MESH_LEVEL_FLAT, MESH_LEVEL_FLAT, MESH_LEVEL_ARCHIPELAGO, MESH_LEVEL_ARCHIPELAGO, MESH_LEVEL_ARCHIPELAGO, MESH_LEVEL_ARCHIPELAGO, MESH_LEVEL_ARCHIPELAGO };
static const bool			gPreset_Tessellation[NB_PRESETS] = { false, false, false, false, false, false, false, false, true };

class DynamicsOnMeshLevel : public TestBase
{
			IceComboBox*	mComboBox_Preset;
			IceComboBox*	mComboBox_ConvexIndex;
			IceComboBox*	mComboBox_ShapeType;
			IceComboBox*	mComboBox_MeshLevel;
			IceEditBox*		mEditBox_GridSize;
			IceEditBox*		mEditBox_GridScale;
			IceEditBox*		mEditBox_Layers;
			IceEditBox*		mEditBox_Altitude;
			IceEditBox*		mEditBox_ShapeSize;
			IceCheckBox*	mCheckBox_Tessellation;
	public:
							DynamicsOnMeshLevel() :
								mComboBox_Preset		(null),
								mComboBox_ConvexIndex	(null),
								mComboBox_ShapeType		(null),
								mComboBox_MeshLevel		(null),
								mEditBox_GridSize		(null),
								mEditBox_GridScale		(null),
								mEditBox_Layers			(null),
								mEditBox_Altitude		(null),
								mEditBox_ShapeSize		(null),
								mCheckBox_Tessellation	(null)	{}
	virtual					~DynamicsOnMeshLevel()		{									}
	virtual	const char*		GetName()			const	{ return "DynamicsOnMeshLevel";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_DynamicsOnMeshLevel;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_PERFORMANCE;		}

	virtual	float	GetRenderData(Point& center)	const	{ return 2000.0f;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 400;
		WD.mHeight	= 300;
		WD.mLabel	= "DynamicsOnMeshLevel";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 130;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			mCheckBox_Tessellation = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Tessellate mesh", &UIElems, false, null, null);
			mCheckBox_Tessellation->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Grid size:", &UIElems);
			mEditBox_GridSize = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "64", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			mEditBox_GridSize->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Grid scale (0 = automatic):", &UIElems);
			mEditBox_GridScale = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_GridScale->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Nb layers:", &UIElems);
			mEditBox_Layers = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			mEditBox_Layers->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Altitude:", &UIElems);
			mEditBox_Altitude = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "10.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_Altitude->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Shape size:", &UIElems);
			mEditBox_ShapeSize = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_ShapeSize->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Shape:", &UIElems);
			mComboBox_ShapeType = CreateShapeTypeComboBox(UI, 4+OffsetX, y, false, SSM_UNDEFINED|SSM_SPHERE|SSM_CAPSULE|SSM_BOX|SSM_CONVEX);
			RegisterUIElement(mComboBox_ShapeType);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Convex:", &UIElems);
			mComboBox_ConvexIndex = CreateConvexObjectComboBox(UI, 4+OffsetX, y, false);
			RegisterUIElement(mComboBox_ConvexIndex);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Mesh level:", &UIElems);
			{
				ComboBoxDesc CBBD;
				CBBD.mID		= 0;
				CBBD.mParent	= UI;
				CBBD.mX			= 4+OffsetX;
				CBBD.mY			= y;
				CBBD.mWidth		= 150;
				CBBD.mHeight	= 20;
				CBBD.mLabel		= "Mesh level";
				IceComboBox* CB = ICE_NEW(IceComboBox)(CBBD);
				CB->Add("Archipelago");
				CB->Add("Test zone");
				CB->Add("Flat mesh plane");
				CB->Select(MESH_LEVEL_ARCHIPELAGO);
				CB->SetVisible(true);
				CB->SetEnabled(false);
				mComboBox_MeshLevel = CB;
			}
			RegisterUIElement(mComboBox_MeshLevel);
			y += YStep;
		}
		{
			y += YStep;
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Presets (PEEL 1.0 scenes):", &UIElems);

			class MyComboBox : public IceComboBox
			{
				DynamicsOnMeshLevel&	mTest;
				public:
								MyComboBox(const ComboBoxDesc& desc, DynamicsOnMeshLevel& test) : IceComboBox(desc), mTest(test)	{}
				virtual			~MyComboBox()																						{}
				virtual	void	OnComboBoxEvent(ComboBoxEvent event)
				{
					if(event==CBE_SELECTION_CHANGED)
					{
						mTest.mMustResetTest = true;
						const udword SelectedIndex = GetSelectedIndex();
						const bool Enabled = SelectedIndex==GetItemCount()-1;
						mTest.mEditBox_GridSize->SetEnabled(Enabled);
						mTest.mEditBox_GridScale->SetEnabled(Enabled);
						mTest.mEditBox_Layers->SetEnabled(Enabled);
						mTest.mEditBox_Altitude->SetEnabled(Enabled);
						mTest.mEditBox_ShapeSize->SetEnabled(Enabled);
						mTest.mCheckBox_Tessellation->SetEnabled(Enabled);
						mTest.mComboBox_ShapeType->SetEnabled(Enabled);
						mTest.mComboBox_ConvexIndex->SetEnabled(Enabled);
						mTest.mComboBox_MeshLevel->SetEnabled(Enabled);

						if(!Enabled && SelectedIndex<NB_PRESETS)
						{
							mTest.mEditBox_GridSize->SetText(_F("%d", gPreset_GridSize[SelectedIndex]));
							mTest.mEditBox_GridScale->SetText(_F("%.2f", gPreset_GridScale[SelectedIndex]));
							mTest.mEditBox_Layers->SetText(_F("%d", gPreset_Layers[SelectedIndex]));
							mTest.mEditBox_Altitude->SetText(_F("%.2f", gPreset_Altitude[SelectedIndex]));
							mTest.mEditBox_ShapeSize->SetText(_F("%.2f", gPreset_ShapeSize[SelectedIndex]));
							mTest.mCheckBox_Tessellation->SetChecked(gPreset_Tessellation[SelectedIndex]);
							mTest.mComboBox_ShapeType->Select(gPreset_ShapeType[SelectedIndex]);
							mTest.mComboBox_ConvexIndex->Select(gPreset_ConvexIndex[SelectedIndex]);
							mTest.mComboBox_MeshLevel->Select(gPreset_MeshLevel[SelectedIndex]);
						}
					}
				}
			};

			ComboBoxDesc CBBD;
			CBBD.mID		= 0;
			CBBD.mParent	= UI;
			CBBD.mX			= 4+OffsetX;
			CBBD.mY			= y;
			CBBD.mWidth		= 250;
			CBBD.mHeight	= 20;
			CBBD.mLabel		= "Presets";
			mComboBox_Preset = ICE_NEW(MyComboBox)(CBBD, *this);
			RegisterUIElement(mComboBox_Preset);
			mComboBox_Preset->Add("32*32 spheres on planar mesh");
			mComboBox_Preset->Add("32*32 boxes on planar mesh");
			mComboBox_Preset->Add("32*32 low-complexity convexes on planar mesh");
			mComboBox_Preset->Add("32*32 high-complexity convexes on planar mesh");
			mComboBox_Preset->Add("64*64 spheres on Archipelago - no tessellation");
			mComboBox_Preset->Add("64*64 capsules on Archipelago - no tessellation");
			mComboBox_Preset->Add("32*32 convexes on Archipelago - no tessellation");
			mComboBox_Preset->Add("32*32*4 convexes on Archipelago - no tessellation");
			mComboBox_Preset->Add("64*64 convexes on Archipelago - tessellation");
			mComboBox_Preset->Add("User-defined");
			mComboBox_Preset->Select(7);
			mComboBox_Preset->SetVisible(true);
			mComboBox_Preset->OnComboBoxEvent(CBE_SELECTION_CHANGED);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 400-16);

		return null;
	}

	virtual	const char*		GetSubName()	const
	{
		if(mComboBox_Preset)
		{
			const udword SelectedIndex = mComboBox_Preset->GetSelectedIndex();
			if(SelectedIndex==0)
				return "SpheresOnPlanarMesh";
			else if(SelectedIndex==1)
				return "BoxesOnPlanarMesh";
			else if(SelectedIndex==2)
				return "SmallConvexesOnPlanarMesh";
			else if(SelectedIndex==3)
				return "BigConvexesOnPlanarMesh";
			else if(SelectedIndex==4)
				return "DynamicSpheresOnArchipelago";
			else if(SelectedIndex==5)
				return "DynamicCapsulesOnArchipelago";
			else if(SelectedIndex==6)
				return "DynamicConvexesOnArchipelago";
			else if(SelectedIndex==7)
				return "DynamicConvexesOnArchipelago2";
			else if(SelectedIndex==8)
				return "DynamicConvexesOnArchipelagoTess";
		}
		return null;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);

		const MeshLevel ML = mComboBox_MeshLevel ? MeshLevel(mComboBox_MeshLevel->GetSelectedIndex()) : MESH_LEVEL_ARCHIPELAGO;
		if(ML==MESH_LEVEL_ARCHIPELAGO)
		{
			desc.mGlobalBounds.SetMinMax(Point(-10.0f, -10.0f, 0.0f), Point(430.0f, 100.0f, 300.0f));
			desc.mCamera[0] = PintCameraPose(Point(7.76f, 29.64f, 10.64f), Point(0.74f, -0.39f, 0.55f));
			desc.mCamera[1] = PintCameraPose(Point(-74.41f, 161.22f, 314.37f), Point(0.66f, -0.61f, -0.45f));
		}
		else if(ML==MESH_LEVEL_FLAT)
		{
			desc.mCamera[0] = PintCameraPose(Point(-32.59f, 5.48f, 31.90f), Point(0.63f, -0.42f, -0.65f));
			desc.mCamera[1] = PintCameraPose(Point(52.18f, 26.82f, 52.73f), Point(-0.65f, -0.40f, -0.65f));
		}
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		const MeshLevel ML = mComboBox_MeshLevel ? MeshLevel(mComboBox_MeshLevel->GetSelectedIndex()) : MESH_LEVEL_ARCHIPELAGO;

		const bool Tessellation = mCheckBox_Tessellation ? mCheckBox_Tessellation->IsChecked() : false;

		if(ML==MESH_LEVEL_ARCHIPELAGO)
			LoadMeshesFromFile_(*this, "Archipelago.bin", null, false, Tessellation ? 2 : 0, TESS_POLYHEDRAL);
		else if(ML==MESH_LEVEL_TESTZONE)
			LoadMeshesFromFile_(*this, "testzone.bin", null, false, Tessellation ? 2 : 0, TESS_POLYHEDRAL);
		else if(ML==MESH_LEVEL_FLAT)
		{
			IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
//			const udword Nb = Tessellation ? 64 : 32;
			const udword Nb = 32;
			bool status = IS->MakePlane(Nb, Nb);
			ASSERT(status);
//			const float S = Tessellation ? 0.05f : 0.1f;
			const float S = 0.1f;
			IS->Scale(Point(S, 1.0f, S));
			IS->Translate(Point(0.0f, 1.0f, 0.0f));
			IS->Flip();
			RegisterSurface(IS);
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)	// Mesh support will be tested in CreateMeshesFromRegisteredSurfaces
			return false;

		const float GridScale = GetFloat(0.0f, mEditBox_GridScale);
		const udword GridSize = GetInt(64, mEditBox_GridSize);
		const udword NbLayers = GetInt(1, mEditBox_Layers);
		const float Altitude = GetFloat(10.0f, mEditBox_Altitude);
		const float ShapeSize = GetFloat(1.0f, mEditBox_ShapeSize);

		const PintShape ShapeType = mComboBox_ShapeType ? PintShape(mComboBox_ShapeType->GetSelectedIndex()) : PINT_SHAPE_UNDEFINED;
		if((ShapeType == PINT_SHAPE_CONVEX) && !caps.mSupportConvexes)
			return false;

		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

		float ScaleX = GridScale;
		float ScaleZ = GridScale;
		Point Offset(0.0f, 0.0f, 0.0f);
		if(GridScale==0.0f)
		{
			Point Extents;
			GetGlobalBounds(Offset, Extents);

			const float Margin = 8.0f;
			ScaleX = Extents.x - Margin;
			ScaleZ = Extents.z - Margin;
		}

		const udword NbX = GridSize;
		const udword NbY = GridSize;
		if(ShapeType == PINT_SHAPE_UNDEFINED)
			return true;
		if(ShapeType == PINT_SHAPE_SPHERE)
		{
			const float Radius = ShapeSize;
			for(udword i=0;i<NbLayers;i++)
			{
				Point CurrentOffset = Offset;
				CurrentOffset.x += float(i)*2.0f;
				GenerateArrayOfSpheres(pint, Radius, NbX, NbY, Altitude+float(i)*3.0f, ScaleX, ScaleZ, 1.0f, 0, &CurrentOffset);
			}
			return true;
		}
		if(ShapeType == PINT_SHAPE_CAPSULE)
		{
			const float Radius = ShapeSize*0.5f;
			const float HalfHeight = 2.0f;
			for(udword i=0;i<NbLayers;i++)
			{
				Point CurrentOffset = Offset;
				CurrentOffset.x += float(i)*2.0f;
				GenerateArrayOfCapsules(pint, Radius, HalfHeight, NbX, NbY, Altitude+float(i)*3.0f, ScaleX, ScaleZ, 1.0f, 0, &CurrentOffset);
			}
			return true;
		}
		if(ShapeType == PINT_SHAPE_BOX)
		{
			const float Radius = ShapeSize;
			for(udword i=0;i<NbLayers;i++)
			{
				Point CurrentOffset = Offset;
				CurrentOffset.x += float(i)*2.0f;
				GenerateArrayOfBoxes(pint, Point(Radius, Radius, Radius), NbX, NbY, Altitude+float(i)*3.0f, ScaleX, ScaleZ, 1.0f, 0, &CurrentOffset);
			}
			return true;
		}
		if(ShapeType == PINT_SHAPE_CONVEX)
		{
			const ConvexIndex Index = mComboBox_ConvexIndex ? ConvexIndex(mComboBox_ConvexIndex->GetSelectedIndex()) : CONVEX_INDEX_0;

			MyConvex C;
			C.LoadFile(Index);
			C.Scale(ShapeSize);

			PINT_CONVEX_CREATE ConvexCreate(C.mNbVerts, C.mVerts);
			ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

			for(udword i=0;i<NbLayers;i++)
			{
				Point CurrentOffset = Offset;
				CurrentOffset.x += float(i)*2.0f;
				CreateArrayOfDynamicConvexes(pint, ConvexCreate, NbX, NbY, Altitude+float(i)*3.0f, ScaleX, ScaleZ, &CurrentOffset);
			}
			return true;
		}
		return false;
	}

}DynamicsOnMeshLevel;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_AddStaticObjects = "Add static objects at runtime.";

START_TEST(AddStaticObjects, CATEGORY_PERFORMANCE, gDesc_AddStaticObjects)

	AABB				mMoving;
	float				mAmplitude;
	float				mBoxSize;
	PINT_BOX_CREATE*	mBoxCreate;
	bool*				mFlags;
	Container			mCreated;
	PtrContainer		mNewActors;

	virtual	udword	GetProfilingFlags()	const
	{
		return PROFILING_SIM_UPDATE|PROFILING_TEST_POST_UPDATE;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.56f, -0.65f, -0.52f));
		SetDefEnv(desc, false);
	}

	virtual	void	CommonRelease()
	{
		ICE_FREE(mFlags);
		DELETEARRAY(mBoxCreate);
		mCreated.Empty();
		mNewActors.Empty();

		TestBase::CommonRelease();
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		mMoving.SetEmpty();

	//	mBoxSize = 4.0f;
		mBoxSize = 8.0f;

		mAmplitude = 40.0f;
	//	mAmplitude = 80.0f;

	//	const udword NbX = 64;
	//	const udword NbY = 64;
		const udword NbX = 128;
		const udword NbY = 128;
		const float Altitude = 0.0f;

		BasicRandom Rnd(42);

		mBoxCreate = ICE_NEW(PINT_BOX_CREATE)[NbX*NbY];
		mFlags = (bool*)ICE_ALLOC(sizeof(bool)*NbX*NbY);

		udword Index = 0;
		for(udword y=0;y<NbY;y++)
		{
			const float CoeffY = 2.0f * ((float(y)/float(NbY-1)) - 0.5f);
			for(udword x=0;x<NbX;x++)
			{
				const float CoeffX = 2.0f * ((float(x)/float(NbX-1)) - 0.5f);

				Point Random;
				UnitRandomPt(Random, Rnd);
				const Point Extents = Random + Point(1.0f, 1.0f, 1.0f);

				mFlags[Index] = false;
				mBoxCreate[Index].mExtents	= Extents;
				mBoxCreate[Index].mRenderer	= CreateBoxRenderer(Extents);
				Index++;

				UnitRandomPt(Random, Rnd);
				const Point Center = Random + Point(CoeffX * mAmplitude, Altitude, CoeffY * mAmplitude);

				AABB Box;
				Box.SetCenterExtents(Center, Extents);
				RegisterAABB(Box);
			}
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);

		const float t = mCurrentTime;
	//	const float t = mCurrentTime * 0.1f;

		const float PosScale = mAmplitude - mBoxSize;
		const float x = sinf(t*0.07f) * cosf(t*1.13f) * PosScale;
		const float y = sinf(t*2.07f) * cosf(t*0.13f) * PosScale;

		mMoving.SetCenterExtents(Point(x, 0.0f, y), Point(mBoxSize, 10.0f, mBoxSize));

		mCreated.Reset();
		udword NbBoxes = GetNbAABBs();
		const AABB* Boxes = (const AABB*)GetAABBs();
		for(udword i=0;i<NbBoxes;i++)
		{
			if(!mFlags[i] && mMoving.Intersect(Boxes[i]))
			{
				mFlags[i] = true;
				mCreated.Add(i);
			}
		}
	}

	virtual void	CommonDebugRender(PintRender& renderer)
	{
		renderer.DrawWireframeAABB(1, &mMoving, Point(1.0f, 0.0f, 0.0f));
		RenderAllAABBs(renderer);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		// We can use a shared array here because the data we store there will be reused
		// immediately in PostUpdate, before another engine is called.
		mNewActors.Reset();

		const AABB* Boxes = (const AABB*)GetAABBs();
		udword NbCreated = mCreated.GetNbEntries();
		const udword* Indices = mCreated.GetEntries();
		while(NbCreated--)
		{
			const udword i = *Indices++;

			Point Center;
			Boxes[i].GetCenter(Center);

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes		= mBoxCreate + i;
			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mPosition	= Center;
			ObjectDesc.mAddToWorld	= false;
			PintActorHandle h = CreatePintObject(pint, ObjectDesc);
			mNewActors.AddPtr(h);
		}
		return mCreated.GetNbEntries();
	}

	virtual udword	PostUpdate(Pint& pint, float dt)
	{
		Pint_Scene* API = pint.GetSceneAPI();
		if(API)
			API->AddActors(mNewActors.GetNbEntries(), reinterpret_cast<const PintActorHandle*>(mNewActors.GetEntries()));
		return mCreated.GetNbEntries();
	}

END_TEST(AddStaticObjects)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_AddStaticObjects2 = "64*64 static boxes... added at runtime to 128*128 other static boxes.";

START_TEST(AddStaticObjects2, CATEGORY_PERFORMANCE, gDesc_AddStaticObjects2)

	PtrContainer	mNewActors;
	bool			mStopTest;
	bool			mAddObjects;

	virtual	udword	GetProfilingFlags()	const
	{
		return PROFILING_SIM_UPDATE|PROFILING_TEST_POST_UPDATE;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.56f, -0.65f, -0.52f));
		SetDefEnv(desc, false);
	}

	virtual	void	CommonRelease()
	{
		mNewActors.Empty();
		TestBase::CommonRelease();
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		mAddObjects = false;
		mStopTest = false;
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return CreateSeaOfStaticBoxes(pint, 100.0f, 128, 128, 0.0f);
	}

	virtual void	CommonUpdate(float dt)
	{
		if(mStopTest)
			return;

		if(mAddObjects)
		{
			mStopTest = true;
			mAddObjects = false;
			return;
		}

		TestBase::CommonUpdate(dt);

		if(mCurrentTime>=1.0f)
			mAddObjects = true;
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		if(mAddObjects)
		{
			mNewActors.Reset();
			CreateSeaOfStaticBoxes(pint, 50.0f, 64, 64, 1.0f, false, &mNewActors);
		}

		return 0;
	}

	virtual udword	PostUpdate(Pint& pint, float dt)
	{
		if(mAddObjects)
		{
			Pint_Scene* API = pint.GetSceneAPI();
			if(API)
				API->AddActors(mNewActors.GetNbEntries(), reinterpret_cast<const PintActorHandle*>(mNewActors.GetEntries()));
		}
		return 0;
	}

END_TEST(AddStaticObjects2)

//static const char* gDesc_AddStaticObjects2 = "64*64 static boxes... added at runtime to 128*128 other static boxes.";

static const udword gNbX = 64;
static const udword gNbY = 64;

START_TEST(AddStaticObjects2b, CATEGORY_PERFORMANCE, gDesc_AddStaticObjects2)

	PtrContainer	mNewActors;

	struct ActorData
	{
		PintShapeRenderer*	mRenderer;
		Point				mExtents;
		Point				mCenter;
	};
	ActorData		mData[gNbX*gNbY];

	virtual	udword	GetProfilingFlags()	const
	{
		return PROFILING_SIM_UPDATE|PROFILING_TEST_UPDATE;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.56f, -0.65f, -0.52f));
		SetDefEnv(desc, false);
	}

	virtual	void	CommonRelease()
	{
		mNewActors.Empty();
		TestBase::CommonRelease();
	}

	virtual bool	CommonSetup()
	{
		BasicRandom Rnd(42);

		const float amplitude = 50.0f;
		const float altitude = 1.0f;
		udword Index = 0;
		for(udword y=0;y<gNbY;y++)
		{
			const float CoeffY = 2.0f * ((float(y)/float(gNbY-1)) - 0.5f);
			for(udword x=0;x<gNbX;x++)
			{
				const float CoeffX = 2.0f * ((float(x)/float(gNbX-1)) - 0.5f);

				Point Random;
				UnitRandomPt(Random, Rnd);
				const Point Extents = Random + Point(1.0f, 1.0f, 1.0f);

				UnitRandomPt(Random, Rnd);
				const Point Center = Random + Point(CoeffX * amplitude, altitude, CoeffY * amplitude);

				mData[Index].mExtents = Extents;
				mData[Index].mCenter = Center;
				mData[Index].mRenderer = CreateBoxRenderer(Extents);
				Index++;
			}
		}
		ASSERT(Index==gNbX*gNbY);

		TestBase::CommonSetup();
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return CreateSeaOfStaticBoxes(pint, 100.0f, 128, 128, 0.0f);
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);
	}

	virtual void	PreUpdate(Pint& pint, float dt)
	{
		{
			mNewActors.Reset();
//			CreateSeaOfStaticBoxes(pint, 50.0f, 64, 64, 1.0f, false, &mNewActors);

			PINT_BOX_CREATE BoxCreate;

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes		= &BoxCreate;
			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mAddToWorld	= false;

			udword Index = 0;
			for(udword y=0;y<gNbY;y++)
			{
				const float CoeffY = 2.0f * ((float(y)/float(gNbY-1)) - 0.5f);
				for(udword x=0;x<gNbX;x++)
				{
					const float CoeffX = 2.0f * ((float(x)/float(gNbX-1)) - 0.5f);

					BoxCreate.mExtents		= mData[Index].mExtents;
					BoxCreate.mRenderer		= mData[Index].mRenderer;
					ObjectDesc.mPosition	= mData[Index].mCenter;
					Index++;

					PintActorHandle h = CreatePintObject(pint, ObjectDesc);
					mNewActors.AddPtr(h);
				}
			}
		}
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		{
			Pint_Scene* API = pint.GetSceneAPI();
			if(API)
				API->AddActors(mNewActors.GetNbEntries(), reinterpret_cast<const PintActorHandle*>(mNewActors.GetEntries()));
		}
		return 0;
	}

	virtual udword	PostUpdate(Pint& pint, float dt)
	{
		const udword NbActors = mNewActors.GetNbEntries();
		PintActorHandle* Actors = reinterpret_cast<PintActorHandle*>(mNewActors.GetEntries());
		for(udword i=0;i<NbActors;i++)
		{
			pint.ReleaseObject(Actors[i]);
		}
		pint.Update(dt);

		return 0;
	}

END_TEST(AddStaticObjects2b)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_AddStaticObjects3 = "Add static objects at runtime - one per frame. Use the debug viz to see the static AABB tree being rebuilt and updated after N frames.";

START_TEST(AddStaticObjects3, CATEGORY_PERFORMANCE, gDesc_AddStaticObjects3)

	AABB				mMoving;
	float				mAmplitude;
	float				mBoxSize;
	PintShapeRenderer*	mRenderer;
	PintActorHandle		mCurrentActor;

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(244.38f, 62.27f, 104.41f), Point(-0.89f, -0.05f, -0.45f));
		SetDefEnv(desc, false);
	}

	virtual	udword	GetProfilingFlags()	const
	{
		return PROFILING_SIM_UPDATE|PROFILING_TEST_POST_UPDATE;
	}

	virtual	void	CommonRelease()
	{
		TestBase::CommonRelease();
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		mMoving.SetEmpty();

		mBoxSize = 1.0f;
		mAmplitude = 80.0f;

		const Point Extents(1.0f, 1.0f, 1.0f);
		mRenderer = CreateBoxRenderer(Extents);
		mCurrentActor = null;

		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);

//		const float t = mCurrentTime * 2.0f;
		const float t = mCurrentTime * 0.5f;

		const float PosScale = mAmplitude - mBoxSize;
		const float x = sinf(t*0.07f) * cosf(t*1.13f) * PosScale;
		const float y = sinf(t*2.07f) * cosf(t*0.13f) * PosScale;
		const float z = sinf(t*1.13333f) * cosf(t*0.789123f) * PosScale;

		mMoving.SetCenterExtents(Point(x, y, z), Point(mBoxSize, 10.0f, mBoxSize));
	}

	virtual void	CommonDebugRender(PintRender& renderer)
	{
//		renderer.DrawWirefameAABB(mMoving, Point(1.0f, 0.0f, 0.0f));
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		PINT_BOX_CREATE BoxCreate(1.0f, 1.0f, 1.0f);
		BoxCreate.mRenderer	= mRenderer;

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes		= &BoxCreate;
		ObjectDesc.mMass		= 0.0f;
		mMoving.GetCenter(ObjectDesc.mPosition);
		ObjectDesc.mPosition.y += 50.0f;
		ObjectDesc.mAddToWorld	= false;
		mCurrentActor = CreatePintObject(pint, ObjectDesc);
		return 0;
	}

	virtual udword	PostUpdate(Pint& pint, float dt)
	{
		Pint_Scene* API = pint.GetSceneAPI();
		if(API)
			API->AddActors(1, &mCurrentActor);

		return 0;
	}

END_TEST(AddStaticObjects3)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_AddDynamicObjects = "Add dynamic objects at runtime - one per frame. Use the debug viz to see the dynamic AABB tree being rebuilt and updated after N frames.";

START_TEST(AddDynamicObjects, CATEGORY_PERFORMANCE, gDesc_AddDynamicObjects)

	AABB				mMoving;
	float				mAmplitude;
	float				mBoxSize;
	PintShapeRenderer*	mRenderer;
	PintActorHandle		mCurrentActor;

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(179.07f, 111.03f, 41.57f), Point(-0.70f, -0.68f, -0.22f));
		SetDefEnv(desc, true);
	}

	virtual	udword	GetProfilingFlags()	const
	{
		return PROFILING_SIM_UPDATE|PROFILING_TEST_POST_UPDATE;
	}

	virtual	float	GetRenderData(Point& center)	const
	{
		return 400.0f;
	}

	virtual	void	CommonRelease()
	{
		TestBase::CommonRelease();
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		mMoving.SetEmpty();

		mBoxSize = 1.0f;
		mAmplitude = 80.0f;

		const Point Extents(1.0f, 1.0f, 1.0f);
		mRenderer = CreateBoxRenderer(Extents);
		mCurrentActor = null;

		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);

		const float t = mCurrentTime * 2.0f;
	//	const float t = mCurrentTime * 0.1f;

		const float PosScale = mAmplitude - mBoxSize;
		const float x = sinf(t*0.07f) * cosf(t*1.13f) * PosScale;
		const float y = sinf(t*2.07f) * cosf(t*0.13f) * PosScale;

		mMoving.SetCenterExtents(Point(x, 0.0f, y), Point(mBoxSize, 10.0f, mBoxSize));
	}

	virtual void	CommonDebugRender(PintRender& renderer)
	{
		renderer.DrawWireframeAABB(1, &mMoving, Point(1.0f, 0.0f, 0.0f));
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		PINT_BOX_CREATE BoxCreate(1.0f, 1.0f, 1.0f);
		BoxCreate.mRenderer	= mRenderer;

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes		= &BoxCreate;
		ObjectDesc.mMass		= 1.0f;
		//ObjectDesc.mMass		= 0.0f;
		mMoving.GetCenter(ObjectDesc.mPosition);
		ObjectDesc.mPosition.y += 50.0f;
		ObjectDesc.mAddToWorld	= false;
		mCurrentActor = CreatePintObject(pint, ObjectDesc);
		return 0;
	}

	virtual udword	PostUpdate(Pint& pint, float dt)
	{
		Pint_Scene* API = pint.GetSceneAPI();
		if(API)
			API->AddActors(1, &mCurrentActor);

		return 0;
	}

END_TEST(AddDynamicObjects)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_AddDynamicObjects2 = "Add dynamic objects at runtime.";

START_TEST(AddDynamicObjects2, CATEGORY_PERFORMANCE, gDesc_AddDynamicObjects2)

	PtrContainer	mNewActors;

	virtual	udword	GetProfilingFlags()	const
	{
		return PROFILING_SIM_UPDATE|PROFILING_TEST_POST_UPDATE;
	}

	virtual	void	CommonRelease()
	{
		mNewActors.Empty();
		TestBase::CommonRelease();
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		return true;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(37.84f, 57.69f, 32.04f), Point(-0.62f, -0.60f, -0.50f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		const float BoxHeight = 8.0f;
		const float BoxSide = 0.01f;
		const float BoxDepth = 20.0f;
		CreateBoxContainer(pint, BoxHeight, BoxSide, BoxDepth);
		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);

		if(mCurrentTime>1.0f)
		{
			mCurrentTime = 0.0f;
		}
	}

	virtual void	CommonDebugRender(PintRender& renderer)
	{
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		// PT: TODO: revisit this test. Creating renderers here pollutes the timings. Not good.
		if(mCurrentTime==0.0f)
		{
			mNewActors.Reset();
			//GenerateArrayOfBoxes(pint, Point(0.5f, 0.5f, 0.5f), 4, 4, 50.0f, 2.0f, 2.0f, 1.0f);
			//GenerateArrayOfBoxes(pint, Point(0.5f, 0.5f, 0.5f), 4, 4, 48.0f, 2.0f, 2.0f, 1.0f);

			PINT_BOX_CREATE BoxDesc(Point(0.5f, 0.5f, 0.5f));
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes		= &BoxDesc;
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mAddToWorld	= false;

			const udword nb_x = 8;
			const udword nb_y = 8;
			const float scale_x = 4.0f;
			const float scale_z = 4.0f;
			const float altitude = 48.0f;

			const float OneOverNbX = OneOverNb(nb_x);
			const float OneOverNbY = OneOverNb(nb_y);
			for(udword y=0;y<nb_y;y++)
			{
				const float CoeffY = 2.0f * ((float(y)*OneOverNbY) - 0.5f);
				for(udword x=0;x<nb_x;x++)
				{
					const float CoeffX = 2.0f * ((float(x)*OneOverNbX) - 0.5f);

					Point Origin(CoeffX * scale_x, altitude, CoeffY * scale_z);

					ObjectDesc.mPosition	= Origin;
					PintActorHandle h = CreatePintObject(pint, ObjectDesc);
					mNewActors.AddPtr(h);

					ObjectDesc.mPosition.y	+= 2.0f;
					h = CreatePintObject(pint, ObjectDesc);
					mNewActors.AddPtr(h);
				}
			}
		}
		return 0;
	}

	virtual udword	PostUpdate(Pint& pint, float dt)
	{
		if(mCurrentTime==0.0f)
		{
			Pint_Scene* API = pint.GetSceneAPI();
			if(API)
				API->AddActors(mNewActors.GetNbEntries(), reinterpret_cast<const PintActorHandle*>(mNewActors.GetEntries()));
		}
		return 0;
	}

END_TEST(AddDynamicObjects2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_AddDynamicObjectsAndDoRaycasts = "Add dynamic objects at runtime, and do raycasts.";

//START_SQ_TEST(AddDynamicObjectsAndDoRaycasts, CATEGORY_PERFORMANCE, gDesc_AddDynamicObjectsAndDoRaycasts)
class AddDynamicObjectsAndDoRaycasts : public TestBase
{
	public:
									AddDynamicObjectsAndDoRaycasts()		{												}
			virtual					~AddDynamicObjectsAndDoRaycasts()		{												}
			virtual	const char*		GetName()						const	{ return "AddDynamicObjectsAndDoRaycasts";		}
			virtual	const char*		GetDescription()				const	{ return gDesc_AddDynamicObjectsAndDoRaycasts;	}
			virtual	TestCategory	GetCategory()					const	{ return CATEGORY_PERFORMANCE;					}

	AABB				mMoving;
	float				mAmplitude;
	float				mBoxSize;
	PintShapeRenderer*	mRenderer;
	PintActorHandle		mNewActors[10];

	virtual	udword	GetProfilingFlags()	const
	{
		return PROFILING_SIM_UPDATE|PROFILING_TEST_POST_UPDATE;
	}

	virtual	void	CommonRelease()
	{
		TestBase::CommonRelease();
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(132.36f, 84.88f, 89.41f), Point(-0.84f, -0.36f, -0.40f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		mMoving.SetEmpty();

		mBoxSize = 1.0f;
		mAmplitude = 80.0f;

		const Point Extents(1.0f, 1.0f, 1.0f);
		mRenderer = CreateBoxRenderer(Extents);

		bool Status = Setup_PotPourri_Raycasts(*this, 4096, 100.0f);
		return Status;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return Setup_PotPourri_Raycasts(pint, caps, 0.0f, 16, 16, 16);
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);

		const float t = mCurrentTime * 2.0f;
	//	const float t = mCurrentTime * 0.1f;

		const float PosScale = mAmplitude - mBoxSize;
		const float x = sinf(t*0.07f) * cosf(t*1.13f) * PosScale;
		const float y = sinf(t*2.07f) * cosf(t*0.13f) * PosScale;

		mMoving.SetCenterExtents(Point(x, 0.0f, y), Point(mBoxSize, 10.0f, mBoxSize));
	}

	virtual void	CommonDebugRender(PintRender& renderer)
	{
		renderer.DrawWireframeAABB(1, &mMoving, Point(1.0f, 0.0f, 0.0f));
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		PINT_BOX_CREATE BoxCreate(1.0f, 1.0f, 1.0f);
		BoxCreate.mRenderer	= mRenderer;

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes		= &BoxCreate;
		ObjectDesc.mMass		= 1.0f;
		mMoving.GetCenter(ObjectDesc.mPosition);
		ObjectDesc.mPosition.y += 50.0f;
		ObjectDesc.mAddToWorld	= false;
		mNewActors[0] = CreatePintObject(pint, ObjectDesc);
		for(udword i=1;i<10;i++)
		{
			ObjectDesc.mPosition.y += 1.0f;
			mNewActors[i] = CreatePintObject(pint, ObjectDesc);
		}

		return 0;
	}

	virtual udword	PostUpdate(Pint& pint, float dt)
	{
		Pint_Scene* API = pint.GetSceneAPI();
		if(API)
			API->AddActors(10, mNewActors);

		return DoBatchRaycasts(*this, pint);
	}

END_TEST(AddDynamicObjectsAndDoRaycasts)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_AddDynamicConvexes = "Add dynamic convexes at runtime.";

START_TEST(AddDynamicConvexes, CATEGORY_PERFORMANCE, gDesc_AddDynamicConvexes)

	AABB				mMoving;
	float				mAmplitude;
	float				mBoxSize;
	PintShapeRenderer*	mRenderer;
	PintActorHandle		mCurrentActor;

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(6.47f, 12.32f, 77.49f), Point(-0.26f, -0.48f, -0.84f));
		SetDefEnv(desc, true);
	}

	virtual	udword	GetProfilingFlags()	const
	{
		return PROFILING_SIM_UPDATE|PROFILING_TEST_POST_UPDATE;
	}

	virtual	float	GetRenderData(Point& center)	const
	{
		return 400.0f;
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		mMoving.SetEmpty();

		mBoxSize = 1.0f;
		mAmplitude = 80.0f;

		const Point Extents(1.0f, 1.0f, 1.0f);
		mRenderer = CreateBoxRenderer(Extents);
		mCurrentActor = null;

		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return true;
	}

	virtual void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);

		const float t = mCurrentTime * 2.0f;
	//	const float t = mCurrentTime * 0.1f;

		const float PosScale = mAmplitude - mBoxSize;
		const float x = sinf(t*0.07f) * cosf(t*1.13f) * PosScale;
		const float y = sinf(t*2.07f) * cosf(t*0.13f) * PosScale;

		mMoving.SetCenterExtents(Point(x, 0.0f, y), Point(mBoxSize, 10.0f, mBoxSize));
	}

	virtual void	CommonDebugRender(PintRender& renderer)
	{
		renderer.DrawWireframeAABB(1, &mMoving, Point(1.0f, 0.0f, 0.0f));
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		static BasicRandom Rnd(42);
		const udword NbVerts = 8;
		Point Verts[NbVerts];
		for(udword i=0;i<NbVerts;i++)
		{			
			Verts[i].x = Rnd.RandomFloat();
			Verts[i].y = Rnd.RandomFloat();
			Verts[i].z = Rnd.RandomFloat();
		}

		PINT_CONVEX_CREATE ConvexCreate(8, Verts);
		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes		= &ConvexCreate;
		ObjectDesc.mMass		= 1.0f;
		//ObjectDesc.mMass		= 0.0f;
		mMoving.GetCenter(ObjectDesc.mPosition);
		//ObjectDesc.mPosition.y += 50.0f;
		ObjectDesc.mAddToWorld	= false;
		mCurrentActor = CreatePintObject(pint, ObjectDesc);
		return 0;
	}

	virtual udword	PostUpdate(Pint& pint, float dt)
	{
		Pint_Scene* API = pint.GetSceneAPI();
		if(API)
			API->AddActors(1, &mCurrentActor);

		return 0;
	}

END_TEST(AddDynamicConvexes)

///////////////////////////////////////////////////////////////////////////////

#include "hacdCircularList.h"
#include "hacdVector.h"
#include "hacdICHull.h"
#include "hacdGraph.h"
#include "hacdHACD.h"

//static const bool gUseRenderMesh = true;

/*static*/ void DecomposeMeshWithHACD(Pint& pint, udword nb, const Point* pos, const SurfaceInterface& surface, bool dynamic, bool use_render_mesh)
{
	std::vector< HACD::Vec3<HACD::Real> > points;
	{
		const udword NbVerts = surface.mNbVerts;
		const Point* Verts = surface.mVerts;
		points.reserve(NbVerts);
		for(udword i=0;i<NbVerts;i++)
		{
			points.push_back(HACD::Vec3<HACD::Real>(Verts[i].x, Verts[i].y, Verts[i].z));
		}
	}

	std::vector< HACD::Vec3<long> > triangles;
	{
		const udword NbTris = surface.mNbFaces;
		points.reserve(NbTris);
		for(udword i=0;i<NbTris;i++)
		{
			const int index = i*3;

			udword VRef0,VRef1,VRef2;
			if(surface.mDFaces)
			{
				VRef0 = surface.mDFaces[index+0];
				VRef1 = surface.mDFaces[index+1];
				VRef2 = surface.mDFaces[index+2];
			}
			else if(surface.mWFaces)
			{
				VRef0 = surface.mWFaces[index+0];
				VRef1 = surface.mWFaces[index+1];
				VRef2 = surface.mWFaces[index+2];
			}

			triangles.push_back(HACD::Vec3<long>(VRef0, VRef1, VRef2));
		}
	}

	HACD::HACD myHACD;
	myHACD.SetPoints(&points[0]);
	myHACD.SetNPoints(points.size());
	myHACD.SetTriangles(&triangles[0]);
	myHACD.SetNTriangles(triangles.size());
	myHACD.SetCompacityWeight(0.1);
	myHACD.SetVolumeWeight(0.0);

	// HACD parameters
	// Recommended parameters: 2 100 0 0 0 0
	size_t nClusters = 2;
	double concavity = 100;
	bool invert = false;
	bool addExtraDistPoints = false;
	bool addNeighboursDistPoints = false;
	bool addFacesPoints = false;       

	myHACD.SetNClusters(nClusters);                     // minimum number of clusters
	myHACD.SetNVerticesPerCH(100);                      // max of 100 vertices per convex-hull
	myHACD.SetConcavity(concavity);                     // maximum concavity
	myHACD.SetAddExtraDistPoints(addExtraDistPoints);   
	myHACD.SetAddNeighboursDistPoints(addNeighboursDistPoints);   
	myHACD.SetAddFacesPoints(addFacesPoints); 

	myHACD.Compute();
	nClusters = myHACD.GetNClusters();	

//	myHACD.Save("output.wrl", false);

	{
		PINT_CONVEX_CREATE* ConvexCreate = ICE_NEW(PINT_CONVEX_CREATE)[nClusters];
		for(size_t c=0;c<nClusters;c++)
		{
			//generate convex result
			size_t nPoints = myHACD.GetNPointsCH(c);
			size_t nTriangles = myHACD.GetNTrianglesCH(c);

			float* vertices = new float[nPoints*3];
//			udword* triangles = new unsigned int[nTriangles*3];
			
			HACD::Vec3<HACD::Real> * pointsCH = new HACD::Vec3<HACD::Real>[nPoints];
			HACD::Vec3<long> * trianglesCH = new HACD::Vec3<long>[nTriangles];
			myHACD.GetCH(c, pointsCH, trianglesCH);

			// points
			for(size_t v = 0; v < nPoints; v++)
			{
				vertices[3*v] = float(pointsCH[v].X());
				vertices[3*v+1] = float(pointsCH[v].Y());
				vertices[3*v+2] = float(pointsCH[v].Z());
			}
/*			// triangles
			for(size_t f = 0; f < nTriangles; f++)
			{
				triangles[3*f] = trianglesCH[f].X();
				triangles[3*f+1] = trianglesCH[f].Y();
				triangles[3*f+2] = trianglesCH[f].Z();
			}*/

			delete [] pointsCH;
			delete [] trianglesCH;

			Point* Verts = (Point*)vertices;

			ConvexCreate[c].mNbVerts	= udword(nPoints);
			ConvexCreate[c].mVerts		= Verts;

			if(1)
			{
				Point Center(0.0f, 0.0f, 0.0f);
				const float Coeff = 1.0f / float(nPoints);
				for(udword i=0;i<nPoints;i++)
					Center += Verts[i] * Coeff;

				ConvexCreate[c].mLocalPos = Center;

				for(udword i=0;i<nPoints;i++)
					Verts[i] -= Center;
			}

			if(use_render_mesh)
				ConvexCreate[c].mRenderer	= CreateNullRenderer();
			else
				ConvexCreate[c].mRenderer	= CreateConvexRenderer(udword(nPoints), Verts);
		}

		if(1)
		{
			for(udword i=0;i<nClusters-1;i++)
				ConvexCreate[i].mNext = &ConvexCreate[i+1];

			PINT_BOX_CREATE BoxDesc(0.01f, 0.01f, 0.01f);
			if(use_render_mesh)
			{
				// ### TODO: revisit CRC here
				BoxDesc.mRenderer	= CreateMeshRenderer(PintSurfaceInterface(surface));
				ConvexCreate[nClusters-1].mNext = &BoxDesc;
			}

			for(udword i=0;i<nb;i++)
			{
				if(dynamic)
				{
					PintActorHandle Handle = CreateDynamicObject(pint, ConvexCreate, pos[i]);
					ASSERT(Handle);
				}
				else
				{
					PintActorHandle Handle = CreateStaticObject(pint, ConvexCreate, pos[i]);
					ASSERT(Handle);
				}
			}

			for(udword i=0;i<nClusters;i++)
			{
				float* v = (float*)ConvexCreate[i].mVerts;
				DELETEARRAY(v);
			}

			DELETEARRAY(ConvexCreate);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

#include "IceBunny.h"

static const char* gDesc_HACD_Test_01 = "HACD test. This uses convex decomposition to simulate mesh-mesh interactions using compounds of convex meshes.";

START_TEST(HACD_Test_01, CATEGORY_PERFORMANCE, gDesc_HACD_Test_01)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(10.88f, 9.75f, 9.58f), Point(-0.68f, -0.33f, -0.65f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportConvexes || !caps.mSupportCompounds)
			return false;

		const udword NbMeshes = 100;
		//const udword NbMeshes = 1;

		Point Pos[NbMeshes];
		for(udword i=0;i<NbMeshes;i++)
			Pos[i].Set(0.0f, 2.0f+float(i)*2.0f, 0.0f);

		Bunny Rabbit;
		//DecomposeMeshWithHACD(pint, NbMeshes, Pos, Rabbit.GetSurface(), true, true);
		DecomposeMeshWithHACD(pint, NbMeshes, Pos, Rabbit.GetSurface(), true, false);

/*		if(0)
		{
			LoadMeshesFromFile(pint, *this, "Prison.bin");

			mCreateDefaultEnvironment = false;
		}*/
		return true;
	}

END_TEST(HACD_Test_01)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_HACD_Test_02 = "64 convex-compounds falling on the Archipelago mesh level.";

START_TEST(HACD_Test_02, CATEGORY_PERFORMANCE, gDesc_HACD_Test_02)

	virtual	float	GetRenderData(Point& center)	const
	{
		const Point Min(-10.0f, -10.0f, 0.0f);
		const Point Max(430.0f, 100.0f, 300.0f);
		center = (Min + Max)*0.5f;
		return 500.0f;
//		return 2000.0f;
	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mGlobalBounds.SetMinMax(Point(-10.0f, -10.0f, 0.0f), Point(430.0f, 100.0f, 300.0f));
		desc.mCamera[0] = PintCameraPose(Point(197.08f, 4.05f, 139.89f), Point(0.60f, -0.28f, 0.75f));
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
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportConvexes || !caps.mSupportCompounds)
			return false;

		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

		Point Center, Extents;
		GetGlobalBounds(Center, Extents);

/*		const udword NbX = 64;
		const udword NbY = 64;
		const float Altitude = 10.0f;
		const float Radius = 1.0f;
		GenerateArrayOfSpheres(pint, Radius, NbX, NbY, Altitude, Extents.x-8.0f, Extents.z-8.0f, 1.0f, 0, &Offset);*/

		const udword NbMeshes = 64;

		Point Pos[NbMeshes];
		for(udword i=0;i<NbMeshes;i++)
			Pos[i].Set(Center.x, Center.y + float(i)*2.0f, Center.z);

		Bunny Rabbit;
		DecomposeMeshWithHACD(pint, NbMeshes, Pos, Rabbit.GetSurface(), true, true);

		return true;
	}

END_TEST(HACD_Test_02)

///////////////////////////////////////////////////////////////////////////////

