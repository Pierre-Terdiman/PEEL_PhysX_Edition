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
#include "Camera.h"
//#include "PintSQ.h"

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_BoxStackConfigurable = "Box stacks. Things to look for are the time it takes to simulate the stacks, \
but also whether they eventually collapse or not. Selecting multiple stacks creates multiple simulation islands that stress engines in different ways.";

#define NB_BOX_STACKS_PRESETS	5
static const udword	gPreset_NbStacks[]		= { 1,   1,  1, 10, 30 };
static const udword	gPreset_NbBaseBoxes[]	= { 20, 30, 50, 20, 10 };

class BoxStackConfigurable : public TestBase//, public PintContactNotifyCallback
{
			ComboBoxPtr		mComboBox_Preset;
			EditBoxPtr		mEditBox_NbBaseBoxes;
			EditBoxPtr		mEditBox_NbStacks;
			CheckBoxPtr		mCheckBox_UseConvexes;
	public:
							BoxStackConfigurable()		{										}
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

static const char* gDesc_BoxContainerConfigurable = "A static box container filled with various dynamic objects.";

#define NB_BOX_CONTAINER_PRESETS	5
static const float	gPreset_SphereRadius[]	= { 0.5f, 0.0f, 0.0f, 0.5f, 0.5f };
static const float	gPreset_CapsuleRadius[]	= { 0.0f, 0.0f, 0.3f, 0.3f, 0.3f };
static const float	gPreset_HalfHeight[]	= { 0.0f, 0.0f, 0.5f, 0.5f, 0.5f };
static const float	gPreset_BoxSize[]		= { 0.0f, 0.3f, 0.0f, 0.3f, 0.3f };
static const float	gPreset_ConvexSize[]	= { 0.0f, 0.0f, 0.0f, 0.0f, 0.4f };

class BoxContainerConfigurable : public TestBase
{
			ComboBoxPtr		mComboBox_Preset;
			EditBoxPtr		mEditBox_NbX;
			EditBoxPtr		mEditBox_NbY;
			EditBoxPtr		mEditBox_NbZ;
			EditBoxPtr		mEditBox_SphereRadius;
			EditBoxPtr		mEditBox_CapsuleRadius;
			EditBoxPtr		mEditBox_HalfHeight;
			EditBoxPtr		mEditBox_BoxSize;
			EditBoxPtr		mEditBox_ConvexSize;
	public:
							BoxContainerConfigurable()	{										}
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

					PINT_OBJECT_CREATE ObjectDesc(Shapes[Index]);
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
				BoxDesc[i].SetNext(&BoxDesc[i+1]);
		}

		PINT_OBJECT_CREATE ObjectDesc(BoxDesc);
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

				// Aggregates can also be useful a single actor, if it has a lot of shapes
				PintAggregateHandle Aggregate = null;
				if(caps.mSupportAggregates)
					Aggregate = pint.CreateAggregate(1, false);
				ObjectDesc.mAddToWorld	= Aggregate==null;

				const PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
				ASSERT(Handle);

				if(Aggregate)
				{
					pint.AddToAggregate(Handle, Aggregate);
					pint.AddAggregateToScene(Aggregate);
				}
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
				BoxDesc[i].SetNext(&BoxDesc[i+1]);
		}

		PINT_OBJECT_CREATE ObjectDesc(BoxDesc);
		ObjectDesc.mPosition.x	= 0.0f;
		ObjectDesc.mPosition.z	= 0.0f;
		ObjectDesc.mPosition.y	= BoxPosY;
		ObjectDesc.mMass		= 1.0f;

		for(udword n=0;n<8;n++)
		{
			ObjectDesc.mPosition.y	= BoxPosY + n * SphereRadius * 2.5f;

			// Aggregates can also be useful a single actor, if it has a lot of shapes
			PintAggregateHandle Aggregate = null;
			if(caps.mSupportAggregates)
				Aggregate = pint.CreateAggregate(1, false);
			ObjectDesc.mAddToWorld	= Aggregate==null;

			PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
			ASSERT(Handle);

			if(Aggregate)
			{
				pint.AddToAggregate(Handle, Aggregate);
				pint.AddAggregateToScene(Aggregate);
			}
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
		BoxDesc[0].SetNext(&BoxDesc[1]);
		BoxDesc[1].SetNext(&BoxDesc[2]);

		PINT_OBJECT_CREATE ObjectDesc(BoxDesc);
		ObjectDesc.mMass	= 1.0f;

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
		BoxDesc[0].SetNext(&BoxDesc[1]);
		BoxDesc[1].SetNext(&BoxDesc[2]);

		PINT_OBJECT_CREATE ObjectDesc(BoxDesc);
		ObjectDesc.mMass	= 1.0f;

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

static const char* gDesc_ConvexClash = "Dynamic convexes are falling towards static convexes. \
This is a stress test for both convex contact generation and the broad phase.";

class ConvexClash : public TestBase
{
			ComboBoxPtr		mComboBox_ConvexIndex;
			EditBoxPtr		mEditBox_Size;
	public:
							ConvexClash()				{								}
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

static const char* gDesc_FallingSpheres = "Falling spheres.";

START_TEST(FallingSpheres, CATEGORY_PERFORMANCE, gDesc_FallingSpheres)

	virtual	float	GetRenderData(Point& center)	const	{ return 400.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-32.59f, 5.48f, 31.90f), Point(0.63f, -0.42f, -0.65f));
		desc.mCamera[1] = PintCameraPose(Point(52.18f, 26.82f, 52.73f), Point(-0.65f, -0.40f, -0.65f));
		SetDefEnv(desc, false);
		desc.mGravity.y = -1.0f;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float Altitude = 1.0f;
		const float Radius = 0.5f;
		const udword NbX = 100;
		const udword NbY = 100;
		return GenerateArrayOfSpheres(pint, Radius, NbX, NbY, Altitude + Radius*2.0f, 60.0f, 60.0f);
	}

END_TEST(FallingSpheres)

///////////////////////////////////////////////////////////////////////////////

// TODO:
// - expose other meshes
// - use tessellation in plane scene
// - name/describe convexes?
// - random shape?
static const char* gDesc_DynamicsOnMeshLevel = "Dynamic objects falling on a mesh level.";

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
			ComboBoxPtr		mComboBox_Preset;
			ComboBoxPtr		mComboBox_ConvexIndex;
			ComboBoxPtr		mComboBox_ShapeType;
			ComboBoxPtr		mComboBox_MeshLevel;
			EditBoxPtr		mEditBox_GridSize;
			EditBoxPtr		mEditBox_GridScale;
			EditBoxPtr		mEditBox_Layers;
			EditBoxPtr		mEditBox_Altitude;
			EditBoxPtr		mEditBox_ShapeSize;
			CheckBoxPtr		mCheckBox_Tessellation;
	public:
							DynamicsOnMeshLevel()		{									}
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

			PINT_OBJECT_CREATE ObjectDesc(mBoxCreate + i);
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

			PINT_OBJECT_CREATE ObjectDesc(&BoxCreate);
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
			pint._ReleaseObject(Actors[i]);
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

		PINT_OBJECT_CREATE ObjectDesc(&BoxCreate);
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

		PINT_OBJECT_CREATE ObjectDesc(&BoxCreate);
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

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
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

		PINT_OBJECT_CREATE ObjectDesc(&BoxCreate);
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

		PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
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
				ConvexCreate[i].SetNext(&ConvexCreate[i+1]);

			PINT_BOX_CREATE BoxDesc(0.01f, 0.01f, 0.01f);
			if(use_render_mesh)
			{
				// ### TODO: revisit CRC here
				BoxDesc.mRenderer	= CreateMeshRenderer(PintSurfaceInterface(surface));
				ConvexCreate[nClusters-1].SetNext(&BoxDesc);
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

#pragma warning( push )
#pragma warning( disable : 4305 )	// truncation from 'double' to 'float'

static const float MyConvexData[] = {
0.839275,-0.478472,0.133933,
0.817428,-0.616350,-0.367357,
-0.744601,0.710312,0.210402,
-0.537063,-0.324568,-0.664535,
-0.440135,-0.127270,-0.710301,
-0.378650,-0.504053,-0.767549,
-0.127964,-0.102213,-0.698528,
0.587587,-0.304288,-0.560920,
-0.485744,-0.466224,-0.635723,
-0.583596,0.682339,-0.131100,
0.422084,-1.02258,0.304937,
-0.190028,-0.682995,-0.646649,
-0.577834,-0.184070,-0.561864,
0.327807,-1.03083,0.205459,
0.662819,-0.236162,-0.511834,
0.691812,-0.496869,-0.530107,
-0.713939,0.567371,0.580461,
-0.551524,-0.0428219,0.533652,
0.654504,-0.0147720,-0.378468,
0.201423,0.582430,-0.561471,
0.447385,0.319719,-0.0735297,
-0.627262,-0.0302035,0.448575,
-0.775861,0.755397,0.468820,
-0.653601,-0.0964046,0.123275,
-0.426024,0.641196,-0.387118,
-0.214228,0.285950,-0.646215,
-0.631980,0.540345,-0.242740,
0.546527,-0.524605,0.425772,
0.413873,0.271450,0.536733,
0.591398,0.0693753,0.105569,
0.781172,-0.456403,0.248893,
-0.320549,-0.611921,-0.553188,
0.379678,-1.06236,-0.0406003,
-0.0847278,0.653405,-0.511332,
-0.115970,0.984778,-0.0604422,
-0.642476,0.487011,0.641943,
-0.578036,-0.131010,0.493890,
0.843970,-0.351357,0.0691215,
-0.0808171,1.00307,0.0471670,
0.875630,-0.424511,-0.0755476,
-0.359864,0.602336,0.656958,
0.144630,-0.699243,0.544215,
-0.0516594,1.04437,0.355406,
0.0185653,0.937246,0.546723,
0.493147,-1.01257,-0.0719024,
-0.114922,1.00529,0.0434823,
-0.591383,0.864626,0.552119,
-0.0832463,1.06157,0.492961,
0.551619,-0.961813,0.0808176,
0.300582,-0.897998,0.504718,
-0.157057,0.963438,0.642924,
0.113207,0.676624,0.662450
};

static float Heights[] = {
1697.214,1697.999,1699.228,1700.216,1701.425,1700.358,1691.916,1682.599,1676.975,1672.228,1667.726,1663.103,1658.656,1654.102,1649.482,1644.056,1639.918,1633.398,1625.292,1618.325,1611.587,1603.915,1595.602,1586.974,1581.63,1577.826,1571.999,1565.04,1560.746,1557.06,1552.949,1549.105,1545.472,1542.456,1538.708,1534.911,1529.325,1519.465,1515.616,1509.489,1504.605,1499.341,1494.566,1490.362,1486.096,1479.792,1474.816,1468.235,1462.277,1456.645,1450.037,1441.248,1431.217,1415.138,1409.669,1406.058,1402.742,1399.278,1395.993,1391.363,1386.653,1382.623,1378.279,1373.879,1369.594,1364.904,1359.994,1355.577,1351.684,1346.176,1342.148,1338.319,1334.181,1329.575,1323.907,1310.45,1296.083,1291.034,1283.948,1277.375,1269.456,1265.008,1261.343,1257.472,1253.121,1249.084,1243.781,1238.357,1234.417,1229.154,1224.209,1220.556,1216.554,1212.871,1209.653,1206.462,1203.231,1200.084,1197.006,1194.292,1191.245,1187.814,1184.748,1181.921,1179.372,1176.451,1172.932,1169.787,1166.879,1165.25,1160.926,1157.87,1155.303,1152.326,1149.537,1147.293,1144.656,1141.724,1138.612,1134.78,1130.933,1127.064,1123.604,1120.6,1116.623,1113.011,1111,1106.345,1101.013,1095.674,1089.943,1086.579,1082.579,1077.71,1072.979,1060.485,1049.075,1040.332,1035.487,1029.58,1026.501,1023.755,1020.946,1017.365,1013.795,1006.901,1003.237,996.362,988.4634,983.7319,977.9876,972.3295,966.6514,963.5321,960.6875,957.6714,955.2143,953.6795,950.0196,947.329,943.784,940.5479,937.0619,933.668,930.9836,927.5396,924.3218,920.006,917.778,915.2449,911.8502,909.2435,906.6941,904.27,900.5798,897.2078,893.6836,890.2756,886.33,883.1892,880.4775,878.5046,875.7347,871.2297,867.6321,865.6197,864.765,863.1351,861.9639,858.5679,855.0115,854.1219,854.1966,852.2262,850.8373,850.3337,849.8434,849.5286,847.4011,843.7786,841.9573,840.6249,838.6537,836.86,835.2357,834.6877,834.0687,833.3855,832.8165,833.7115,834.4149,834.5753,834.4157,834.1261,833.8929,833.4783,833.4591,833.0317,832.2527,831.5901,830.5542,829.9818,829.4453,829.4774,829.8809,830.2495,828.1496,824.7491,823.5166,821.7253,
1698.663,1700.048,1700.905,1702.448,1703.564,1702.682,1692.95,1681.39,1675.077,1670.226,1665.845,1661.754,1657.204,1653.302,1649.34,1644.875,1641.356,1636.411,1628.777,1619.615,1609.861,1603.77,1594.644,1584.287,1579.304,1575.296,1570.112,1566.001,1561.401,1556.949,1552.828,1549.125,1545.175,1541.463,1537.901,1533.306,1527.013,1518.155,1511.472,1506.945,1502.893,1498.646,1494.2,1489.979,1485.558,1478.596,1474.01,1468.758,1461.151,1454.347,1449.873,1441.818,1431.772,1418.788,1408.711,1403.736,1399.994,1396.665,1393.578,1389.26,1386.315,1382.839,1378.709,1374.43,1370.501,1365.052,1360.242,1355.681,1350.451,1346.12,1342.497,1338.673,1335.291,1329.339,1321.424,1307.735,1294.039,1286.87,1281.531,1275.055,1267.525,1262.851,1259.092,1255.208,1251.821,1247.878,1242.714,1237.634,1232.637,1228.329,1224.382,1220.225,1216.423,1212.446,1208.878,1205.432,1202.598,1199.414,1195.976,1192.644,1189.591,1186.403,1183.715,1180.84,1178.043,1174.686,1171.179,1167.999,1164.792,1161.951,1159.241,1156.923,1153.974,1151.613,1149.759,1147.75,1145.085,1141.939,1137.505,1133.037,1130.058,1125.888,1122.822,1118.105,1114.499,1110.574,1105.777,1103.355,1099.072,1096.454,1092.96,1088.195,1084.985,1078.809,1073.228,1061.886,1046.229,1038.589,1034.826,1029.455,1026.035,1023.59,1021.655,1017.941,1010.91,1006.919,999.2947,992.586,985.5667,979.8389,975.6658,970.3002,965.4848,963.2317,959.7652,956.189,953.3293,951.1626,948.2234,945.746,941.6813,938.5439,934.8407,931.7479,928.6113,924.9582,922.1152,919.6571,917.3026,915.04,911.3662,909.0092,907.2676,903.5645,899.8365,896.0853,892.0521,888.462,885.6371,883.0974,880.0029,877.8884,876.0164,872.7806,869.5239,867.7697,867.0278,864.8612,865.1864,860.3753,857.6281,854.888,855.1582,854.8516,851.3968,850.0006,849.5089,849.3724,847.2086,844.5703,842.5826,841.2172,839.4995,838.0302,837.2888,836.5125,835.8705,834.9263,835.0502,835.7151,836.0137,835.8961,835.5413,835.0602,835.0012,834.9697,834.4489,834.1774,833.7933,832.9272,831.7393,831.1675,830.5469,830.4721,830.4129,830.6263,828.9,826.0572,825.5287,823.662,
1700.729,1702.044,1702.916,1704.238,1705.399,1705.52,1697.529,1681.114,1672.135,1668.21,1663.564,1659.349,1655.511,1651.567,1648.686,1646.017,1641.531,1633.157,1624.44,1616.299,1608.353,1602.072,1591.745,1581.912,1576.078,1571.834,1568.996,1565.29,1561.192,1556.889,1553.261,1549.039,1544.569,1540.223,1536.715,1531.41,1522.209,1515.321,1511.022,1505.188,1501.395,1497.742,1493.304,1489.627,1485.768,1479.577,1471.135,1466.992,1459.552,1453.328,1447.729,1440.297,1432.709,1425.241,1408.171,1403.384,1397.672,1393.871,1390.295,1387.124,1384.764,1381.205,1378.168,1374.712,1370.075,1364.583,1360.638,1356.139,1350.931,1346.601,1343.206,1338.891,1334.077,1327.118,1314.463,1303.092,1292.738,1286.526,1280.431,1271.716,1265.527,1261.262,1257.276,1253.537,1249.399,1244.672,1239.723,1235.635,1231.083,1228.212,1223.81,1219.883,1215.911,1211.72,1207.811,1204.171,1200.966,1197.763,1194.32,1190.892,1187.484,1183.876,1181.062,1177.779,1174.836,1172.551,1169.385,1165.87,1162.343,1160.25,1156.779,1154.505,1152.659,1150.539,1148.841,1146.898,1143.826,1141.124,1136.751,1132.578,1128.937,1125.534,1121.694,1118.075,1114.631,1109.167,1106.027,1103.384,1100.479,1097.835,1094.248,1089.546,1084.29,1081.019,1073.663,1064.07,1045.613,1036.776,1033.486,1028.596,1026.416,1023.306,1020.547,1016.928,1009.775,1000.904,994.6718,990.2037,984.2339,978.6862,975.1221,970.1656,964.9264,961.2342,958.6412,955.4596,951.9391,949.3784,946.1145,943.8361,940.8156,937.2504,933.6134,930.427,927.5334,923.4643,920.6026,917.9743,915.5413,913.2529,910.817,907.6856,905.5777,902.4598,899.2125,895.9783,891.4944,887.5853,884.7103,882.1395,879.3965,877.5682,875.844,874.4375,870.8071,869.3524,867.9436,867.0179,865.0625,863.1159,859.7758,855.4647,855.6887,856.0581,853.0623,850.7216,849.4556,848.7793,847.4589,845.2411,843.5602,842.5319,840.4572,839.5648,838.7991,838.2669,837.5707,836.5591,836.6534,837.0265,837.1202,837.1907,836.5906,836.7826,836.7884,836.6016,836.1646,835.5532,834.84,834.1591,833.4516,832.4026,832.1,831.5866,831.139,831.1849,828.687,826.9017,827.0687,825.4962,
1702.818,1703.237,1704.742,1706.239,1707.034,1707.172,1701.988,1684.553,1670.816,1665.91,1661.153,1656.922,1653.191,1649.052,1646.532,1643.196,1638.021,1631.311,1619.735,1613.362,1607.492,1601.838,1591.555,1580.812,1573.721,1569.331,1566.022,1562.803,1559.499,1555.665,1551.753,1547.932,1543.771,1539.793,1533.86,1528.362,1521.332,1514.603,1508.552,1503.948,1499.592,1495.841,1491.974,1487.882,1482.003,1476.632,1470.328,1466.104,1460.434,1455.205,1447.315,1441.175,1433.314,1424.675,1409.935,1403.393,1396.38,1390.774,1387.694,1383.984,1380.642,1377.383,1374.528,1370.228,1366.768,1363.218,1359.96,1355.42,1350.592,1346.255,1343.027,1338.18,1328.558,1316.545,1307.318,1299.037,1291.641,1286.285,1279.72,1270.204,1263.742,1259.737,1255.703,1251.895,1247.472,1242.974,1238.087,1234.284,1230.611,1227.183,1223.176,1219.381,1215.708,1211.077,1206.434,1202.565,1198.859,1196.013,1193.051,1189.289,1185.139,1180.857,1178.302,1175.65,1172.851,1169.938,1166.4,1163,1160.014,1157.414,1154.679,1152.22,1149.941,1148.326,1146.441,1143.953,1141.096,1137.512,1134.811,1131.093,1127.419,1124.262,1120.931,1117.633,1114.591,1110.536,1107.689,1104.911,1101.779,1098.719,1094.096,1090.011,1086.01,1081.065,1073.376,1055.255,1046.091,1035.476,1032.053,1029.005,1025.865,1022.876,1019.985,1016.441,1009.848,999.346,996.1002,990.3718,985.2584,979.5826,974.958,971.0042,964.508,960.1347,956.799,953.2863,950.3834,947.5941,944.7368,941.4822,939.8585,936.485,933.1985,930.3262,927.3207,923.8447,920.6777,918.2622,915.6704,913.0787,911.1725,908.4824,905.2868,901.7148,898.2421,896.086,892.0997,888.3328,884.8951,882.2031,880.12,878.4517,877.1903,875.3391,872.3243,871.0079,869.9485,868.504,866.0915,864.5179,862.6036,857.3331,856.3201,856.4341,854.7015,852.1911,850.1482,848.7499,847.9793,846.5734,845.6808,843.1464,842.1977,841.3517,840.7238,840.3219,839.9033,839.3817,839.1135,838.9988,839.0198,839.1823,838.9176,838.3214,837.7224,837.172,836.769,836.718,835.5288,834.7845,834.0521,833.3748,832.9163,832.2411,831.728,831.0239,827.7434,828.5578,827.6043,827.3409,
1704.492,1705.059,1706.491,1707.395,1708.777,1708.76,1699.296,1685.573,1670.769,1664.836,1660.183,1655.16,1650.096,1646.693,1642.186,1638.325,1630.802,1625.859,1617.242,1610.824,1606.666,1601.295,1590.407,1579.088,1572.721,1567.446,1563.961,1560.662,1557.077,1553.632,1550.159,1546.912,1541.832,1539.255,1531.516,1525.376,1519.896,1514.528,1510.436,1504.549,1497.956,1493.972,1489.744,1485.256,1480.189,1475.849,1469.983,1466.013,1460.364,1456.807,1448.731,1440.865,1434.586,1423.275,1411.999,1405.813,1397.004,1389.08,1384.614,1380.354,1376.737,1373.3,1368.793,1364.725,1363.162,1360.014,1358.131,1353.893,1349.465,1345.118,1341.427,1335.398,1326.143,1316.768,1304.841,1297.861,1291.695,1285.929,1279.432,1270.217,1262.895,1258.495,1253.018,1248.571,1243.698,1239.757,1236.194,1232.322,1228.699,1224.95,1221.651,1218.327,1214.715,1210.174,1204.636,1200.887,1197.53,1194.567,1190.734,1186.671,1183.61,1179.324,1176.185,1172.895,1169.861,1166.85,1163.804,1161.028,1158.047,1155.25,1152.12,1149.753,1146.622,1144.936,1142.406,1140.378,1136.588,1134.506,1132.575,1129.348,1126.19,1122.966,1119.803,1115.806,1112.866,1110.538,1107.224,1104.596,1102.057,1099.143,1094.239,1089.78,1085.567,1078.236,1071.871,1058.211,1046.355,1034.704,1031.867,1028.222,1024.834,1022.579,1017.511,1014.726,1007.693,999.936,996.5515,992.6222,984.2847,979.5228,975.8376,971.1731,964.0403,960.3848,956.6947,952.8842,951.1583,948.1393,944.6411,940.3978,937.2836,934.7435,932.1033,929.2988,926.9965,924.5665,921.7491,919.1811,916.095,913.5862,911.4287,908.3776,904.6735,900.6303,898.7712,896.0386,892.9446,889.1841,886.0876,883.4603,881.6609,880.2209,878.0187,876.1849,874.1978,872.7076,871.9482,870.1788,868.136,866.4064,861.1224,858.7755,857.5736,857.0781,855.4755,852.6114,850.9775,849.5347,848.0621,847.1127,846.6577,844.9019,844.0441,843.3928,842.7902,842.6788,842.2459,841.7611,841.3337,840.7119,840.264,840.113,840.3862,840.0668,838.9828,838.3837,838.4005,837.5986,837.1327,836.4391,835.272,834.5034,833.6783,832.8065,831.9688,829.9849,828.0532,829.1415,828.9189,827.6957,
1705.881,1706.634,1708.004,1709.296,1709.797,1707.651,1697.825,1683.03,1670.593,1663.759,1657.675,1652.39,1645.5,1642.597,1639.694,1634.977,1626.533,1621.444,1613.897,1607.439,1604.584,1597.752,1586.316,1577.68,1573.054,1560.915,1561.936,1558.574,1554.903,1551.539,1548.455,1544.318,1540.185,1536.747,1529.933,1523.001,1518.512,1515.015,1511.313,1505.431,1496.067,1492.397,1488.605,1483.872,1479.646,1475.614,1470.015,1464.866,1458.382,1452.033,1447.967,1440.525,1434.198,1422.102,1412.535,1404.666,1396.168,1387.05,1382.147,1377.809,1374.074,1369.315,1364.988,1361.689,1357.239,1354.814,1353.783,1351.237,1347.495,1343.92,1340.401,1331.847,1324.567,1317.1,1306.328,1298.872,1292.793,1286.532,1280.961,1270.126,1262.379,1257.301,1251.655,1247.047,1243.036,1238.289,1234.5,1231.102,1226.794,1222.595,1219.272,1215.738,1212.771,1208.08,1203.601,1200.469,1196.198,1192.563,1189.128,1185.259,1181.853,1178.359,1174.755,1171.3,1168.207,1164.799,1161.799,1159.4,1156.887,1152.581,1149.558,1147.086,1143.914,1140.947,1138.21,1136.668,1133.564,1132.526,1130.631,1128.168,1125.533,1121.52,1117.729,1113.487,1110.764,1108.417,1105.554,1102.286,1100.623,1098.24,1094.796,1089.366,1080.988,1073.173,1065.881,1057.036,1042.198,1034.985,1031.689,1028.349,1024.983,1021.325,1015.958,1011.95,1005.838,1000.927,996.0457,992.8167,984.159,979.3597,974.4044,970.2283,964.2528,959.6502,956.3981,952.6961,949.9382,947.3336,943.694,939.8407,936.1071,933.3394,930.7009,928.4185,925.7922,923.6509,920.8588,918.4908,915.7281,912.2648,909.8505,907.0436,903.7528,900.1182,898.0175,896.4974,894.0374,890.7876,888.4097,886.1564,884.1443,882.331,879.9611,877.545,875.4644,873.49,871.722,871.095,868.1136,865.7811,864.8492,861.2411,859.3351,857.6819,855.7023,853.8083,852.2113,850.8283,850.3124,849.1046,846.594,845.8215,845.1804,844.6642,844.121,843.6277,843.3353,842.77,842.8083,843.0479,842.1807,842.2242,842.1685,841.642,840.4867,840.2394,839.061,839.0082,837.8328,836.8729,836.1547,835.2237,834.3672,832.7023,831.6232,829.2966,828.6572,829.7922,829.0466,828.0143,
1707.372,1708.19,1709.303,1708.491,1708.049,1702.736,1693.156,1678.455,1667.446,1661.125,1655.393,1648.827,1641.828,1639.442,1636.282,1631.466,1619.836,1614.683,1609.491,1605.007,1599.812,1590.568,1581.229,1576.712,1572.381,1566.96,1558.967,1556.176,1549.581,1547.643,1545.297,1541.855,1538.205,1534.824,1526.628,1521.617,1517.153,1513.848,1511.107,1504.029,1494.617,1489.691,1486.467,1482.106,1477.863,1473.717,1469.762,1463.381,1456.472,1449.112,1443.822,1440.124,1432.403,1420.708,1411.712,1403.523,1392.259,1385.199,1380.411,1376.091,1371.745,1366.673,1362.644,1358.652,1354.265,1351.959,1349.266,1347.406,1345.132,1341.093,1336.334,1329.606,1323.809,1316.227,1308.997,1300.11,1294.451,1288.058,1279.506,1269.407,1262.294,1257.62,1253.619,1249.134,1244.125,1237.849,1233.413,1229.698,1225.147,1220.942,1217.292,1213.344,1209.892,1206.193,1202.715,1198.367,1194.934,1191.381,1188.111,1184.133,1180.837,1177.221,1173.708,1169.547,1166.33,1162.839,1160.007,1156.963,1153.846,1151.968,1147.698,1145.1,1141.363,1138.318,1135.307,1132.742,1131.136,1129.504,1128.151,1126.612,1124.72,1120.405,1116.011,1111.393,1108.705,1106.326,1103.339,1100.048,1098.377,1095.894,1093.783,1089.415,1077.748,1072.807,1066.099,1056.684,1043.276,1035.887,1032.272,1029.398,1025.728,1020.578,1015.852,1011.647,1006.505,1001.352,996.944,991.5397,983.9451,980.3975,975.4952,971.0966,963.6136,958.8765,954.9974,951.6362,949.2249,946.7084,943.3397,939.5674,936.0674,932.5931,929.6088,927.4443,925.5009,922.1097,919.9889,917.2441,914.5461,911.5297,908.771,906.2906,903.6249,900.5307,898.6694,896.585,895.0176,892.3401,889.5892,887.5994,885.2352,882.4865,880.2377,878.626,876.275,873.7551,873.7455,871.2296,869.5629,867.0143,864.5784,862.164,860.1195,858.3066,857.4708,855.437,854.0959,852.6556,851.0648,849.2717,847.4322,846.3035,846.2719,846.1077,845.8502,846.2709,847.2431,846.0604,845.9267,845.4589,844.8299,844.2265,843.7779,843.61,845.0679,843.0522,840.5884,840.0626,838.9678,837.462,836.3331,835.5446,834.7068,833.3149,831.6587,830.4796,830.858,830.5195,829.3641,829.3354,
1709.326,1710.218,1706.122,1701.489,1698.456,1693.985,1684.516,1673.299,1665.397,1658.353,1651.306,1645.296,1638.201,1634.81,1628.726,1620.384,1613.219,1608.351,1604.537,1599.964,1592.036,1585.375,1580.214,1576.43,1570.067,1566.308,1557.721,1552.472,1548.131,1544.952,1542.921,1539.427,1535.406,1530.341,1524.44,1519.96,1515.582,1512.206,1508.756,1505.115,1494.068,1487.711,1484.226,1479.794,1476.033,1472.27,1469.331,1464.97,1455.821,1447.389,1441.646,1437.372,1429.413,1422.256,1413.705,1402.888,1395.304,1383.652,1379.188,1374.809,1370.386,1365.817,1361.414,1357.482,1353.086,1349.99,1346.006,1343.349,1341.535,1339.277,1335.358,1329.149,1322.558,1316.36,1309.25,1302.085,1295.066,1290.191,1281.976,1269.64,1263.156,1258.53,1254.404,1249.678,1243.373,1237.168,1233.746,1229.901,1226.155,1221.393,1217.84,1213.251,1209.067,1204.43,1200.202,1196.309,1192.44,1189.339,1185.861,1182.513,1178.864,1175.341,1171.255,1167.729,1164.38,1160.826,1157.603,1154.32,1151.508,1148.338,1146.28,1141.924,1138.99,1136.461,1133.066,1130.713,1129.098,1127.603,1126.162,1124.493,1122.014,1117.401,1113.193,1108.698,1106.458,1104.906,1101.228,1097.349,1095.197,1094.257,1091.349,1082.098,1078.104,1071.445,1063.064,1051.043,1042.108,1037.262,1033.107,1029.769,1026.155,1021.544,1017.054,1010.391,1007.026,1001.339,997.1725,991.7973,985.3044,980.483,975.2859,970.8992,963.2108,958.2375,954.3777,951.326,948.747,945.7983,942.4454,939.3562,935.8123,932.3568,929.2397,926.5318,923.9089,922.1926,918.8481,916.4461,913.8585,910.9517,908.5155,905.7887,902.5923,900.6552,897.9622,895.7006,893.2811,891.4611,889.0526,886.9843,884.9069,882.6217,880.3141,878.2473,877.1252,875.5892,873.3206,872.2252,869.9058,867.5342,865.1767,863.4868,861.5513,859.353,858.4655,856.6805,855.0429,853.3372,851.8049,850.3604,849.1005,848.7975,848.6733,848.4958,848.4259,848.8506,848.271,848.2092,847.645,847.1671,845.923,845.1431,844.7417,844.1916,844.1572,842.1672,842.0547,841.2582,840.7297,838.8677,836.8903,835.8339,835.385,834.3381,833.0532,831.8662,830.9561,830.4187,830.0222,830.7814,
1710.962,1711.668,1708.775,1694.828,1687.715,1684.17,1675.261,1670.202,1664.87,1657.471,1650.89,1644.422,1637.813,1631.915,1623.453,1616.244,1608.288,1603.852,1600.422,1594.387,1589.242,1584.465,1577.786,1574.469,1566.364,1563.043,1554.352,1549.077,1544.742,1542.285,1539.81,1536.252,1532.144,1528.013,1524.674,1519.715,1514.486,1510.447,1506.963,1501.514,1493.74,1487.096,1482.023,1478.05,1474.932,1472.026,1468.767,1463.596,1456.128,1449.904,1442.047,1436.628,1429.594,1423.895,1414.77,1402.585,1392.478,1382.402,1378.412,1374.235,1369.89,1365.182,1360.917,1356.459,1352.512,1348.38,1343.955,1338.498,1336.995,1335.767,1332.408,1328.205,1321.23,1315.255,1309.471,1304.88,1298.173,1290.448,1280.297,1267.694,1263.176,1257.674,1254.007,1243.329,1239.643,1235.164,1231.119,1227.759,1224.811,1222.246,1217.931,1214.156,1210.212,1204.987,1199.251,1194.872,1190.718,1187.554,1184.19,1180.839,1177.059,1173.698,1171.072,1166.657,1163.007,1160.656,1157.493,1153.401,1149.774,1146.256,1143.811,1141.764,1138.507,1134.479,1131.77,1129.337,1127.203,1125.168,1123.911,1122.022,1120.002,1115.6,1111.111,1108.008,1104.747,1102.506,1098.726,1095.392,1092.904,1091.121,1086.478,1081.782,1075.934,1068.483,1059.678,1049.525,1042.329,1039.132,1034.386,1029.628,1026.432,1020.461,1011.859,1007.424,1005.001,999.7838,995.0891,990.2698,984.0841,981.6744,977.1369,971.0715,963.9258,957.7241,954.153,951.6072,947.8472,944.7229,941.6148,939.1293,936.0743,932.4744,929.8077,927.098,924.6444,921.816,919.213,916.2222,913.7012,910.9724,908.4496,905.5432,903.1647,900.508,897.9684,895.8326,893.7923,891.2333,888.2448,886.6555,884.5938,882.7963,881.376,878.8514,876.9431,875.6677,873.9525,872.2708,870.0201,868.4002,866.3932,863.5842,861.6494,859.8719,859.2276,857.1306,855.6988,854.3972,852.7122,851.5092,852.3796,853.1301,851.2288,850.6482,850.8591,851.2629,850.4498,850.149,849.5908,848.4811,847.0038,846.8781,845.7252,845.3256,844.3608,843.1072,842.9196,842.0052,842.2828,840.6865,838.7354,837.7537,836.0732,835.2926,834.7417,833.4971,831.894,832.3925,832.5005,832.8243,
1713.236,1713.708,1710.252,1694.162,1688.107,1680.056,1672.872,1667.151,1662.416,1657.561,1650.567,1643.062,1637.048,1631.353,1625.02,1620.948,1611.875,1602.032,1597.783,1590.922,1585.482,1581.279,1576.984,1572.713,1565.355,1561.045,1558.179,1550.459,1543.932,1541.465,1538.577,1534.086,1530.3,1526.431,1522.102,1517.408,1513.335,1509.341,1505.384,1498.225,1490.649,1486.049,1480.578,1475.898,1472.235,1469.719,1466.122,1461.427,1453.901,1447.126,1440.78,1435.112,1426.627,1422.551,1412.704,1401.726,1388.309,1381.433,1377.652,1373.41,1369.074,1364.617,1360.537,1356.455,1351.893,1347.273,1341.695,1333.259,1332.613,1331.594,1328.576,1325.698,1319.172,1313.607,1309.403,1303.862,1297.555,1284.474,1272.823,1267.489,1261.246,1254.975,1249.054,1239.97,1235.994,1232.463,1228.719,1225.355,1222.043,1220.649,1216.109,1212.021,1207.767,1203.459,1198.701,1194.268,1189.842,1186.536,1183.038,1179.347,1175.771,1171.955,1168.372,1164.759,1162.44,1158.318,1155.057,1152.521,1148.473,1144.135,1140.396,1137.306,1136.477,1133.318,1130.606,1128.231,1126.042,1124.044,1121.672,1119.599,1116.055,1113.637,1109.521,1106.407,1103.283,1100.388,1096.853,1093.671,1090.495,1087.71,1083.868,1079.835,1076.145,1071.387,1059.927,1050.935,1043.665,1039.476,1034.018,1029.238,1024.662,1018.544,1010.122,1005.494,1001.409,999.2161,993.8631,990.2129,987.2046,982.7457,978.7429,972.657,964.3311,957.2223,953.9951,950.6258,947.2216,943.712,941.4784,938.0964,935.6644,933.3483,930.7795,928.0227,925.6052,923.0452,920.7021,917.7826,915.1657,911.5665,909.0012,906.577,904.0931,901.2386,898.9612,896.8536,894.0775,891.5818,888.9232,886.8985,885.0821,883.658,881.7401,879.8832,878.3373,875.9559,873.9932,872.043,870.4483,868.4825,866.1045,863.8167,862.6267,860.9937,860.0988,857.561,856.081,854.683,853.8209,854.4487,854.8172,854.0435,853.2529,852.6122,853.4614,852.9303,852.6885,851.8506,851.3341,850.1441,849.3597,848.7226,847.1533,846.3962,844.8773,844.2815,843.5491,842.9122,842.8736,842.3523,840.7852,839.5223,837.9662,836.609,836.1371,833.3347,832.5531,833.3376,834.2181,834.3419,
1715.221,1715.937,1710.301,1698.821,1689.927,1681.638,1672.957,1666.946,1662.1,1656.019,1650.975,1643.333,1634.951,1629.005,1623.384,1618.584,1614.593,1604.349,1598.397,1592.627,1588.043,1581.296,1577.132,1573.879,1568.313,1562.282,1557.447,1551.554,1543.04,1538.355,1536.606,1532.754,1528.959,1524.939,1519.594,1516.012,1512.47,1509.012,1504.553,1499.778,1492.984,1486.677,1480.527,1474.523,1470.212,1466.025,1461.959,1455.991,1452.717,1447.531,1440.392,1434.482,1426.248,1419.051,1407.3,1399.539,1390.168,1380.049,1376.493,1372.859,1368.331,1364.256,1360.31,1356.045,1350.864,1346.917,1338.059,1332.298,1327.818,1328.067,1325.252,1322.005,1316.403,1312.603,1307.894,1302.904,1295.152,1281.196,1271.579,1265.843,1258.388,1252.794,1248.557,1240.335,1234.135,1231.182,1227.461,1222.862,1218.828,1216.796,1212.689,1208.353,1203.905,1199.62,1196.52,1191.886,1187.935,1184.455,1181.22,1177.865,1174.317,1170.576,1166.99,1163.194,1159.611,1156.245,1153.014,1149.912,1146.864,1144.213,1139.611,1134.285,1132.346,1130.665,1128.618,1127.005,1124.775,1122.869,1119.911,1119.323,1114.36,1110.185,1106.386,1102.838,1099.936,1097.695,1095.874,1091.334,1088.867,1085.422,1081.435,1079.326,1077.319,1073.369,1054.136,1046.425,1043.975,1038.441,1033.742,1028.27,1022.161,1014.187,1007.275,1003.417,999.0457,996.1663,992.8855,989.586,986.0479,982.93,979.678,975.4089,964.1581,958.3648,954.3982,949.7338,946.9982,942.6996,940.1967,937.5532,935.77,933.3165,930.9011,928.9985,926.9866,924.4504,921.7469,919.0682,915.7836,913.2047,910.4186,907.5717,904.0178,901.2681,899.1935,896.1552,893.6678,891.4561,888.8727,887.7849,885.8411,883.7114,881.6642,879.6253,877.814,875.9406,874.2844,872.3749,870.3682,868.3812,866.8887,865.4268,863.7474,861.7241,860.9237,858.527,856.8124,855.9741,857.1424,857.4523,856.8651,855.8992,855.0765,855.0343,855.0079,854.0073,854.561,853.1651,853.1357,851.5012,850.5618,849.5576,848.4575,847.3525,846.2297,845.4465,844.2982,843.6296,843.2785,842.7259,842.1404,841.5662,839.7716,838.1684,837.3412,836.2086,835.243,836.424,837.1191,834.6408,
1717.067,1718.222,1717.014,1703.409,1696.349,1684.165,1673.671,1667.523,1662.618,1656.824,1652.871,1645.644,1634.115,1626.664,1620.064,1614.369,1609.241,1602.468,1596.889,1593.135,1589.258,1583.206,1578.065,1573.908,1564.629,1561.038,1551.427,1544.13,1538.312,1536.795,1534.112,1529.87,1525.681,1522.328,1518.18,1514.557,1510.022,1506.636,1503.459,1495.353,1490.485,1482.965,1478.058,1473.812,1469.438,1463.66,1457.576,1452.021,1448.781,1443.987,1440.218,1434.721,1425.438,1419.531,1408.871,1399.28,1389.864,1376.403,1373.314,1371.178,1367.661,1363.257,1358.645,1355.14,1350.596,1344.672,1335.199,1329.708,1323.063,1322.872,1320.184,1317.784,1313.386,1310.04,1305.742,1299.538,1290.618,1278.671,1268.913,1260.431,1255.697,1250.774,1245.472,1239.485,1235.498,1231.862,1227.951,1222.591,1217.57,1212.805,1209.548,1204.045,1199.875,1195.056,1191.822,1188.325,1185.09,1181.503,1179.145,1176.791,1171.779,1169.004,1165.299,1161.613,1158.115,1154.908,1151.937,1148.139,1144.609,1141.061,1136.755,1133.273,1130.467,1128.1,1125.815,1123.751,1122.811,1122.326,1120.194,1116.008,1112.76,1108.43,1101.091,1097.522,1095.463,1096.214,1093.704,1089.168,1085.952,1083.053,1079.386,1076.915,1074.662,1064.769,1049.524,1042.387,1038.247,1035.431,1029.793,1025.475,1017.772,1012.567,1007.556,1003.045,998.6772,995.7145,993.3463,991.1014,986.0078,983.18,979.0264,975.3918,966.2484,959.5155,953.6965,949.1151,945.339,941.7439,939.8619,937.3919,935.1896,932.9848,930.6933,928.5067,926.1801,922.4971,920.7543,917.6385,914.3253,911.3745,909.1653,908.3564,904.2442,900.3337,898.4181,896.2745,895.4319,892.754,891.1454,889.2295,886.8776,884.0121,881.929,879.9707,878.138,876.2783,874.7302,872.7101,870.5895,868.7319,867.0782,865.0947,863.3414,861.6826,861.4434,860.7825,858.9737,858.485,859.3716,859.264,858.3328,857.5848,856.8052,857.1331,856.2046,856.1024,855.2657,854.2838,854.0204,852.773,851.2888,850.9901,849.374,848.0148,847.0352,846.4294,845.1468,844.8907,843.9773,844.0012,843.7475,843.4082,842.3977,840.5234,839.072,838.2493,837.2662,837.9866,837.8367,836.0137,
1718.747,1719.875,1719.889,1710.77,1701.462,1683.759,1672.309,1667.756,1664.465,1659.871,1652.742,1642.152,1631.609,1623.457,1617.074,1610.614,1606.289,1600.962,1596.666,1590.757,1586.448,1581.443,1576.799,1571.557,1565.85,1560.138,1548.424,1540.001,1534.856,1533.329,1531.18,1527.583,1523.591,1519.467,1516.192,1511.201,1508.008,1504.57,1500.61,1491.901,1487.607,1482.958,1477.068,1473.42,1467.306,1460.935,1455.968,1451.342,1445.159,1442.544,1438.008,1431.59,1425.265,1419.821,1410.964,1399.648,1390.46,1374.238,1368.685,1367.673,1366.176,1360.818,1356.521,1352.769,1347.053,1343.018,1333.743,1327.503,1320.168,1317.301,1317.744,1314.337,1310.447,1306.879,1302.528,1296.431,1288.736,1278.722,1267.732,1258.411,1254.447,1249.381,1245.6,1240.411,1235.636,1230.97,1226.71,1221.656,1216.91,1211.579,1206.447,1201.433,1197.764,1192.48,1188.384,1185.252,1181.545,1178.36,1175.332,1172.55,1168.71,1164.985,1161.489,1158.309,1155.568,1152.766,1150.556,1146.569,1142.657,1139.346,1135.839,1132.314,1129.257,1126.191,1123.889,1121.486,1120.127,1119.664,1118.656,1115.02,1111.946,1106.931,1100.506,1096.01,1089.932,1088.721,1090.486,1084.557,1081.297,1079.338,1075.087,1071.456,1065.958,1054.396,1044.91,1039.698,1035.022,1032.075,1027.912,1022.659,1017.51,1011.566,1007.258,1003.285,999.5388,995.6436,992.5501,989.3766,986.0812,981.6616,977.8942,973.6943,968.1042,961.6149,953.0888,947.5687,944.4552,942.0527,939.5475,936.8312,934.0096,930.5157,927.3211,925.5861,923.4574,921.0233,919.28,917.7725,915.3218,913.1664,910.978,906.8983,902.8978,902.286,900.1638,898.1464,895.2408,893.2035,890.8115,889.5025,886.7881,884.8522,882.8862,880.7502,878.9417,877.0133,874.6867,872.9156,871.3243,869.4313,867.2864,865.2087,864.1895,863.156,861.8608,862.204,861.4795,861.4232,861.9767,860.9676,859.6387,859.0137,858.9772,858.2856,857.3744,857.0447,856.4128,855.9722,855.6476,854.3474,853.0248,852.1572,851.0297,849.6055,848.3552,847.2258,846.8885,846.4922,846.3496,846.142,846.1846,845.7295,844.0414,841.9918,839.8594,838.7266,838.1825,838.276,838.0414,837.5536,
1720.42,1721.689,1721.488,1716.66,1699.484,1677.843,1667.211,1664.356,1659.965,1657.727,1650.197,1638.985,1628.44,1618.998,1613.5,1608.852,1603.85,1600.108,1595.618,1589.873,1585.689,1581.469,1578.17,1572.945,1563.473,1556.591,1550.242,1542.272,1535.109,1531.62,1528.842,1524.009,1521.158,1517.27,1513.8,1509.898,1505.621,1502.493,1494.942,1489.582,1485.273,1481.938,1476.656,1471.858,1465.223,1459.233,1454.798,1449.659,1445.007,1438.548,1434.673,1429.926,1424.33,1418.507,1412.537,1400.94,1390.75,1372.418,1365.523,1362.357,1360.969,1358.514,1353.475,1350.386,1344.628,1337.973,1331.589,1326.107,1317.995,1311.85,1312.317,1310.765,1308.106,1303.552,1298.126,1294.373,1284.41,1274.74,1262.184,1257.913,1253.973,1249.119,1244.732,1239.909,1234.637,1229.663,1226.129,1221.611,1215.537,1211.307,1207.707,1201.876,1197.565,1192.495,1188.308,1184.841,1180.802,1176.651,1173.086,1170.962,1167.338,1163.065,1159.258,1155.701,1153.642,1150.427,1147.455,1144.151,1140.921,1136.644,1133.239,1129.683,1127.031,1123.861,1120.916,1118.072,1115.893,1115.509,1115.781,1112.758,1109.449,1105.296,1099.608,1094.955,1086.146,1080.689,1077.35,1074.627,1074.182,1071.832,1070.096,1066.558,1060.826,1052.188,1043.123,1037.36,1033.265,1027.775,1023.648,1020.335,1015.862,1010.88,1006.252,1001.999,998.5902,994.7791,992.682,988.2641,985.5287,981.1275,976.3819,970.2416,964.4653,961.606,952.2576,947.1417,943.7849,941.0491,937.9724,935.436,932.6576,931.4938,930.1504,927.8934,925.4293,923.3631,921.1899,919.1821,916.803,915.6822,913.8082,908.9714,906.3224,905.1075,901.5053,897.9521,895.3401,893.4623,891.1211,889.3467,887.463,884.8928,883.6108,881.5607,879.6232,877.3262,875.3049,873.7278,871.9425,869.9275,868.178,867.0019,866.7607,864.7349,863.1063,862.8914,863.246,863.6268,863.2748,862.0347,860.8826,860.1727,859.7196,858.8775,858.2684,858.2336,857.7269,857.2706,856.7256,855.7532,854.0704,853.7128,852.6379,852.2296,849.7382,849.4203,848.7345,848.4944,848.427,848.4957,847.8723,847.1639,845.3998,843.7995,842.0805,840.7156,839.6024,840.8254,841.0378,839.2003,
1721.718,1723.037,1723.819,1716.319,1699.53,1678.764,1664.356,1659.148,1655.19,1652.029,1647.148,1638.675,1625.827,1616.349,1610.944,1606.725,1602.574,1598.667,1594.22,1589.78,1584.966,1581.208,1577.514,1573.259,1563.736,1558.245,1549.955,1543.721,1534.435,1529.018,1526.051,1522.009,1518.772,1514.672,1511.203,1506.578,1503.697,1498.53,1492.439,1487.388,1483.944,1479.868,1475.848,1471.229,1464.673,1456.94,1452.085,1447.463,1444.031,1438.094,1432.64,1428.34,1424.562,1417.452,1410.334,1399.118,1383.468,1370.324,1363.177,1358.474,1355.755,1353.075,1349.752,1346.19,1342.089,1335.657,1328.638,1322.166,1315.173,1308.368,1306.057,1306.761,1305.105,1298.333,1294.063,1286.184,1278.179,1269.728,1261.994,1258.124,1253.291,1248.065,1243.944,1240.376,1237.051,1231.961,1228,1222.689,1215.812,1211.122,1207.111,1201.264,1196.255,1191.646,1186.786,1184.503,1180.319,1177,1172.116,1169.589,1166.889,1162.72,1160.115,1154.914,1152.487,1148.813,1145.536,1142.573,1139.11,1137.154,1134.67,1131.754,1128.376,1125.05,1121.019,1118.141,1113.695,1112.431,1111.028,1109.457,1108.502,1104.267,1099.88,1093.645,1086.775,1080.03,1075.49,1070.73,1067.837,1067.433,1066.226,1064.469,1059.429,1051.42,1043.214,1035.786,1029.874,1024.336,1019.264,1014.597,1010.951,1008.652,1004.406,1000.222,996.5646,993.124,990.4482,987.0435,983.5536,978.3357,975.5015,967.0043,962.0121,959.1649,950.8259,947.4475,943.0931,940.3961,939.9808,938.8553,936.5284,933.8154,931.6613,929.3889,927.1357,924.7503,922.6169,920.5103,919.1561,916.9321,914.7733,911.6758,909.2924,906.6313,903.2542,900.2672,897.9042,894.9142,892.4376,890.4036,888.5087,885.7142,884.3695,881.9893,879.8812,877.2381,875.9673,874.8766,872.9333,871.9323,869.8796,868.7999,867.3019,865.9471,865.1912,864.6544,864.0106,864.155,864.246,863.26,862.2127,861.5457,860.5702,860.4638,860.2206,860.2261,858.8311,858.2744,857.951,856.7593,855.0983,855.0959,854.5375,853.0452,851.3355,849.95,849.8223,849.6027,850.0289,849.5217,849.0329,848.131,846.782,845.098,844.2061,841.9177,840.4345,842.1181,842.9846,842.6285,
1723.5,1724.63,1725.631,1724.535,1702.142,1677.586,1663.742,1654.318,1651.049,1647.316,1642.767,1638.69,1625.046,1614.568,1608.809,1604.936,1600.538,1597.092,1592.888,1588.161,1583.07,1578.915,1574.947,1571.31,1562.397,1558.134,1549.969,1541.417,1535.693,1526.263,1524.021,1520.113,1516.235,1511.961,1506.021,1503.493,1499.632,1496.092,1491.537,1486.118,1481.444,1477.085,1472.32,1468.285,1461.095,1453.73,1448.518,1444.291,1440.763,1436.267,1432.626,1427.721,1421.969,1416.668,1408.914,1397.974,1384.125,1370.088,1361.42,1355.072,1351.13,1348.161,1345.423,1342.391,1337.32,1330.409,1322.609,1316.75,1311.212,1306.359,1301.572,1302.252,1295.924,1287.633,1284.985,1277.899,1273.645,1268.212,1262.8,1258.669,1254.373,1249.214,1246.521,1243.625,1237.889,1231.726,1226.911,1221.687,1214.647,1210.433,1206.214,1201.272,1196.11,1192.07,1189.444,1184.982,1182.512,1178.61,1174.721,1171.671,1168.322,1164.923,1161.029,1158.029,1154.489,1150.333,1147.948,1145.439,1142.331,1138.947,1135.666,1132.265,1128.754,1124.966,1121.847,1118.214,1114.434,1110.354,1107.431,1105.514,1104.231,1102.066,1099.769,1093.564,1088.876,1082.831,1078.258,1072.168,1064.925,1060.404,1061.299,1061.06,1057.989,1052.092,1041.689,1034.032,1027.259,1021.139,1015.829,1012.565,1009.414,1005.324,1002.358,998.6334,995.0568,991.2205,988.5394,984.9008,980.9854,977.4187,971.8539,964.8974,960.2855,956.9803,950.8025,946.9394,944.6801,944.5266,942.785,941.1898,938.4783,935.2657,932.2287,929.5704,927.1115,924.9008,923.0927,920.7835,918.3909,917.1186,914.8475,912.8463,910.5553,907.2567,903.9488,901.1805,899.5577,896.9136,894.3792,892.2548,889.4675,886.9815,884.4016,882.9319,880.5983,878.2762,876.5645,875.9759,874.1351,872.6309,870.8243,869.9484,868.203,867.2146,866.3401,866.0038,865.7269,864.6439,864.9432,864.2704,863.4485,862.6243,861.8719,861.6914,861.0834,860.8559,859.7807,859.361,858.9747,857.3582,856.3934,856.1717,855.8811,854.7134,853.324,851.7286,851.0679,851.2338,851.1667,850.3028,849.6024,848.2176,847.1209,845.2653,843.363,841.8655,842.0948,841.7173,842.569,843.1343,
1725.403,1726.438,1726.616,1724.043,1698.119,1675.68,1662.731,1651.299,1646.238,1642.546,1639.199,1635.417,1628.167,1617.03,1608.503,1603.051,1599.494,1595.561,1591.528,1586.805,1583.232,1578.849,1574.116,1567.153,1560.297,1556.585,1548.767,1541.413,1533.693,1523.56,1520.971,1517.744,1513.765,1509.429,1504.237,1499.811,1496.576,1494.079,1490.117,1483.12,1480.156,1474.698,1468.853,1464.928,1456.944,1451.141,1446.179,1442.138,1438.714,1435.162,1431.309,1425.732,1419.513,1409.568,1401.909,1394.965,1384.545,1370.299,1359.857,1354.725,1349.614,1345.424,1341.559,1339.123,1333.573,1327.719,1320.364,1313.208,1308.187,1304.428,1300.471,1296.345,1291.769,1285.494,1280.775,1276.692,1272.126,1267.644,1263.324,1259.321,1256.075,1252.998,1249.436,1243.739,1234.904,1228.846,1225.46,1220.297,1213.769,1209.306,1205.731,1201.71,1197.654,1196.992,1193.939,1187.422,1183.621,1179.792,1176.175,1172.643,1169.001,1165.521,1162.361,1158.374,1154.928,1151.965,1149.512,1146.605,1143.374,1140.866,1137.854,1135.247,1131.346,1127.126,1124.34,1121.378,1116.199,1112.873,1109.475,1104.871,1101.253,1097.284,1094.466,1092.593,1088.843,1084.332,1082.409,1076.872,1070.36,1060.329,1057.086,1057.602,1055.332,1049.192,1040.999,1030.948,1024.181,1017.998,1013.408,1010.717,1008.81,1004.356,1000.482,996.907,993.9493,988.7661,986.005,983.5669,978.0977,973.865,967.1036,962.6685,958.4047,953.5317,949.7791,948.908,948.9012,948.1848,945.5422,942.4205,940.2192,935.8201,932.8538,929.9412,927.4799,925.0883,923.1981,921.5215,918.9564,916.524,914.5619,912.7768,910.6286,908.0692,904.6102,902.0214,900.1431,897.1709,895.0139,892.5626,890.4955,887.1645,884.8945,883.4572,881.6026,879.1578,878.1644,876.6271,874.6114,873.4091,871.7192,870.203,869.4058,868.658,867.2551,867.3009,867.0538,866.0358,865.4907,865.4211,864.2443,864.0534,862.8441,863.2989,862.5519,861.3915,860.6912,859.9521,858.8166,858.2504,857.62,857.2919,857.4076,857.0783,855.9562,854.2361,853.0475,852.8813,852.582,851.5446,850.6572,849.235,847.3201,845.4793,843.7661,843.3052,843.2664,842.2233,842.1275,843.2408,
1726.74,1727.338,1727.475,1720.192,1695.988,1673.965,1663.078,1647.662,1642.065,1638.369,1635.212,1629.919,1624.663,1615.49,1608.605,1602.47,1597.821,1594.552,1590.813,1586.736,1583.518,1578.987,1573.361,1560.917,1555.832,1553.598,1546.118,1539.685,1532.313,1521.211,1519.315,1515.69,1512.309,1507.953,1503.731,1498.335,1493.826,1490.3,1484.103,1480.008,1476.675,1471.014,1456.089,1456.362,1452.907,1448.837,1444.269,1439.749,1436.576,1432.553,1428.626,1424.267,1418.415,1409.821,1405.522,1397.057,1388.287,1378.23,1361.519,1356.884,1349.759,1344.115,1339.198,1335.159,1330.127,1324.468,1319.264,1311.323,1306.016,1302.149,1298.619,1294.149,1289.205,1283.258,1278.999,1275.368,1271.594,1268.032,1264.475,1261.033,1258.431,1253.522,1249.606,1241.948,1231.577,1227.025,1222.823,1218.05,1213.406,1209.174,1205.58,1202.903,1203.584,1200.646,1194.943,1190.008,1186.389,1181.83,1178.197,1174.767,1170.855,1166.804,1162.991,1159.171,1156.029,1152.551,1150.19,1147.485,1145.236,1142.078,1138.771,1135.392,1131.945,1128.174,1125.005,1121.502,1117.328,1114.177,1108.931,1105.039,1101.368,1097.18,1092.957,1089.982,1087.194,1082.953,1078.547,1073.85,1066.591,1058.804,1052.562,1052.663,1050.07,1043.666,1036.899,1029.047,1022.925,1017.136,1012.627,1009.489,1006.574,1003.125,999.947,994.8934,991.6697,986.9609,983.2603,980.4083,975.0263,970.702,965.8931,961.4991,956.7476,954.0662,953.7173,953.1828,953.0579,950.73,947.8791,944.7974,941.663,937.744,934.4509,931.5916,929.4919,926.8651,924.7365,922.0969,919.6494,916.9626,914.4639,912.8257,910.7353,907.3856,904.3533,901.6167,899.3336,897.0754,894.5813,892.2914,890.7692,888.0903,885.9005,883.7087,881.9844,880.1151,879.1859,877.3805,875.2311,874.6611,872.6598,871.9493,870.4637,868.9984,868.515,868.418,867.8088,867.4529,866.4354,866.2755,865.3401,865.0514,863.8842,864.1771,863.2877,861.5747,861.1264,860.6257,860.1523,859.1405,858.6844,858.4651,858.252,857.6464,857.0853,856.1111,855.2363,853.9561,853.424,852.7316,851.0013,849.1938,847.5146,846.2834,844.4391,844.3851,843.7968,842.4065,842.574,845.0483,
1727.735,1728.869,1727.877,1712.922,1694.067,1671.547,1657.711,1644.834,1638.774,1635.11,1629.686,1625.972,1621.319,1612.098,1607.733,1600.304,1596.268,1592.965,1589.539,1586.377,1582.593,1580.329,1571.862,1561.032,1552.455,1544.414,1538.765,1530.785,1526.392,1521.61,1518.771,1514.821,1510.871,1507.627,1501.849,1497.155,1492.812,1488.329,1480.714,1477.836,1473.765,1465.351,1454.826,1452.228,1450.225,1447.727,1442.981,1438.812,1434.785,1429.883,1425.564,1420.737,1417.359,1410.581,1405.335,1398.009,1390.837,1378.748,1367.809,1355.963,1348.885,1344.341,1339.182,1334.997,1330.857,1325.905,1323.082,1311.475,1307.028,1303.975,1299.851,1295.715,1290.761,1285.18,1280.518,1277.281,1273.844,1269.33,1265.634,1260.882,1256.495,1251.349,1248.329,1236.795,1230.166,1225.149,1220.734,1216.529,1213.365,1210.392,1210.274,1210.167,1207.103,1202.756,1197.09,1192.62,1188.683,1184.419,1180.884,1177.049,1173.027,1169.417,1165.334,1161.581,1157.434,1155.161,1152.016,1148.827,1145.253,1141.935,1138.521,1134.918,1131.345,1127.781,1124.723,1120.862,1116.703,1112.958,1108.998,1104.605,1099.885,1096.276,1092.048,1087.365,1083.386,1078.795,1073.991,1068.472,1058.344,1051.018,1048.855,1048.16,1047.268,1043.094,1038.137,1029.421,1022.507,1016.465,1012.444,1007.243,1001.833,999.4584,996.4134,992.1409,988.8984,984.9099,982.2762,978.6139,973.4955,969.0583,964.8491,960.5365,958.5436,957.3195,957.5814,956.491,955.1859,952.5754,949.2481,946.5269,943.3743,939.5279,936.3746,933.4396,931.1816,928.5205,925.9683,923.8439,920.2548,917.1934,914.9971,912.9479,909.6749,906.5106,903.3879,900.8542,899.2268,896.4786,894.7893,892.5135,890.2941,888.4963,886.6769,884.9146,883.281,880.8956,878.6537,877.5697,876.9216,875.3058,874.5626,872.772,871.9417,869.8254,869.1563,868.7031,868.6752,868.3234,867.8602,866.9703,866.7104,865.6524,865.1663,864.9199,863.3848,862.351,862.252,861.804,860.7672,860.2388,859.9833,859.718,859.7241,858.7283,858.4076,857.7556,856.5891,855.9137,854.8069,853.5335,851.9197,849.4526,848.4628,848.3643,846.7778,845.3423,844.2382,843.095,845.0333,847.2739,
1729.13,1729.74,1727.218,1715.359,1690.264,1666.01,1649.475,1641.773,1636.576,1630.559,1625.622,1621.347,1616.931,1609.013,1604.3,1598.575,1594.884,1591.249,1587.888,1585.12,1581.53,1577.397,1568.36,1558.811,1552.281,1546.134,1538.331,1528.833,1525.183,1519.107,1517.723,1513.525,1509.198,1505.08,1500.6,1495.905,1491.13,1484.752,1477.984,1474.571,1471.056,1464.42,1455.529,1453.024,1449.695,1444.043,1439.777,1435.878,1432.715,1429.016,1424.321,1419.878,1413.291,1407.891,1402.015,1397.766,1390.837,1378.78,1366.137,1358.043,1351.333,1344.184,1340.618,1336.745,1333.248,1329.77,1325.903,1320.26,1312.186,1308.75,1304.09,1299.275,1292.672,1289.033,1283.762,1279.625,1275.34,1271.496,1266.348,1260.804,1256.15,1251.067,1245.466,1236.74,1231.948,1227.886,1223.501,1220.379,1218.946,1218.057,1215.611,1213.71,1210.443,1204.06,1201.774,1197.504,1192.566,1186.634,1182.834,1179.021,1174.502,1171.13,1168.143,1164.568,1160.699,1156.974,1153.131,1148.875,1145.277,1141.367,1138.074,1134.062,1130.587,1127.423,1124.256,1120.57,1116.397,1112.602,1108.702,1104.532,1100.092,1095.924,1092.013,1087.383,1083.031,1078.14,1069.72,1062.496,1055.537,1048.602,1045.174,1044.79,1043.493,1040.682,1035.945,1027.753,1022.539,1016.425,1010.989,1004.712,997.9275,991.8293,990.5728,987.9019,985.1516,982.3219,978.2285,975.8491,971.2993,966.8336,964.4954,962.161,961.4415,961.3863,960.0589,958.4092,955.5865,952.9635,950.2175,946.9145,944.2023,940.5461,937.1667,934.5947,931.6852,929.6653,927.8255,924.4731,921.1962,918.3447,916.1523,913.6342,910.4094,907.4865,904.5806,902.6488,900.1091,896.551,894.4394,892.8072,890.0283,889.366,887.6192,885.894,884.866,882.1396,879.7007,878.4703,877.7368,876.4509,875.796,874.5224,872.6857,871.1806,870.1956,870.0684,869.6823,869.2499,868.6754,868.0793,867.7349,867.0676,865.9677,865.6798,864.61,863.3334,863.2299,862.8502,861.9305,861.5558,860.9542,860.5998,860.8518,859.7283,859.3792,858.6949,857.8712,856.9326,855.6295,853.2376,851.9875,851.7756,849.7056,849.1315,847.8577,845.6345,844.4307,844.1495,846.6005,847.7143,
1730.082,1729.473,1727.035,1708.713,1695.292,1668.563,1649.844,1640.372,1632.973,1627.627,1621.726,1616.99,1611.985,1605.907,1602.158,1597.159,1593.203,1589.063,1585.68,1582.115,1579.337,1572.661,1562.656,1555.547,1550.474,1544.055,1537.475,1531.963,1528.274,1525.125,1518.243,1513.044,1508.284,1504.177,1499.626,1494.9,1489.162,1482.555,1476.545,1471.479,1468.2,1461.675,1452.626,1451.673,1446.512,1443.705,1437.315,1433.276,1430.201,1426.837,1422.895,1418.49,1412.523,1406.425,1399.696,1393.81,1388.451,1378.091,1366.186,1360.274,1352.314,1346.557,1342.733,1339.354,1336.012,1332.569,1328.834,1323.661,1316.33,1312.508,1307.984,1303.047,1297.311,1291.301,1286.202,1281.442,1276.711,1272.73,1268.395,1261.686,1254.592,1249.589,1244.402,1238.789,1233.738,1228.756,1229.507,1230.192,1227.634,1224.87,1221.001,1217.944,1215.213,1212.819,1209.493,1203.975,1198.904,1191.612,1185.314,1181.641,1176.457,1172.896,1170.317,1166.929,1163.331,1159.436,1153.6,1148.341,1144.828,1141.096,1138.021,1134.348,1130.892,1126.989,1123.821,1120.329,1116.92,1112.859,1109.226,1104.769,1100.558,1097.475,1093.177,1089.125,1085.686,1077.697,1069.598,1061.212,1054.724,1048.275,1042.358,1042.062,1039.463,1036.551,1032.027,1026.209,1019.582,1014.427,1011.113,1004.3,997.7062,991.446,987.4156,984.9972,982.3745,978.6614,976.3623,973.1753,969.9263,969.1405,967.6703,966.5467,965.6196,964.1002,961.6194,959.481,956.8188,953.3672,950.4571,947.9381,945.2933,942.1217,938.7396,935.3409,932.3342,929.9124,927.3975,924.8948,921.6481,919.2822,916.1469,913.4443,910.5028,907.7905,904.931,902.8888,900.4461,897.9164,895.3087,893.7148,891.28,889.6402,888.3842,886.261,884.5273,882.6914,880.8516,878.9774,877.7868,877.3128,876.3337,875.7531,874.8503,872.6902,871.9357,871.4879,870.97,870.1711,869.1182,868.9105,868.2423,868.3307,867.1672,866.8147,865.7144,864.6623,864.4839,863.6863,862.9356,862.271,861.7368,861.2091,860.8485,860.2117,859.7729,859.1172,858.8366,857.8174,856.4831,854.8367,853.2117,850.9384,849.8303,848.8771,848.9547,846.8691,844.9902,845.3403,845.5573,846.8588,
1731.618,1730.37,1727.261,1705.559,1686.933,1668.731,1655.88,1642.506,1632.763,1625.164,1620.316,1614.973,1608.45,1603.897,1600.003,1595.335,1591.459,1587.546,1583.025,1578.798,1575.84,1573.101,1560.818,1554.388,1547.728,1541.722,1537.619,1534.883,1529.204,1523.034,1518.041,1513.001,1508.655,1505.322,1501.917,1496.662,1488.131,1481.465,1475.681,1471.513,1467.325,1454.36,1449.863,1446.149,1443.403,1439.431,1435.134,1431.226,1427.073,1424.572,1420.228,1416.574,1410.651,1403.524,1398.139,1393.519,1386.468,1375.943,1366.44,1359.256,1353.484,1349.908,1345.524,1341.237,1337.1,1333.49,1328.723,1325.102,1319.882,1314.057,1308.991,1303.482,1299.395,1292.147,1286.666,1281.806,1276.788,1272.326,1267.347,1260.257,1254.141,1249.129,1244.859,1241.59,1238.865,1234.476,1234.813,1233.075,1230.675,1228.141,1225.496,1222.167,1219.447,1217.452,1212.609,1208.646,1204.762,1198.525,1191.849,1185.315,1180.991,1176.419,1172.848,1169.029,1165.32,1162.027,1156.17,1149.29,1145.457,1141.717,1137.731,1134.553,1131.23,1128.134,1124.37,1121.464,1118.838,1114.36,1110.094,1105.933,1101.757,1097.603,1093.583,1089.297,1086.015,1080.648,1073.121,1065.192,1056.259,1050.062,1042.764,1039.779,1038.036,1036.07,1032.158,1027.563,1022.603,1015.83,1010.596,1005.266,998.4474,993.0341,987.2545,985.8145,983.4685,981.2703,978.6047,975.8967,974.7054,974.5627,973.078,970.7184,968.3538,965.239,962.3862,959.5984,957.6216,954.3911,952.1759,949.4523,946.773,944.49,940.8318,936.9136,933.6122,930.7966,927.9111,924.475,922.8409,919.8296,916.5375,913.7383,910.7968,908.5527,905.6246,903.7459,901.5404,899.3889,896.9953,895.2803,892.8929,890.6892,889.335,887.5908,885.1991,883.3627,882.6109,880.3083,879.3802,878.1569,876.7073,876.4869,876.2654,874.4271,873.5526,872.9562,872.6185,871.327,869.572,869.4012,868.9406,869.0637,868.2667,867.4702,867.442,866.322,865.644,865.291,863.7855,863.4287,862.8925,862.0432,861.9327,861.0958,860.0955,859.0844,858.4543,858.0445,857.0471,855.364,853.2713,851.6493,850.5918,850.1056,849.9008,847.9319,846.0621,846.4186,847.0994,847.6169,
1731.761,1728.995,1716.65,1701.407,1682.452,1668.881,1661.145,1655.21,1641.376,1629.227,1619.242,1612.451,1607.793,1602.362,1598.308,1594.969,1590.789,1586.571,1582.411,1577.062,1572.467,1568.469,1559.36,1554.387,1549.956,1547.155,1541.537,1536.065,1530.98,1527.503,1522.177,1513.648,1510.482,1505.789,1501.838,1497.352,1490.9,1482.948,1475.849,1470.506,1465.206,1456.287,1450.43,1445.435,1440.281,1435.401,1433.308,1430.366,1425.047,1421.132,1418.065,1412.713,1407.239,1401.359,1396.567,1390.428,1384.636,1377.536,1368.125,1359.432,1356.078,1353.154,1349.208,1343.89,1338.003,1333.85,1329.267,1325.667,1321.241,1315.93,1310.755,1304.684,1299.176,1292.881,1287.276,1281.927,1276.968,1272.089,1267.473,1260.378,1254.459,1249.775,1245.803,1242.32,1239.584,1238.258,1238.991,1237.313,1233.766,1229.962,1227.588,1225.732,1225.397,1220.703,1215.727,1211.868,1208.185,1203.514,1195.577,1187.432,1183.999,1179.422,1174.304,1170.851,1167.458,1164.229,1159.348,1151.22,1146.415,1142.953,1138.947,1135.464,1132.039,1129.252,1126.046,1123.344,1120.884,1117.251,1113.188,1109.099,1103.594,1098.798,1094.68,1090.606,1085.834,1080.885,1075.559,1066.965,1057.893,1051.637,1046.518,1042.203,1040.039,1036.989,1032.154,1026.506,1020.814,1015.567,1011.719,1005.569,999.4996,994.4729,991.998,991.7005,988.5317,985.9382,984.965,982.488,980.9208,979.6374,977.6178,973.2054,969.4417,966.1798,962.8736,960.4225,958.5891,956.0882,953.309,949.9127,947.1655,944.0978,940.6286,937.4335,934.6505,931.3931,928.7822,925.4878,922.0662,919.6851,917.0657,913.9464,911.2261,909.233,907.2709,903.9346,902.0488,900.3306,898.3072,896.4532,893.3524,892.171,891.3484,888.9673,886.5801,885.2676,885.0099,881.5234,880.7659,879.2198,877.9004,877.4583,876.9193,875.9345,875.1013,874.5659,874.21,872.6382,870.8669,870.5694,870.3195,869.6502,869.1288,868.4233,867.587,867.209,866.1872,865.6843,864.7843,864.8902,864.3037,863.3076,862.7575,861.9796,860.7563,859.4614,858.5501,857.6121,856.0394,854.7116,853.2286,851.8306,851.7266,851.2923,849.8701,847.782,846.447,847.8248,848.061,849.814,
1732.434,1719.792,1699.721,1689.379,1679.109,1669.462,1664.166,1655.525,1644.209,1632.104,1620.536,1613.461,1608.417,1602.918,1598.095,1594.324,1590.292,1586.414,1580.454,1576.336,1571.875,1567.453,1561.432,1556.528,1553.549,1549.04,1543.415,1537.167,1531.302,1528.521,1524.075,1518.293,1508.481,1505.142,1501.086,1496.511,1491.804,1486.263,1473.757,1468.927,1465.098,1457.392,1449.522,1444.235,1440.174,1435.643,1430.506,1426.57,1422.736,1418.9,1415.25,1411.345,1404.956,1398.895,1394.462,1389.315,1383.066,1376.777,1369.396,1361.359,1357.501,1353.659,1348.79,1343.615,1338.526,1333.92,1330.305,1326.517,1321.702,1317.36,1312.112,1306.316,1302.339,1293.966,1288.87,1282.99,1277.066,1271.775,1265.179,1259.696,1254.631,1249.612,1247.001,1245.994,1242.432,1242.861,1242.039,1239.393,1235.759,1232.827,1230.399,1229.63,1228.029,1225.102,1219.133,1214.901,1210.869,1205.01,1197.7,1190.241,1186.218,1182.295,1177.593,1172.691,1169.287,1166.268,1162.214,1154.161,1147.553,1144.061,1140.291,1137.082,1133.998,1130.675,1128.745,1126.381,1123.048,1119.41,1115.476,1111.404,1106.061,1099.6,1096.572,1092.406,1085.863,1080.044,1074.521,1066.266,1058.912,1052.924,1048.858,1045.981,1041.172,1038.982,1033.486,1027.779,1021.812,1015.395,1010.444,1006.171,1002.103,996.6952,994.6152,993.1361,991.0712,988.8434,987.1962,984.5983,982.6982,980.1164,976.6611,972.4333,968.8555,965.7917,963.8254,961.3691,959.0494,956.38,953.7159,950.5665,947.6985,944.6262,941.7199,938.7404,935.4861,932.2884,930.0417,927.2741,924.2792,921.1823,917.8351,915.1093,912.4823,910.2657,908.4103,905.881,903.9308,901.8882,898.9779,896.0558,894.4977,893.9897,891.26,889.7045,887.4843,885.0583,884.886,884.4539,882.3544,880.7589,879.5752,879.0329,878.2776,877.5331,876.9437,876.3763,876.0008,873.728,872.0148,871.5568,871.4714,870.5345,870.4771,869.1724,868.4356,867.7527,866.9468,866.4235,865.9843,865.8385,865.0914,864.0902,863.299,862.3229,861.3516,859.758,859.0518,857.8641,855.6143,854.1159,853.8531,852.8326,851.7449,850.0163,849.1448,848.0037,848.6086,849.7418,850.3027,852.5477,
1722.013,1705.427,1688.48,1682.777,1677.537,1671.041,1663.822,1655.029,1646.423,1635.008,1626.399,1616.205,1608.733,1605.466,1601.237,1593.36,1589.638,1585.77,1579.618,1574.731,1570.581,1566.632,1560.375,1555.023,1551.973,1547.878,1543.744,1536.17,1531.531,1526.928,1522.632,1515.234,1506.729,1503.282,1498.767,1494.473,1490.084,1483.125,1476.158,1468.352,1462.293,1458.409,1453.131,1445.57,1438.622,1433.921,1430.201,1425.475,1421.086,1416.867,1412.935,1407.189,1404.173,1397.757,1393.506,1387.649,1383.754,1375.171,1369.861,1360.231,1356.81,1352.168,1347.488,1342.625,1337.669,1333.875,1330.717,1325.507,1321.437,1316.788,1313.163,1308.706,1302.245,1294.755,1290.115,1285.581,1277.071,1270.989,1264.787,1259.174,1253.842,1252.917,1251.216,1247.678,1246.73,1245.979,1244.444,1242.493,1239.42,1236.323,1234.555,1232.286,1230.789,1228.412,1223.691,1218.934,1214.519,1208.539,1201.144,1194.835,1192.153,1188.671,1180.902,1175.939,1171.71,1167.634,1163.251,1156.339,1149.591,1145.037,1142.362,1139.102,1136.071,1133.513,1131.33,1128.66,1125.166,1121.527,1117.762,1113.694,1109.277,1102.633,1097.707,1094.1,1089.909,1082.697,1073.27,1067.183,1060.103,1055.625,1051.43,1045.943,1041.67,1039.003,1033.938,1028.181,1023.278,1016.862,1013.848,1010.644,1005.604,998.877,994.0521,992.345,991.0194,988.7477,986.1898,983.0336,980.3356,977.6841,975.156,971.7788,968.072,965.4644,963.1643,960.8665,958.4024,955.7903,953.0273,950.4379,947.2783,944.4255,941.3812,938.611,935.9537,933.1455,931.0123,928.563,925.85,922.3961,919.52,916.5102,914.6616,911.3477,909.6445,907.167,904.4967,902.2702,899.1482,897.8378,896.5458,895.3784,892.4933,889.7514,887.6682,885.7919,886.8409,885.1285,883.5099,881.8629,880.5242,879.995,879.618,878.7856,878.2946,877.6533,876.6118,875.1778,873.0558,872.637,872.2229,871.5551,871.0266,870.2917,868.777,868.2291,867.4034,867.4254,867.2586,866.5108,865.5917,864.7543,863.3288,862.5268,861.1365,859.8687,858.8871,857.4726,855.2493,852.9362,852.4693,851.5021,851.3033,849.7809,849.5109,850.3175,851.007,850.8773,853.4234,853.499,
1709.678,1694.043,1685.242,1677.664,1672.246,1667.673,1662.597,1654.158,1648.304,1640.997,1630.87,1618.738,1608.113,1602.896,1599.06,1591.102,1587.069,1583.704,1578.751,1574.096,1568.674,1563.518,1559.216,1554.754,1551.089,1546.988,1541.782,1535.221,1530.163,1524.848,1521.194,1514.135,1506.137,1502.081,1498.088,1493.044,1487.878,1483.158,1476.744,1470.495,1459.911,1456.368,1452.013,1445.128,1438.977,1432.946,1429.362,1425.086,1420.536,1417.233,1413.238,1406.61,1400.863,1395.673,1391.868,1386.12,1380.415,1375.071,1367.866,1359.603,1354.612,1349.979,1345.791,1341.024,1337.76,1333.173,1329.426,1324.935,1319.442,1315.274,1311.347,1305.263,1300.347,1293.898,1289.788,1281.751,1275.466,1269.687,1264.834,1259.414,1258.948,1255.769,1252.579,1250.842,1250.382,1250.133,1248.432,1245.602,1243.438,1240.213,1237.66,1235.164,1233.746,1230.4,1225.823,1222.626,1219.562,1212.803,1205.804,1198.176,1196.091,1191.276,1184.081,1180.756,1173.29,1168.579,1165.094,1158.757,1153.534,1148.464,1145.094,1141.899,1138.919,1136.123,1133.073,1130.268,1127.198,1123.647,1119.909,1116.062,1113.198,1107.672,1097.417,1093.73,1090.51,1081.109,1070.092,1064.731,1059.865,1055.334,1051.332,1046.588,1041.507,1038.93,1036.005,1032.09,1025.685,1020.517,1018.773,1015.28,1009.249,998.3776,993.397,991.2475,989.8923,987.8492,985.1987,982.36,979.1547,976.2004,973.7726,970.6379,968.1391,965.4451,962.0774,959.3413,957.3717,955.1793,952.676,950.1858,947.3334,945.6733,942.0637,939.1452,935.9563,933.899,931.393,929.0571,926.6592,923.6654,921.0683,917.8407,915.7571,912.8065,910.5216,907.6121,904.6674,902.464,900.5037,899.6818,898.8502,896.6608,893.7892,890.8811,888.755,888.1208,888.1796,886.9844,885.0618,882.8242,881.6269,880.8206,880.5049,879.8962,879.2513,878.55,877.9646,875.7646,874.1758,873.4426,873.4753,872.5978,871.6935,871.2463,869.6245,869.7154,869.0376,868.5886,868.0646,867.3039,866.0685,865.257,864.6394,863.8528,862.9149,860.525,858.7342,856.5751,854.559,851.8087,851.0128,850.6644,850.5502,850.4354,850.8406,850.9157,851.9974,853.2146,853.6649,854.6084,
1706.779,1689.773,1682.979,1677.357,1671.633,1666.652,1660.502,1652.254,1646.305,1639.857,1633.357,1618.107,1606.012,1598.463,1594.995,1590.503,1586.181,1582.389,1578.015,1573.58,1568.796,1562.782,1555.29,1552.15,1549.085,1544.729,1540.418,1533.47,1527.268,1523.087,1520.501,1514.622,1504.947,1501.314,1497.132,1491.706,1487.54,1483.12,1478.243,1472.266,1456.282,1452.334,1448.433,1443.189,1438.588,1432.606,1428.25,1423.575,1419.375,1415.433,1412.441,1406.886,1401.062,1395.172,1390.325,1383.681,1377.579,1373.051,1367.456,1360.876,1354.043,1349.414,1346.066,1342.095,1338.128,1334.536,1330.279,1327.01,1320.781,1316.298,1311.72,1305.072,1299.395,1293.492,1285.036,1276.981,1272.309,1268.283,1264.559,1263.151,1261.072,1257.477,1255.242,1254.794,1255.215,1254.462,1252.969,1249.26,1246.399,1243.251,1240.878,1238.468,1235.781,1232.62,1229.275,1225.924,1222.065,1215.038,1208.261,1202.901,1200.052,1192.843,1187.495,1184.292,1177.144,1169.739,1166.598,1162.449,1157.624,1152.564,1147.884,1144.48,1141.614,1138.863,1132.972,1130.346,1128.407,1125.74,1122.115,1117.599,1113.29,1107.338,1095.868,1091.541,1087.536,1077.334,1067.916,1062.809,1058.575,1054.281,1051.034,1046.903,1042.368,1040.238,1037.176,1033.938,1029.894,1024.656,1021.147,1015.519,1008.22,998.2715,995.0044,990.5128,989.0073,987.0562,984.8908,982.2132,978.8195,975.6088,973.0595,970.0937,967.7671,964.8959,961.8165,958.4292,956.4766,954.4464,951.6823,949.295,947.4654,945.4343,942.3911,939.9741,937.1857,934.6053,931.8621,929.4614,926.558,923.9887,921.5961,919.0975,915.7146,913.2885,910.9799,908.288,906.1701,903.8013,902.9581,902.4238,900.3837,897.6108,894.6807,891.8196,889.9857,889.0438,889.2174,887.598,885.3069,884.0605,883.1437,882.3942,881.0979,880.7459,880.4247,879.9567,879.4117,877.1461,875.3165,874.3033,874.5632,873.5224,872.3945,872.1519,870.9389,870.7239,870.1898,869.9153,868.9979,868.0037,867.3261,866.3247,865.1672,864.1766,863.2042,861.3085,858.6808,855.7209,854.3287,853.3872,852.4908,852.6161,852.3007,852.3799,851.9172,852.509,852.4656,853.9912,854.8501,856.4524,
1719.394,1692.92,1686.47,1680.133,1675.813,1671.422,1660.736,1654.728,1647.11,1638.618,1628.157,1614.081,1605.301,1597.525,1592.953,1589.373,1584.836,1580.987,1577.541,1572.649,1568.83,1562.437,1553.357,1549.364,1546.714,1544.246,1538.385,1529.515,1525.419,1522.317,1518.951,1514.305,1504.625,1499.997,1495.781,1490.917,1486.475,1482.291,1477.444,1469.046,1457.061,1450.309,1445.651,1442.257,1438.097,1432.395,1427.022,1422.305,1417.58,1414.149,1410.646,1405.555,1399.927,1395.253,1390.445,1386.36,1380.079,1374.997,1369.354,1361.142,1355.133,1351.792,1348.097,1344.222,1338.895,1335.135,1331.706,1328.251,1323.272,1315.471,1309.475,1304.383,1299.76,1293.154,1282.742,1277.043,1272.771,1269.667,1268.079,1265.709,1262.685,1260.173,1259.884,1259.024,1258.504,1258.431,1255.396,1252.369,1249.333,1246.785,1243.872,1241.039,1238.291,1234.311,1231.152,1227.901,1223.65,1217.758,1209.726,1206.547,1203.405,1197.459,1193.624,1188.329,1181.236,1171.147,1168.651,1164.837,1160.762,1155.139,1151,1146.995,1143.663,1139.963,1133.72,1129.196,1126.781,1124.997,1120.998,1115.745,1109.628,1101.842,1091.814,1088.343,1084.216,1076.539,1064.799,1060.646,1056.172,1053.429,1050.448,1046.831,1043.254,1041.379,1037.97,1034.61,1030.646,1026.424,1022.076,1016.437,1009.032,999.0835,993.5743,989.8646,987.7448,986.3617,984.7321,982.3207,979.4926,976.4735,972.6705,969.847,967.0785,964.8856,961.8695,958.76,956.2537,954.2468,951.501,949.715,946.9824,944.9122,942.063,939.5785,937.1964,934.2316,931.9916,928.9637,926.3434,924.5594,922.0501,919.5637,917.2295,915.228,912.3096,909.8973,907.4888,905.3003,904.6215,903.1729,901.4636,898.5015,895.7083,892.9572,891.2568,890.0062,889.7475,887.4432,885.9418,884.8841,883.946,882.9705,882.2897,881.951,881.8492,881.5403,880.3453,878.1832,876.4921,875.5568,875.4804,874.8683,873.7225,872.9815,872.1569,871.8542,871.2915,870.6484,869.582,868.7352,867.9138,866.6041,865.3853,864.2313,862.2128,861.1983,858.1661,855.6396,855.4448,854.3604,855.3325,855.2482,854.0425,854.2845,854.8015,854.7378,854.2573,855.0873,855.2296,856.8162,
1712.278,1703.581,1695.124,1683.946,1677.401,1670.369,1660.338,1653.722,1646.784,1637.058,1620.56,1608.249,1603.298,1598.021,1593.43,1588.61,1584.5,1580.088,1576.042,1571.623,1566.544,1560.67,1552.965,1546.464,1541.612,1538.426,1534.685,1526.811,1523.346,1520.046,1516.963,1512.639,1506.515,1498.634,1494.835,1490.267,1485.051,1480.387,1476.386,1467.619,1456.912,1450.7,1443.376,1440.733,1435.59,1431.135,1426.459,1420.363,1416.41,1412.906,1409.196,1405.059,1399.489,1395.257,1390.453,1387.191,1382.036,1377.39,1372.221,1360.492,1354.704,1350.724,1347.143,1343.183,1339.568,1335.635,1331.862,1327.431,1323.177,1315.956,1305.814,1301.601,1296.281,1288.544,1283.434,1277.601,1274.511,1272.973,1270.488,1267.725,1264.927,1264.206,1264.133,1263.493,1263.622,1262.116,1259.558,1256.029,1252.442,1249.907,1246.948,1243.513,1240.216,1236.806,1233.247,1229.832,1226.184,1221.775,1216.744,1212.404,1208.133,1202.62,1197.938,1192.148,1182.062,1173.852,1171.154,1167.91,1164.434,1159.856,1154.04,1149.548,1144.844,1140.202,1134.665,1129.453,1125.629,1122.419,1118.632,1112.245,1106.682,1094.558,1088.615,1084.62,1078.118,1069.093,1063.383,1059.068,1055.482,1052.687,1049.79,1046.523,1043.45,1041.34,1038.993,1033.646,1030.12,1026.795,1023.657,1018.997,1009.95,1001.991,992.3192,988.1251,986.459,984.3156,983.2719,980.9123,978.2402,975.7317,972.7527,970.1209,967.8652,965.019,962.0772,959.235,956.1631,953.6338,950.8585,948.8665,946.637,944.3992,941.5673,938.9952,936.8068,934.0936,931.7774,928.6721,926.3912,924.2557,922.0202,919.798,917.8774,915.46,913.0967,910.2078,908.3874,906.3327,905.3514,903.3291,900.3344,898.394,895.6581,893.942,892.3316,890.9207,889.8546,887.7264,886.8464,886.3716,884.7449,883.6943,883.4813,883.3024,883.376,882.4819,881.2433,879.1906,877.459,876.7202,877.0263,875.6521,874.9464,873.5932,873.1188,872.6087,872.1074,871.3624,870.4528,869.2544,868.4713,867.226,865.1111,863.8004,861.9529,860.223,857.4136,855.7628,856.1033,857.0247,857.8906,857.2418,856.6611,857.2765,856.3354,854.7411,856.4139,856.2081,856.5505,857.8466,
1719.33,1712.522,1701.117,1690.292,1675.222,1668.585,1659.985,1651.22,1644.388,1631.279,1616.419,1607.278,1602.752,1598.274,1592.973,1588.137,1583.9,1579.695,1575.127,1570.177,1565.885,1561.142,1552.551,1544.859,1540.315,1536.55,1531.807,1526.033,1521.863,1517.36,1514.799,1511.638,1505.271,1497.421,1492.961,1489.357,1485.012,1479.601,1475.699,1469.255,1457.887,1449.015,1442.152,1437.793,1433.069,1429.031,1424.9,1419.746,1416.261,1411.582,1407.12,1402.914,1399.083,1394.543,1389.873,1385.432,1381.562,1376.568,1370.519,1358.194,1353.431,1349.76,1345.485,1342.538,1338.354,1334.785,1330.363,1325.236,1320.827,1315.996,1304.531,1298.418,1292.766,1288.901,1283.528,1279.241,1277.586,1275.281,1272.268,1270.029,1268.254,1268.015,1268.242,1267.283,1267.208,1265.476,1263.145,1259.506,1255.493,1251.844,1248.76,1245.523,1241.55,1238.034,1234.834,1231.962,1228.064,1223.62,1219.894,1215.988,1209.18,1205.413,1200.079,1193.881,1184.369,1177.37,1173.773,1170.579,1167.732,1164.668,1159.112,1151.522,1143.719,1139.28,1134.37,1129.222,1124.906,1119.531,1113.619,1104.313,1099.837,1089.876,1086.293,1080.842,1072.945,1066.383,1062.43,1057.836,1054.598,1051.627,1048.494,1045.079,1042.402,1040.466,1036.49,1031.697,1028.856,1026.53,1023.422,1019.444,1009.419,1000.325,991.2582,987.6152,985.1302,982.7832,980.8505,979.0395,976.6375,974.5749,971.8027,968.83,966.8379,963.7137,960.6934,958.3935,955.7837,953.2983,950.871,948.1764,945.8591,943.4456,940.6072,938.0897,936.1143,933.8274,931.2209,928.9665,926.688,924.8008,922.672,920.7809,919.6455,917.23,913.8546,910.9473,908.0637,905.8387,903.476,902.3134,900.6925,898.6922,895.7615,894.4463,892.8442,891.5819,889.6349,888.6283,887.9,886.9586,885.4089,885.1206,884.7468,884.5765,884.033,883.5412,881.6551,879.5795,878.3179,877.7734,878.5999,876.6858,875.7346,874.5378,874.1101,873.5705,872.86,871.7749,870.9964,869.6751,868.6846,867.5255,865.5271,862.8181,860.8851,858.6864,857.4835,856.1786,856.5991,857.4698,858.2278,858.0156,857.8931,858.4583,858.0076,857.3159,857.2471,857.6797,857.7968,860.0715,
1721.879,1716.528,1701,1686.268,1671.22,1665.888,1655.581,1645.92,1637.865,1624.958,1608.729,1604.463,1601.595,1597.593,1594.195,1590.18,1584.386,1578.946,1575.062,1570.509,1565.959,1560.639,1552.249,1545.83,1540.378,1536.769,1532.064,1526.676,1521.447,1516.438,1512.582,1509.156,1503.321,1496.94,1491.463,1487.353,1483.345,1478.675,1473.94,1467.669,1460.983,1453.693,1446.223,1435.724,1430.986,1426.545,1422.148,1418.281,1414.57,1410.324,1406.451,1401.775,1397.503,1393.142,1388.89,1384.183,1380.189,1375.328,1368.883,1359.113,1351.756,1347.677,1344.183,1341.04,1337.881,1333.234,1328.418,1322.26,1318.582,1312.594,1303.125,1296.98,1290.976,1287.531,1284.959,1283.215,1280.392,1276.882,1275.466,1274.197,1273.425,1272.362,1271.055,1270.164,1269.898,1268.432,1265.089,1260.68,1257.388,1253.802,1250.392,1247.188,1244.337,1241.182,1238.659,1236.043,1231.603,1223.748,1220.321,1216.079,1210.255,1205.54,1200.635,1195.61,1187.887,1180.732,1176.94,1173.823,1171.3,1167.859,1159.4,1150.647,1142.064,1137.904,1134.006,1128.462,1122.117,1116.958,1109.619,1099.37,1093.434,1086.779,1082.391,1077.145,1069.581,1065.478,1061.032,1057.012,1053.233,1050.24,1046.935,1043.57,1041.069,1037.514,1033.729,1029.715,1026.604,1024.276,1022.646,1018.271,1008.377,1000.318,992.5667,988.0672,985.0502,981.5508,979.6254,977.2102,975.3741,972.7062,970.5776,967.6425,965.1617,962.3111,959.9108,957.3429,954.7925,952.3998,950.1522,946.7436,944.8508,942.4091,940.1233,937.6243,935.7352,932.8076,930.3055,928.0882,926.2286,924.6646,923.2196,921.7234,919.5479,918.5966,915.0065,911.6243,907.9773,905.0445,903.0411,902.1542,901.1774,899.0356,896.0394,893.7154,893.1964,891.0797,890.2577,889.3592,888.4479,887.622,886.9497,886.4636,885.8625,885.4448,884.9339,884.1226,882.5079,880.4422,879.6723,878.9673,879.9574,878.5212,877.2589,875.7955,874.8586,874.8215,873.9965,872.5123,871.4729,869.4821,868.688,868.1754,864.6055,862.8638,860.4307,860.4106,859.5093,858.1138,856.7625,857.6714,858.5881,858.9147,858.8506,858.6036,858.2527,858.1506,857.9935,857.863,858.629,859.8118,
1721.583,1712.679,1695.973,1679.683,1667.284,1662.756,1654.327,1643.868,1633.312,1615.053,1605.284,1601.951,1598.765,1596.326,1592.502,1588.686,1584.096,1578.264,1574.203,1569.602,1565.346,1560.868,1553.247,1547.711,1541.143,1536.36,1532.263,1527.346,1521.113,1515.935,1512.172,1508.027,1503.535,1498.469,1490.828,1486.787,1482.298,1476.756,1472.25,1467.655,1461.134,1452.378,1445.067,1436.129,1429.59,1425.64,1421.175,1417.138,1412.832,1408.84,1405.355,1400.826,1396.49,1392.436,1387.831,1383.301,1378.919,1375.097,1370.17,1360.106,1351.286,1346.591,1342.267,1338.993,1334.933,1329.961,1325.883,1320.686,1316.634,1310.097,1301.339,1296.145,1294.155,1294.028,1288.924,1285.593,1282.263,1280.083,1279.446,1278.381,1275.965,1274.372,1273.7,1273.333,1272.357,1270.729,1267.486,1262.456,1258.67,1255.276,1252.161,1249.319,1246.586,1244.001,1241.889,1239.53,1234.446,1226.32,1220.557,1215.582,1210.405,1205.561,1200.639,1195.03,1188.988,1183.607,1180.165,1175.804,1172.999,1166.593,1157.931,1148.866,1141.389,1135.587,1132.799,1127.831,1120.461,1115.744,1108.325,1096.016,1090.611,1083.55,1078.791,1074.217,1067.266,1063.081,1059.211,1055.2,1051.364,1048.155,1045.126,1042.472,1039.808,1036.705,1032.677,1027.866,1023.936,1021.214,1018.433,1013.262,1007.964,997.7999,992.1803,989.0503,985.6945,982.0901,979.1241,976.858,973.9006,971.5974,969.283,966.8574,964.2005,961.596,958.5526,956.3243,953.998,951.3889,949.0135,946.4515,943.4653,941.085,939.0238,936.6313,934.0975,931.3971,929.3055,927.0165,924.6243,923.0814,921.1185,919.7596,918.6226,916.4858,913.8242,911.0999,907.8007,905.8453,904.1223,902.7758,901.3453,899.0522,896.8,894.2684,892.8885,892.4847,891.4295,890.7427,889.5298,888.5427,887.9381,887.5869,887.2966,886.3444,885.7048,884.7128,882.6514,881.8407,880.717,880.5286,880.4695,879.0192,878.0409,877.141,875.3388,875.4398,874.4275,873.17,871.723,870.6711,868.6182,867.9153,864.3243,863.3608,861.7852,861.3162,861.0497,859.3245,858.5722,858.2704,859.4683,859.5726,859.4059,858.702,858.9117,858.7474,858.528,858.4257,858.2223,859.2666,
1722.016,1709.999,1698.211,1674.528,1666.986,1660.88,1653.496,1643.304,1632.025,1614.166,1604.212,1599.21,1596.98,1594.392,1591.24,1586.952,1582.84,1577.871,1573.241,1569.149,1564.956,1560.058,1553.454,1548.4,1542.452,1537.007,1533.661,1528.391,1521.711,1515.134,1510.31,1506.604,1502.582,1498.598,1491.171,1485.191,1480.996,1475.65,1470.715,1464.921,1458.91,1452.09,1442.518,1434.649,1429.193,1425.315,1421.313,1417.253,1411.659,1407.989,1404.055,1400.279,1395.055,1389.929,1385.419,1381.772,1378.176,1373.618,1368.777,1360.457,1352.983,1344.265,1340.306,1336.292,1332.542,1327.052,1322.891,1318.948,1314.429,1305.711,1301.718,1301.381,1298.264,1295.208,1290.787,1287.301,1285.611,1284.979,1283.792,1282.082,1278.984,1277.304,1276.922,1275.987,1274.041,1272.024,1268.962,1263.562,1260.137,1256.976,1253.884,1251.213,1248.637,1246.092,1243.931,1241.072,1234.908,1226.826,1220.97,1215.231,1209.458,1204.56,1199.229,1193.424,1187.68,1184.081,1179.161,1175.234,1173.354,1165.398,1154.509,1143.511,1137.267,1133.024,1129.267,1125.037,1118.751,1110.624,1101.551,1092.186,1084.77,1080.415,1076.117,1072.036,1066.304,1061.697,1057.763,1053.717,1049.916,1046.21,1042.966,1039.949,1036.811,1034.123,1030.813,1025.54,1020.803,1016.513,1012.475,1007.81,1004.136,999.0482,994.0613,989.2503,985.9687,982.5813,979.5513,976.9534,974.9387,971.236,968.238,965.6292,963.134,960.504,957.7253,955.1804,951.7479,950.2223,947.5283,945.0275,942.5573,940.5912,938.6168,936.3456,933.5605,930.1479,928.3469,926.1953,923.9775,921.1573,919.8412,917.384,916.1624,914.4644,911.8167,909.8252,908.096,906.6956,904.4006,902.5431,900.3539,898.3275,897.8973,895.9938,895.2656,893.8449,892.4124,891.455,890.645,889.6597,888.8351,888.879,888.3151,886.637,886.1211,884.8767,883.2491,883.1191,880.9938,882.0219,881.0992,879.6951,878.9453,878.2451,876.1579,875.9101,874.3829,872.9058,871.8527,870.1643,869.1382,868.3345,865.5628,864.7078,864.2291,863.577,862.0715,860.6757,859.2374,859.5432,859.7761,859.6193,859.7553,860.2307,859.5671,859.1824,859.1923,859.2332,859.6951,860.0934,
1723.667,1711.138,1694.549,1673.759,1665.897,1658.511,1653.25,1640.963,1629.961,1611.432,1602.819,1597.883,1595.003,1592.416,1589.605,1586.049,1581.805,1577.447,1571.813,1568.471,1564.005,1557.7,1553.159,1549.707,1544.055,1540.607,1536.399,1528.335,1520.672,1514.87,1509.662,1505.292,1500.912,1496.242,1489.506,1483.326,1479.083,1474.756,1470.151,1464.977,1458.466,1452.087,1442.065,1434.094,1428.651,1424.343,1420.762,1416.727,1410.823,1405.462,1402.374,1397.65,1393.42,1387.162,1383.539,1379.506,1375.666,1372.166,1367.526,1359.523,1351.185,1342.297,1338.597,1334.548,1330.525,1325.598,1321.745,1317.074,1310.701,1307.631,1306.461,1303.18,1299.788,1296.484,1295.846,1293.766,1290.569,1290.065,1288.059,1285.539,1283.175,1280.601,1279.204,1277.488,1275.371,1272.828,1269.16,1265.822,1262.342,1258.712,1255.563,1252.858,1249.907,1247.247,1243.937,1239.638,1234.251,1226.313,1220.952,1214.884,1208.828,1202.02,1197.012,1191.848,1185.82,1181.657,1175.965,1172.381,1171.042,1160.362,1148.921,1137.93,1131.873,1128.418,1124.923,1121.297,1116.257,1104.884,1096.453,1088.518,1082.242,1077.826,1073.811,1069.714,1064.526,1060.157,1056.301,1052.309,1048.721,1044.594,1040.997,1036.784,1033.754,1030.139,1027.154,1023.209,1016.807,1013.182,1008.791,1002.546,999.4706,997.337,993.2115,989.1611,985.9437,982.5558,980.2006,977.7733,974.2855,970.7646,967.6956,964.6325,962.8936,960.0154,957.3074,953.9609,952.0585,948.7583,946.3041,944.0046,941.8777,940.1108,937.9124,936.1512,933.6864,930.8783,928.4641,926.4799,923.5887,920.072,918.7026,916.8058,914.7315,914.7513,912.2725,909.2478,907.3742,905.7524,904.2336,901.6644,899.9905,898.5424,897.9209,897.5306,896.8461,895.5153,894.1401,893.1802,891.9303,891.1191,889.9359,889.6036,889.22,888.3695,887.0093,885.672,884.2084,883.3071,882.2734,882.6285,881.4501,880.9878,879.753,879.233,877.0748,876.3091,874.8733,873.7186,872.7977,871.5324,870.4001,868.3517,866.7159,866.0451,865.4407,864.3875,863.3208,861.2958,860.5805,860.2945,860.0906,860.9351,860.7458,860.8503,860.3007,860.0386,860.0679,860.4089,860.8415,861.8019,
1724.032,1705.762,1691.415,1674.931,1665.187,1656.628,1650.406,1642.713,1625.237,1607.238,1600.954,1597.233,1593.539,1590.391,1587.011,1583.972,1580.384,1576.79,1571.067,1567.609,1561.302,1555.493,1552.564,1549.022,1544.145,1540.359,1536.484,1526.875,1519.963,1515.567,1510.755,1506.016,1499.675,1495.239,1489.028,1483.488,1479.032,1472.873,1468.595,1463.727,1457.407,1451.286,1442.177,1433.702,1429.167,1423.404,1419.783,1416.028,1410.281,1404.038,1399.448,1395.288,1390.131,1385.287,1381.797,1377.752,1374.089,1369.983,1365.921,1356.689,1347.887,1342.472,1338.424,1334.224,1329.96,1325.209,1320.933,1316.696,1314.465,1311.819,1308.436,1305.15,1302.346,1300.982,1301.317,1301.104,1297.504,1293.691,1291.311,1288.784,1286.36,1283.734,1281.564,1279.102,1276.838,1274.54,1271.247,1267.831,1264.586,1260.438,1256.911,1253.916,1250.471,1247.254,1242.892,1238.293,1233.471,1226.445,1220.612,1214.459,1208.215,1201.793,1195.442,1190.834,1185.035,1178.788,1172.747,1169.724,1167.799,1156.261,1144.092,1132.502,1127.836,1124.951,1122.302,1118.741,1112.629,1102.82,1092.224,1084.837,1078.961,1074.891,1071.587,1067.326,1062.638,1058.643,1054.197,1050.655,1046.511,1042.039,1038.038,1034.598,1031.627,1028.617,1024.827,1020.901,1014.423,1010.257,1005.68,1001.231,997.9235,994.9781,992.0112,989.0269,985.8654,982.7182,980.5539,977.8523,974.3401,970.9435,967.8759,964.5981,962.6046,960.0811,957.369,954.2722,950.9545,948.0475,945.2266,942.9399,940.8932,939.3926,938.0615,936.5082,934.6669,931.9468,929.4579,926.9724,923.9644,920.5671,918.1425,915.7849,914.2745,913.1897,911.2777,909.7334,907.263,905.447,904.0078,900.4019,899.2983,898.4606,897.4278,896.952,896.5556,896.1518,895.4195,894.5085,893.233,892.3154,890.9194,890.6934,890.0634,888.6725,887.3622,886.1862,885.1697,883.5554,883.9954,882.9448,882.5614,882.1108,880.808,879.5466,878.0212,877.246,875.6438,874.6078,873.7667,873.1783,870.7148,868.8167,867.1929,866.5403,865.6368,864.6553,863.8763,862.6652,861.7905,861.5297,860.9121,860.8469,862.1619,861.8069,861.1468,861.0474,860.8911,861.6236,862.907,864.5802,
1724.008,1702.61,1689.907,1674.822,1664.038,1655.029,1649.174,1641.223,1617.762,1609.23,1600.56,1595.298,1591.433,1588.003,1584.658,1581.264,1578.228,1574.403,1569.824,1565.093,1560.682,1553.039,1550.167,1548.58,1542.852,1538.863,1534.052,1527.894,1521.573,1517.146,1512.096,1506.375,1500.159,1491.694,1487.324,1484.057,1477.239,1472.869,1467.531,1462.993,1457.592,1449.438,1443.241,1433.566,1428.674,1423.307,1418.954,1414.521,1409.977,1404.367,1398.635,1394.418,1390.975,1386.76,1380.122,1375.287,1371.872,1367.974,1364.491,1357.625,1348.276,1342.688,1338.878,1334.053,1329.334,1325.251,1322.274,1320.075,1317.077,1313.774,1311.145,1309.14,1309.409,1307.852,1305.851,1304.941,1303.166,1298.441,1294.99,1292.32,1289.772,1286.452,1283.665,1281.359,1278.554,1276.649,1273.965,1271.431,1268.669,1263.426,1258.409,1253.894,1250.32,1246.554,1241.956,1237.411,1232.619,1226.316,1220.067,1213.794,1207.349,1201.475,1193.483,1188.353,1183.089,1177.169,1171.892,1166.507,1163.235,1151.586,1139.224,1130.824,1125.725,1122.247,1118.962,1114.551,1108.398,1097.245,1087.778,1081.363,1077.493,1072.244,1068.761,1064.268,1059.85,1055.531,1052.995,1048.349,1044.19,1040.815,1037.211,1032.914,1029.671,1026.195,1021.815,1017.943,1013.799,1009.097,1004.972,1001.024,997.5104,993.7952,990.9148,988.0792,985.0883,982.1059,980.1832,977.6368,974.4005,971.4971,968.3848,965.5793,962.4401,959.3568,957.5239,954.8778,951.5303,948.1105,944.9006,942.9881,940.4634,938.3845,936.9032,935.376,933.8664,932.2208,929.8639,927.3764,924.8019,922.0757,919.1309,915.9385,913.9655,912.7671,911.001,908.9697,906.7636,905.2931,903.2845,900.6528,899.7295,899.3057,898.2458,897.7823,897.0367,896.4726,895.5029,894.7926,894.2135,893.6009,892.3792,891.5209,890.4586,889.1121,887.7861,887.0491,885.6031,885.2139,884.3996,884.4097,883.5708,882.6236,881.5426,880.8099,878.6013,877.9349,876.8155,875.9969,875.0771,873.7784,871.2846,868.6338,867.1713,866.4108,865.1755,863.9476,863.5364,863.2466,863.3005,862.931,861.9553,861.4935,862.6058,862.8813,863.8272,863.6285,864.2959,865.2051,865.9355,867.6422,
1723.533,1698.921,1689.444,1675.768,1664.662,1654.381,1647.484,1635.07,1623.094,1614.141,1601.2,1592.312,1589.448,1586.049,1581.582,1578.542,1575.095,1571.81,1567.562,1563.012,1558.737,1551.859,1549.097,1547.475,1544.596,1539.697,1533.252,1528.074,1522.296,1516.536,1512.256,1506.18,1499.464,1490.554,1484.533,1479.275,1475.638,1472.012,1466.458,1462.609,1455.762,1449.631,1442.869,1436.654,1428.36,1423.161,1418.58,1412.901,1407.44,1403.558,1399.467,1395.112,1390.902,1387.19,1380.825,1374.483,1370.831,1365.869,1362.356,1356.501,1347.304,1343.267,1338.576,1333.382,1330.525,1328.947,1325.591,1322.745,1319.323,1317.056,1316.022,1314.433,1314.438,1313.658,1310.494,1308.741,1307.986,1304.594,1300.855,1295.493,1292.023,1289.005,1285.932,1283.501,1280.691,1278.689,1275.688,1273.56,1269.663,1264.41,1258.903,1254.457,1249.334,1245.458,1240.507,1235.918,1231.282,1225.603,1219.404,1213.129,1206.836,1200.701,1192.421,1185.504,1181.132,1176.285,1170.772,1164.247,1154.542,1144.608,1135.145,1130.599,1123.844,1119.233,1115.17,1111.183,1102.206,1095.252,1086.1,1079.504,1073.594,1069.935,1066.437,1062.003,1057.415,1052.873,1049.671,1046.099,1042.268,1038.743,1035.173,1032.555,1027.952,1023.5,1019.853,1017.268,1012.713,1008.307,1004.562,1001.546,997.1754,993.9525,990.3447,987.7335,984.8689,981.3863,979.1227,977.2242,974.4495,972.3809,967.9901,965.7183,962.6752,959.0877,956.6727,954.0779,951.6995,949.0033,945.451,942.191,939.437,937.5573,935.6165,933.7346,932.588,930.7769,928.6432,926.1883,924.0298,921.6841,919.3716,916.3776,914.7155,913.0092,910.7148,908.1655,906.838,905.4207,903.1075,900.7086,900.363,900.1968,899.1444,898.5171,898.1488,897.3504,896.4196,895.2729,894.2538,893.5289,892.9288,892.2684,891.2183,889.6854,889.2206,887.4016,886.3701,887.1512,886.4147,885.0914,884.165,883.301,882.242,880.8099,879.769,878.5973,878.1858,877.3417,876.3658,875.0091,872.0263,869.1169,867.2568,866.3324,865.3451,865.2451,865.1956,864.9927,864.8103,863.5058,862.3242,863.6808,864.3685,865.1607,866.4717,867.3173,867.7271,868.637,870.0829,870.9078,
1722.996,1695.957,1686.374,1673.454,1666.159,1651.821,1643.895,1636.146,1630.113,1618.133,1602.611,1590.529,1587.155,1584.154,1579.744,1576.198,1571.469,1568.335,1563.802,1561.149,1555.602,1550.417,1547.572,1546.283,1542.565,1539.473,1533.766,1527.864,1520.423,1515.769,1511.919,1505.493,1498.968,1489.627,1481.971,1477.094,1473.906,1470.975,1465.641,1460.72,1454.867,1447.182,1441.771,1435.608,1429.045,1422.802,1417.527,1411.584,1406.709,1403.025,1397.306,1392.491,1388.711,1384.613,1379.048,1372.767,1368.236,1363.561,1360.123,1355.949,1348.26,1342.445,1337.908,1336.135,1333.984,1332.089,1328.625,1324.784,1322.472,1321.758,1321.437,1320.816,1319.847,1317.488,1316.034,1314.6,1313.121,1308.79,1304.686,1298.485,1294.652,1291.651,1288.888,1285.236,1282.292,1279.747,1276.462,1272.727,1268.075,1262.505,1257.112,1253.031,1248.545,1244.499,1239.538,1234.9,1230.233,1224.264,1218.245,1212.426,1206.151,1199.28,1191.368,1186.595,1179.071,1169.958,1161.908,1152.96,1143.072,1138.292,1133.604,1129.083,1123.387,1120.281,1115.819,1110.313,1100.82,1093.096,1085.28,1078.347,1071.741,1068.31,1064.275,1060.226,1055.961,1051.768,1048.371,1044.545,1040.72,1037.57,1033.869,1030.488,1027.636,1023.467,1019.73,1015.812,1012.081,1008.28,1004.518,1000.76,997.422,993.7913,990.309,987.5582,985.1213,981.5308,979.1461,976.7597,973.459,970.5188,967.021,964.2683,961.2764,958.1379,955.6631,952.8606,950.0634,948.0873,945.3366,941.3787,939.3226,936.9509,935.1475,933.3168,930.543,928.8539,927.2899,924.7416,923.3878,921.1994,919.9286,917.2822,914.8438,912.2171,910.1125,907.7972,906.7922,905.6597,903.8312,901.873,901.0795,900.8074,899.7831,899.361,898.7075,897.7637,897.185,896.1899,895.203,894.2107,893.3678,892.6467,891.6418,890.8805,890.2906,889.0164,888.3769,888.1721,886.6869,886.0685,884.907,884.2175,882.8882,881.5394,880.2103,879.2025,879.2953,879.1674,877.1869,875.2863,872.0671,870.3694,869.9284,868.6987,867.7382,866.9958,867.0073,866.2465,863.8819,864.2115,864.7752,866.45,867.2686,869.0928,869.8946,871.4253,871.987,872.3205,873.3848,874.1536,
1725.571,1692.071,1678.062,1670.355,1664.086,1650.413,1642.538,1637.92,1631.898,1619.151,1606.138,1589.169,1585.102,1582.273,1578.803,1574.248,1569.676,1564.731,1561.045,1557.699,1554.26,1549.11,1545.763,1543.358,1540.918,1537.657,1532.833,1526.581,1518.833,1514.68,1509.65,1504.586,1496.259,1489.001,1479.565,1474.357,1471.305,1469.592,1464.805,1458.061,1453.445,1444.065,1438.995,1433.604,1425.953,1421.062,1417.164,1410.979,1407.383,1401.778,1396.345,1390.723,1386.24,1381.697,1376.375,1372.196,1367.451,1363.08,1358.275,1352.98,1347.518,1344.57,1341.447,1339.846,1337.986,1335.963,1332.423,1329.034,1327.603,1326.328,1326.389,1326.6,1325.61,1324.049,1321.618,1319.888,1317.419,1311.209,1306.639,1300.933,1297.512,1292.914,1290.225,1286.74,1283.519,1279.669,1275.08,1270.429,1266.077,1260.36,1255.332,1251.722,1247.027,1242.538,1238.232,1233.587,1228.212,1222.383,1216.334,1210.171,1204.399,1197.65,1190.477,1182.832,1175.954,1166.909,1157.484,1148.12,1141.524,1136.149,1132.529,1127.167,1120.737,1116.893,1113.078,1108.933,1098.152,1091.05,1082.218,1076.939,1071.935,1068.328,1063.875,1060.206,1056.324,1052.649,1050.346,1045.224,1040.989,1037.768,1034.328,1029.287,1026.767,1023.231,1019.599,1015.536,1011.716,1008.07,1004.146,1000.718,998.1713,994.1384,990.3717,987.1263,984.2538,980.6487,979.1137,974.905,972.1016,967.6465,965.1673,962.3643,959.756,957.1842,954.4781,951.6664,948.4822,946.2576,943.7846,940.8538,938.5876,936.1093,933.7828,931.8943,929.3551,927.314,925.4296,923.4168,922.1425,920.5518,919.0325,917.1355,914.1547,911.9609,909.6302,908.1017,906.8887,906.0604,904.9764,903.3814,902.6295,901.8438,900.7407,900.0982,898.7852,898.2643,898.3142,896.8381,896.0404,894.7736,894.666,893.9495,892.3335,891.0812,890.2191,889.6403,889.1182,888.5417,887.6863,887.1434,885.9177,884.348,883.4307,882.3702,881.3274,880.6346,879.9236,879.8931,876.9083,874.2997,872.8461,872.0044,871.7012,870.917,870.8689,870.3964,869.0645,867.1052,865.1031,866.2196,867.297,868.3397,870.5665,872.9984,873.7369,874.8753,875.458,875.8929,877.2595,877.4143,
1731.098,1693.788,1679.123,1674.224,1664.57,1651.728,1639.408,1635.248,1629.461,1618.977,1608.75,1591.144,1584.511,1579.937,1576.722,1572.38,1568.308,1563.08,1559.566,1556.22,1550.049,1547.228,1544.251,1540.928,1537.775,1535.538,1531.581,1525.986,1518.393,1512.986,1508.502,1502.454,1494.358,1487.904,1481.273,1471.639,1468.115,1467.205,1463.151,1457.106,1450.395,1443,1436.905,1432.572,1425.5,1419.731,1416.395,1411.567,1405.796,1400.138,1394.269,1389.261,1385.298,1381.531,1377.88,1373.856,1368.393,1362.455,1358.167,1353.799,1349.542,1347.248,1346.283,1344.924,1342.407,1340.416,1338.265,1334.149,1331.845,1330.841,1331.23,1331.458,1331.092,1329.268,1326.24,1324.305,1319.62,1313.02,1307.982,1303.739,1299.208,1294.587,1291.389,1287.204,1283.538,1279.686,1274.093,1270.476,1264.482,1258.297,1253.54,1249.417,1245.785,1241.087,1236.288,1231.081,1225.125,1220.172,1214.187,1207.536,1201.502,1195.165,1187.333,1179.143,1172.703,1163.765,1155.01,1147.445,1142.747,1137.258,1130.582,1125.871,1120.318,1116.015,1111.566,1106.157,1097.571,1089.663,1083.051,1078.517,1073.918,1069.815,1065.433,1062.394,1058.974,1054.776,1051.602,1047.433,1043.231,1037.669,1032.411,1026.353,1024.528,1021.562,1018.008,1014.333,1010.49,1007.04,1003.399,1000.085,997.3923,995.7736,990.9943,987.2738,985.3113,980.9455,977.4736,974.1296,970.4303,967.0655,964.2034,961.8949,958.3616,956.0646,953.0261,949.8893,947.2025,944.6332,942.5861,940.4098,937.8907,935.2397,932.9253,930.9776,928.1275,925.8676,923.9626,922.1314,921.6452,919.979,918.3127,916.1977,913.5463,911.5213,910.1434,907.9965,907.2536,906.1079,905.5447,904.422,903.6206,902.5378,902.6423,901.2177,900.3295,899.6453,899.2042,898.0468,897.7161,896.0813,895.7677,894.8549,893.8804,892.1539,890.8977,889.5992,889.4456,888.977,888.0103,887.2336,886.5696,885.2811,884.1443,883.617,882.574,881.6053,880.5573,880.5045,878.8815,875.3289,874.0734,873.5488,873.2641,872.6766,872.4748,873.1091,870.489,869.3922,868.7256,868.6085,871.366,872.796,874.4668,875.2814,876.0852,877.4354,878.7767,879.9105,880.3848,880.8423,
1737.093,1695.315,1681.81,1678.085,1671.453,1651.615,1636.098,1631.943,1626.332,1620.117,1611.344,1592.4,1583.018,1578.9,1574.914,1570.81,1566.843,1561.96,1558.291,1554.122,1547.241,1544.991,1542.078,1538.806,1535.197,1532.39,1529.047,1525.783,1517.619,1510.705,1503.398,1496.749,1490.905,1485.056,1480.289,1469.685,1464.351,1463.707,1461.997,1455.973,1446.827,1440.25,1434.409,1428.913,1424.836,1418.621,1414.197,1410.561,1406.204,1398.155,1392.462,1388.806,1385.015,1381.604,1378.036,1374.628,1368.989,1362.207,1358.689,1355.7,1352.947,1351.95,1351.277,1348.878,1347.051,1345.542,1342.765,1340.422,1338.2,1335.655,1334.942,1335.031,1335.714,1335.146,1331.343,1327.62,1321.65,1314.427,1310.119,1305.283,1299.908,1295.609,1291.781,1287.303,1283.917,1279.718,1274.229,1270.71,1264.931,1258.327,1253.202,1248.221,1244.122,1240.562,1235.636,1228.455,1221.814,1217.352,1211.565,1206.39,1199.254,1192.417,1185.091,1174.8,1168.326,1164.915,1158.22,1148.442,1144.598,1140.281,1132.335,1129.173,1125.089,1117.832,1111.642,1105.317,1100.934,1096.597,1091.83,1083.104,1078.175,1073.172,1068.819,1064.033,1060.841,1057.299,1053.994,1049.581,1043.902,1037.562,1029.652,1025.068,1022.183,1019.027,1015.907,1013.113,1009.553,1006.287,1002.95,999.3276,997.1188,994.1217,991.6172,988.9249,986.0103,981.2058,976.2156,972.6046,969.5092,966.4061,962.846,960.6105,956.8088,953.6938,950.9573,948.1187,945.6917,942.6265,940.1811,937.7549,935.4802,934.1819,932.519,930.5067,927.4958,924.8539,922.8203,921.3475,920.9665,919.3276,917.689,915.8947,913.4837,910.9395,909.643,908.5003,907.9682,906.4802,905.7354,905.2415,904.6951,903.8271,903.1573,902.998,901.9565,901.1509,900.0464,898.8593,898.0304,897.0896,897.2523,896.0557,895.2379,892.6597,890.8981,890.5211,890.4409,890.0903,889.7356,888.5367,886.8566,885.8664,885.0695,884.2663,883.0942,882.2415,881.0206,881.0854,879.8744,876.9356,875.3759,874.637,874.8179,874.0432,872.5213,872.8951,871.4467,870.6609,870.5629,871.7814,874.3279,876.2775,878.1428,879.8774,880.4832,880.6427,882.4938,882.8007,883.8,883.8403,
1728.776,1695.681,1679.897,1674.542,1670.004,1650.514,1633.73,1629.214,1623.335,1618.378,1610.968,1589.908,1581.757,1577.85,1573.655,1569.712,1565.577,1561.172,1556.855,1551.83,1545.38,1542.551,1539.641,1536.058,1532.497,1529.536,1525.723,1522.186,1515.71,1508.848,1500.846,1494.584,1488.802,1483.095,1478.284,1469.825,1461.944,1458.447,1456.017,1450.837,1444.353,1439.267,1433.941,1427.998,1424.042,1417.846,1413.25,1409.026,1404.113,1397.662,1392.444,1388.929,1385.503,1381.306,1377.212,1373.245,1368.912,1364.586,1361.227,1358.93,1357.813,1356.665,1354.527,1352.962,1351.221,1350.612,1348.671,1345.705,1343.156,1342.225,1341.237,1340.913,1340.345,1337.999,1335.333,1329.246,1323.245,1316.811,1310.918,1305.101,1300.203,1294.829,1290.863,1286.339,1282.822,1278.489,1274.477,1269.977,1266.073,1257.791,1252.931,1247.774,1243.268,1238.864,1235.529,1228.789,1221.094,1215.057,1209.989,1203.776,1197.284,1191.789,1184.479,1174.982,1162.851,1162.169,1160.468,1150.243,1144.625,1140.127,1135.155,1130.664,1126.881,1120.004,1114.194,1110.881,1107.34,1105.278,1100.575,1088.294,1079.316,1075.406,1069.675,1065.559,1062.651,1059.246,1055.289,1051.807,1045.016,1037.838,1028.407,1021.654,1018.822,1015.866,1013.871,1010.663,1007.583,1004.604,1001.321,998.2675,995.4299,991.1882,988.2053,983.1138,982.0016,977.679,974.1884,970.8641,969.3286,966.008,962.6981,959.7728,956.3593,952.7234,950.186,947.0052,944.2859,941.3036,939.3293,936.9717,934.6949,933.3937,931.7216,930.1266,927.3914,924.6583,923.0226,921.6522,920.4168,919.1169,917.3196,915.2554,912.0515,910.9341,909.9811,909.0016,908.5193,907.2972,906.5641,906.0146,905.4257,904.6165,903.7839,903.8262,903.488,902.3334,901.196,899.6949,899.1487,898.4803,897.7097,897.4012,895.2513,892.8822,892.1913,891.9418,891.7443,891.1334,890.3015,889.0482,887.4964,886.3101,885.0461,884.3593,883.3715,882.2866,881.7601,881.6773,880.6689,878.6203,876.652,875.648,876.1808,875.8516,874.5815,872.9246,872.1859,873.5164,873.45,875.4188,877.9623,879.3653,881.2955,882.517,884.3969,884.7416,884.8644,886.4649,886.5206,887.255,
1723.916,1699.472,1683.689,1677.399,1667.072,1660.294,1631.944,1626.369,1621.448,1617.979,1614.206,1588.131,1583.72,1579.09,1574.35,1569.13,1565.233,1560.566,1555.663,1551.329,1543.504,1540.946,1537.766,1534.719,1531.06,1527.659,1524.248,1519.053,1511.488,1505.412,1498.75,1492.232,1487.415,1481.186,1475.677,1468.575,1460.22,1456.567,1453.243,1447.842,1443.199,1438.069,1432.769,1426.761,1421.517,1416.731,1411.797,1408.212,1403.411,1396.911,1392.673,1389.101,1384.757,1380.173,1376.929,1374.085,1370.378,1367.057,1364.254,1364.808,1363.358,1361.961,1358.711,1356.694,1355.605,1354.977,1354.183,1352.057,1351.041,1347.833,1346.184,1345.413,1344.322,1342.025,1337.87,1332.461,1323.883,1318.3,1310.857,1305.964,1301.006,1294.683,1290.497,1285.836,1281.528,1277.435,1273.117,1268.994,1263.531,1258.266,1253.356,1249.15,1243.481,1238.966,1234.375,1225.995,1220.09,1217.213,1210.783,1203.057,1193.445,1191.708,1175.41,1165.929,1157.537,1157.804,1157.124,1148.54,1143.589,1141.114,1133.264,1128.079,1124.168,1120.829,1115.695,1111.165,1108.163,1104.795,1102.628,1092.097,1082.992,1076.079,1070.562,1067.208,1063.781,1060.702,1055.469,1050.229,1044.912,1037.765,1030.276,1020.692,1015.862,1012.898,1010.439,1007.749,1005.4,1002.458,998.8761,996.6219,992.8066,989.0369,985.1314,981.3657,979.4973,976.4602,972.8295,970.2064,968.2814,966.5214,962.5169,959.1729,957.4686,953.0926,949.6763,946.0551,943.2262,940.8785,938.6938,937.663,934.3342,932.8659,930.498,928.96,926.9788,925.3376,924.0523,922.7474,920.4149,919.0778,917.3838,915.3679,913.4735,911.0654,910.6888,910.1141,909.6828,909.0129,908.4634,907.3881,906.2305,905.2225,904.2371,904.5084,904.4427,903.4406,902.1523,900.9521,900.5739,899.9998,899.2021,897.2141,896.0792,895.9343,894.7026,894.2256,892.8924,892.3855,891.1345,889.5936,888.2025,887.6039,886.4891,884.1911,883.4814,882.7518,882.3981,882.0924,881.8083,880.0917,878.2604,876.5886,876.5073,876.4809,874.4998,875.3208,874.8661,876.0671,877.1888,878.0657,880.6617,883.083,885.075,885.9724,886.8031,887.0728,888.0055,889.4622,888.9697,892.6799,
1733.213,1705.603,1688.128,1680.526,1666.382,1657.995,1635.049,1628.799,1624.919,1621.428,1619.459,1607.114,1588.072,1582.442,1579.411,1574.697,1570.269,1564.741,1558.578,1551.069,1542.44,1538.242,1535.243,1532.309,1528.973,1525.922,1522.166,1519.377,1509.505,1503.316,1496.747,1489.9,1485,1479.443,1473.726,1467.512,1460.124,1453.743,1451.5,1445.743,1440.609,1435.511,1430.006,1424.828,1419.797,1414.811,1409.754,1405.499,1401.138,1396.95,1393.344,1390.31,1387.046,1383.36,1380.94,1379.934,1373.868,1372.006,1370.896,1370.209,1368.964,1368.301,1363.369,1361.585,1360.609,1359.546,1357.976,1356.852,1356.872,1355.744,1356.249,1353.812,1350.424,1345.299,1340.895,1337.234,1328.744,1321.812,1312.961,1307.551,1301.465,1295.583,1290.801,1287.008,1282.464,1278.934,1275.007,1270.984,1264.682,1261.233,1258.083,1254.929,1245.621,1239.595,1234.359,1228.311,1223.149,1219.072,1212.288,1198.659,1187.129,1177.558,1162.435,1151.828,1153.365,1153.962,1154.066,1139.303,1139.258,1138.941,1131.817,1125.038,1120.103,1116.66,1114.635,1110.937,1107.758,1103.466,1100.828,1094.981,1085.089,1077.342,1071.517,1068.279,1064.405,1060.142,1054.217,1048.31,1042.888,1035.987,1029.172,1022.261,1015.644,1010.999,1007.239,1004.336,1002.54,1000.402,997.3639,993.9265,991.1896,987.9186,984.0782,980.2732,977.4066,975.0049,971.8777,968.978,966.2551,964.0336,961.3424,958.8813,956.7803,952.9318,949.4717,945.5215,943.6228,940.6672,938.7831,937.3791,935.3048,932.8484,930.7131,928.7682,927.3262,926.0753,924.2805,922.3211,919.9797,919.2311,918.6025,917.7303,916.9587,914.579,911.6886,911.0508,910.5876,910.1011,910.048,909.0408,906.9199,905.8184,905.3972,905.4775,905.1177,904.6245,903.4302,902.4672,902.4133,900.5693,899.1458,898.4698,898.1836,897.3088,895.7217,895.2346,893.7375,893.2999,891.9408,890.9163,889.495,888.1161,886.8853,886.1635,884.7389,883.9236,883.9476,883.8422,883.4498,881.517,879.8148,878.1285,876.7793,876.7332,877.1945,878.8124,877.7331,877.3415,878.7161,880.7309,883.2687,885.9166,887.4471,889.4291,890.0582,889.9588,890.6332,892.0024,893.0828,895.6875,
1728.073,1701.852,1688.319,1674.466,1664.222,1652.877,1643.875,1634.068,1630.444,1626.352,1620.482,1611.867,1593.279,1588.611,1584.462,1580.376,1574.995,1569.625,1564.182,1558.498,1546.591,1536.288,1532.471,1529.716,1526.747,1523.49,1519.95,1516.459,1511.919,1506.483,1499.796,1488.464,1483.835,1477.345,1470.158,1465.487,1459.515,1451.813,1448.229,1443.759,1439.02,1434.338,1428.524,1422.93,1418.295,1414.208,1409.503,1406.14,1403.053,1400.267,1396.752,1394.187,1390.374,1388.299,1386.672,1383.709,1380.09,1378.456,1377.447,1374.143,1373.359,1373.245,1368.98,1366.831,1365.326,1363.572,1362.409,1361.286,1361.514,1361.267,1360.718,1360.477,1355.53,1349.045,1344.612,1343.529,1339.822,1335.407,1317.464,1307.743,1302.895,1297.092,1292.528,1288.367,1284.265,1280.807,1276.815,1272.278,1267.786,1264.85,1261.376,1256.4,1248.825,1244.342,1234.519,1228.705,1225.088,1220.83,1213.132,1197.773,1183.862,1174.392,1160.356,1149.051,1148.137,1149.542,1144.279,1131.648,1135.047,1134.2,1124.297,1121.826,1113.957,1112.999,1112.007,1110.172,1106.815,1102.429,1099.253,1093.869,1086.745,1079.104,1071.668,1068.094,1064.405,1058.7,1052.68,1046.682,1041.036,1033.693,1026.986,1022.715,1016.184,1011.921,1006.91,1002.164,999.5458,997.4219,995.4171,992.8615,989.2138,985.6697,982.7411,978.7053,975.6462,973.679,970.9906,967.6885,964.3478,962.7754,959.2022,957.1037,953.8309,950.6467,948.5375,945.6304,942.9469,940.3611,937.762,936.9647,935.6691,932.9535,930.8186,928.8156,927.6011,926.321,924.1557,922.0097,921.1581,919.7584,919.7634,918.98,918.3389,916.5336,912.7808,912.791,912.2711,911.2104,910.6057,910.1314,908.6517,907.8737,906.8073,906.5644,905.8397,905.834,904.7134,903.395,901.5252,901.1,900.6085,899.708,898.3804,897.2045,896.9811,896.1916,895.2056,894.062,892.5148,891.5046,889.8767,888.4433,889.0573,887.6855,887.3023,886.4714,885.553,885.1134,884.7442,882.7323,879.8698,878.6677,877.9594,877.8438,880.0587,882.3237,879.5851,880.2314,881.6284,883.8779,887.131,888.8457,891.1597,893.3864,893.8178,893.8477,894.5974,895.0771,896.5167,898.0912,
1735.798,1704.862,1687.663,1667.84,1660.814,1653.069,1643.813,1636.532,1633.728,1625.571,1620.155,1613.604,1597.067,1592.255,1590.168,1586.733,1581.885,1575.47,1570.007,1564.13,1552.217,1535.061,1531.657,1527.585,1524.363,1520.773,1517.74,1514.658,1510.573,1505.515,1498.226,1489.431,1481.113,1476.407,1469.339,1462.781,1455.558,1449.321,1446.627,1442.615,1438.018,1432.37,1427.906,1422.674,1419.183,1415.221,1412.423,1409.893,1406.601,1403.175,1400.482,1397.63,1395.572,1393.1,1390.231,1388.704,1385.819,1383.364,1380.163,1378.561,1378.172,1377.232,1374.545,1371.405,1370.071,1367.765,1366.968,1365.952,1365.996,1365.651,1364.837,1363.065,1359.226,1353.417,1349.064,1347.752,1346.932,1343.808,1329.326,1311.506,1302.971,1298.319,1294.194,1290.218,1286.403,1281.714,1277.76,1273.871,1270.116,1266.492,1262.365,1258.52,1251.157,1237.359,1226.543,1224.338,1222.543,1218.354,1207.278,1189.24,1173.129,1168.745,1156.576,1144.204,1143.601,1136.545,1129.501,1127.071,1127.483,1123.668,1120.229,1117.45,1109.242,1107.763,1108.328,1107.511,1104.389,1100.438,1096.51,1089.516,1082.314,1075.387,1070.628,1068.48,1061.809,1056.004,1050.43,1045.52,1038.934,1030.728,1024.152,1019.911,1016.413,1012.577,1007.216,999.7988,996.7079,994.5371,991.7043,989.6052,986.6316,983.9116,981.3326,978.0851,974.2914,972.2228,969.9303,966.7953,963.8176,961.2424,958.0626,955.2768,952.501,950.5512,949.4869,944.681,942.8386,941.0794,938.2673,936.7028,935.0964,934.1031,930.7667,929.1606,928.3371,926.3817,923.934,922.9498,921.845,920.8267,919.7056,918.6626,917.7056,915.7835,913.8049,913.9573,913.2495,911.9883,911.4322,911.1536,909.8884,909.5444,908.4466,907.9908,906.76,906.4119,905.6705,903.1818,903.058,902.6543,900.5106,899.7546,898.9523,898.9304,898.4718,897.325,896.2647,895.4649,893.3306,892.3692,890.6533,890.5298,890.7413,890.1977,889.2872,888.3639,887.4442,886.3056,885.4772,883.7296,881.3221,880.4218,879.2704,879.8973,882.6777,883.9412,881.485,883.0332,884.8479,887.1924,889.4908,891.768,894.4562,895.6025,896.3345,897.027,898.16,898.6563,899.597,900.8816,
1745.475,1716.225,1686.961,1664.812,1658.812,1650.667,1643.504,1637.273,1632.197,1624.473,1617.488,1613.02,1602.087,1598.227,1595.808,1592.339,1588.888,1582.836,1575.616,1570.694,1559.995,1534.86,1530.042,1526.624,1523.374,1520.551,1516.821,1513.485,1508.977,1505.198,1497.753,1491.942,1481.419,1476.325,1468.737,1461.752,1454.341,1449.363,1445.989,1442.31,1437.454,1432.481,1428.675,1426.528,1421.869,1418.569,1415.326,1412.236,1410.443,1408.849,1407.467,1403.428,1400.345,1396.008,1393.256,1391.514,1388.98,1386.376,1383.857,1382.618,1382.275,1380.074,1377.791,1375.854,1374.038,1372.135,1371.119,1370.208,1370.071,1370.054,1369.286,1367.095,1364.237,1359.849,1354.744,1352.044,1350.802,1348.387,1343.815,1325.509,1304.985,1299.497,1295.675,1291.314,1287.68,1283.345,1279.053,1275.616,1271.476,1268.233,1264.135,1253.056,1236.976,1225.372,1220.004,1217.966,1216.462,1209.268,1193.834,1180.48,1167.785,1159.683,1152.148,1139.14,1137.829,1132.24,1126.325,1122.653,1121.715,1119.949,1113.121,1107.935,1104.577,1102.23,1102.313,1102.967,1100.921,1098.341,1092.468,1085.797,1077.68,1070.912,1068.465,1065.141,1059.681,1054.652,1049.711,1044.644,1036.651,1028.363,1020.974,1016.039,1013.639,1011.56,1005.84,997.1312,994.5387,992.3437,989.2865,986.7272,983.8201,981.3817,979.771,976.8522,973.3702,970.8333,967.8463,965.8712,963.6351,960.161,956.762,954.1777,951.7004,949.7814,949.0856,945.7609,944.2788,942.137,940.6099,938.4285,936.6944,934.7017,931.7629,930.2092,928.2686,925.4757,924.7667,923.9689,922.3544,922.0305,920.248,919.051,917.7667,916.8057,916.0121,915.0737,914.5687,913.2467,912.1926,911.7918,911.5959,910.7792,909.886,909.1531,907.8661,906.8761,906.548,905.5032,904.1,902.8609,901.5829,900.5961,900.1597,900.0709,899.2393,898.0059,897.2088,896.069,895.0175,892.6083,892.624,891.8273,891.7318,891.7468,891.296,890.3224,889.3768,888.3126,885.8484,884.2753,882.825,882.8681,882.7564,882.2429,884.3371,885.7158,883.5823,885.5923,887.6805,889.2981,891.5953,894.1421,897.1326,898.5017,899.8409,900.1067,901.8964,901.9423,902.9361,904.1183,
1756.479,1728.211,1699.05,1669.245,1657.107,1649.82,1642.575,1638.166,1632.443,1621.127,1614.722,1610.424,1606.198,1602.831,1601.453,1598.251,1595.128,1589.728,1582.051,1575.759,1569.13,1539.052,1528.951,1525.959,1522.946,1519.771,1516.363,1512.863,1508.932,1504.673,1500.466,1491.845,1482.942,1476.971,1469.805,1463.099,1454.167,1450.025,1446.48,1442.475,1438.056,1435.02,1430.782,1427.357,1424.63,1420.887,1418.078,1416.23,1415.241,1414.854,1415.417,1414.53,1411.125,1403.551,1397.432,1394.909,1392.397,1390.412,1390.248,1389.305,1386.024,1383.559,1381.687,1379.939,1378.767,1376.617,1376.209,1374.993,1373.784,1373.538,1372.693,1371.182,1367.987,1364.414,1358.756,1356.48,1353.544,1350.963,1344.824,1330.236,1304.529,1300.691,1297.104,1292.813,1288.998,1284.98,1280.513,1276.829,1273.039,1269.073,1264.564,1251.21,1228.797,1219.489,1216.985,1213.406,1210.696,1204.543,1194.529,1175.538,1163.702,1139.44,1135.768,1132.572,1132.698,1128.079,1120.176,1118.959,1116.757,1113.758,1109.672,1106.186,1101.811,1098.763,1096.329,1094.781,1093.421,1093.092,1088.254,1081.953,1073.041,1067.557,1064.219,1060.707,1057.946,1053.929,1048.975,1042.676,1034.453,1028.867,1020.07,1011.686,1008.814,1005.995,1001.653,993.8427,992.4674,990.7631,988.178,984.7527,981.6367,979.424,977.4818,974.8312,972.8034,969.7108,965.9601,964.3784,960.5848,959.1284,956.8563,954.3517,952.3798,950.7872,949.6583,948.0258,946.0804,943.4938,941.7408,940.6079,937.6074,934.5625,932.4434,930.8572,928.5909,927.5444,925.6748,925.1704,924.1233,923.1221,920.8644,920.3172,919.1336,918.231,916.6723,916.0064,915.7336,914.6933,913.1903,912.316,911.712,911.0505,910.7418,910.4853,909.0752,907.6147,906.8494,906.0798,905.0009,903.518,902.362,901.9443,901.4538,900.7355,899.7202,898.666,898.0715,897.0951,894.6965,893.9163,893.5012,892.7435,892.5989,893.353,893.349,892.2866,890.659,888.6187,887.7435,884.5505,885.1445,884.8435,884.9259,886.3795,888.0781,888.1724,886.9545,889.0229,890.5408,892.2656,894.1228,896.5851,898.8405,900.806,902.4462,903.1392,904.6343,905.1685,906.4739,907.8568,
1762.558,1736.521,1700.682,1668.874,1653.221,1646.364,1640.248,1636.091,1628.327,1622.803,1619.334,1614.542,1610.833,1606.621,1605.699,1603.848,1600.847,1595.94,1589.627,1581.902,1573.532,1550.518,1529.029,1525.834,1521.439,1518.545,1515.75,1511.993,1508.55,1504.418,1499.06,1491.656,1484.836,1478.413,1470.185,1462.824,1456.183,1450.924,1447.577,1443.722,1439.844,1436.181,1433.315,1429.789,1426.354,1423.343,1421.53,1419.934,1420.167,1420.584,1421.491,1421.506,1414.41,1405.102,1400.622,1398.408,1396.438,1395.045,1395.724,1394.423,1390.58,1387.729,1386.271,1385.086,1383.482,1381.49,1380.559,1379.026,1378.187,1377.422,1376.998,1375.98,1371.876,1368.621,1362.379,1360.075,1357.166,1353.244,1347.6,1336.911,1306.346,1301.541,1297.812,1294.02,1289.905,1286.158,1281.568,1277.803,1275.015,1263.296,1253.904,1242.714,1224.676,1218.211,1214.635,1210.157,1205.529,1203.64,1194.668,1180.561,1157.663,1136.514,1131.64,1129.421,1126.212,1122.548,1118.278,1115.362,1113.505,1110.523,1106.709,1103.602,1099.213,1095.311,1092.599,1088.625,1085.952,1085.918,1081.167,1074.783,1068.714,1065.611,1061.469,1058.263,1055.274,1052.532,1048.311,1041.39,1037.371,1032.156,1020.596,1009.243,1003.388,999.9854,996.5258,993.3322,992.2513,990.1769,987.261,983.1288,980.8887,978.6425,976.577,974.278,972.4523,969.1564,965.7549,963.9703,962.0531,959.7814,957.5038,955.1058,953.4756,952.3657,950.6734,948.2288,947.0596,945.1348,942.8771,940.6021,937.9952,935.7228,933.4382,931.4917,930.3958,930.3003,928.5121,926.4664,924.9972,924.1038,921.8873,921.5565,920.4363,919.7739,918.1704,917.228,917.3536,917.2813,914.9301,913.6634,913.3422,912.4758,912.2252,911.3506,908.9627,908.109,907.7771,905.9233,905.1819,904.082,903.8171,902.913,902.0635,901.142,899.9356,899.1387,898.2392,897.49,896.1279,895.3688,894.5931,893.9356,894.2818,894.6869,894.8102,893.3954,891.6077,889.5322,885.6782,885.8246,886.374,885.689,886.7858,888.0677,889.5024,890.6521,889.1212,891.2252,893.5101,895.6689,896.8531,899.6347,901.9232,903.702,905.3662,906.6146,908.2836,908.7611,910.3446,911.7344,
1764.862,1724.99,1686.54,1661.192,1651.88,1647.066,1644.107,1638.188,1633.22,1629.687,1627.221,1620.894,1616.011,1612.26,1609.644,1607.75,1605.189,1601,1595.68,1590.313,1581.285,1556.516,1537.788,1529.14,1522.909,1517.376,1513.957,1510,1506.435,1502.538,1498.44,1493.035,1485.251,1478.893,1469.382,1461.874,1456.459,1452.422,1448.991,1445.717,1442.301,1438.923,1435.617,1431.846,1429.146,1427.832,1426.099,1426.457,1427.495,1428.682,1426.24,1425.052,1418.039,1408.055,1404.038,1402.082,1401.785,1404.211,1404.088,1401.1,1397.658,1394.624,1392.231,1391.848,1390.727,1386.064,1384.553,1383.511,1382.29,1381.982,1381.182,1379.722,1376.403,1371.897,1364.883,1362.205,1358.35,1354.986,1351.667,1343.13,1316.348,1303.258,1298.485,1294.107,1290.197,1286.844,1282.076,1278.054,1268.988,1256.628,1239.571,1236.169,1221.889,1215.274,1212.424,1207.724,1202.674,1199.899,1192.122,1175.154,1158.357,1136.792,1129.411,1127.037,1123.908,1120.652,1116.625,1112.855,1110.762,1108.443,1104.645,1100.667,1097.048,1093.719,1090.802,1086.882,1084.429,1079.561,1076.214,1071.595,1068.054,1063.363,1059.122,1055.603,1052.293,1049.441,1047.187,1044.364,1041.158,1035.759,1023.107,1009.602,1003.69,1000.942,997.845,995.0325,992.9047,990.3243,987.4698,984.0461,981.3566,978.2007,975.2798,973.8848,971.4863,968.8047,966.2988,964.9229,963.376,960.8739,957.9474,955.5963,954.0189,953.3964,951.0876,948.9175,946.8982,946.8209,944.2358,941.8703,938.9393,935.7195,934.0276,933.1702,932.2249,932.0975,931.7031,928.5149,926.1039,924.6039,923.8957,922.2276,921.5461,921.0038,919.9214,918.3151,918.6089,918.616,916.9745,915.3013,914.3686,913.8914,913.6366,911.6068,909.207,908.7689,908.5276,907.3547,906.2556,905.5511,904.8131,903.9484,902.986,901.1303,900.2653,899.3797,899.0193,898.4589,897.8811,897.0173,896.2846,895.932,895.9547,895.5272,895.2573,894.2905,892.0233,889.6917,886.4846,887.2648,886.7877,886.7983,888.9527,890.996,892.6114,893.017,891.2789,893.5931,895.9515,898.1658,900.0418,901.7205,904.3715,906.1829,908.5232,910.0859,911.292,913.1181,914.5967,915.5233,
1765.827,1728.126,1677.261,1657.726,1654.83,1655.169,1647.232,1644.197,1640.862,1637.295,1633.102,1628.65,1622.832,1618.585,1614.496,1611.761,1608.522,1606.432,1600.841,1595.412,1586.839,1570.49,1543.755,1537.57,1528.278,1520.697,1512.968,1509.46,1505.519,1500.786,1496.93,1493.147,1488.215,1477.923,1467.945,1461.927,1458.458,1455.535,1452.662,1449.624,1446.049,1442.972,1438.494,1435.085,1435.377,1434.61,1436.091,1434.822,1438.269,1437.428,1433.646,1428.844,1422.238,1414.836,1408.319,1407.643,1406.932,1408.025,1408.072,1405.207,1402.497,1399.561,1397.724,1397.935,1395.712,1393.122,1393.43,1396.06,1386.642,1386.3,1386.03,1381.819,1376.794,1371.942,1367.943,1363.549,1359.62,1356.122,1351.407,1344.93,1319.642,1303.534,1299.257,1294.844,1290.964,1285.275,1279.839,1269.328,1255.504,1250.813,1234.199,1223.307,1216.497,1210.875,1207.608,1203.243,1195.535,1194.859,1178.651,1160.395,1150.09,1136.88,1128.928,1126.069,1122.669,1119.044,1115.31,1111.483,1108.429,1105.819,1102.513,1098.902,1094.837,1091.211,1088.151,1084.899,1081.677,1077.667,1073.069,1069.523,1066.543,1061.49,1057.851,1053.674,1048.683,1046.131,1044.218,1041.662,1036.801,1031.57,1021.466,1013.295,1005.943,1002.846,999.561,996.2678,994.2625,990.6277,987.5093,985.1017,981.9604,979.1495,976.2313,973.9291,971.3947,968.532,966.6256,964.918,962.4437,960.0226,958.7308,956.2347,954.5831,953.8055,952.175,950.2598,949.0354,947.5612,945.4106,943.0443,939.8798,936.6314,935.7956,934.1265,933.1594,931.9855,931.5209,930.7186,928.2275,926.0491,924.7729,923.2622,922.5137,922.4109,922.3202,919.9908,920.0896,919.5169,917.8844,916.9028,915.2233,915.2489,914.722,912.1089,909.7194,909.2932,909.4742,908.2321,907.2045,906.5041,905.6737,905.2281,903.5276,902.46,901.5288,900.1829,899.7672,899.6614,898.5048,897.9444,897.8916,897.423,896.9304,896.4437,896.4366,894.7875,893.0141,892.1224,888.6248,887.7607,887.644,888.3591,890.1546,892.6695,895.1663,894.8873,893.7217,895.6564,897.7301,900.1451,902.5316,904.7338,906.9337,908.722,910.606,912.6129,915.1975,916.6677,917.5928,919.357,
1767.813,1737.589,1681.088,1662.053,1659.327,1659.229,1655.291,1649.635,1645.298,1644.241,1640.654,1634.366,1630.925,1626.554,1621.371,1616.745,1614.368,1609.503,1604.425,1600.832,1596.11,1587.048,1563.181,1546.818,1538.977,1526.165,1515.926,1511.93,1508.938,1502.231,1496.262,1491.007,1485.715,1479.391,1472.805,1463.714,1461.226,1459.041,1455.626,1452.271,1449.929,1445.368,1441.38,1439.394,1439.497,1439.699,1440.51,1441.451,1442.133,1442.102,1441.231,1435.962,1428.824,1421.276,1415.252,1415.234,1413.848,1414.761,1413.188,1409.534,1406.467,1405.139,1404.662,1403.817,1402.098,1401.41,1403.811,1403.609,1397.023,1391.764,1389.823,1383.356,1377.482,1373.547,1369.664,1364.959,1361.131,1355.861,1349.249,1337.712,1310.907,1305.167,1300.114,1295.679,1291.286,1286.738,1282.291,1273.587,1251.251,1241.551,1228.863,1217.705,1213.561,1209.75,1204.699,1198.597,1193.066,1185.767,1178.876,1162.829,1154.224,1133.927,1129.238,1125.631,1122.04,1118.532,1114.948,1110.233,1106.964,1103.408,1100.766,1097.321,1093.899,1089.691,1085.407,1081.952,1078.446,1075.192,1072.525,1068.442,1064.96,1059.42,1055.871,1049.892,1045.386,1043.536,1040.609,1035.93,1030.327,1023.296,1017.408,1011.058,1007.809,1004.557,1001.159,998.0287,995.6185,991.399,988.6992,985.6403,982.9236,980.6105,978.5143,975.5355,971.4358,968.8521,966.6549,964.801,963.6782,961.4643,959.0416,956.7373,955.1362,954.5427,953.4938,951.6312,950.2267,948.0996,946.3704,944.1362,941.0956,938.5285,936.5829,935.2525,932.6879,931.879,931.1588,930.7191,930.1941,928.1755,925.7233,924.2438,923.8685,923.479,923.1418,922.275,921.4011,920.2216,918.8925,917.4128,916.3789,916.2582,915.8321,913.4087,911.1622,910.0577,910.0027,909.5056,907.8916,907.5195,907.1553,905.8716,904.1031,903.5266,902.8525,901.5663,900.9587,899.8782,899.3549,898.8401,898.4532,898.1677,898.6877,897.7698,896.9718,895.3035,892.8719,893.0794,891.4697,888.6393,889.8641,891.0812,893.214,895.4335,897.3583,897.1708,896.2017,898.2732,899.9769,902.3784,904.9785,907.0294,909.7972,911.6589,913.5376,916.8714,918.581,921.3173,921.4813,922.6605,
1770.199,1762.515,1695.324,1671.847,1665.349,1662.174,1660.328,1657.636,1653.592,1649.692,1644.791,1640.739,1636.879,1633.213,1628.9,1622.147,1618.613,1615.417,1610.707,1606.551,1601.427,1593.216,1577.286,1554.486,1544.866,1535.615,1520.401,1516.112,1513.191,1508.358,1500.6,1492.234,1486.975,1479.902,1471.837,1466.597,1464.617,1460.834,1458.433,1456.037,1453.049,1449.973,1447.462,1443.204,1443.403,1443.603,1443.995,1444.118,1445.46,1446.373,1445.595,1444.466,1433.957,1427.941,1421.181,1418.804,1419.082,1418.693,1415.501,1413.194,1411.433,1411.53,1412.845,1409.54,1410.704,1409.04,1409.737,1407.674,1402.041,1396.893,1392.95,1385.904,1379.171,1375.231,1371.044,1364.511,1359.032,1353.816,1346.268,1331.44,1312.664,1305.414,1300.642,1296.493,1292.108,1287.319,1280.812,1270.122,1250.599,1239.621,1226.449,1217.507,1212.473,1208.429,1204.188,1189.88,1189.45,1183.38,1174.175,1158.991,1150.271,1132.744,1128.657,1124.976,1121.235,1117.664,1114.528,1110.015,1105.816,1102.474,1099.145,1095.379,1092.281,1088.477,1084.219,1080.607,1076.38,1072.675,1070.823,1067.839,1063.534,1055.982,1052.252,1047.179,1043.623,1040.413,1036.135,1030.907,1025.184,1020.254,1016.594,1012.785,1009.598,1006.308,1002.848,999.7434,997.1836,992.7668,989.7441,986.7602,984.4972,981.8694,980.4141,977.4799,973.7025,969.5317,967.1314,965.2854,964.7751,962.3913,959.9648,958.3704,957.157,955.7337,954.8015,953.2589,950.8149,948.9176,946.639,944.8704,942.2427,940.2802,938.0131,934.0162,932.9185,933.1843,931.9003,931.0612,929.9453,929.4287,928.3265,926.2407,925.1002,924.5912,923.9485,923.15,921.642,921.0197,920.226,918.8678,918.1023,917.5972,916.7097,915.6747,915.8861,913.5081,910.5908,910.7368,909.2757,908.4998,908.234,906.9202,905.267,904.1874,903.6126,902.8581,901.554,900.4437,900.1953,899.3822,900.1835,900.2355,899.4035,898.8798,897.6807,895.4448,895.1731,894.7399,893.2195,890.6289,890.715,893.0687,895.7905,897.9442,899.6611,900.4033,898.1459,900.4099,902.4281,904.2863,906.3497,909.0098,912.3294,914.1027,917.3843,919.4243,921.8506,923.52,924.9343,926.8821,
1772.716,1768.99,1719.146,1683.17,1677.039,1671.577,1665.572,1660.931,1657.975,1654.199,1649.537,1645.032,1641.987,1638.67,1635.023,1630.633,1626.496,1620.447,1617.47,1614.741,1611.076,1600.287,1581.644,1563.491,1552.494,1541.37,1526.083,1521.311,1517.18,1509.769,1501.7,1490.916,1484.946,1479.726,1472.82,1469.826,1467.487,1464.433,1462.212,1458.566,1455.627,1453.152,1449.945,1448.654,1447.323,1447.109,1447.428,1448.047,1448.133,1448.695,1449.256,1449.457,1445.676,1436.083,1431.273,1426.705,1423.054,1421.503,1418.927,1416.882,1416.088,1415.694,1417.519,1418.138,1417.831,1416.021,1413.162,1410.93,1406.259,1401.21,1396.572,1389.686,1382.674,1377.139,1371.202,1364.993,1358.803,1353.055,1343.691,1325.709,1310.795,1305.745,1301.298,1297.366,1292.717,1288.285,1280.171,1268.543,1245.239,1236.095,1230.207,1222.219,1213.135,1206.626,1202.497,1194.885,1182.643,1179.286,1169.672,1157.108,1149.906,1133.922,1129.181,1124.409,1120.265,1116.297,1112.94,1109.617,1105.499,1101.826,1097.182,1093.161,1090.061,1086.605,1083.881,1080.427,1076.243,1072.436,1067.461,1061.738,1057.306,1052.962,1050.074,1046.285,1042.809,1038.659,1034.334,1029.853,1025.915,1021.364,1017.414,1014.17,1011.014,1007.443,1004.373,1001.002,997.6964,993.7855,991.0125,987.7087,984.762,982.6179,981.2161,979.5082,975.9626,970.9865,967.4216,966.4429,965.3185,963.8715,961.7878,959.9315,958.2999,957.3721,955.8083,954.2768,952.2207,949.2894,946.6442,944.7111,943.6343,940.5131,938.0975,933.8783,933.7408,934.6661,933.5528,932.4081,931.3344,929.7899,928.7087,928.2059,927.1818,925.9831,925.2992,924.0184,923.2864,922.6575,921.9978,921.1216,919.9673,919.0039,917.4706,917.0938,917.753,915.6669,912.4606,911.2759,911.2059,909.9562,908.7284,906.9273,906.3491,905.5474,904.8656,903.7817,902.6641,901.301,900.7475,900.2465,901.4424,901.1965,900.1428,899.1263,898.3756,896.1611,895.905,895.4069,893.6903,892.662,892.5086,896.4502,899.1892,899.7243,900.7924,901.7148,900.637,902.7202,904.3558,906.4261,908.615,911.2798,913.8002,916.1437,920.1846,921.6264,925.1236,926.3501,927.4779,928.7766,
1774.512,1772.196,1727.909,1691.259,1685.807,1676.692,1674.287,1669.03,1661.243,1657.682,1655.148,1649.252,1646.407,1642.679,1639.689,1637.131,1632.041,1627.696,1625.091,1620.897,1615.874,1607.453,1587.531,1571.091,1561.103,1554.511,1542.985,1533.337,1520.165,1510.177,1502.001,1490.108,1483.021,1478.92,1475.539,1473.093,1470.341,1467.839,1464.86,1461.463,1458.887,1455.831,1454.497,1452.8,1452.432,1451.32,1450.992,1451.806,1451.877,1452.144,1453.034,1453.072,1447.515,1440.368,1436.702,1432.57,1426.864,1424.503,1423.358,1421.391,1420.401,1421.034,1421.693,1421.735,1421.352,1419.845,1417.832,1414.945,1411.781,1406.424,1399.305,1392.468,1383.208,1379.258,1372.112,1365.79,1359.346,1353.504,1340.979,1320.024,1310.876,1305.655,1301.048,1296.358,1291.909,1288.473,1284.258,1266.735,1245.206,1235.476,1227.354,1224.532,1213.361,1203.922,1197.469,1188.651,1180.914,1169.994,1163.937,1159.984,1152.493,1140.91,1131.941,1126.674,1118.906,1115.063,1111.64,1108.377,1104.623,1102.769,1097.524,1093.447,1089.226,1085.836,1083.904,1080.711,1075.381,1071.55,1066.065,1061.313,1057.056,1053.233,1049.327,1045.711,1041.348,1038.099,1034.026,1030.539,1026.8,1023.88,1018.876,1016.075,1012.195,1008.958,1005.379,1002.828,999.0421,994.9721,992.2259,988.7018,986.1526,983.2242,981.7679,979.4433,977.4373,973.5679,969.5244,968.2423,966.3202,964.2565,963.0151,961.3002,959.3791,957.9717,956.5147,954.5905,953.7775,951.2094,948.8137,946.3943,944.3105,941.9702,939.1464,935.4586,934.1969,934.7312,934.7331,934.511,933.6096,931.9803,930.0912,928.5762,927.7796,927.0015,926.3131,925.6337,924.9533,924.3765,923.7876,923.1334,922.3241,921.473,918.8118,918.8973,918.4916,917.201,915.2225,913.1766,912.1172,911.4418,909.0255,908.4041,907.3106,906.2551,905.6185,904.7207,903.4801,902.2597,901.5922,901.187,902.4574,902.1071,900.5649,899.4508,899.33,899.4641,898.3672,895.5617,894.5791,892.7042,893.6909,898.5941,901.4166,901.9536,903.0184,903.5189,902.2554,904.6127,906.0225,908.8284,910.8395,913.3049,916.2399,918.7709,923.0383,923.9368,927.0284,929.1963,930.1688,931.7043,
1774.159,1771.778,1737.803,1697.756,1692.509,1685.704,1681.063,1673.688,1666.913,1661.845,1658.562,1654.209,1650.752,1647.167,1643.927,1641.644,1637.625,1634.086,1629.818,1626.205,1619.702,1613.823,1594.86,1573.694,1566.604,1560.134,1554.132,1535.245,1526.646,1514.111,1503.526,1492.111,1484.025,1481.223,1478.951,1476.278,1473.615,1471.128,1467.794,1464.944,1461.956,1459.468,1457.958,1456.973,1456.243,1456.088,1455.239,1455.052,1455.484,1456.108,1457.056,1456.221,1450.44,1442.822,1441.226,1436.979,1431.059,1429.455,1428.12,1426.206,1424.909,1424.799,1425.121,1425.075,1424.32,1423.074,1420.811,1419.233,1416.135,1411.044,1404.046,1395.988,1388.235,1379.209,1374.396,1366.32,1357.604,1350.941,1339.812,1322.504,1311.186,1304.438,1299.568,1294.295,1290.244,1286.385,1282.489,1266.978,1247.125,1242.728,1223.088,1214.572,1207.478,1201.365,1196.852,1189.874,1181.036,1170.929,1164.851,1160.759,1154.869,1147.763,1142.156,1128.142,1121.696,1117.674,1110.09,1106.565,1103.517,1099.605,1096.342,1092.517,1088.952,1085.471,1082.185,1079.803,1074.547,1070.708,1065.827,1061.279,1057.867,1052.74,1049.187,1045.444,1041.982,1038.179,1034.299,1030.47,1027.119,1023.559,1019.965,1016.789,1013.024,1010,1006.899,1003.994,1001.109,996.6163,993.7004,990.1774,987.3832,984.5623,982.1373,979.1454,976.7491,974.5086,971.3366,969.9434,967.7303,965.1865,963.6146,962.0764,960.2883,958.3384,956.2811,954.2553,953.3454,951.7844,949.7708,947.4886,945.6838,942.6459,940.3058,938.7585,936.3182,934.9664,935.3895,936.1223,935.1943,933.6197,932.0378,930.8154,929.5955,928.8129,927.86,926.3405,925.1359,924.6918,924.0439,923.5626,922.7198,922.1058,921.1117,920.1347,919.3407,918.8419,917.1475,914.8676,912.6406,912.9628,911.4053,909.7206,907.9108,907.1054,906.1836,905.3695,904.2262,904.2407,907.1444,906.0289,904.1486,903.0466,901.0604,899.842,900.916,899.5894,897.6301,896.2677,895.0096,897.338,897.6019,900.6519,902.7031,904.3438,905.2039,905.7955,904.7525,907.4214,907.701,910.4128,913.1752,915.2222,918.4045,921.562,924.665,927.3159,929.7767,931.246,932.4704,934.8829,
1772.859,1768.402,1741.263,1704.385,1698.341,1691.433,1686.464,1678.929,1674.201,1670.212,1664.232,1660.371,1655.411,1652.242,1647.69,1645.381,1642.889,1639.676,1634.866,1630.488,1623.104,1615.482,1600.963,1587.016,1573.35,1562.983,1550.533,1538.705,1527.259,1516.092,1506.162,1493.622,1487.502,1485.008,1482.439,1479.625,1477.067,1474.302,1471.265,1467.376,1465.037,1463.231,1462.037,1460.968,1460.254,1459.632,1459.533,1458.89,1459.525,1460.26,1460.472,1460.085,1457.894,1452.049,1445.598,1440.895,1437.458,1434.328,1432.629,1430.793,1428.828,1429.399,1428.859,1428.578,1428.207,1425.725,1422.04,1420.013,1416.162,1411.966,1407.618,1400.876,1386.359,1379.576,1373.012,1362.525,1355.391,1346.1,1335.444,1326.808,1315.733,1304.238,1296.703,1292.561,1287.979,1283.444,1278.332,1269.977,1254.238,1248.152,1220.472,1209.405,1205.161,1199.061,1194.195,1189.486,1182.996,1174.392,1165.35,1161.404,1156.419,1150.112,1144.339,1134.5,1126.523,1120.085,1109.326,1106.115,1102.582,1098.726,1095.343,1091.912,1088.219,1084.279,1080.447,1077.138,1073.686,1070.022,1065.793,1061.801,1057.733,1053.021,1049.547,1045.484,1042.39,1038.865,1034.729,1031.486,1027.521,1024.172,1020.509,1017.152,1014.57,1010.77,1008.031,1004.853,1000.888,998.3226,993.9285,990.7377,988.4275,986.2604,982.5695,979.8179,977.4498,974.7187,972.0306,971.5004,967.1848,964.6993,963.6476,962.5416,960.5177,958.4769,956.8235,954.16,952.5231,951.1641,949.9705,948.3277,946.7043,943.5775,940.8437,940.1973,939.6823,936.7027,935.7966,936.5397,936.8467,935.6058,934.1028,933.1485,931.97,931.2324,930.4369,928.5658,927.0195,925.6727,925.2526,924.9587,923.7653,922.2169,921.4172,920.8707,919.8954,918.8133,917.5306,915.6672,914.0817,913.4766,912.3255,909.8911,908.5751,907.7352,907.2782,906.3388,906.0632,906.847,909.934,910.9498,906.8525,903.971,901.2804,900.8289,900.4972,900.2581,898.7516,896.9624,897.1786,898.0252,900.4257,901.9442,904.9794,906.514,906.9366,908.1583,907.1498,909.5812,910.6037,912.1773,915.7619,917.9355,920.9175,923.9766,926.8079,928.9463,931.9727,933.7781,935.1251,937.8889,
1773.729,1768.079,1736.656,1712.112,1707.63,1698.963,1695.238,1688.697,1680.197,1676.426,1672.909,1667.273,1662.668,1655.215,1651.848,1649.803,1647.695,1644.533,1640.173,1636.211,1627.313,1618.041,1604.129,1590.641,1578.792,1566.456,1555.858,1541.959,1528.443,1518.519,1507.583,1496.053,1490.889,1488.636,1485.813,1482.909,1479.971,1477.918,1474.228,1470.714,1468.721,1466.694,1465.842,1465.373,1464.589,1464.005,1463.78,1464.042,1464.259,1464.893,1465.303,1464.621,1462.686,1459.349,1452.27,1447.594,1444.519,1439.917,1436.941,1434.785,1433.822,1433.169,1432.836,1433.2,1430.335,1426.892,1422.604,1418.792,1415.561,1411.875,1408.059,1403.325,1389.295,1379.645,1370.909,1360.579,1351.031,1341.996,1335.108,1328.414,1312.869,1299.124,1293.985,1289.828,1285.023,1280.297,1275.68,1270.647,1263.168,1254.187,1216.04,1206.982,1202.01,1197.246,1192.759,1188.839,1183.779,1178.472,1169.625,1162.313,1157.267,1152.165,1146.263,1138.465,1127.268,1123.355,1109.584,1106.421,1103.223,1098.935,1095.505,1092.073,1087.476,1083.943,1080.015,1076.874,1072.71,1069.255,1065.561,1061.561,1057.72,1053.398,1049.932,1046.243,1042.607,1039.384,1036.025,1031.864,1028.142,1024.845,1021.427,1018.12,1014.84,1011.877,1008.164,1005.208,1001.862,998.1865,994.5198,990.964,988.1384,985.6752,983.5037,981.0267,978.7535,978.298,973.3021,971.2672,968.2492,965.0236,964.2805,962.838,960.8716,959.0367,958.4399,956.2952,953.0535,950.7935,950.3193,949.4079,947.0171,944.8444,942.3743,941.5958,940.9153,939.6351,937.0385,936.8633,937.4445,937.4167,936.7876,935.7765,934.7032,933.7742,932.5023,930.8058,929.0682,926.5096,925.9694,925.5444,924.6569,923.2977,921.6717,920.6539,920.0301,919.2282,918.4102,916.7548,915.3719,914.2806,914.2221,911.3937,909.6477,908.9618,908.6479,908.9628,908.8596,909.2936,911.3112,911.6783,909.6179,906.1241,902.6076,901.2725,900.9715,900.6539,900.3131,899.4229,898.0617,899.6983,901.1178,903.4101,907.2247,908.9945,909.1373,910.2965,909.4909,911.5598,912.16,913.9833,917.4365,920.3117,922.651,925.5416,928.3392,930.8104,934.0866,935.9325,937.3792,940.5604,
1775.436,1769.453,1734.748,1717.021,1712.507,1707.031,1701.329,1699.271,1692.896,1681.953,1678.253,1673.742,1667.687,1663.198,1656.857,1653.426,1652.361,1649.612,1646.835,1638.435,1627.955,1617.375,1604.344,1591.848,1578.917,1570.074,1557.319,1544.025,1528.947,1519.826,1507.735,1499.198,1494.8,1491.654,1488.865,1486.425,1483.244,1480.441,1477.161,1476.181,1473.266,1470.585,1469.762,1470.01,1469.425,1468.177,1467.803,1470.26,1470.332,1469.202,1469.501,1469.021,1465.582,1462.325,1457.332,1452.435,1448.177,1443.681,1440.64,1439.266,1437.505,1433.777,1434.415,1431.94,1430.509,1427.558,1422.213,1418.337,1414.782,1411.056,1407.821,1403.43,1389.763,1379.458,1369.315,1360.16,1351.995,1344.971,1340.636,1316.826,1302.844,1297.445,1292.741,1288.119,1283.018,1277.38,1273.339,1268.745,1263.586,1239.796,1213.98,1204.564,1198.018,1194.824,1191.216,1187.307,1183.009,1177.38,1170.545,1164.123,1158.703,1152.864,1148.289,1135.538,1126.823,1122.798,1108.975,1105.837,1103.106,1099.229,1096.193,1091.949,1088.221,1084.872,1080.514,1077.112,1073.129,1069.28,1065.731,1062.442,1059.491,1054.403,1049.849,1046.118,1042.606,1038.838,1035.457,1031.912,1028.586,1024.915,1021.741,1018.187,1014.346,1011.711,1008.398,1005.476,1001.886,998.3062,995.2104,991.9302,989.2974,986.9058,984.9901,981.9051,981.502,977.8832,974.6608,971.5941,968.2878,967.3356,966.8177,964.6213,961.2629,960.2487,958.6646,956.6078,954.0041,951.7532,950.7929,950.7879,948.4465,946.2584,944.6896,943.2231,942.37,940.6272,939.1553,937.2876,937.5657,938.3817,938.5396,937.8737,936.7698,935.5098,934.0056,932.3809,930.1675,927.8362,927.4473,926.8878,925.9809,924.1133,922.1525,920.6984,919.8962,919.4597,918.9072,917.6821,916.2184,914.9363,914.8367,914.3704,911.0984,910.1494,910.0306,911.5958,911.8438,911.1808,911.5074,911.3682,910.283,907.3017,904.3738,902.5123,900.6129,901.8093,901.1461,901.1329,901.6242,903.3145,905.2065,905.0682,907.3242,910.3423,911.0021,912.717,912.7827,913.7832,914.3589,916.3216,919.1982,922.1604,924.2904,927.9189,930.6782,933.7991,935.9188,938.4588,940.841,942.5109,
1778.486,1776.565,1737.572,1721.872,1716.164,1711.4,1707.269,1705.461,1700.737,1694.937,1685.066,1679.67,1672.753,1669.228,1664.696,1660.57,1657.13,1653.887,1649.574,1639.136,1628.527,1615.179,1603.595,1592.336,1578.12,1570.545,1550.819,1535.224,1522.101,1513.907,1505.661,1500.784,1497.605,1494.811,1491.962,1490.119,1486.456,1482.997,1481.239,1480.906,1477.855,1475.283,1474.155,1474.103,1473.528,1471.536,1472.671,1474.27,1474.914,1473.443,1473.571,1472.864,1471.189,1466.823,1460.992,1455.565,1451.44,1447.448,1443.857,1441.651,1437.013,1432.283,1428.607,1427.428,1426.038,1425.849,1421.589,1417.658,1413.867,1410.693,1407.387,1402.349,1388.215,1378.863,1369.308,1360.044,1354.15,1349.565,1334.619,1307.919,1302.353,1297.295,1292.549,1288.216,1283.827,1280.37,1275.369,1268.346,1260.42,1231.203,1215.183,1202.421,1196.904,1192.167,1188.775,1184.534,1180.992,1176.091,1169.634,1163.605,1158.522,1151.446,1148.297,1138.279,1127.604,1122.614,1109.425,1105.991,1102.493,1099.275,1095.586,1091.979,1088.239,1084.717,1080.301,1077.04,1073.488,1069.302,1065.525,1061.315,1057.783,1054.448,1050.419,1046.39,1042.703,1039.186,1035.637,1032.397,1029.03,1025.267,1022.014,1018.561,1015.226,1012.47,1009.289,1006.095,1002.668,999.3363,996.7352,994.0046,992.0992,988.4205,985.9189,982.8276,981.0358,977.4761,974.421,972.4952,970.353,968.8328,967.1057,964.9711,963.5983,961.8318,960.0598,957.3896,954.9542,953.2745,951.6703,949.9333,948.8199,947.1997,945.2848,944.7065,943.5281,942.6168,942.1547,938.8256,938.0854,938.6397,939.2651,939.1115,938.3505,936.9862,935.8433,933.6284,931.4516,929.4648,928.9543,928.2545,927.2272,925.2413,923.9622,922.9609,920.3523,919.1566,918.8813,918.347,916.9918,916.0284,915.312,915.4138,912.8961,911.3381,912.1722,913.5522,913.3651,913.5027,912.39,911.7326,910.0245,908.8055,906.4068,904.6536,902.149,904.0918,906.5862,901.9921,902.7501,905.0939,907.5376,906.6858,908.4014,911.6784,913.272,914.8509,915.085,915.1834,916.9412,917.5987,920.6793,923.9088,926.507,930.1849,933.0696,936.0447,938.523,940.434,942.4818,944.1979,
1780.843,1777.581,1743.371,1725.428,1719.114,1715.317,1711.623,1708.893,1705.535,1702.704,1697.969,1687.292,1679.649,1676.654,1671.341,1666.885,1662.911,1657.459,1649.72,1639.133,1628.195,1614.501,1602.69,1594.025,1572.35,1566.292,1535.914,1524.823,1518.04,1510.92,1506.927,1503.297,1500.462,1498.154,1495.929,1492.822,1489.104,1486.516,1485.51,1485.236,1483.789,1482.035,1478.489,1478.72,1478.941,1478.197,1478.828,1480.014,1478.146,1477.165,1477.586,1477.325,1475.672,1471.698,1466.33,1457.049,1450.365,1445.126,1438.218,1433.441,1430.192,1427.065,1423.856,1422.536,1421.484,1421.594,1419.846,1416.646,1413.425,1409.719,1406.358,1402.102,1396.193,1381.943,1370.111,1362.272,1357.432,1353.329,1322.788,1308.667,1303.518,1299.704,1293.734,1289.845,1283.981,1278.868,1270.547,1262.23,1252.367,1227.974,1220.655,1208.24,1201.873,1193.081,1187.086,1183.221,1179.232,1174.194,1167.7,1162.918,1158.106,1150.879,1145.172,1136.411,1127.993,1123.432,1113.252,1106.358,1102.589,1098.977,1095.435,1091.861,1088.027,1084.43,1080.46,1077.635,1073.637,1069.745,1066.364,1062.391,1057.794,1053.974,1050.345,1046.73,1043.137,1039.646,1036.272,1032.581,1029.296,1025.906,1022.845,1019.56,1016.011,1013.633,1010.406,1007.251,1003.117,1000.207,997.623,994.3442,993.3118,989.2432,986.126,983.1838,980.8221,978.3049,975.5958,973.1053,971.2172,969.2377,968.0073,965.1459,963.053,962.0742,961.4019,959.6747,957.2956,954.4186,952.1554,950.4379,948.9876,947.6613,946.2866,945.5037,944.4943,944.7716,944.6071,943.5604,939.189,939.156,939.7834,939.8973,939.8745,938.8625,937.3221,934.9424,932.7073,931.0583,930.3205,929.5843,928.5665,926.6428,924.165,922.4171,922.205,920.2403,918.3803,917.976,917.5494,916.0872,915.9753,915.6213,914.1255,912.727,913.8755,914.6995,914.7379,913.6427,913.039,912.4469,911.0395,909.3415,906.691,904.8916,904.2816,906.0201,908.9417,907.908,907.511,909.6041,909.5696,909.4006,909.9479,912.8542,915.9142,918.0087,917.7436,917.6438,920.0716,920.0157,922.8751,926.4874,929.0282,932.988,934.3734,936.9198,940.5421,942.7475,944.6468,946.3399,
1783.851,1782.125,1755.472,1743.222,1723.51,1718.659,1715.264,1713.176,1709.74,1706.924,1703.176,1697.931,1689.169,1684.854,1676.938,1672.439,1667.916,1658.763,1646.475,1636.233,1627.191,1614.474,1602.855,1580.286,1568.432,1549.494,1534.079,1526.972,1522.079,1514.598,1508.705,1506.006,1504.309,1501.879,1498.829,1495.286,1492.464,1491.42,1490.627,1490.626,1492.121,1489.503,1485.963,1482.659,1484.959,1487.069,1489.651,1491.531,1491.637,1489.76,1486.893,1481.157,1477.768,1472.486,1461.167,1453.521,1442.452,1436.243,1428.948,1423.795,1420.043,1416.873,1415.605,1414.926,1415.734,1414.662,1414.248,1414.419,1411.827,1407.353,1404.164,1400.279,1393.815,1379.716,1370.539,1364.732,1359.515,1355.605,1325.431,1311.444,1306.364,1301.663,1295.175,1290.742,1279.923,1273.952,1265.51,1257.917,1253.557,1235.426,1229.955,1216.111,1204.246,1191.01,1186.074,1182.521,1178.237,1172.941,1166.375,1160.694,1156.171,1149.938,1146.377,1138.31,1127.864,1123.844,1119.987,1105.324,1102.052,1098.719,1094.996,1091.76,1088.074,1084.56,1080.761,1076.7,1073.264,1069.548,1065.852,1062.543,1058.371,1055.427,1050.729,1047.172,1043.576,1040.109,1036.653,1033.318,1029.425,1026.521,1023.127,1020.167,1017.179,1014.698,1010.973,1007.358,1003.756,1001.754,998.5364,994.6725,991.2114,988.0379,986.3854,983.8588,981.5387,978.8031,975.8665,975.2186,972.7697,970.4417,968.5177,965.5898,963.5914,961.4507,960.9426,959.934,958.8217,956.1944,953.663,952.4075,951.3647,949.6821,948.6181,948.9352,949.5302,946.8253,945.298,942.9324,939.9696,939.5386,940.1379,940.7659,940.9055,940.4534,938.8836,936.6384,933.8654,932.5098,931.6873,930.6001,928.3906,926.0447,924.8369,924.3534,923.4519,920.5757,918.2182,917.7085,917.1226,916.801,916.6767,915.2091,915.8515,914.9533,915.731,915.5992,915.67,916.3275,914.8859,913.8328,912.6057,909.7007,908.7823,906.957,905.8768,908.0625,909.1938,908.3723,909.816,910.7019,911.0004,911.6718,912.1188,913.8138,916.9592,919.6815,920.7807,920.8361,921.6856,922.5456,924.5283,927.9636,931.0076,933.487,936.483,939.3339,943.0754,945.7291,946.9014,949.4351,
1785.531,1783.748,1762.36,1747.398,1728.414,1722.409,1719.812,1717.303,1715.281,1712.879,1707.846,1702.172,1697.243,1688.691,1683.925,1678.225,1673.895,1658.953,1641.711,1635.915,1626.809,1614.309,1600.411,1573.751,1555.553,1545.823,1538.693,1532.868,1526.278,1518.744,1511.788,1509.327,1507.735,1504.879,1501.533,1499.48,1497.474,1496.227,1496.342,1496.525,1498.184,1497.004,1491.686,1488.08,1488.928,1495.62,1499.548,1501.666,1501.099,1499.578,1493.345,1489.408,1479.062,1467.037,1453.178,1446.301,1438.879,1428.704,1419.661,1414.017,1409.971,1407.923,1407.74,1406.572,1406.7,1407.713,1406.64,1410.631,1410.264,1407.521,1403.473,1398.81,1391.469,1376.78,1369.469,1365.513,1360.24,1354.99,1335.163,1321.84,1312.086,1304.465,1296.954,1291.383,1277.275,1269.352,1262.995,1258.095,1253.11,1239.696,1232.273,1217.052,1207.371,1190.746,1185.854,1181.677,1176.738,1171.204,1165.467,1159.474,1153.24,1148.385,1144.316,1139.575,1128.615,1124.421,1120.688,1106.89,1101.814,1098.587,1095.247,1091.829,1088.415,1084.565,1080.512,1077.084,1073.405,1069.803,1065.965,1062.423,1058.859,1055.714,1051.957,1047.746,1044.458,1040.699,1037.255,1033.541,1029.668,1026.996,1023.842,1021.037,1018.147,1014.861,1011.545,1007.875,1004.287,1002.012,998.9355,995.6743,992.0344,988.6693,987.2054,984.7193,981.7113,978.3776,977.7744,975.437,972.9598,971.1257,968.9257,966.4191,965.0648,962.3952,961.2115,960.1,959.543,957.909,954.961,953.8077,952.0701,950.9811,950.9641,949.9908,948.585,946.5908,945.4261,943.4032,941.693,940.0435,940.6332,941.2029,941.6243,941.3488,940.0903,937.9465,935.1437,933.8192,932.6315,931.2204,927.6667,927.386,927.0132,925.8083,924.4741,923.206,919.8225,919.9984,919.3062,917.8772,917.9934,917.8876,918.4102,918.1675,918.0784,917.5539,917.4915,918.5052,917.5939,915.4315,913.9836,911.5677,909.8088,907.436,907.4089,909.2305,909.7832,910.4074,911.3391,911.7914,911.3704,913.0839,913.4498,915.9603,918.5992,921.3121,923.0096,923.6315,924.5018,926.1392,926.4543,928.8088,933.5902,937.2763,939.3136,940.8439,943.1438,946.1392,948.7098,951.3469,
1787.003,1786.796,1777.215,1748.65,1736.389,1729.092,1727.048,1722.49,1719.809,1716.3,1710.684,1706.692,1700.407,1695.667,1690.702,1682.815,1673.709,1650.247,1641.781,1633.709,1626.362,1613.447,1595.232,1566.689,1553.918,1548.729,1544.529,1539.008,1532.149,1522.898,1515.395,1513.295,1510.734,1507.429,1504.706,1502.835,1502.708,1502.625,1503.494,1503.92,1504.043,1502.598,1500.381,1493.389,1493.2,1501.601,1505.868,1505.963,1505.404,1504.93,1500.561,1490.77,1485.413,1464.69,1445.152,1438.726,1433.945,1423.594,1413.839,1405.777,1401.488,1400.568,1399.55,1398.532,1397.93,1397.777,1399.381,1402.849,1405.272,1405.859,1404.105,1397.251,1392.839,1373.867,1366.804,1362.233,1358.542,1354.423,1342.604,1328.468,1316.045,1312.065,1303.225,1287.843,1277.851,1270.166,1261.239,1255.195,1248.199,1240.251,1236.576,1220.275,1209.587,1192.884,1184.359,1180.424,1174.457,1169.387,1164.614,1159.029,1151.247,1144.1,1139.865,1135.505,1128.109,1123.918,1121.501,1114.835,1102.468,1098.835,1095.569,1092.247,1088.538,1085.163,1081.485,1077.626,1074.57,1071.504,1067.274,1063.805,1060.406,1057.085,1053.51,1049.456,1045.139,1042.576,1038.463,1035.462,1030.975,1027.584,1024.712,1020.909,1017.905,1016.499,1012.432,1008.631,1004.879,1001.638,998.9927,996.2355,993.176,990.6674,988.5031,986.1372,983.9404,980.2579,979.1545,975.9079,973.9883,972.0699,970.2516,967.5212,965.3605,963.6874,962.4367,961.391,960.493,959.4779,956.883,954.5981,953.6116,952.6749,951.9359,950.1722,949.3178,947.6813,946.904,943.9991,942.5044,940.7762,940.9035,941.6262,941.989,942.1515,940.7756,938.9081,936.0504,935.0018,933.6161,931.8404,929.3806,929.7515,927.8979,926.5872,925.182,924.0975,923.3053,923.1368,922.6803,922.136,922.967,922.0759,921.0372,920.2054,920.0904,920.332,920.0287,919.3484,918.2763,916.0295,914.8236,913.6962,910.5049,910.3071,910.7789,910.2393,910.1984,912.1985,912.3968,913.5419,912.9792,914.2517,915.3972,916.8444,919.8567,923.2368,925.3751,926.9094,926.5566,928.9583,929.6253,930.8452,933.1713,939.0302,941.3821,941.8087,944.2156,946.9125,949.774,952.9352,
1788.353,1788.704,1783.731,1758.224,1749.241,1739.856,1735.931,1729.177,1724.936,1719.54,1715.932,1709.17,1705.894,1701.369,1695.285,1688.318,1674.933,1655.153,1644.939,1633.991,1619.363,1605.837,1572.768,1565.812,1557.914,1552.708,1549.797,1543.744,1535.385,1526.225,1519.169,1516.303,1513.69,1510.815,1508.216,1506.94,1506.186,1506.695,1507.985,1508.336,1508.926,1508.334,1507.554,1502.314,1497.636,1503.823,1516.935,1519.579,1515.636,1505.543,1498.612,1493.054,1479.943,1450.408,1437.344,1432.522,1427.459,1417.937,1407.012,1400.83,1396.578,1392.482,1389.688,1387.884,1383.323,1382.53,1384.17,1390.833,1399.112,1399.193,1397.445,1393.682,1381.491,1371.587,1365.281,1355.066,1352.235,1344.917,1341.282,1328.08,1310.703,1302.557,1295.756,1286.04,1276.822,1265.595,1256.919,1251.265,1247.87,1244.63,1238.134,1222.195,1209.795,1195.408,1186.612,1182.316,1173.801,1167.887,1162.493,1156.977,1149.205,1142.554,1136.269,1130.963,1125.759,1122.539,1119.577,1115.565,1108.067,1099.742,1096.78,1093.114,1089.556,1085.883,1082.243,1078.25,1074.743,1070.925,1067.351,1064.024,1060.155,1057.142,1053.14,1049.833,1045.632,1042.721,1039.092,1035.147,1031.88,1028.697,1025.579,1021.35,1018.778,1015.598,1013.012,1010.157,1006.576,1002.55,999.0624,995.8575,993.4277,991.0959,989.2658,987.3192,984.8853,982.4718,978.8006,976.8235,974.8846,972.7375,970.654,968.7883,965.8671,964.2621,963.6394,963.1006,962.1909,960.0372,958.0347,956.3301,955.3741,954.2634,952.0851,951.3735,950.4945,949.0275,946.9332,945.4553,943.6537,942.0928,941.3738,941.8066,942.7001,942.7868,941.6552,939.6781,936.9499,935.9719,934.5602,932.4513,931.2438,931.2395,928.3821,927.3196,926.0787,926.0705,926.0485,926.0562,925.3676,924.4772,923.8019,923.1945,922.2937,922.5158,922.7568,922.9107,922.4301,921.2436,919.9678,917.0742,916.5843,915.2817,913.7659,913.291,911.9813,911.556,912.6028,915.2258,914.4021,914.4589,914.8553,917.0598,918.7939,919.424,921.2632,924.5615,927.3858,928.9275,929.5162,930.5308,932.2324,932.6403,934.8735,936.7603,942.3687,944.5477,946.4568,948.4976,950.6661,954.1672,
1789.465,1790.384,1789.458,1776.442,1759.773,1749.355,1745.155,1738.983,1728.838,1723.587,1720.121,1713.873,1708.376,1703.923,1699.17,1692.074,1677.068,1656.557,1641.633,1613.821,1601.585,1577.311,1574.408,1570.058,1562.961,1557.7,1553.186,1545.35,1536.723,1528.195,1522.334,1519.059,1516.27,1513.96,1512.288,1511.192,1510.74,1510.897,1511.887,1512.728,1512.962,1513.02,1512.422,1508.084,1503.585,1506.273,1522.53,1523.319,1522.74,1504.803,1497.367,1489.197,1475.207,1443.374,1431.608,1424.318,1415.551,1411.657,1403.54,1398.658,1392.71,1387.469,1383.685,1379.441,1375.533,1372.437,1370.387,1375.897,1383.777,1384.46,1383.423,1377.954,1373.407,1369.192,1360.265,1333.536,1320.424,1315.138,1329.193,1313.389,1306.184,1298.505,1294.764,1283.961,1271.74,1262.889,1260.886,1256.973,1252.391,1249.586,1246.028,1226.427,1212.103,1203.234,1197.335,1186.838,1175.903,1168.182,1162.235,1153.652,1147.679,1141.523,1137.257,1131.752,1126.138,1122.712,1117.937,1114.771,1106.199,1100.189,1096.952,1093.757,1089.999,1086.053,1082.44,1078.627,1074.999,1071.365,1067.285,1064.008,1060.479,1057.018,1052.902,1049.188,1046.269,1042.488,1039.344,1035.923,1032.897,1029.127,1025.627,1022.639,1019.26,1015.664,1012.864,1011.322,1008.34,1004.324,1000.473,996.6827,993.9459,991.2038,989.4983,987.4465,984.6944,982.8364,980.5054,977.8654,975.5599,972.5319,970.2703,970.1578,969.6512,966.5211,965.4805,964.7316,963.4258,961.9304,959.6835,958.6704,956.6599,954.5228,953.4026,953.2612,951.524,949.6658,947.8179,946.3976,945.4498,943.5271,942.2307,942.3218,942.8585,943.092,942.2253,940.1379,938.0276,936.8748,935.5505,933.3597,932.9919,932.9758,930.3881,928.1737,927.9335,928.2587,928.5839,928.7683,928.0485,926.7025,925.0117,924.1505,924.3643,924.8647,925.6686,926.1748,925.1136,923.5468,922.2488,919.4741,918.3342,916.7032,915.8406,915.6087,913.826,913.0746,914.9475,916.7289,917.8854,917.2588,916.866,919.2014,921.5197,923.3137,924.255,926.4594,928.6486,931.0195,932.3651,932.1962,934.7651,936.9745,937.4379,938.195,940.9659,944.5744,946.8356,949.4671,952.1673,955.8172,
1791.107,1791.872,1791.74,1786.173,1765.3,1756.533,1750.032,1742.541,1732.647,1727.96,1724.822,1720.962,1715.086,1707.812,1703.037,1696.769,1683.759,1658.264,1643.2,1597.823,1587.591,1582.965,1577.976,1573.766,1569.09,1562.806,1557.63,1547.32,1538.305,1528.811,1524.611,1521.412,1518.492,1516.445,1515.564,1515.336,1515.625,1516.002,1516.057,1517.317,1519.014,1520.4,1516.647,1515.667,1514.437,1517.631,1525.047,1523.904,1515.497,1502.969,1495.607,1485.572,1461.751,1436.744,1424.836,1417.839,1408.249,1404.252,1398.225,1394.518,1389.73,1385.029,1382.43,1378.611,1372.746,1368.193,1366.167,1366.676,1371.961,1378.213,1379.103,1374.372,1369.493,1362.549,1337.759,1317.054,1312.693,1308.031,1302.971,1301.488,1300.802,1295.303,1292.593,1287.739,1276.856,1269.535,1266.5,1261.426,1257.363,1254.852,1249.158,1227.945,1215.311,1206.257,1197.936,1189.491,1178.516,1170.923,1165.19,1155.681,1146.831,1141.068,1136.633,1132.588,1126.091,1120.963,1116.361,1111.847,1105.357,1100.36,1096.737,1093.579,1089.747,1086.2,1082.457,1078.901,1075.271,1071.601,1067.811,1063.764,1060.162,1056.389,1052.838,1049.452,1046.364,1042.709,1040.137,1036.543,1033.528,1029.396,1026.162,1023.503,1019.889,1016.079,1012.372,1009.954,1007.337,1004.39,1000.721,998.5266,994.6445,991.3138,989.7824,987.5124,985.3614,983.3312,980.88,978.7475,976.7623,974.9117,972.6105,972.7816,971.5374,968.8188,967.3831,966.1456,964.7132,963.642,961.7808,960.4589,957.7468,956.4317,955.162,954.466,952.5536,950.8264,949.268,947.7172,947.1017,944.9424,943.7045,942.9707,943.1999,943.8279,943.0881,941.2106,939.071,938.0775,937.0808,934.6479,933.1332,931.9497,930.9645,930.7053,930.3977,930.3619,930.8452,930.4587,930.068,929.2861,927.6247,926.2443,926.2529,927.1257,927.9753,928.3513,926.6363,924.9243,923.2772,921.7541,919.7137,918.5198,918.528,917.1685,916.8651,917.3974,917.765,918.2523,918.7011,918.7438,918.8549,921.3818,925.1642,926.716,927.4465,928.629,930.3729,933.3533,934.9575,934.8638,935.7888,938.9384,940.6619,942.1761,943.6731,945.8271,948.0214,950.4844,952.743,956.1989,
1792.394,1793.674,1793.942,1788.456,1771.739,1765.989,1758.01,1747.027,1737.579,1732.437,1728.631,1726.985,1719.723,1712.733,1707.802,1701.368,1689.64,1661.641,1637.746,1603.144,1594.054,1587.226,1581.644,1577.592,1573.478,1566.98,1560.822,1550.811,1538.929,1529.753,1525.81,1522.047,1519.344,1517.401,1516.776,1517.115,1516.688,1516.525,1518.931,1521.186,1522.204,1523.303,1523.827,1520.278,1515.066,1515.731,1522.067,1519.384,1508.002,1501.32,1492.703,1479.856,1452.872,1434.331,1419.785,1410.631,1405.246,1400.482,1395.179,1391.162,1387.538,1383.792,1379.943,1375.847,1369.081,1365.11,1360.31,1361.875,1364.248,1362.668,1363.103,1366.661,1361.428,1349.443,1329.859,1310.397,1307.723,1303.688,1299.763,1296.656,1293.488,1290.902,1289.655,1286.024,1281.797,1276.578,1272.133,1267.249,1262.189,1259.634,1253.23,1241.032,1218.328,1210.977,1204.956,1190.752,1182.504,1176.351,1173.868,1160.122,1148.177,1144.901,1140.654,1133.636,1128.764,1125.349,1118.605,1110.215,1104.677,1101.407,1097.736,1094.091,1089.965,1086.133,1082.444,1078.6,1074.926,1071.317,1067.671,1064.071,1060.979,1056.992,1053.115,1049.301,1045.648,1042.405,1038.69,1035.999,1032.887,1029.518,1026.369,1023.782,1020.329,1016.963,1013.522,1010.13,1008.202,1005.955,1002.978,999.2271,995.3801,992.0247,990.2166,988.8777,987.4171,985.0913,982.0193,978.7706,977.3441,976.742,975.2903,974.4766,971.9168,970.1595,968.8466,967.6777,966.5938,964.954,962.4783,960.4313,958.639,958.0236,956.4285,955.2461,953.7242,952.9809,951.2461,949.4305,947.5684,946.7881,945.5839,944.3732,943.7202,944.0455,944.0274,941.9901,940.4079,939.3626,938.3706,936.5859,934.4085,932.8154,933.4858,934.3381,934.3332,933.8793,932.9459,932.5328,932.0999,931.3723,929.8095,928.3171,929.4039,929.6758,930.005,929.9713,928.3984,926.8643,924.7149,923.2668,921.4527,920.04,918.9833,918.7722,917.9031,918.7612,920.5297,919.9409,920.1691,920.1851,920.3592,923.2292,926.125,929.0934,929.6943,931.4578,932.9197,935.1068,937.5599,938.4099,937.9066,940.6956,943.5095,944.5322,945.6008,948.292,950.2939,953.0396,955.2817,958.1405,
1793.198,1794.781,1795.48,1793.556,1772.404,1766.49,1760.082,1752.286,1743.795,1737.781,1734.215,1731.442,1727.02,1719.643,1713.239,1701.101,1683.745,1662.843,1638.027,1608.993,1599.395,1592.582,1586.646,1581.529,1577.326,1571.217,1565.874,1555.85,1541.277,1535.207,1527.69,1521.416,1516.921,1514.722,1513.942,1514.737,1514.984,1515.492,1518.422,1522.176,1524.74,1526.315,1524.554,1520.641,1513.118,1512.773,1507.556,1507.059,1504.637,1497.764,1490.211,1463.627,1446.288,1429.77,1412.736,1405.162,1400.893,1397.591,1393.354,1390.139,1386.145,1381.717,1376.195,1369.262,1364.169,1360.259,1355.222,1357.231,1356.219,1343.805,1340.102,1341.519,1331.399,1318.292,1310.855,1307.803,1305.116,1300.758,1297.231,1293.815,1291.923,1287.664,1285.426,1281.73,1279.906,1276.615,1272.813,1269.644,1265.901,1261.7,1250.331,1240.267,1227.278,1214.783,1210.936,1194.558,1189.771,1183.77,1180.104,1167.652,1154.65,1149.759,1146.995,1142.855,1132.802,1129.929,1120.793,1111.939,1102.596,1099.391,1096.368,1092.926,1089.405,1085.513,1081.69,1078.213,1074.562,1070.83,1067.395,1064.662,1060.607,1057.072,1053.656,1049.947,1046.464,1041.577,1038.819,1035.027,1032.067,1029.176,1026.251,1022.972,1020.649,1016.972,1013.317,1010.955,1009.12,1007.185,1004.046,1000.4,997.7268,994.512,992.5688,990.726,989.241,986.5167,984.8278,981.0109,979.3592,977.6041,976.5004,975.975,973.6513,971.6345,970.3707,969.2471,967.5749,964.845,963.1273,962.1288,961.0254,959.7493,957.9437,956.4802,954.6754,954.3466,952.3784,950.4383,948.7318,947.7708,946.7282,945.8694,944.6786,944.3895,944.7532,943.2935,941.2442,940.6034,939.355,937.4982,936.1066,936.2213,936.4288,936.8109,937.4042,936.1062,935.4241,935.0374,934.5974,933.1495,931.9747,931.431,932.2424,932.0029,931.9178,932.0328,929.3983,927.7917,925.7644,923.873,922.2161,920.6368,919.7305,919.9563,919.8586,919.0243,921.6408,922.058,921.862,922.0834,922.5763,924.3624,927.6594,930.5865,931.9523,932.9654,934.1477,937.7095,939.5636,941.5461,942.3453,942.7346,946.1266,948.9152,949.219,950.4169,952.6542,954.9209,956.8425,959.2935,
1794.793,1795.765,1796.806,1796.595,1787.872,1768.158,1760.965,1753.518,1746.393,1742.138,1738.582,1734.984,1731.675,1723.504,1715.808,1698.362,1674.109,1664.239,1645.562,1612.263,1606.112,1598.87,1592.596,1586.573,1581.833,1575.094,1570.775,1566.319,1556.436,1546.618,1538.866,1524.375,1514.677,1510.81,1509.441,1509.98,1510.379,1510.731,1513.661,1522.245,1524.985,1524.67,1518.849,1519.643,1515.092,1509.963,1496.917,1502.557,1502.077,1494.297,1481.74,1465.51,1453.281,1428.541,1407.219,1401.956,1397.972,1394.324,1390.261,1387.353,1383.156,1377.675,1370.073,1363.763,1358.434,1356.867,1350.255,1347.37,1343.704,1332.637,1322.626,1319.529,1318.451,1311.314,1307.979,1304.894,1302.417,1298.327,1295.057,1290.871,1287.006,1284.286,1282.554,1279.315,1276.848,1273.984,1270.875,1268.131,1265.924,1261.251,1250.103,1242.34,1232.031,1222.764,1214.065,1201.278,1194.325,1187.578,1184.475,1179.667,1167.286,1155.741,1153.915,1147.584,1136.618,1131.319,1120.733,1113.573,1105.351,1096.543,1094.117,1091.152,1087.949,1084.597,1080.74,1077.052,1073.612,1070.179,1066.876,1063.35,1060.484,1056.61,1053.506,1051.254,1046.182,1042.311,1038.776,1035.406,1032.265,1029.095,1026.482,1023.42,1021.206,1019.3,1014.876,1011.009,1009.167,1007.675,1004.625,1002.663,1000.632,997.7028,994.9446,992.7397,990.9022,989.2128,986.8927,983.8027,981.1321,979.7029,978.6705,977.5851,976.3697,974.482,972.3901,972.105,969.1908,966.5098,966.352,964.3787,962.5187,960.9139,959.5855,959.5068,956.2277,955.1146,953.2761,951.3303,950.0064,948.9799,948.0135,947.3711,945.7516,944.9481,945.3267,944.5409,943.3188,941.8988,940.041,939.8859,938.6981,938.8571,938.9064,939.0035,939.5874,938.3599,937.9011,937.4003,936.2504,935.3725,934.5425,934.6216,934.6487,934.3915,933.9437,933.1542,931.1893,930.2398,925.9683,923.8019,922.2366,921.0949,920.4147,920.8434,923.6712,922.954,921.9601,923.369,923.2941,923.2149,924.057,925.83,929.6365,932.2803,934.1411,935.2385,936.3423,938.5949,941.2254,943.4473,945.4986,947.2239,948.1663,950.8969,952.3266,953.0721,953.9811,957.3056,958.6973,961.1027,
1796.016,1797.143,1798.425,1798.823,1795.7,1785.184,1771.218,1759.241,1751.405,1746.503,1742.087,1738.821,1736.883,1725.462,1714.225,1698.608,1671.415,1662.402,1642.062,1616.658,1609.514,1603.714,1596.724,1590.892,1584.336,1579.312,1575.363,1570.879,1560.429,1550.298,1539.123,1528.309,1515.057,1508.156,1504.683,1505.421,1505.549,1504.569,1507.172,1519.608,1521.727,1517.393,1511.672,1513.476,1508.157,1500.129,1492.152,1491.277,1493.046,1490.823,1480.109,1467.498,1444.38,1419.969,1404.634,1400.281,1396.833,1392.916,1388.713,1381.641,1375.383,1368.079,1362.535,1353.024,1346.22,1346.018,1338.525,1337.848,1332.077,1326.538,1322.412,1318.158,1313.747,1310.186,1306.539,1302.115,1299.542,1295.642,1291.789,1287.866,1283.327,1280.522,1277.568,1275.972,1273.08,1269.979,1266.098,1264.064,1259.981,1256.009,1250.038,1241.552,1232.765,1224.824,1216.605,1209.045,1198.422,1191.616,1187.376,1180.811,1174.594,1167.109,1155.531,1148.277,1139.282,1130.783,1121.093,1114.843,1109.082,1095.708,1091.324,1088.854,1085.698,1082.727,1079.619,1075.976,1072.721,1069.352,1066.085,1062.792,1059.409,1055.849,1053.145,1050.161,1046.646,1042.717,1039.642,1035.273,1031.909,1028.576,1026.477,1023.702,1021.71,1019.438,1015.5,1011.287,1009.612,1008.142,1006.302,1003.941,1001.966,999.2352,997.1873,995.1703,992.5103,990.5939,988.1092,985.5321,985.2535,982.7822,981.0803,980.0117,978.3138,975.904,974.3775,972.4247,970.8329,969.0875,967.4289,965.2048,964.0642,962.3689,961.4487,960.4587,957.4782,955.7946,953.9921,952.3517,951.1292,949.9422,948.9199,947.8801,945.8652,945.3035,945.9066,945.5367,944.1788,943.2464,942.1925,941.9033,941.2999,941.3928,941.5677,941.6119,941.2968,940.6886,940.039,939.3498,938.3668,938.22,936.8748,936.4297,936.2689,936.5893,935.8693,934.282,932.2457,930.5906,926.3305,925.7447,922.6912,923.1644,921.8578,922.063,922.4253,924.2685,924.3122,924.6506,924.4604,924.5818,926.6342,927.7982,930.111,933.5716,936.5157,937.1401,937.8552,940.1391,943.114,945.7726,947.7959,949.5758,949.8575,952.9562,955.825,956.6167,957.0267,958.6723,960.4078,962.6446,
1797.084,1798.37,1799.751,1800.04,1796.796,1790.448,1778.257,1765.886,1759.607,1752.276,1747.822,1742.438,1737.202,1730.227,1718.945,1696.882,1668.214,1657.083,1634.487,1624.268,1617.342,1609.733,1603.088,1595.598,1589.578,1581.424,1576.617,1571.958,1562.204,1551.885,1538.792,1527.521,1516.11,1507.464,1501.191,1499.275,1499.37,1501.244,1507.028,1511.294,1515.794,1512.305,1504.668,1506.329,1497.171,1484.372,1479.052,1483.713,1485.58,1485.656,1477.912,1468.525,1451.207,1428.411,1405.269,1398.988,1395.32,1391.494,1387.556,1379.517,1371.99,1366.599,1360.087,1349.984,1340.959,1338.116,1335.032,1332.742,1329.322,1325.234,1321.038,1317.117,1313.041,1308.852,1303.939,1300.279,1296.774,1292.926,1289.329,1286.11,1281.5,1277.44,1273.872,1272.153,1269.538,1265.384,1260.693,1258.202,1254.637,1251.231,1244.959,1239.604,1232.603,1228.065,1220.329,1212.441,1204.99,1197.553,1191.958,1182.759,1175.082,1166.443,1157.188,1147.547,1141.176,1135.923,1122.68,1115.388,1108.94,1097.034,1089.633,1085.84,1083.918,1080.44,1077.673,1074.855,1071.487,1068.816,1065.542,1062.009,1058.837,1055.678,1052.587,1049.532,1046.436,1043.156,1039.364,1035.378,1032.044,1028.935,1026.794,1024.837,1022.117,1020.387,1016.902,1012.164,1010.973,1008.681,1006.793,1004.831,1002.757,1000.533,998.172,995.7664,993.9479,991.994,990.3447,987.8847,986.7801,984.4901,982.7793,981.9124,980.5538,978.1469,975.2468,973.3608,972.0428,970.842,969.0088,967.2951,965.8533,963.921,961.6956,960.6022,958.7822,956.5216,954.9702,953.5753,952.2706,950.9529,949.4754,948.4373,946.8174,946.7238,946.4326,946.4538,945.4626,944.8394,943.9967,944.1683,943.7496,943.5671,943.5267,943.6653,942.9691,942.7332,942.1131,941.6594,941.4821,940.441,939.4744,939.1706,938.7227,937.6537,936.4669,935.5852,931.5342,928.7167,927.0186,925.9893,924.8182,923.7783,923.069,922.6917,923.6976,924.0348,926.1579,925.7115,925.7968,926.4572,929.3647,930.1713,931.5518,934.862,938.1973,939.7214,940.9861,942.3261,944.3085,947.0247,949.6667,952.304,954.61,954.3292,956.7604,959.9226,960.6849,961.7579,962.733,965.2402,
1799.053,1800.471,1801.396,1801.572,1799.327,1793.857,1780.152,1769.207,1764.75,1755.797,1744.089,1738.103,1727.955,1716.842,1687.022,1668.03,1664.646,1643.08,1632.954,1627.965,1621.54,1618.531,1606.097,1600.446,1592.79,1587.347,1581.528,1572.703,1563.141,1553.632,1539.48,1525.666,1516.939,1506.053,1499.517,1495.275,1493.733,1498.882,1500.466,1499.728,1503.528,1505.482,1498.067,1494.167,1488.987,1473.617,1473.292,1475.942,1479.048,1479.751,1469.903,1456.963,1444.833,1428.652,1407.131,1398.151,1394.722,1391.01,1387.227,1375.479,1369.574,1363.302,1350.094,1342.84,1338.528,1335.861,1332.872,1329.942,1326.264,1323.114,1319.418,1315.997,1311.66,1306.928,1303.173,1298.743,1294.836,1290.98,1286.888,1283.138,1280.021,1274.394,1270.209,1267.47,1265.598,1260.03,1256.427,1253.431,1250.752,1246.601,1241.911,1236.152,1231.326,1226.299,1221.799,1216.749,1208.082,1198.264,1189.792,1181.13,1173.094,1162.603,1154.652,1148.021,1139.475,1133.259,1124.27,1115.257,1107.963,1097.007,1090.814,1084.198,1080.867,1078.867,1075.686,1073.692,1070.426,1067.737,1064.568,1062.028,1059.193,1055.653,1051.753,1049.066,1046.195,1043.273,1040.157,1035.515,1032.883,1030.281,1028.187,1025.491,1022.951,1021.183,1020.957,1013.69,1012.426,1010.178,1008.276,1005.262,1003.366,1001.672,999.0376,998.4191,996.2029,993.6621,991.2231,989.9551,987.9661,986.1865,984.4142,983.3602,981.4513,979.4964,977.3162,975.0403,973.7734,972.6912,970.8188,968.9432,966.7461,964.5309,962.7929,960.7954,959.6012,957.3847,955.8783,954.3401,953.0384,952.087,950.7335,949.6392,948.5724,948.5947,947.038,947.2277,946.8745,946.4396,945.8668,946.1443,946.287,945.799,945.7175,945.7059,945.2318,944.7228,944.1323,943.801,943.5964,942.6069,941.3611,940.0253,938.8239,937.7735,936.7222,935.2263,932.5149,928.8,926.8599,926.5877,926.6866,926.412,926.5207,925.4774,926.6012,926.0789,927.6365,927.079,927.2139,928.2715,930.98,931.8331,933.823,936.0559,937.9583,940.5474,943.7109,945.1055,946.7407,948.899,950.9578,954.6233,956.6885,959.1396,961.346,962.4612,964.1263,964.8068,966.1585,967.683,
1800.406,1801.438,1802.454,1803.436,1801.59,1797.38,1789.871,1773.331,1766.777,1751.823,1725.64,1719.468,1714.495,1681.926,1672.849,1660.925,1655.694,1643.605,1638.163,1632.253,1628.065,1620.863,1608.74,1605.843,1599.26,1591.756,1583.996,1575.311,1565.985,1554.662,1540.979,1526.931,1517.039,1509.066,1500.258,1492.641,1489.176,1491.053,1490.597,1489.458,1492.698,1498.066,1489.564,1486.041,1477.902,1468.118,1467.561,1470.23,1472.697,1473.937,1468.597,1461.434,1437.219,1416.344,1402.081,1397.829,1394.036,1389.199,1384.358,1374.9,1366.44,1351.315,1342.633,1339.195,1335.584,1333.472,1330.707,1327.216,1323.766,1320.67,1317.267,1313.09,1309.647,1305.589,1301.191,1297.353,1293.27,1289.149,1283.931,1279.815,1277.186,1271.854,1266.935,1264.67,1260.701,1257.494,1253.864,1250.465,1247.867,1243.337,1240.027,1234.696,1229.578,1223.422,1217.807,1214.272,1204.041,1194.61,1184.691,1176.881,1168.558,1158.643,1150.775,1144.578,1138.473,1128.93,1119.93,1113.082,1102.509,1095.279,1090.671,1084.224,1079.08,1076.79,1074.228,1071.12,1068.805,1066.159,1063.414,1060.363,1057.224,1054.436,1051.408,1049.019,1045.782,1043.314,1039.811,1036.257,1033.292,1031.317,1029.164,1026.704,1023.996,1022.003,1019.643,1015.074,1013.405,1011.326,1009.443,1007.143,1005.643,1003.89,1001.177,1000.635,998.0608,995.7141,993.3004,991.7208,989.5271,987.763,986.2242,984.6027,982.9362,980.6826,978.5429,976.7985,974.8921,973.2719,971.4847,969.3768,966.9778,965.0581,963.4131,961.7297,960.0873,958.039,956.7181,954.7979,953.9081,953.3951,952.8412,950.9108,950.3671,949.9401,948.7808,947.9772,948.0326,948.1008,947.635,947.7426,947.8069,947.5831,947.9671,948.4558,947.4382,946.7511,946.1288,945.5269,944.5918,943.4315,942.1569,940.847,939.5315,937.986,936.8433,935.7155,933.8247,930.3447,928.006,927.8884,928.5461,929.4161,929.2719,928.3162,927.1907,928.9382,929.0483,928.9782,928.6315,930.6694,932.6261,933.8312,935.4757,936.9426,938.6829,942.8364,945.0209,947.6559,950.1578,951.3917,952.921,955.2238,958.7958,960.5851,963.0097,964.5198,966.2799,968.7239,969.1751,970.2676,
1801.409,1803.076,1804.019,1805.074,1804.061,1800.308,1793.254,1779.349,1769.328,1759.098,1717.174,1706.018,1693.451,1674.918,1666.821,1659.057,1654.667,1648.082,1643.009,1638.018,1632.446,1623.074,1612.486,1609.327,1606.234,1596.957,1589.028,1577.806,1566.664,1555.689,1542.336,1531,1519.565,1513.342,1503.403,1489.607,1482.948,1479.144,1480.288,1477.891,1494.061,1485.314,1481.521,1478.05,1471.91,1461.31,1461.93,1460.465,1458.934,1457.605,1454.559,1446.678,1428.839,1408.882,1400.265,1397.061,1393.417,1386.042,1378.266,1366.939,1354.502,1342.155,1339.884,1336.514,1333.035,1331.3,1327.796,1324.881,1321.24,1317.293,1314.272,1310.661,1307.172,1303.503,1299.511,1295.701,1291.447,1286.742,1282.009,1278.249,1273.465,1269.498,1265.852,1261.501,1259.308,1255.394,1251.383,1247.715,1243.532,1240.406,1237.781,1233.948,1226.286,1217.371,1213.007,1206.268,1198.524,1191.147,1179.473,1172.829,1163.627,1154.038,1147.775,1141.782,1134.891,1128.41,1118.293,1106.902,1098.476,1093.903,1089.098,1082.95,1079.156,1075.88,1073.748,1071.345,1067.936,1064.73,1062.392,1059.736,1056.927,1053.831,1050.967,1048.021,1045.519,1042.406,1038.821,1036.228,1034.279,1032.065,1030.05,1027.836,1024.601,1022.678,1019.979,1017.913,1015.351,1012.493,1010.448,1008.44,1006.715,1005.442,1002.911,1000.958,999.4234,997.4493,995.147,993.4664,991.2054,989.3519,988.1749,986.3881,984.0308,981.7239,979.5471,977.7233,975.2028,974.1614,972.3693,970.381,968.467,966.6278,964.3267,962.5603,960.3075,958.7831,957.4167,956.2705,955.3486,954.7237,953.678,952.1062,951.9362,951.7635,950.0167,949.5969,948.9626,948.9188,949.1521,949.0349,949.2953,949.1054,949.1961,949.2917,948.8357,947.9707,947.0637,946.2729,945.2736,943.9421,942.7076,941.4637,940.2164,938.9615,937.6877,936.0744,935.05,932.4122,929.6916,928.3582,929.0865,929.6508,929.0563,927.8693,928.1955,930.4257,929.8154,930.5203,930.5167,931.1894,933.8646,936.6808,938.3179,939.1018,940.5577,942.8126,946.1639,950.3443,952.312,953.6082,955.5498,956.8172,959.5385,962.9308,964.5291,966.2391,967.7297,970.2649,972.5621,973.5254,
1802.861,1804.305,1805.927,1806.688,1806.018,1802.163,1796.682,1786.404,1775.286,1765.527,1723.938,1704.187,1695.878,1682.302,1663.996,1659.254,1655.989,1652.104,1648.174,1643.26,1633.682,1624.871,1615.965,1613.151,1611.464,1606.593,1591.985,1580.304,1569.73,1557.564,1543.085,1530.264,1520.884,1515.008,1506.991,1492.189,1481.758,1474.533,1471.711,1471.69,1482.03,1477.79,1476.932,1470.694,1464.426,1456.452,1457.587,1455.463,1451.888,1444.344,1438.507,1432.266,1410.719,1403.07,1398.657,1394.553,1391.442,1375.608,1366.831,1354.155,1343.192,1340.068,1337.036,1333.283,1331.048,1328.562,1325.436,1322.455,1318.467,1314.875,1311.297,1308.015,1304.825,1301.339,1297.84,1293.988,1289.885,1285.244,1281.414,1277.01,1273.179,1268.195,1263.938,1260.333,1256.565,1253.672,1249.732,1245.143,1241.076,1237.225,1234.038,1230.339,1223.76,1216.125,1211.68,1204.714,1197.18,1189.105,1179.792,1170.395,1163.518,1152.95,1145.226,1140.467,1132.497,1125.82,1119.58,1109.869,1096.93,1092.722,1088.424,1084.073,1080.18,1076.997,1074.656,1071.536,1068.23,1064.199,1061.774,1059.429,1057.117,1053.787,1051.343,1048.324,1045.184,1042.464,1039.658,1037.646,1035.557,1033.443,1030.862,1028.893,1026.221,1023.061,1021.243,1019.129,1016.821,1014.193,1011.812,1010.491,1008.646,1006.767,1005.576,1003.513,1001.32,999.2953,996.9223,995.959,993.0434,991.0256,989.5127,987.3032,985.0695,983.517,981.3911,978.1935,976.0548,975.0373,973.6182,971.6956,969.0486,966.8121,964.227,962.1445,961.1465,960.1865,958.5468,957.665,957.2623,956.2784,954.793,953.2488,953.5214,953.4663,951.2733,950.7122,950.4927,949.758,949.8577,950.308,950.3464,950.2633,949.7668,949.9413,949.5574,948.7272,947.634,947.1345,945.7294,944.457,943.2228,941.9883,940.7484,939.4996,938.2514,937.0383,935.8213,934.2889,932.0649,929.3034,929.1208,930.1376,929.7005,928.0099,930.1916,931.4833,931.4388,931.9222,932.9784,933.1144,935.0623,937.4512,939.9295,942.1426,943.2256,944.6431,948.0348,951.417,953.2977,955.9457,958.1249,959.6805,961.8181,964.0223,966.6772,969.0762,969.8669,972.301,975.3594,977.0825,
1804.993,1805.968,1807.486,1808.215,1808.048,1804.378,1799.405,1794.296,1783.249,1774.67,1735.812,1709.909,1699.304,1684.502,1667.786,1662.544,1658.745,1654.683,1650.794,1644.889,1634.818,1630.299,1628.746,1621.922,1618.304,1610.416,1596.378,1583.529,1570.352,1559.852,1547.323,1533.091,1523.128,1516.364,1508.989,1495.442,1484.465,1472.768,1468.308,1465.582,1466.442,1466.516,1466.63,1465.113,1455.778,1449.262,1449.498,1443.498,1437.063,1430.317,1425.651,1418.448,1409.391,1402.221,1397.046,1393.229,1382.573,1369.203,1362.465,1345.064,1339.852,1337.503,1334.309,1331.256,1328.52,1325.748,1322.383,1319.318,1315.725,1312.199,1308.928,1305.465,1302.181,1298.31,1295.022,1291.008,1287.405,1283.662,1280.473,1276.525,1272.18,1267.92,1262.325,1257.981,1255.354,1251.985,1249.256,1245.174,1240.483,1236.455,1231.373,1227.264,1222.279,1217.415,1211.817,1205.902,1195.825,1186.249,1178.714,1170.725,1163.3,1152.801,1144.526,1138.946,1132.72,1122.549,1119.933,1113.396,1099.473,1092.66,1088.742,1085.75,1081.332,1077.922,1074.335,1071.546,1069.016,1065.368,1062.133,1059.979,1057.847,1055.893,1052.001,1048.794,1046.21,1044.184,1041.543,1039.313,1036.879,1034.584,1032.486,1030.084,1027.429,1025.236,1022.914,1021.191,1018.519,1016.35,1014.327,1013.019,1010.196,1008.076,1006.938,1004.953,1003.467,1000.841,999.0321,997.8063,995.1331,992.7259,990.9407,989.0698,986.913,984.8438,982.7492,980.0335,977.7147,975.9908,974.794,972.3608,969.8095,966.0131,964.5675,963.7672,962.6652,961.1167,960.2244,958.9416,958.5603,957.0459,955.3826,955.3513,954.2768,953.3084,952.3729,952.205,951.8904,952.3197,951.9031,950.7609,951.2027,951.1615,950.4762,949.8387,950.1303,949.4738,948.1825,947.8826,946.2001,944.9714,943.7378,942.5039,941.2698,940.0333,938.7853,937.6301,937.7416,936.1886,933.7483,931.5523,930.5063,930.4235,930.3274,929.2036,929.9053,932.2859,933.8412,933.7048,935.139,935.1959,936.5963,939.6582,941.0502,943.674,945.771,947.8096,950.4112,951.9526,954.4417,957.9268,961.0679,962.2728,964.1479,965.7573,968.432,972.0289,973.4709,974.1683,976.7171,979.6514,
1806.317,1807.32,1809.175,1810.158,1810.182,1807.859,1802.278,1796.23,1791.758,1778.281,1746.199,1711.781,1700.322,1685.657,1668.403,1664.156,1660.833,1657.089,1652.945,1647.172,1640.342,1636.489,1634.193,1628.625,1624.223,1615.789,1598.675,1585.704,1573.709,1561.921,1550.245,1536.447,1524.259,1515.889,1508.667,1497.58,1485.61,1474.389,1465.646,1461.925,1459.638,1458.228,1457.474,1458.636,1450.877,1444.949,1445.256,1439.266,1431.917,1423.049,1418.325,1414.797,1408.41,1401.464,1397.172,1390.754,1377.206,1366.253,1355.324,1341.378,1337.356,1334.69,1331.916,1329.151,1325.896,1322.606,1320.899,1316.731,1313.631,1309.448,1305.945,1302.895,1300,1295.524,1291.978,1288.838,1285.32,1281.691,1278.06,1274.939,1270.693,1266.728,1262.157,1256.378,1251.709,1248.424,1245.787,1243.248,1239.324,1236.612,1232.285,1226.753,1220.739,1214.874,1207.746,1202.526,1194.771,1182.825,1178.863,1168.043,1159.149,1153.86,1143.904,1136.03,1130.388,1110.63,1107.043,1109.437,1097.846,1093.693,1090.455,1087.243,1082.747,1079.526,1076.372,1073.584,1069.993,1067.686,1064.421,1061.548,1059.124,1056.073,1053.239,1051.466,1048.836,1045.559,1042.945,1040.343,1038.132,1035.852,1033.65,1031.467,1029.396,1027.29,1024.937,1024.13,1020.988,1018.978,1018.745,1014.083,1011.815,1009.628,1008.259,1006.678,1004.802,1003.233,1001.702,998.93,996.077,994.1173,992.9152,990.0772,988.486,986.0626,984.9993,980.8248,979.42,977.5915,975.8034,972.862,969.4985,967.2652,966.5853,965.556,963.9494,962.8125,961.1223,960.3428,959.0371,958.7791,958.1693,956.4522,955.5845,954.6866,954.2073,954.0374,953.48,953.6791,953.9648,951.926,951.6251,952.1097,951.9033,951.1622,950.494,949.7271,948.5139,947.7758,946.7031,945.4798,944.2514,943.0228,941.7889,940.5536,939.3196,938.3015,938.437,937.5258,933.923,931.6841,930.7545,930.5477,930.037,930.3228,932.5026,933.3444,934.4844,935.0855,937.4392,937.6637,938.5823,940.4118,942.395,944.7311,947.4066,950.2851,952.3524,954.4089,956.0423,958.2792,962.0507,965.011,966.6131,968.4501,969.7982,973.5235,975.7891,977.6722,978.6649,982.614,
1807.61,1809.034,1810.685,1811.578,1811.846,1810.543,1805.801,1800.539,1793.36,1787.521,1747.078,1719.188,1701.065,1691.031,1670.03,1666.177,1662.702,1658.794,1654.521,1650.231,1646.971,1643.848,1639.891,1630.466,1625.811,1616.37,1600.4,1587.172,1575.885,1565.58,1554.191,1543.138,1528.22,1516.244,1506.698,1498.15,1486.133,1474.646,1464.621,1459.819,1454.573,1452.849,1450.39,1448.902,1443.628,1438.55,1440.796,1436.375,1429.743,1421.506,1415.09,1409.304,1404.281,1400.38,1394.608,1388.496,1377.714,1366.006,1349.681,1338.338,1335.771,1332.394,1329.341,1326.694,1323.457,1320.019,1317.304,1313.823,1310.783,1307.144,1303.735,1300.651,1296.66,1293.169,1289.773,1286.438,1282.042,1278.973,1275.55,1272.201,1268.174,1264.679,1260.477,1256.461,1249.606,1243.452,1241.786,1240.153,1237.609,1234.422,1231.615,1226.017,1219.055,1212.412,1203.615,1198.563,1191.433,1180.832,1175.23,1167.679,1159.284,1152.666,1146.534,1134.951,1132.244,1110.292,1105.411,1102.037,1098.498,1094.63,1091.098,1087.359,1084.246,1080.971,1078.077,1074.92,1071.803,1068.978,1066.12,1063.301,1060.893,1058.165,1055.407,1052.751,1049.548,1046.955,1044.194,1041.882,1039.875,1037.432,1035.337,1032.91,1031.206,1029.733,1027.836,1025.358,1024.063,1022.24,1020.537,1016.589,1013.659,1011.759,1010.609,1009.019,1006.764,1005.17,1002.466,1000.102,998.0158,995.9992,993.3414,991.3293,988.9609,986.756,984.4633,982.7281,980.8961,978.6912,976.8812,973.4485,970.533,969.2856,967.8382,966.4047,964.8838,963.0453,962.176,961.5382,960.7563,962.288,960.6924,958.2327,957.1668,956.2856,956.1646,955.8053,954.9821,954.8477,954.9567,952.8836,952.7018,952.3694,953.0991,952.5734,950.8412,950.174,949.3203,948.0693,947.1613,945.7255,944.3914,943.1868,941.9789,940.7722,939.5511,939.1154,939.1332,937.2578,934.286,932.4396,931.1613,930.1715,930.2518,931.0818,933.4402,934.5283,935.536,936.9233,937.5243,939.8394,940.1111,940.9758,943.1713,945.6219,948.4438,950.9362,953.4136,956.3701,958.456,959.7569,962.652,965.7523,968.3404,970.8505,973.0702,974.6658,976.9382,979.8636,982.2582,984.0956,
1808.693,1810.197,1811.896,1812.9,1813.595,1813.033,1810.066,1806.878,1798.721,1788.8,1757.186,1724.109,1706.212,1697.726,1673.362,1668.139,1664.172,1660.314,1655.762,1651.009,1648.551,1643.18,1636.345,1628.557,1623.86,1615.286,1601.459,1588.453,1576.244,1567.958,1557.826,1548.069,1532.269,1520.147,1505.319,1498.637,1486.178,1475.526,1463.303,1458.433,1452.343,1447.479,1443.867,1441.887,1436.815,1433.548,1434.581,1432.844,1427.326,1420.06,1412.69,1406.961,1403.336,1399.144,1393.663,1383.851,1374.295,1360.608,1340.738,1335.765,1333.505,1331.129,1327.86,1324.489,1320.837,1317.545,1314.243,1311.107,1307.821,1304.518,1301.107,1297.38,1293.456,1290.178,1286.494,1283.561,1279.366,1276.185,1272.548,1269.064,1265.458,1261.893,1257.764,1253.188,1245.159,1240.581,1237.581,1236.689,1233.985,1230.843,1226.798,1220.697,1211.122,1205.905,1197.944,1191.437,1181.295,1177.179,1173.037,1158.45,1159.057,1153.393,1148.06,1132.627,1118.95,1109.815,1106.003,1102.596,1099.088,1095.467,1091.528,1088.714,1085.558,1082.928,1080.695,1076.674,1073.596,1071.098,1068.487,1064.885,1062.377,1059.611,1056.756,1054.085,1050.967,1048.28,1045.921,1043.365,1040.694,1039.194,1037.764,1035.312,1033.726,1032.273,1030.127,1027.249,1025.253,1023.263,1020.933,1019.564,1017.076,1015.091,1012.82,1010.777,1009.113,1006.863,1003.682,1001.732,999.5699,996.8962,994.8094,992.5541,989.9327,987.1276,985.2656,983.1191,981.2094,978.4749,975.3137,973.7396,971.8888,969.3835,968.5863,967.4431,966.2538,965.2137,963.8289,963.394,962.452,963.1305,960.8659,959.068,958.4613,957.9774,957.6103,957.2097,956.1962,956.2808,955.097,953.7529,954.2227,953.6215,953.6239,953.696,952.0789,951.2199,950.097,948.223,947.2729,946.5641,945.1596,943.2646,941.9967,940.8106,939.8177,939.9244,939.168,936.3061,934.6038,932.2279,930.7654,930.1382,930.636,931.1965,933.5237,935.2556,936.3807,937.8704,939.2061,941.027,942.2952,942.7878,944.4185,947.8711,950.5372,952.3505,955.0552,957.3271,960.3577,962.7945,965.6145,967.7061,969.8056,973.0872,975.345,976.5701,979.3643,982.6903,984.7266,985.7818,
1810.005,1811.688,1813.302,1814.436,1815.677,1815.33,1812.415,1809.374,1801.299,1791.731,1752.084,1726.767,1709.342,1704.519,1677.141,1669.442,1664.674,1661.071,1657.095,1650.473,1641.185,1636.895,1630.821,1623.741,1619.294,1614.619,1605.722,1590.317,1578.96,1570.75,1561.1,1551.164,1536.898,1523.819,1506.578,1498.033,1488.362,1475.766,1463.862,1457.503,1452.602,1446.46,1439.954,1434.525,1431.574,1429.482,1429.522,1429.394,1426.98,1418.291,1410.526,1405.782,1402.079,1396.876,1392.024,1382.271,1370.062,1355.985,1338.524,1333.506,1331.301,1328.343,1325.402,1322.372,1318.622,1315.318,1311.737,1308.895,1305.602,1302.085,1298.41,1294.695,1291.145,1287.458,1284.098,1280.212,1277.198,1273.846,1271.189,1266.924,1263.03,1258.832,1254.8,1249.906,1244.26,1239.164,1233.4,1230.499,1228.629,1226.1,1223.298,1217.268,1209.068,1202.889,1191.616,1184.742,1178.849,1174.746,1170.125,1153.595,1152.764,1148.355,1137.966,1129.749,1114.919,1110.731,1106.859,1103.371,1100.454,1096.404,1093.619,1090.586,1088.306,1084.93,1081.02,1078.516,1075.653,1072.95,1070.111,1066.933,1063.964,1060.822,1057.99,1055.199,1052.265,1049.662,1047.188,1044.832,1042.566,1040.884,1038.626,1036.832,1035.148,1032.591,1030.762,1029.27,1027.573,1025.967,1023.652,1021.56,1019.546,1017.158,1014.623,1012.799,1009.676,1007.288,1006.119,1002.99,1000.864,998.7141,995.709,993.4709,990.7939,988.5261,985.746,983.0815,980.5854,978.0741,976.0063,973.5969,972.0779,970.7333,969.7697,968.5378,968.1268,967.7867,966.0871,964.6562,963.4417,962.1245,960.8743,959.5776,958.775,959.1249,959.2014,958.3503,957.7806,958.6915,956.5606,955.1499,955.3216,954.4538,954.1245,954.3617,952.944,951.3901,950.3311,949.8908,948.0554,947.5284,946.09,944.6554,942.602,941.1176,940.3668,939.8474,938.0876,935.993,934.1913,932.1763,930.8637,930.6891,930.3553,931.1172,933.7409,936.0487,937.2495,939.0012,940.4357,942.5453,944.4081,945.5074,947.0063,949.9088,953.4509,955.8149,957.1252,959.6273,962.166,964.9769,967.2026,970.5649,972.8792,975.0229,977.3713,980.0018,982.3186,985.4896,987.7499,989.5272,
1811.433,1813.117,1814.597,1815.662,1816.329,1816.363,1813.624,1809.122,1803.33,1776.559,1761.396,1724.634,1710.647,1691.871,1679.171,1672.951,1666.654,1662.714,1658.329,1653.386,1642.305,1632.101,1627.598,1622.468,1617.894,1614.089,1608.182,1592.277,1580.37,1572.048,1562.221,1558.444,1540.063,1524.654,1508.533,1498.395,1490.485,1477.678,1465.986,1457.395,1453.786,1445.041,1438.478,1431.158,1427.393,1426.041,1424.318,1425.032,1423.437,1416.403,1409.405,1405.124,1400.593,1395.561,1390.766,1375.814,1367.18,1356.857,1338.272,1331.688,1328.16,1325.239,1322.116,1319.934,1316.678,1313.514,1310.016,1306.422,1303.411,1299.165,1295.661,1292.13,1288.815,1284.824,1281.323,1278.476,1274.149,1271.041,1268.195,1264.286,1260.743,1256.773,1250.74,1245.433,1240.913,1236.994,1230.477,1225.22,1223.917,1221.066,1216.519,1212.034,1207.685,1201.25,1193.926,1182.935,1177.331,1172.423,1163.456,1149.951,1145.885,1143.928,1134.018,1125.602,1116.146,1112.084,1108.564,1105.256,1101.616,1097.837,1095.403,1092.317,1089.022,1086.273,1083.313,1080.317,1077.306,1073.972,1071.086,1067.981,1064.995,1061.894,1058.49,1055.868,1053.326,1050.511,1048.445,1046.911,1044.137,1042.87,1040.758,1038.69,1036.659,1034.832,1032.796,1031.305,1029.112,1026.559,1023.773,1021.32,1019.477,1017.621,1015.913,1013.762,1011.208,1009.091,1007.191,1003.503,1002.099,999.3818,995.5551,993.682,991.3095,988.4485,986.7435,984.5632,982.3756,980.415,977.2821,975.7471,974.2715,972.642,971.7952,970.2272,969.3259,967.8344,966.7044,965.8475,964.0333,962.4786,961.8043,959.9433,959.851,960.239,960.5887,959.6046,958.9778,958.2546,956.6432,956.4421,956.1129,955.2314,954.4966,955.002,954.1542,951.5209,950.1808,949.9442,948.535,948.0151,946.912,944.7085,942.7587,941.2661,939.3484,938.5075,937.6637,935.6222,933.5375,931.9232,931.4363,931.408,930.3265,931.2411,933.7393,935.621,937.6216,939.5286,941.8104,943.6804,946.1394,948.1334,950.4988,952.4509,956.0213,958.0476,959.7789,962.5843,964.6984,967.0258,968.8483,971.5262,975.916,978.1166,979.9695,982.1414,984.6929,987.0387,990.1265,992.7808,
1812.595,1814.572,1815.941,1816.871,1817.791,1818.172,1816.576,1812.195,1807.922,1775.524,1755.566,1721.408,1710.673,1697.554,1681.205,1675.694,1669.183,1664.594,1660.543,1655.294,1645.692,1632.136,1623.931,1619.902,1615.241,1611.204,1607.121,1594.53,1583.01,1574.484,1564.05,1559.536,1546.601,1525.818,1516.02,1499.673,1491.489,1480.428,1470.024,1457.544,1451.23,1443.372,1438.053,1429.091,1424.721,1422.263,1420.427,1421.737,1420.33,1413.787,1408.974,1403.575,1398.874,1395.073,1385.896,1374.885,1366.337,1360.41,1334.957,1328.983,1325.102,1322.631,1319.53,1316.209,1314.489,1310.439,1307.597,1304.133,1300.275,1296.693,1293.318,1289.719,1286.605,1282.503,1278.936,1275.149,1271.75,1268.719,1265.643,1261.937,1258.444,1250.953,1247.086,1242.227,1237.545,1231.744,1228.807,1223.104,1217.996,1214.71,1211.237,1208.657,1202.142,1194.652,1184.887,1181.523,1176.457,1170.901,1162.548,1143.387,1134.51,1130.565,1124.997,1121.078,1117.173,1113.668,1110.088,1107.079,1103.302,1099.595,1096.72,1093.874,1090.726,1087.576,1084.67,1081.144,1078.083,1074.893,1071.211,1068.428,1065.37,1062.13,1059.039,1056.331,1055.184,1052.45,1050.472,1048.537,1046.499,1044.648,1042.661,1040.607,1038.193,1036.265,1034.253,1031.052,1028.162,1025.183,1023.456,1021.644,1019.102,1017.347,1015.029,1012.127,1010.732,1007.983,1006.827,1005.189,1002.609,999.7358,996.2974,994.8073,993.1774,991.0647,987.8385,985.8491,983.8623,982.3043,979.2521,976.6706,975.7354,974.8441,973.717,972.0491,970.1268,968.6653,967.7606,967.0258,965.2089,963.4019,961.4233,960.9572,961.1122,961.4269,961.7213,960.9721,959.9816,958.8378,957.891,957.3685,956.4279,955.8188,955.6322,955.5445,954.8062,951.9052,949.5688,949.0023,948.5092,947.3621,946.0809,943.7365,941.1924,940.3475,937.863,937.5477,935.664,935.6794,934.3333,932.2531,932.3132,932.9737,932.0778,931.6268,932.1845,935.0192,937.5118,940.2607,942.4973,944.7574,946.3787,948.5148,951.1255,953.8764,956.0165,958.225,962.3006,964.569,966.8592,969.0626,971.238,973.5126,976.5884,979.8905,983.186,984.9793,987.3958,990.3234,992.3338,995.0486,
1814.186,1815.737,1817.129,1818.349,1819.773,1820.016,1819.247,1815.871,1800.005,1796.236,1753.249,1733.218,1711.472,1695.005,1685.735,1677.755,1672.303,1668.382,1661.993,1656.705,1649.104,1635.848,1621.134,1616.715,1612.724,1608.544,1603.965,1597.419,1586.069,1576.611,1567.345,1561.167,1556.56,1529.758,1519.481,1502.944,1492.666,1482.598,1475.614,1457.51,1448.759,1442.941,1435.265,1427.621,1422.66,1419.232,1416.373,1416.552,1418.63,1411.969,1407.922,1402.916,1398.735,1393.263,1383.91,1370.218,1357.72,1350.509,1328.917,1325.203,1323.259,1320.182,1316.726,1313.55,1310.514,1307.629,1304.882,1302.033,1297.829,1294.163,1291.02,1287.349,1284.169,1280.372,1276.619,1272.812,1270.451,1266.349,1262.638,1258.891,1252.485,1248.823,1244.446,1239.931,1236.015,1227.603,1223.485,1220.172,1214.893,1211.007,1208.322,1202.534,1195.718,1187.419,1183.887,1180.11,1173.898,1165.045,1153.11,1138.461,1132.299,1129.143,1125.629,1121.927,1118.229,1114.94,1111.088,1107.504,1105.039,1101.257,1098.158,1095.112,1092.092,1089.055,1085.989,1082.571,1078.718,1075.11,1072.091,1069.4,1066.103,1063.653,1060.736,1058.627,1056.868,1055.334,1052.538,1051.097,1048.999,1046.783,1044.329,1041.749,1039.65,1036.939,1033.719,1031.124,1029.035,1027.168,1025.233,1022.636,1019.576,1017.242,1015.227,1013.131,1011.194,1007.928,1005.116,1002.866,1000.305,997.984,995.4017,993.5585,991.1711,988.7004,986.4324,984.6825,983.2648,981.252,979.8513,978.0676,976.5876,975.3802,973.986,972.1111,969.7361,969.0109,968.4591,967.8837,966.5969,964.6028,962.8441,962.1324,962.5675,963.2741,963.261,962.3153,961.423,960.1594,959.3096,958.3133,957.3707,956.9795,955.8098,956.2164,955.2864,952.6658,949.283,948.7963,947.5953,946.2882,944.6877,942.443,940.6998,939.5889,937.9548,937.4217,937.2838,935.2267,933.6797,933.3528,933.8764,934.3364,932.4474,931.9135,932.2177,934.6432,938.1538,940.036,941.5926,943.3375,946.1124,949.1238,951.8446,954.6926,957.0464,959.0176,962.4589,966.0295,968.6622,971.5122,973.9081,976.5997,979.2242,982.1877,984.7914,987.9213,989.9637,992.2429,994.813,996.8477,
1815.354,1817.077,1818.426,1819.67,1821.125,1822.104,1821.809,1819.911,1797.398,1779.222,1777.103,1716.827,1708.271,1697.137,1689.377,1683.755,1677.554,1673.66,1664.025,1654.576,1648.88,1637.914,1618.785,1613.609,1610.259,1605.076,1601.02,1596.756,1588.997,1579.486,1568.687,1563.104,1558.877,1538.977,1522.677,1510.402,1496.92,1487.698,1475.484,1461.203,1445.226,1437.792,1431.211,1425.663,1421.193,1416.562,1412.458,1411.097,1412.523,1409.286,1406.185,1402.432,1396.965,1390.124,1379.77,1361.029,1349.457,1336.452,1325.414,1322.412,1319.501,1317.903,1314.029,1310.6,1307.634,1304.359,1300.969,1298.154,1295.853,1292.055,1288.449,1285.172,1282.078,1278.188,1274.008,1270.769,1267.018,1262.744,1258.991,1254.812,1248.088,1244.441,1241.646,1236.617,1233.927,1223.124,1215.827,1212.801,1209.982,1207.732,1203.355,1196.732,1189.959,1185.533,1182.008,1177.012,1166.124,1155.715,1145.973,1136.254,1132.885,1129.727,1125.702,1122.056,1118.894,1115.488,1112.293,1109.265,1106.157,1103.306,1099.824,1096.565,1093.633,1090.628,1087.172,1083.137,1079.482,1076.626,1073.068,1070.033,1067.555,1064.776,1063.267,1061.114,1059.067,1057.223,1055.203,1053.834,1051.117,1048.108,1045.48,1042.995,1040.035,1037.248,1035.148,1033.354,1031.489,1028.843,1025.729,1022.969,1020.332,1017.437,1014.637,1012.102,1010.006,1008.616,1006.172,1003.158,1001.448,999.4534,997.4707,995.6342,992.7954,989.9142,987.1606,984.7216,982.6852,980.6842,979.0919,978.1161,976.8976,975.7289,973.6511,970.9445,969.5401,969.0325,968.3592,967.7961,967.0938,966.4294,965.6697,965.127,964.7137,964.1069,963.4989,962.7457,962.0997,961.4101,960.3984,959.3229,958.1431,957.7224,956.7535,956.7992,955.6115,952.338,949.4874,947.5533,947.457,945.9896,944.387,941.789,940.0764,938.9632,938.4319,937.5102,936.8901,935.9269,935.3367,936.0878,933.813,933.8465,932.7087,932.7764,932.5196,934.844,938.0812,938.9438,941.0488,943.2809,947.1868,950.9073,953.4882,956.1642,958.829,960.6087,962.9664,966.4991,968.8383,972.2341,975.7932,978.9064,981.7191,984.7681,986.3466,989.7916,992.4139,994.7745,996.4401,999.1019,
1816.178,1817.91,1819.346,1821.566,1821.84,1823.175,1824.312,1823.378,1816.067,1767.953,1758.508,1716.705,1705.308,1698.837,1691.924,1688.456,1681.416,1673.989,1665.847,1649.591,1641.953,1627.528,1616.232,1610.215,1606.81,1602.291,1598.136,1593.456,1587.015,1581.951,1572.084,1562.775,1558.544,1550.146,1525.931,1514.91,1501.447,1487.899,1475.771,1463.364,1444.392,1431.032,1426.931,1422.401,1418.44,1414.312,1410.11,1406.248,1404.994,1406.007,1403.566,1397.487,1393.295,1384.292,1374.494,1355.391,1338.551,1328.797,1323.337,1319.721,1317.017,1314.023,1311.456,1308.646,1305.094,1301.739,1298.774,1296.102,1292.03,1289.106,1286.351,1282.786,1279.739,1276.446,1272.415,1268.601,1265.243,1260.596,1254.833,1251.321,1245.809,1241.856,1236.265,1233.251,1230.425,1223.77,1212.192,1206.801,1205.628,1203.805,1200.922,1193.666,1184.773,1172.44,1175.055,1168.277,1157.485,1146.521,1140.185,1137.516,1133.86,1130.217,1127.146,1124.149,1120.465,1116.908,1113.948,1110.712,1107.765,1104.72,1101.197,1098.311,1094.978,1091.923,1088.795,1085.356,1082.731,1079.693,1076.843,1072.719,1069.202,1067.578,1065.792,1063.507,1061.431,1059.815,1057.593,1054.002,1051.454,1048.932,1045.931,1043.944,1041.58,1039.398,1038.075,1035.197,1032.278,1029.206,1026.809,1024.251,1021.39,1018.418,1015.115,1012.052,1009.6,1007.52,1005.504,1003.374,1001.972,999.6165,997.7935,996.4656,995.0699,989.9974,987.1096,984.988,983.2508,981.9933,979.7062,978.3115,977.4619,975.8514,973.4551,970.4465,970.0634,969.8064,969.067,968.3638,967.9911,966.551,965.9401,965.3645,965.2026,965.2052,964.5012,963.4664,962.7709,961.4114,960.7001,960.1511,959.3687,958.4802,957.502,957.5253,955.751,951.7867,948.9904,947.1512,946.4709,944.2761,943.2469,941.3715,940.7221,939.2161,938.6177,937.4357,936.2759,935.4053,935.0553,934.7172,933.3867,933.8302,934.1882,933.6366,933.279,935.546,937.3504,939.5576,942.5211,946.3066,950.4571,952.9677,955.4994,958.072,960.376,963.915,965.9695,967.675,969.9252,973.5065,975.879,979.4133,982.9537,986.6168,989.2459,992.0015,995.2118,997.0912,999.8454,1001.439,
1817.292,1818.736,1820.255,1821.571,1823.133,1824.701,1826.074,1825.797,1816.189,1756.263,1737.383,1716.29,1701.343,1698.317,1693.914,1690.986,1682.358,1673.028,1664.044,1646.155,1624.62,1616.634,1611.695,1607.469,1603.303,1598.596,1594.225,1589.756,1584.717,1578.883,1573.095,1565.664,1555.571,1545.524,1530.182,1519.233,1504.612,1489.495,1476.202,1466.865,1447.774,1428.318,1424.62,1420.074,1416.475,1411.872,1408.001,1402.596,1398.852,1397.164,1393.546,1389.289,1387.157,1382.148,1372.712,1350.121,1334.47,1325.739,1321.777,1317.654,1314.36,1311.048,1308.32,1305.434,1302.106,1299.237,1295.857,1292.875,1289.688,1286.446,1283.47,1280.745,1277.055,1273.767,1270.834,1266.802,1263.353,1257.874,1252.813,1248.233,1242.806,1237.896,1233.02,1229.881,1227.76,1222.573,1211.366,1202.828,1200.401,1199.859,1197.637,1193.202,1177.877,1160.255,1158.189,1153.713,1148.928,1145.066,1141.768,1138.477,1134.881,1131.81,1128.704,1125.344,1121.988,1118.33,1115.46,1112.39,1109.175,1106.226,1102.738,1099.555,1096.46,1093.545,1090.284,1087.656,1084.998,1083.013,1079.722,1074.681,1071.841,1069.521,1067.614,1065.833,1063.783,1061.25,1057.886,1055.182,1051.813,1048.911,1047.145,1045.125,1043.08,1041.616,1038.787,1035.673,1033.201,1030.249,1027.341,1025.12,1023.239,1020.631,1015.661,1013.038,1010.733,1009.241,1006.335,1004.328,1002.757,1000.081,998.462,997.2502,994.4075,989.9288,987.9137,985.9354,984.9087,983.4438,980.4984,979.1977,978.2557,976.4706,974.2695,970.8749,971.0696,970.9443,970.0887,969.3075,968.3649,967.0886,966.704,966.6861,966.6848,966.2127,965.3552,964.1997,963.4681,963.0568,961.9094,960.2367,959.4424,958.7901,958.319,957.9604,953.7466,950.778,948.854,947.6835,946.1484,943.9133,941.5891,941.0348,940.1361,939.295,937.6602,936.9894,936.5695,936.1515,935.0173,934.7463,934.6364,934.6878,933.582,934.1924,934.6913,938.1437,940.3684,942.2601,945.1992,948.9738,953.2791,955.0302,957.4414,959.3486,962.9875,964.4218,966.7827,969.7505,972.6213,974.9064,977.4536,980.8923,984.9418,988.6754,991.1445,993.6563,997.0467,999.9052,1002.796,1004.62,
1818.421,1819.661,1821.556,1822.888,1824.642,1825.838,1827.343,1825.933,1798.23,1769.655,1740.375,1719.563,1700.085,1695.638,1691.392,1688.698,1679.819,1673.53,1666.945,1619.046,1618.479,1613.427,1608.242,1603.724,1599.535,1595.627,1590.797,1586.555,1581.305,1575.74,1570.134,1563.25,1553.583,1549.492,1540.855,1523.947,1505.435,1488.064,1473.983,1465.991,1451.174,1427.846,1423.651,1419.347,1414.978,1411.231,1407.337,1403.782,1396.672,1389.599,1385.413,1383.479,1382.48,1379.792,1366.111,1345.635,1330.416,1324.215,1318.579,1316.063,1312.146,1307.92,1305.101,1301.937,1299.025,1296.219,1293.615,1290.82,1287.105,1283.742,1281.06,1278.321,1275.119,1271.57,1267.575,1264.463,1259.658,1255.947,1249.257,1244.217,1237.792,1234.072,1229.786,1225.977,1223.69,1218.568,1210.502,1199.903,1195.749,1195.649,1194.752,1189.102,1180.718,1161.023,1158.023,1153.998,1150.128,1146.724,1143.128,1139.699,1136.245,1133.25,1129.917,1126.331,1123.011,1119.935,1116.634,1113.651,1110.415,1107.348,1104.484,1101.447,1098.208,1095.013,1092.384,1089.106,1086.473,1083.953,1080.321,1076.907,1074.155,1071.832,1069.65,1067.587,1065.389,1061.568,1057.825,1054.444,1053.164,1051.801,1049.376,1046.575,1044.525,1041.671,1038.869,1036.079,1033.344,1030.79,1027.605,1024.859,1022.459,1019.781,1016.565,1013.88,1011.789,1009.801,1007.602,1006.006,1003.312,1000.858,999.0447,996.3728,992.7375,990.8428,988.9548,987.2523,985.4831,983.5162,981.2203,980.0308,978.9026,977.0671,974.5283,971.717,971.6,972.0535,971.1033,970.1344,968.9695,968.8422,968.5873,968.3788,967.9472,967.3537,966.8839,965.5382,964.3907,964.1089,963.4235,961.808,960.0097,958.7912,958.2724,956.2211,953.2891,950.5022,949.1828,948.2486,945.7303,943.6014,942.5226,941.5613,941.0353,940.2617,937.4314,937.0141,936.6275,936.0421,936.1484,936.1671,937.9584,937.3138,936.5219,936.5475,938.1075,941.0872,942.5494,945.6659,948.552,951.8654,956.0081,958.4474,959.7266,961.8041,963.6213,965.5275,967.5607,970.9051,973.601,976.4482,978.8503,981.3266,985.159,988.3071,991.1773,994.6093,998.4945,1000.898,1003.73,1006.751,
1819.521,1820.65,1822.854,1823.691,1825.67,1827.015,1828.13,1824.009,1784.032,1762.339,1745.178,1722.049,1698.428,1692.631,1687.525,1685.5,1679.902,1669.183,1635.302,1616.416,1614.141,1610.026,1605.187,1599.771,1595.761,1592.291,1587.801,1583.051,1577.218,1571.863,1566.938,1559.947,1550.208,1544.309,1535.173,1513.878,1491.485,1485.218,1463.201,1456.409,1438.908,1427.519,1423.756,1420.23,1414.779,1410.727,1406.657,1402.655,1396.666,1385.307,1382.13,1379.576,1378.311,1362.903,1343.659,1333.229,1328.449,1321.338,1315.375,1312.111,1309.42,1305.95,1301.845,1299.14,1295.854,1294.082,1290.778,1288.62,1284.942,1281.577,1278.197,1274.874,1272.208,1268.672,1266.218,1262.192,1257.525,1253.583,1245.729,1241.281,1234.676,1230.526,1225.759,1221.949,1218.084,1215.482,1208.136,1196.293,1191.039,1190.246,1188.269,1185.229,1173.378,1163.238,1159.634,1156.051,1151.824,1148.151,1144.33,1141.027,1137.58,1134.381,1131.137,1127.797,1124.586,1121.604,1118.38,1115.485,1112.16,1109.622,1106.885,1103.503,1099.72,1096.159,1093.537,1090.58,1087.194,1084.211,1081.323,1078.816,1075.961,1073.725,1071.618,1068.384,1065.027,1061.926,1058.65,1056.925,1056.062,1053.818,1050.69,1047.755,1044.468,1041.987,1039.582,1036.818,1034.174,1031.585,1028.121,1025.998,1022.755,1019.922,1016.692,1014.731,1012.601,1010.54,1008.859,1006.763,1004.497,1002.123,999.5088,996.9906,995.2493,992.9883,990.9385,988.3699,984.9662,983.4501,981.5618,980.2032,979.4385,978.2123,976.0207,973.7786,972.2529,972.4664,973.1385,971.1927,970.3795,970.216,969.8067,969.1614,968.9817,968.3627,968.1077,966.5886,965.4071,965.248,964.9002,963.1874,960.3234,959.1805,956.0961,954.8181,952.3524,950.2808,948.5434,947.2238,945.7632,944.8093,943.1202,941.8119,941.0021,939.9713,939.4511,937.2152,936.7404,936.4984,940.6068,940.4211,942.158,942.2173,941.5667,941.8901,942.9287,944.4926,946.0931,948.2087,951.4328,955.0107,958.9549,960.5865,962.1956,964.0638,966.5869,968.8452,970.0865,972.3385,974.7334,977.2637,979.9392,982.7102,985.1804,989.0739,992.4984,996.0623,998.7399,1001.449,1004.373,1007.993,
1820.125,1821.44,1821.967,1819.958,1821.661,1821.442,1825.329,1802.476,1767.387,1758.174,1741.313,1724.312,1699.53,1687.106,1683.542,1678.548,1678.705,1655.282,1623.619,1616.649,1610.357,1606.52,1600.856,1596.2,1592.421,1588.649,1583.74,1578.829,1573.611,1569.337,1564.221,1556.516,1547.177,1540.316,1518.091,1493.077,1480.499,1468.273,1461.388,1456.769,1442.219,1429.038,1425.007,1420.393,1415.446,1411.246,1407.496,1402.869,1398.169,1385.982,1381.477,1377.579,1371.289,1349.735,1336.495,1331.108,1326.805,1318.052,1312.635,1309.109,1305.017,1302.264,1299.639,1296.595,1292.873,1289.731,1287.429,1284.441,1280.856,1278.344,1275.328,1272.078,1269.188,1266.907,1262.312,1259.664,1254.234,1249.805,1241.817,1236.653,1231.791,1227.814,1222.685,1216.704,1213.809,1211.689,1209.077,1194.893,1186.334,1183.972,1180.211,1177.198,1168.506,1163.923,1160.133,1156.234,1152.521,1149.273,1145.503,1141.822,1139.156,1135.874,1132.701,1129.627,1126.831,1123.926,1120.539,1117.607,1114.56,1111.498,1108.762,1104.605,1101.91,1097.481,1094.377,1091.586,1088.562,1085.43,1082.81,1080.12,1077.777,1074.87,1071.49,1068.099,1065.204,1062.523,1061.306,1060.234,1057.835,1054.764,1051.733,1048.594,1045.1,1042.302,1039.801,1037.282,1034.753,1032.478,1029.849,1026.829,1024.515,1021.843,1018.515,1015.936,1013.74,1011.564,1010.643,1008.582,1006.353,1003.074,1000.232,997.9284,996.2999,994.2661,992.0505,988.474,985.1758,983.9111,982.3954,981.4497,980.283,979.3804,977.5474,975.9078,975.0197,973.4412,973.4788,973.0339,971.4778,971.1782,970.8883,970.1133,969.9698,969.423,968.6642,967.108,966.9895,965.777,965.0995,963.8856,960.9402,958.9532,955.7206,953.4633,952.5434,951.657,949.7582,948.6243,946.6808,945.0783,943.2533,941.9792,940.6223,938.8395,938.4114,937.1309,937.2914,937.7529,942.3002,943.5577,946.231,946.236,946.0404,946.2648,946.5434,947.8699,948.26,950.7552,954.6406,957.5592,959.5428,961.7601,963.821,966.2257,968.3187,969.798,972.2574,974.3385,976.7394,978.8225,981.635,983.3631,987.8701,989.9705,993.3728,996.2413,999.6278,1003.49,1006.849,1009.38,
1820.996,1822.548,1820.131,1809.07,1805.779,1804.317,1809.241,1788.589,1752.627,1751.759,1744.151,1722.967,1701.651,1682.821,1678.908,1671.083,1646.339,1629.216,1623.67,1615.043,1606.236,1602.275,1597.189,1592.937,1588.924,1583.785,1579.062,1575.05,1570.805,1567.303,1560.615,1553.717,1545.112,1514.13,1493.034,1481.883,1477.236,1471.323,1463.945,1458.06,1443.24,1430.642,1427.854,1422.939,1417.385,1412.753,1407.545,1403.055,1397.988,1388.416,1383.533,1377.522,1369.919,1344.348,1338.112,1332.776,1330.757,1326.895,1314.24,1309.126,1304.532,1300.123,1295.571,1293.446,1290.336,1287.343,1284.286,1281.866,1278.914,1275.749,1272.892,1269.297,1265.67,1261.136,1257.255,1252.05,1246.049,1242.746,1239.271,1232.781,1228.037,1222.509,1218.998,1212.688,1207.847,1202.27,1197.187,1189.158,1186.447,1181.283,1177.384,1173.295,1168.679,1164.907,1161.203,1157.018,1153.33,1150.175,1147.047,1143.557,1141.065,1137.312,1134.176,1131.061,1128.036,1125.199,1121.742,1118.266,1115.547,1112.317,1108.354,1105.29,1102.188,1098.694,1095.333,1092.764,1089.804,1087.031,1084.194,1081.575,1077.639,1074.484,1070.745,1069.068,1067.581,1066.265,1064.478,1061.695,1058.869,1055.455,1052.071,1049.486,1046.648,1044.393,1041.394,1038.187,1035.583,1032.881,1030.466,1028.342,1026.147,1023.428,1020.156,1017.436,1015.032,1012.929,1011.654,1010.724,1008.515,1006.037,1001.639,998.4608,996.1437,993.8891,991.6714,989.4387,987.1576,985.2901,984.0461,983.0309,981.7968,980.4294,979.013,977.8002,976.4792,975.0498,974.0248,974.1934,972.4255,972.3623,971.642,971.2946,971.0268,970.1774,969.5154,968.2856,967.6724,966.6665,964.6934,963.2338,961.1815,959.6147,957.2783,955.9763,954.4902,952.7921,951.6712,949.793,947.5072,945.6597,943.713,942.0775,940.5974,939.1397,937.8024,938.2876,938.582,940.3555,943.9482,945.5461,947.4478,948.5512,948.4376,949.3589,949.5441,950.9785,952.0478,953.3289,955.8384,958.5031,961.1001,963.0172,965.2374,967.5645,969.9404,972.1611,974.8082,977.0409,979.2128,981.0519,983.1072,985.5232,988.5928,991.3057,994.2036,997.7331,1000.623,1003.764,1008.022,1011.724,
1823.399,1825.215,1806.045,1786.151,1778.392,1777.39,1777.89,1755.535,1742.134,1742.469,1742.507,1711.458,1684.948,1677.817,1674.277,1649.912,1630.179,1625.7,1620.745,1608.569,1601.537,1597.726,1593.587,1590.31,1585.354,1580.581,1576.295,1573.011,1569.303,1564.073,1557.587,1551.422,1522.936,1496.09,1490.209,1484.575,1481.293,1476.165,1467.102,1460.869,1437.406,1432.91,1429.12,1425.085,1419.188,1414.624,1408.871,1400.23,1396.66,1388.802,1380.398,1373.113,1357.166,1345.871,1339.979,1335.251,1331.039,1320.61,1313.018,1309.714,1305.821,1301.441,1296.938,1292.057,1288.182,1284.927,1280.951,1278.589,1275.328,1272.602,1269.934,1266.846,1263.04,1259.035,1255.563,1249.804,1242.202,1236.632,1232.244,1226.873,1223.954,1219.835,1215.811,1208.042,1203.852,1197.62,1192.958,1189.629,1185.677,1182.046,1177.233,1173.224,1169.197,1166.23,1162.869,1159.16,1154.844,1151.962,1148.468,1144.973,1142.352,1139.036,1135.876,1132.315,1129.23,1125.703,1122.405,1119.709,1116.059,1112.677,1109.834,1106.726,1103.805,1099.675,1096.366,1093.83,1090.802,1087.702,1083.975,1080.575,1076.877,1075.399,1073.803,1072.497,1071.032,1068.402,1065.921,1062.672,1059.06,1055.982,1052.986,1050.821,1049.077,1045.993,1042.94,1040.247,1037.183,1034.539,1032.326,1029.965,1027.438,1024.384,1021.772,1019.161,1016.307,1014.145,1012.611,1012.176,1010.799,1008.997,1002.934,998.719,995.8478,993.5752,991.4055,989.8382,988.6244,987.232,986.1135,984.6936,983.0832,981.5626,980.4936,978.9495,977.7902,976.1481,975.1526,974.9767,973.7639,973.1094,972.7419,972.5475,972.1168,971.0405,969.943,969.5301,968.2242,966.5576,964.9271,963.4139,962.2524,960.9767,959.3914,957.9935,956.4733,954.8483,952.8976,950.6191,948.3497,946.4625,943.942,942.0418,941.0264,939.9847,940.0703,938.9243,939.8665,941.8906,945.5324,947.0857,948.4613,950.0876,950.6666,951.2463,951.7192,953.0685,954.1177,956.0013,957.6356,959.9224,961.9638,963.759,965.8946,968.6808,970.9043,973.5078,976.4432,979.196,981.2705,984.3896,986.2092,988.092,990.1235,992.8178,995.9222,998.8959,1002.36,1005.121,1008.639,1011.836,
1819.654,1818.753,1803.615,1775.935,1758.774,1759.326,1764.122,1739.96,1736.177,1729.076,1708.423,1682.857,1668.46,1659.233,1639.378,1629.867,1626.658,1621.63,1615.507,1609.343,1598.728,1594.416,1590.877,1586.696,1581.274,1578.167,1575.009,1570.926,1565.775,1560.829,1553.354,1547.903,1540.022,1502.736,1495.619,1490.538,1485.386,1474.724,1464.8,1455.324,1436.157,1432.737,1428.958,1425.016,1420.511,1416.069,1412.484,1390.917,1387.715,1381.241,1371.022,1363.836,1355.225,1348.452,1342.888,1336.024,1328.36,1316.478,1311.862,1309.108,1305.353,1301.521,1298.23,1293.303,1289.64,1284.403,1279.706,1276.557,1272.63,1270.49,1267.89,1264.33,1261.307,1257.145,1253.413,1247.442,1238.887,1233.581,1227.862,1222.22,1218.128,1214.882,1211.795,1204.63,1200.981,1196.119,1192.445,1188.885,1184.979,1181.163,1177.33,1173.967,1171.012,1167.744,1164.592,1160.609,1157.726,1153.145,1149.64,1145.988,1143.367,1140.338,1137.097,1134.004,1129.593,1126.691,1123.6,1120.475,1117.419,1114.449,1111.427,1108.576,1106.095,1103.005,1097.57,1094.046,1091.14,1087.773,1084.646,1081.638,1079.4,1078.481,1077.262,1075.388,1072.826,1069.999,1066.762,1063.892,1061.239,1057.924,1055.682,1052.862,1050.965,1048.202,1045.234,1042.177,1039.13,1036.973,1034.01,1031.122,1028.611,1025.799,1023.5,1020.993,1018.153,1015.184,1013.643,1012.438,1010.673,1010.028,1004.902,1000.718,997.8262,994.5983,992.6526,991.6528,990.2801,988.2428,987.0215,985.4448,983.8517,982.6815,981.9398,980.2515,977.9395,976.7056,975.997,975.6249,975.2136,974.0493,973.7919,973.6508,972.9725,971.8672,971.0814,970.5483,969.4764,967.8878,966.2416,965.1423,963.7082,963.1996,962.4635,960.6344,958.6757,956.3947,953.9941,951.8206,949.847,948.6259,944.963,943.1437,941.6424,940.8548,940.6711,940.2319,940.2769,943.9084,946.4017,947.6776,949.1279,951.8129,952.9001,953.1264,953.7422,954.6438,955.9474,956.9839,959.2665,960.7917,962.858,964.517,966.6431,968.8936,971.5002,973.7492,977.1102,979.3304,981.7211,984.8561,987.8306,990.3971,992.9357,994.5524,996.7527,1000.324,1003.303,1005.981,1009.401,1012.216,
1789.629,1780.306,1784.287,1768.857,1736.733,1727.198,1730.595,1730.247,1726.761,1710.014,1682.81,1672.68,1656.421,1636.497,1630.973,1627.534,1623.437,1617.657,1613.239,1606.745,1598.097,1591.193,1588.432,1582.778,1578.342,1575.376,1572.35,1567.261,1560.808,1556.414,1550.34,1544.204,1528.043,1514.582,1497.212,1491.943,1482.923,1469.225,1456.545,1448.697,1435.763,1432.101,1428.503,1425.171,1420.773,1401.888,1399.243,1385.76,1382.384,1374.206,1369.078,1362.71,1356.227,1350.612,1344.479,1337.016,1324.448,1315.027,1311.445,1308.478,1304.655,1301.04,1296.667,1292.948,1288.82,1285.003,1280.758,1276.842,1271.464,1267.479,1264.191,1261.604,1260.038,1255.404,1250.899,1243.502,1235.76,1230.246,1224.196,1219.977,1216.258,1211.551,1207.996,1203.979,1199.89,1196.274,1192.158,1188.877,1185.346,1182.039,1179.17,1175.71,1172.462,1169.585,1165.884,1162.515,1158.76,1155.366,1150.854,1147.338,1145.192,1141.77,1138.664,1134.704,1131.299,1128.449,1125.542,1122.48,1118.877,1116.081,1113.012,1110.214,1105.544,1101.614,1097.235,1095.541,1092.918,1089.135,1086.719,1085.101,1084.423,1082.26,1079.953,1077.552,1074.286,1071.458,1069.32,1065.292,1062.742,1060.418,1057.277,1053.527,1051.636,1049.658,1046.837,1044.483,1041.538,1038.323,1035.14,1032.191,1029.315,1027.326,1025.011,1022.45,1019.783,1016.851,1014.466,1012.564,1010.263,1008.044,1005.648,1002.983,1000.13,996.2477,994.5576,993.6637,992.3394,990.1715,987.9116,986.2412,984.6212,983.7969,983.1417,981.2087,979.3293,977.5877,976.7535,976.2576,976.4862,975.0685,974.891,974.8425,974.0024,973.2222,972.5927,972.2697,971.3792,969.96,968.105,966.8485,965.4792,964.2231,963.2681,961.6957,959.5492,957.41,955.6383,953.3976,950.5607,947.5213,946.8297,945.0604,943.3339,941.7097,941.1946,941.2231,940.517,941.8058,946.0321,948.1545,949.5239,951.4255,953.7556,955.0823,955.4573,956.9024,957.7894,958.5546,960.4258,962.3544,963.7468,965.7784,967.6003,970.2183,971.8196,973.7668,976.9244,979.7397,982.3525,985.4207,989.0181,991.8399,994.549,996.953,998.537,1001.328,1004.035,1007.278,1010.118,1012.995,
1784.783,1768.949,1770.074,1757.122,1730.205,1721.988,1722.995,1722.143,1713.684,1690.979,1677.363,1662.213,1646.963,1632.941,1629.037,1624.381,1619.427,1613.977,1610.598,1606.528,1595.809,1585.259,1583.981,1578.308,1575.443,1572.958,1566.737,1562.301,1557.416,1553.211,1547.495,1540.55,1520.067,1507.032,1501.633,1494.688,1482.152,1459.96,1447.053,1440.969,1436.414,1432.281,1428.426,1422.965,1414.786,1394.685,1387.729,1381.742,1376.731,1371.771,1366.893,1361.514,1357.24,1351.336,1345.378,1336.546,1320.756,1315.088,1311.58,1307.708,1303.984,1300.401,1296.158,1292.532,1288.608,1284.397,1280.174,1276.167,1271.887,1266.895,1261.393,1258.582,1256.203,1252.516,1245.79,1239.018,1231.248,1225.953,1221.518,1217.824,1214.181,1211.065,1207.938,1204.152,1200.989,1197.346,1194.035,1190.167,1186.547,1183.094,1179.912,1176.653,1173.882,1170.126,1166.774,1162.837,1159.601,1156.038,1152.339,1148.732,1145.803,1142.448,1139.421,1136.046,1132.696,1130.025,1126.767,1123.068,1119.901,1115.875,1112.728,1109.561,1104.265,1101.126,1100.196,1097.908,1095.072,1092.214,1090.143,1089.311,1086.974,1084.169,1081.852,1078.606,1075.35,1072.865,1069.856,1066.555,1063.558,1061.558,1057.99,1055.578,1053.263,1049.981,1046.624,1044.561,1041.718,1038.936,1036.203,1033.031,1031.063,1028.217,1025.902,1023.598,1021.265,1018.622,1016.852,1015.064,1011.694,1007.017,1005.262,1004.38,1001.074,997.16,996.3145,995.5992,994.3912,992.0098,989.1376,986.9172,985.9908,985.0214,984.0457,982.5344,981.1741,979.3542,977.4617,977.2305,977.1971,976.2605,976.1204,975.8798,975.1029,974.4833,974.5955,974.4013,973.6514,972.353,970.3672,968.6597,967.1155,965.1902,964.1364,962.7082,960.5135,958.2891,955.8897,954.7361,951.6526,948.4404,948.0526,947.3558,945.7116,944.0468,943.4342,942.1042,941.1042,942.5025,946.1436,947.9237,949.5349,951.158,953.938,956.0149,956.7914,957.904,959.7761,960.51,961.8625,963.4836,965.4631,967.1384,968.6691,970.6652,972.5915,974.6836,977.3719,980.2789,983.4119,986.6431,989.3786,991.9434,994.7442,997.3802,1000.527,1003.157,1005.808,1008.997,1011.818,1015.137,
1792.8,1748.875,1750.497,1739.807,1724.814,1714.307,1717.051,1715.299,1695.541,1681.439,1666.738,1650.283,1637.84,1630.157,1626.586,1621.721,1616.914,1611.901,1606.841,1601.039,1590.321,1581.435,1577.808,1573.27,1571.029,1566.331,1563.535,1556.919,1554.862,1549.317,1541.872,1533.755,1512.885,1503.049,1500.072,1495.109,1481.438,1454.369,1444.684,1439.857,1436.172,1431.872,1428.306,1422.522,1406.077,1392.109,1388.129,1381.695,1377.435,1372.297,1367.764,1363.294,1357.329,1352.31,1342.217,1330.941,1319.832,1315.14,1311.466,1307.586,1303.578,1299.696,1295.43,1291.533,1287.422,1283.422,1279.538,1275.364,1271.674,1267.478,1263.064,1258.121,1254.209,1249.87,1243.556,1236.924,1228.806,1226.756,1224.159,1221.044,1217.059,1213.637,1210.16,1206.288,1203.275,1199.805,1196.248,1192.443,1188.624,1185.036,1181.137,1177.688,1174.397,1170.733,1167.2,1163.733,1159.875,1156.336,1153.324,1150.101,1146.523,1143.364,1139.96,1136.556,1132.819,1129.678,1125.875,1121.641,1119.16,1115.209,1112.14,1109.514,1105.849,1103.711,1102.088,1099.539,1096.776,1094.978,1093.624,1090.889,1088.499,1085.864,1082.92,1079.918,1076.8,1074.352,1071.571,1068.122,1064.895,1062.305,1059.503,1056.285,1054.277,1051.265,1047.875,1045.463,1042.984,1040.026,1037.63,1034.643,1032.584,1029.527,1027.008,1024.636,1022.295,1019.797,1019.017,1017.901,1013.276,1009.337,1006.18,1004.36,1001.428,998.2318,997.324,997.0454,996.2657,993.458,990.5801,987.7747,986.721,985.4793,984.3412,983.3671,983.0444,980.3983,978.6262,978.2736,977.8344,977.7761,977.0624,976.7389,976.1636,975.8506,976.3974,976.2313,975.6375,974.5991,972.7658,970.1426,968.7381,967.2035,966.0425,963.6916,961.4146,959.2,956.5922,954.3913,951.4131,950.015,950.0529,948.8649,946.4656,944.6994,943.3205,942.0267,941.3792,942.8358,946.469,948.9406,950.927,951.6791,952.9783,955.0116,957.7463,959.3154,961.2001,962.8793,963.7081,965.0359,967.1536,968.3828,970.4165,971.7393,973.9396,976.4067,978.9775,982.0054,984.5326,987.2252,989.6821,992.513,995.2078,997.5937,1001.506,1004.083,1007.294,1009.853,1013.551,1016.724,
1775.682,1734.676,1734.138,1718.187,1696.998,1705.016,1712.976,1701.185,1682.214,1667.832,1650.645,1640.663,1628.555,1621.826,1620.334,1617.993,1613.469,1607.302,1600.568,1592.712,1584.663,1579.583,1576.233,1572.745,1568.558,1562.547,1558.789,1554.04,1550.181,1542.135,1534.204,1527.388,1509.664,1498.585,1493.117,1487.405,1469.797,1450.239,1441.322,1437.938,1435.12,1430.927,1426.479,1421.651,1406.219,1395.424,1390.106,1385.341,1380.132,1372.77,1368.004,1364.036,1358.261,1348.239,1337.143,1326.078,1321.458,1317.615,1314.443,1307.394,1302.288,1297.97,1294.354,1290.724,1286.037,1281.955,1278.117,1274.429,1270.503,1266.841,1262.917,1258.809,1255.344,1249.633,1242.583,1236.911,1232.252,1229.27,1225.7,1222.605,1218.987,1215.297,1211.529,1207.709,1204.679,1200.904,1197.215,1193.354,1189.412,1186.544,1182.799,1178.52,1175.912,1171.642,1168.068,1164.345,1160.63,1157.362,1153.671,1150.348,1147.166,1143.646,1140.271,1135.586,1131.011,1127.409,1124.48,1122.247,1120.472,1117.101,1113.614,1111.142,1108.831,1106.136,1103.588,1101.062,1099.308,1097.848,1095.151,1092.564,1090.308,1087.176,1085.065,1082.019,1079.657,1076.437,1073.167,1069.584,1066.443,1063.392,1060.56,1057.588,1055.37,1053.3,1049.838,1046.321,1044.583,1041.166,1039.213,1036.128,1033.528,1030.913,1028.433,1025.923,1023.401,1021.469,1020.125,1018.1,1015.042,1011.847,1008.486,1006.172,1002.904,998.3918,997.7169,997.3944,996.8599,995.1723,991.9566,988.9167,987.1284,985.6312,984.46,983.5822,982.8121,980.2819,979.3829,979.4006,978.8387,978.6005,978.62,977.6705,977.1382,977.763,978.1788,977.9338,977.2756,975.8972,974.0455,971.2867,970.3602,968.8875,967.5355,965.3121,962.8053,960.2795,957.4138,956.5147,953.149,950.8832,951.1319,950.7707,948.0608,945.1591,943.0695,942.2288,942.3525,943.7552,946.1315,950.2595,952.5419,952.9967,953.2777,954.7935,957.2295,960.1267,962.0593,964.2589,965.5581,967.1372,968.3949,970.3254,972.0854,973.9124,975.6072,977.6638,981.0767,983.7119,986.4413,988.4021,990.3998,993.5469,995.8817,998.331,1001.289,1005.112,1007.938,1011.264,1014.335,1017.462,
1754.671,1723.979,1717.618,1713.405,1691.078,1692.133,1699.276,1693.338,1676.263,1665.037,1654.979,1640.356,1630.312,1622.719,1615.25,1609.674,1604.655,1600.678,1595.224,1591.046,1584.578,1577.556,1573.892,1570.863,1566.639,1560.025,1556.716,1553.011,1549.493,1544.674,1537.58,1529.934,1507.299,1491.958,1486.999,1481.329,1470.427,1446.679,1439.215,1434.872,1431.804,1426.727,1421.424,1414.603,1402.569,1396.772,1392.304,1387.758,1380.286,1373.773,1368.79,1364.56,1360.029,1347.81,1332.231,1327.728,1324.068,1321.131,1316.649,1311.266,1301.732,1297.314,1293.121,1288.862,1284.562,1281.085,1277.316,1272.871,1269.102,1264.94,1261.083,1257.946,1253.21,1247.405,1241.594,1237.343,1233.85,1230.306,1226.342,1223.064,1219.754,1215.559,1211.761,1208.225,1204.186,1200.585,1196.992,1192.957,1189.292,1185.647,1181.832,1178.586,1175.109,1171.864,1168.247,1164.869,1162.135,1158.294,1154.67,1151.201,1147.056,1142.78,1139.54,1136.274,1130.91,1128.828,1126.624,1124.32,1121.801,1118.655,1116.553,1113.151,1110.774,1108.912,1106.584,1104.059,1101.62,1099.654,1096.712,1093.743,1091.128,1088.53,1085.143,1082.733,1080.406,1077.049,1074.533,1071.021,1067.788,1065.1,1062.241,1059.408,1056.549,1054.64,1051.506,1048.009,1045.552,1043.187,1039.997,1037.376,1034.67,1031.852,1029.481,1027.051,1024.747,1023.351,1021.207,1018.391,1015.283,1011.987,1009.112,1006.039,1001.091,998.1491,997.5955,997.0047,996.3456,995.7194,993.8411,990.5537,987.2194,986.4329,984.8422,983.4718,982.7137,981.4035,980.3167,979.8635,979.7242,979.2863,979.5512,979.3848,979.3856,979.7138,979.5323,978.956,978.0714,976.8156,974.9456,972.6588,971.3528,969.9488,967.1121,964.3523,962.2056,959.772,958.787,958.0826,956.8858,956.4531,953.1923,951.4784,948.1407,945.7522,943.8218,943.4341,943.0519,944.0587,947.311,952.0637,953.9293,954.0308,954.9695,956.1121,958.0032,960.2995,963.1568,965.4977,967.3793,968.9093,970.1798,971.6329,973.105,975.3777,976.9049,979.0934,981.2773,984.6041,988.1132,990.3691,992.0341,995.1117,998.0545,1000.618,1003.291,1005.796,1009.251,1012.583,1015.592,1018.314,
1731.446,1716.669,1712.187,1707.275,1685.32,1678.24,1688.435,1685.33,1682.707,1671.772,1662.185,1647.909,1634.046,1625.821,1618.935,1610.212,1604.508,1599.62,1594.505,1589.185,1582.45,1574.399,1569.639,1567.157,1562.036,1558.645,1554.952,1551.181,1547.208,1541.418,1536.22,1528.579,1506.817,1492.991,1480.652,1475.524,1460.062,1443.401,1437.372,1431.874,1426.943,1422.415,1416.396,1409.875,1402.538,1398.35,1394.596,1389.072,1382.473,1376.191,1371.037,1366.154,1360.101,1349.251,1341.848,1333.229,1328.494,1324.244,1317.658,1310.713,1300.449,1295.912,1291.047,1287.559,1283.286,1278.891,1275.335,1271.526,1267.291,1263.629,1259.725,1254.669,1250.746,1246.684,1242.488,1238.847,1235.045,1231.443,1227.209,1223.451,1219.507,1215.987,1211.927,1208.515,1204.417,1200.682,1197.139,1193.57,1189.781,1185.722,1182.541,1179.194,1175.874,1172.082,1168.68,1163.843,1158.82,1155.391,1151.875,1148.486,1145.225,1142.845,1140.715,1137.927,1138.176,1135.32,1132.019,1128.086,1123.091,1120.637,1118.314,1115.777,1114.119,1111.119,1109.149,1105.986,1103.153,1100.527,1097.363,1094.333,1091.31,1088.866,1085.563,1083.489,1080.751,1077.584,1074.923,1072.003,1069.093,1065.9,1063.189,1060.6,1058.005,1055.736,1053.421,1050.147,1046.739,1044.097,1041.281,1038.548,1035.337,1032.668,1030.609,1028.18,1025.882,1024.196,1021.66,1018.897,1015.512,1012.045,1009.154,1005.766,1000.528,998.5026,999.0689,998.2779,996.1706,995.4868,994.2787,992.6603,987.9992,986.5117,985.4862,983.9637,983.3252,982.6246,982.1519,981.189,981.231,980.8802,980.1273,980.2895,981.0685,981.1441,980.5955,979.8116,979.0391,977.591,975.3193,973.0067,971.762,970.3416,966.6147,963.8158,963.1989,960.9177,959.7538,959.7274,958.7263,956.8432,953.7858,951.3256,947.6439,945.5021,944.658,945.0718,945.0652,947.8775,950.7122,952.5413,953.976,955.3881,956.4377,957.511,959.6114,961.413,964.2465,965.8216,967.343,969.1158,971.2559,972.8805,974.4855,976.7819,978.57,981.1384,983.2001,986.0182,988.7715,991.7094,993.9851,996.7358,999.5638,1003.2,1006.587,1008.768,1010.947,1014.003,1017.019,1020.171,
1732.102,1711.549,1707.52,1699.989,1684.751,1674.39,1677.428,1676.979,1677.94,1674.087,1662.03,1647.901,1638.141,1629.981,1625.144,1618.45,1602.511,1598.784,1591.932,1585.137,1577.346,1572.32,1567.844,1560.242,1557.831,1556.884,1553.089,1549.761,1544.708,1539.674,1533.269,1525.961,1504.789,1489.544,1474.624,1466.92,1456.202,1444.155,1438.166,1433.913,1427.239,1420.35,1413.842,1406.73,1401.585,1398.538,1395.738,1392.492,1384.616,1379.582,1372.845,1368.074,1360.015,1353.191,1344.303,1336.983,1331.649,1326.031,1318.829,1310.189,1299.384,1294.861,1289.839,1286.144,1282.286,1278.596,1273.995,1270.513,1266.526,1262.388,1258.187,1254.55,1250.453,1246.8,1242.86,1239.241,1235.449,1231.955,1228.306,1225.03,1222.42,1217.64,1213.303,1209.634,1205.304,1201.526,1198.263,1194.733,1190.2,1186.474,1183.212,1179.779,1176.099,1172.643,1168.362,1162.759,1158.034,1155.723,1153.351,1150.931,1148.897,1146.216,1145.031,1144.398,1143.804,1141.327,1138.365,1135.233,1130.772,1123.611,1121.17,1118.524,1116.38,1112.855,1110.097,1106.698,1103.707,1101.094,1098.037,1094.87,1091.564,1088.418,1085.8,1083.529,1080.915,1077.791,1075.398,1071.946,1069.31,1066.911,1064.952,1061.909,1059.37,1056.849,1054.111,1051.325,1048.24,1045.408,1042.216,1038.991,1035.807,1033.379,1030.872,1028.53,1026.201,1024.126,1021.8,1019.254,1016.245,1012.548,1010.079,1006.775,1000.632,999.2154,999.6967,998.5022,996.4747,994.9451,994.4564,992.5262,990.2597,987.0811,985.5284,984.8124,984.7974,984.015,983.309,983.3437,983.1019,982.3839,982.2755,981.2592,981.0671,981.5556,981.5316,980.5319,979.7095,977.7228,975.0914,972.824,972.1611,970.72,966.4927,964.5125,963.3535,961.8468,961.5159,960.341,959.0187,956.5314,954.8821,952.3259,948.3689,946.298,945.2911,945.6702,947.7305,950.0239,951.9265,952.8712,954.2452,955.8264,957.3891,958.7092,960.5054,962.9015,965.2854,967.3896,969.7291,970.8018,972.5745,974.2484,976.2712,978.3849,980.5671,982.761,985.7472,988.2701,990.5386,992.8687,995.9996,998.486,1001.463,1004.213,1007.924,1010.604,1013,1015.796,1019.196,1022.159,
1728.99,1712.353,1706.494,1700.94,1685.234,1672.585,1667.774,1667.872,1672.98,1664.771,1656.597,1639.3,1632.843,1628.081,1622.346,1621.871,1601.465,1593.889,1586.702,1580.25,1574.626,1570.552,1565.268,1558.607,1551.253,1551.489,1549.548,1546.56,1539.83,1534.746,1529.703,1524.242,1500.747,1481.87,1471.925,1462.894,1455.31,1446.929,1443.287,1436.183,1431.541,1426.252,1420.6,1408.499,1400.208,1394.835,1391.276,1389.129,1382.186,1374.357,1365.749,1358.658,1352.281,1345.654,1337.834,1332.483,1327.627,1320.866,1316.642,1309.247,1301.186,1293.362,1288.855,1285.039,1281.713,1278.317,1273.585,1270.284,1266.331,1262.795,1258.411,1255.027,1251.338,1247.211,1243.49,1239.927,1236.316,1233.374,1230.256,1227.107,1224.054,1220.607,1215.592,1210.18,1206.437,1202.497,1199.501,1195.827,1192.432,1188.826,1185.906,1182.071,1177.76,1172.984,1167.927,1164.202,1161.985,1159.541,1157.236,1153.465,1150.853,1150.037,1147.805,1146.11,1144.101,1140.148,1135.846,1132.945,1129.153,1125.061,1121.977,1119.597,1117.821,1114.417,1110.544,1107.386,1104.974,1101.456,1098.708,1095.115,1092.237,1088.818,1086.094,1083.388,1081.071,1077.805,1075.667,1072.109,1069.105,1066.899,1064.508,1062.647,1060.646,1057.809,1054.892,1051.974,1049.839,1046.257,1042.502,1039.307,1036.799,1034.522,1031.668,1028.876,1026.557,1024.037,1021.876,1019.226,1016.297,1013.406,1010.489,1007.311,1002.18,999.6828,999.8502,998.189,996.5453,995.128,994.1113,993.1449,990.2562,988.4951,988.3102,987.2228,986.4174,985.1657,984.3013,983.8386,983.6985,983.1887,983.0669,982.6842,981.5831,981.8052,981.9974,981.1542,980.2844,977.7624,974.9166,972.575,971.8981,970.4129,967.6328,965.2969,964.3405,963.4001,961.9152,960.4803,959.2953,957.2304,955.5813,953.9,950.8267,949.6578,947.5651,947.8432,949.4235,950.6544,952.7354,953.283,954.4047,956.0779,957.9404,959.0536,960.9912,963.904,965.615,967.5553,969.9089,972.2505,973.9037,975.7366,978.2109,979.8779,982.1851,984.5831,987.0519,989.5623,992.1109,994.1324,996.7592,999.8927,1003.114,1005.646,1008.838,1012.124,1014.548,1017.499,1020.694,1023.94,
1724.563,1712.705,1703.686,1698.225,1691.13,1680.446,1654.458,1659.84,1660.119,1654.864,1648.033,1634.98,1630.405,1625.115,1618.542,1610.126,1600.55,1591.946,1582.438,1577.802,1571.278,1566.426,1560.194,1553.385,1544.313,1546.046,1545.569,1541.417,1536.063,1530.376,1525.03,1520.301,1505.302,1489.147,1478.69,1467.618,1458.77,1454.845,1446.839,1440.523,1433.938,1430.138,1424.074,1411.079,1399.953,1394.431,1386.886,1383.843,1376.24,1365.748,1355.625,1349.47,1344.45,1340.086,1333.967,1326.19,1321.966,1317.558,1312.883,1308.654,1298.259,1292.947,1288.991,1285.386,1282.554,1278.623,1274.006,1270.54,1267.024,1263.225,1259.687,1255.712,1252.017,1248.104,1244.526,1240.719,1237.772,1234.603,1230.981,1227.803,1224.537,1220.839,1217.305,1213.612,1207.282,1204.364,1200.292,1196.01,1193.181,1189.448,1186.245,1183.044,1178.261,1173.111,1169.75,1168.286,1165.104,1161.63,1158.42,1155.896,1154.525,1152.189,1149.403,1146.185,1142.877,1139.746,1135.803,1132.323,1129.989,1126.529,1123.466,1120.286,1117.289,1115.518,1111.949,1108.34,1105.946,1101.836,1099.354,1095.907,1092.421,1088.664,1087.18,1084.244,1082.056,1078.94,1075.574,1072.514,1069.62,1067.271,1065.313,1063.639,1061.209,1058.137,1055.214,1052.286,1049.764,1046.111,1042.865,1040.054,1038.042,1035.875,1033.356,1030.093,1026.927,1024.41,1021.957,1018.94,1015.814,1013.918,1010.524,1007.506,1002.819,1000.331,1000.302,998.3975,996.75,995.4999,994.0117,993.1376,992.2377,990.7057,989.8607,988.6952,987.9127,986.7868,986.1185,985.4042,984.6122,984.0673,983.9388,983.3441,983.5124,982.3701,982.4742,981.9529,980.5577,977.9241,975.3717,972.5226,971.8033,970.1484,968.8736,967.1439,966.5524,964.2567,962.6312,961.1932,959.7004,958.1658,956.6688,955.5201,953.4727,951.6503,950.3276,950.0669,950.1542,951.6207,952.615,953.6062,955.0712,955.8735,957.9845,960.364,962.3379,964.1796,966.306,968.6708,970.3983,972.7498,975.1073,978.0231,980.0136,981.8942,983.3837,984.7646,987.7169,990.2123,992.5556,995.6131,998.6124,1001.192,1004.071,1006.4,1009.196,1012.566,1015.455,1018.629,1021.679,1025.102,
1721.61,1714.703,1699.55,1695.582,1690.128,1678.63,1651.436,1653.175,1654.568,1651.502,1641.935,1632.438,1627.048,1621.022,1614.012,1604.357,1597.398,1588.167,1579.239,1574.957,1567.769,1562.518,1558.309,1535.948,1529.654,1532.139,1540.339,1537.607,1532.254,1525.947,1521.304,1513.129,1501.984,1496.371,1477.224,1467.692,1455.429,1456.675,1454.453,1444.926,1441.443,1432.393,1427.634,1407.125,1401.871,1395.935,1380.483,1375.689,1368.121,1359.786,1347.268,1342.827,1337.533,1333.207,1329.617,1323.646,1319.556,1315.734,1311.32,1306.806,1299.434,1294.105,1290.561,1286.738,1283.174,1279.377,1275.256,1271.289,1267.694,1263.9,1260.064,1256.734,1253.291,1249.24,1246.259,1242.867,1238.186,1234.697,1230.945,1227.296,1223.732,1220.782,1216.747,1213.183,1207.854,1202.957,1200.524,1195.887,1192.376,1188.561,1185.332,1181.32,1177.21,1174.132,1172.418,1169.382,1165.969,1162.935,1160.589,1158.473,1156.537,1153.509,1149.996,1146.614,1143.755,1139.936,1136.43,1133.899,1131.093,1127.79,1124.212,1120.792,1117.889,1116.074,1112.524,1108.99,1105.223,1102.1,1099.277,1096.7,1093.811,1089.662,1087.612,1085.057,1081.961,1078.85,1075.779,1072.971,1070.455,1068.055,1066.119,1064.163,1062.023,1058.406,1055.489,1052.71,1049.802,1046.508,1043.662,1041.83,1039.797,1037.187,1034.504,1031.461,1027.992,1024.822,1021.857,1018.841,1015.787,1013.575,1010.299,1006.975,1003.749,1001.06,1000.941,999.6111,997.2168,995.8651,994.6227,993.3438,992.5035,991.7762,991.1561,990.5884,989.8333,989.296,988.3078,987.1484,986.1624,985.186,985.0921,984.6968,984.0306,983.3439,982.9106,981.9662,980.4088,978.7332,975.639,974.7168,972.3287,971.2012,969.6697,969.4692,968.3844,965.7889,963.6827,961.6108,959.9501,957.6263,956.8913,956.4895,955.1175,951.1503,951.3467,951.2139,950.8779,951.6884,953.114,954.1975,955.3145,956.6,957.4076,960.1499,963.8818,965.474,967.1057,969.6577,971.1939,973.1968,975.4609,977.6453,980.6936,981.8192,983.7233,985.5328,988.6594,991.4819,993.4179,995.9003,998.2907,1001.247,1004.382,1006.98,1010.204,1013.075,1016.157,1019.342,1022.256,1025.16,
1717.769,1705.454,1694.685,1691.203,1684.598,1674.931,1658.072,1646.508,1645.989,1642.268,1636.335,1628.855,1623.088,1614.146,1608.774,1601.26,1593.203,1583.779,1576.467,1570.275,1565.263,1561.052,1555.835,1531.211,1523.679,1525.917,1525.858,1528.075,1527.66,1523.345,1511.865,1493.149,1493.443,1490.895,1470.055,1457.037,1450.844,1450.772,1452.57,1450.928,1441.43,1432.747,1427.178,1412.422,1403.585,1392.737,1381.448,1370.653,1365.03,1356.235,1345.857,1339.704,1335.836,1331.751,1327.37,1323.578,1319.36,1314.883,1310.367,1305.528,1300.28,1296.385,1292.229,1288.069,1284.101,1279.9,1275.789,1271.852,1267.829,1264.055,1260.341,1256.682,1252.922,1249.318,1246.071,1242.342,1237.26,1233.601,1230.312,1226.786,1222.29,1218.118,1214.621,1209.508,1204.926,1201.679,1198.204,1195.457,1192.469,1187.993,1184.253,1182.186,1180.441,1178.992,1175.286,1170.657,1167.419,1164.881,1161.976,1158.792,1155.878,1152.698,1149.46,1146.379,1143.08,1139.853,1136.522,1134.338,1130.772,1127.811,1124.474,1121.589,1118.257,1115.025,1112.419,1108.999,1106.231,1102.734,1099.323,1096.921,1094.119,1090.484,1088.1,1084.998,1082.1,1078.944,1076.105,1073.368,1071.279,1068.823,1066.128,1064.047,1062.475,1058.796,1055.876,1053.12,1050.417,1047.157,1045.167,1043.203,1041.074,1038.702,1035.595,1031.891,1028.695,1025.174,1021.687,1018.795,1015.88,1012.786,1010.126,1007.238,1004.167,1001.51,1001.594,1000.373,998.55,996.6422,995.6755,994.4271,992.9865,992.1915,991.6249,990.8783,989.9952,989.3644,988.6202,988.0368,987.366,986.7715,986.2659,985.2838,984.5824,983.8108,983.3798,982.3955,980.6071,979.6694,976.2164,974.7056,973.5956,972.9043,971.8539,969.9117,968.0667,966.5718,964.8276,961.4692,959.7766,958.6638,957.0446,956.1876,955.2805,953.1493,952.2119,952.301,951.5959,952.0754,953.0107,955.3743,955.9269,957.4457,959.3704,961.1353,963.573,965.5571,967.3262,970.1286,972.0656,974.3781,976.3845,978.228,980.2009,982.0624,984.3598,986.0598,988.388,991.3471,994.1226,996.104,998.4966,1000.772,1003.574,1006.115,1009.94,1013.187,1016.366,1019.219,1022.231,1025.932,
1731.965,1710.632,1689.687,1683.52,1677.809,1661.365,1657.498,1641.915,1639.69,1634.823,1630.826,1624.966,1618.745,1610.856,1604.92,1597.188,1589.399,1580.989,1574.543,1565.424,1563.194,1558.562,1549.735,1520.573,1517.717,1522.584,1520.894,1519.34,1519.311,1512.8,1496.332,1483.031,1483.73,1480.769,1459.443,1446.305,1446.317,1446.358,1447.211,1448.113,1439.491,1431.787,1426.553,1416.121,1401.183,1391.376,1381.776,1373.375,1368.32,1356.528,1344.36,1338.412,1334.62,1330.31,1326.765,1322.863,1319.083,1314.515,1309.595,1305.033,1300.79,1296.902,1292.681,1288.029,1284.002,1279.945,1276.214,1272.01,1268.101,1263.832,1259.618,1256.324,1251.541,1247.186,1244.231,1240.79,1235.871,1232.254,1228.29,1224.127,1220.791,1216.811,1213.64,1210.684,1205.844,1201.701,1198.253,1195.255,1192.799,1189.249,1187.102,1184.01,1181.001,1178.095,1174.495,1171.446,1168.472,1165.632,1163.747,1159.25,1155.936,1153.124,1149.607,1146.231,1143.067,1139.672,1136.665,1133.597,1130.334,1127.284,1124.804,1120.748,1117.887,1114.471,1111.638,1108.606,1106.576,1102.534,1099.161,1096.318,1093.621,1091.162,1088.107,1084.961,1082.223,1079.231,1076.721,1073.981,1070.725,1068.496,1065.36,1063.315,1061.176,1058.198,1055.541,1052.668,1049.965,1047.761,1046.238,1044.179,1042.016,1038.593,1034.994,1032.041,1030.366,1026.359,1022.41,1018.554,1016.428,1013.84,1011.015,1008.069,1005.324,1002.384,1002.247,1002.175,1000.226,998.0374,996.7307,995.1939,993.5848,992.8068,992.3661,991.76,990.7273,990.0949,989.6555,988.8528,987.7496,986.5693,986.164,985.484,984.8269,984.0967,983.9113,983.7677,981.6425,980.018,977.4574,976.4147,975.3091,975.0223,972.9448,971.6339,969.0401,966.3597,964.6587,964.847,961.7139,960.7233,958.7897,958.7402,957.9096,957.2488,955.8019,955.9514,954.2227,953.9707,953.3198,955.0193,956.8378,959.0538,960.2635,962.5723,965.1171,965.4754,967.4014,969.6613,971.1726,973.9323,976.5317,978.8312,981.0255,982.6712,984.1512,986.7835,988.3353,990.726,993.7501,996.0892,998.1938,1001.256,1003.112,1006.45,1009.8,1012.381,1016.13,1019.456,1022.259,1026.252,
1727.247,1717.51,1690.165,1678.236,1663.99,1657.867,1651.681,1636.84,1636.403,1632.766,1624.873,1615.953,1611.158,1604.63,1599.05,1593.082,1584.121,1578.315,1568.568,1560.805,1560.184,1553.85,1545.913,1503.802,1509.567,1514.649,1511.635,1510.064,1511.233,1498.022,1477.925,1473.103,1476.84,1468.315,1450.125,1443.162,1440.219,1441.039,1440.831,1440.273,1435.705,1429.314,1421.607,1410.842,1400.391,1388.246,1381.654,1376.475,1372.069,1355.051,1343.841,1338.516,1332.568,1329.613,1325.389,1321.924,1317.586,1314.076,1309.039,1304.635,1300.942,1297.243,1292.523,1288.15,1284.051,1279.943,1275.526,1271.42,1267.253,1263.071,1258.75,1254.694,1249.451,1245.349,1242.314,1238.318,1234.208,1230.513,1227.275,1224.583,1220.268,1216.816,1213.31,1208.272,1205.228,1201.743,1198.853,1196.943,1194.648,1191.324,1188.047,1184.734,1181.417,1178.292,1174.978,1171.767,1169.051,1165.98,1163.867,1159.392,1155.671,1152.667,1148.668,1146.424,1142.166,1139.436,1136.135,1132.923,1129.996,1126.918,1123.416,1120.604,1118.363,1114.929,1111.173,1108.253,1105.954,1102.384,1099.354,1096.23,1093.707,1090.674,1088.082,1084.95,1081.96,1079.852,1078.26,1075.295,1070.281,1067.561,1064.308,1061.693,1060.013,1057.879,1054.95,1052.171,1049.368,1047.909,1045.635,1044.664,1041.576,1037.857,1034.768,1033.139,1032.3,1028.458,1024.439,1019.976,1017.581,1014.548,1012.151,1010.249,1007.504,1003.878,1002.702,1003.081,1001.904,1000.136,997.7498,995.9474,994.2577,993.3453,992.7997,992.3206,991.59,991.1368,990.6635,989.8223,988.936,987.7348,986.4591,985.4739,984.9548,984.5096,984.3898,984.5365,983.0806,980.9634,978.752,977.1447,977.5485,977.0726,975.761,974.2083,971.732,969.3793,967.7687,966.1592,964.6702,963.3278,960.8546,960.8309,961.1744,959.4081,958.4073,956.5871,957.1077,955.7816,954.7927,956.416,958.3545,959.8522,961,962.1945,963.8022,965.6541,967.7026,969.3569,971.4143,973.5386,976.1693,978.288,979.9288,981.8281,984.5978,986.2699,988.6902,990.885,993.1276,995.8867,998.0836,1000.503,1003.686,1006.139,1008.689,1012.813,1015.207,1018.585,1022.363,1026.109,
1720.603,1708.214,1684.396,1668.137,1660.702,1654.197,1647.833,1636.265,1631.967,1631.065,1623.901,1612.727,1602.969,1598.873,1590.988,1586.63,1580.177,1574.385,1565.245,1556.43,1553.133,1549.717,1540.414,1500.503,1505.474,1508.534,1504.979,1501.911,1495.023,1469.65,1463.678,1464.088,1457.04,1451.711,1446.243,1440.495,1434.538,1434.594,1437.37,1436.7,1433.645,1424.962,1414.218,1405.885,1397.207,1384.514,1377.275,1366.781,1363.803,1351.973,1346.052,1337.257,1331.59,1328.029,1324.628,1320.893,1316.617,1312.783,1308.286,1304.024,1300.249,1295.442,1290.816,1286.617,1282.37,1278.56,1274.282,1269.459,1265.492,1261.145,1256.108,1252.617,1248.586,1244.778,1241.22,1237.875,1234.375,1231.54,1228.45,1225.488,1220.185,1217.704,1213.624,1211.025,1209.058,1205.554,1201.411,1198.122,1195.283,1192.041,1188.895,1185.649,1182.027,1178.8,1175.307,1171.8,1168.896,1165.749,1162.084,1158.816,1155.563,1152.143,1148.853,1146.007,1143.08,1139.204,1136.169,1132.473,1128.918,1126.565,1123.717,1120.044,1117.419,1113.935,1110.397,1107.197,1104.816,1100.981,1098.526,1095.507,1092.208,1089.328,1087.114,1083.681,1081.196,1078.979,1077.229,1073.845,1069.337,1066.495,1063.887,1060.594,1058.709,1057.037,1054.84,1051.959,1048.862,1047.325,1045.383,1042.697,1040.676,1036.649,1034.834,1034.292,1033.734,1030.264,1025.348,1021.847,1018.914,1016.851,1014.802,1012.727,1009.337,1004.476,1003.271,1003.575,1003.379,1001.9,999.9296,997.3885,995.745,995.0673,993.1116,992.952,992.5511,992.3356,991.5266,990.4661,989.7379,988.959,988.2537,987.4371,986.0871,985.4244,985.1295,985.141,981.9751,980.4202,978.8439,977.7489,978.7939,979.1587,978.7167,976.6586,973.9545,973.9343,970.909,969.139,968.6794,967.7229,965.5474,963.8679,963.3298,961.7911,960.6012,958.4204,957.8499,957.5809,958.1829,958.2452,959.223,960.3907,961.8884,962.8257,963.8336,965.8979,967.8359,969.04,971.8975,973.9213,976.7932,978.257,980.4113,981.5808,983.639,985.7568,988.8246,992.3813,993.9706,995.8607,998.424,1000.659,1004.013,1005.764,1008.204,1011.431,1014.64,1018.11,1022.651,1024.406,
1712.618,1694.209,1678.583,1664.66,1657.509,1650.323,1633.234,1618.191,1627.4,1625.862,1618.437,1610.052,1602.657,1595.281,1589.183,1580.503,1574.412,1567.68,1561.594,1554.017,1547.312,1546.803,1513.008,1492.872,1499.06,1502.052,1496.646,1492.328,1477.509,1464.33,1458.157,1456.678,1451.747,1447.666,1442.937,1437.984,1428.778,1430.218,1433.926,1432.645,1426.629,1418.475,1409.522,1397.895,1387.248,1378.591,1368.464,1357.629,1351.331,1347.245,1341.36,1335.892,1332.042,1328.132,1324.1,1320.766,1316.365,1312.654,1307.828,1303.195,1298.825,1294.402,1287.896,1283.898,1279.747,1275.156,1271.962,1266.739,1262.916,1259.695,1255.77,1251.916,1248.133,1244.395,1241.2,1237.762,1234.377,1231.611,1228.99,1224.904,1222.329,1220.25,1217.599,1214.097,1211.345,1207.768,1202.715,1199.13,1195.953,1193.125,1189.994,1186.579,1182.8,1179.358,1175.879,1172.517,1168.799,1165.579,1162.211,1158.845,1156.682,1152.765,1149.414,1146.638,1142.629,1138.955,1135.324,1131.979,1128.168,1126.125,1123.292,1120.176,1116.549,1113.207,1109.907,1106.762,1103.665,1101.082,1097.629,1095.246,1091.343,1088.98,1087.281,1083.432,1080.902,1078.093,1075.522,1072.556,1069.003,1066.454,1064.003,1059.651,1057.178,1056.216,1054.012,1051.161,1049.036,1046.017,1044.638,1041.624,1036.57,1035.482,1034.872,1034.566,1033.951,1030.965,1026.313,1023.251,1021.107,1018.547,1015.678,1012.854,1009.849,1006.325,1004.072,1004.146,1003.901,1002.682,1000.818,999.3341,998.2214,997.9355,995.5391,993.8769,993.7272,993.3561,992.4703,991.1798,990.5242,989.8401,989.2532,987.9916,986.7719,985.8977,984.5748,982.9648,981.1267,980.5561,978.8322,979.3256,979.4763,978.5685,978.6386,977.421,976.0104,973.9504,972.8563,971.0686,970.6727,970.2014,968.0477,966.8419,965.7985,964.4352,962.472,960.2844,961.0592,961.7874,962.3788,961.5093,961.2534,961.6295,962.8651,963.8118,966.108,966.803,967.6804,970.3041,973.0605,974.9858,976.6061,978.8021,980.4503,981.9934,984.9988,987.3749,989.07,991.1342,993.5763,996.4493,998.8047,1001.161,1004.651,1006.047,1009.162,1012.125,1015.065,1017.802,1020.865,1023.255,
1704.367,1683.357,1667.717,1653.352,1647.658,1639.98,1617.396,1614.626,1622.82,1620.405,1614.25,1607.161,1599.404,1592.399,1583.5,1575.677,1570.42,1564.185,1557.55,1550.209,1546.525,1538.817,1499.818,1485.682,1490.835,1489.656,1475.253,1467.269,1463.535,1457.717,1452.594,1449.596,1448.623,1443.174,1437.792,1429.199,1421.8,1425.338,1427.158,1427.64,1426.681,1411.048,1403.353,1389.276,1382.506,1375.523,1364.443,1356.234,1349.205,1344.76,1340.72,1336.662,1332.413,1328.165,1324.594,1320.387,1316.487,1312.39,1307.606,1302.633,1298.094,1292.22,1285.958,1281.755,1276.908,1273.301,1269.543,1266.265,1262.636,1259.136,1254.961,1251.903,1247.64,1244.552,1242.065,1239.107,1237.056,1234.767,1232.185,1228.998,1225.675,1222.361,1219.157,1215.249,1211.547,1207.277,1203.577,1200.298,1196.61,1193.335,1190.033,1188.043,1183.647,1180.174,1176.202,1172.978,1169.913,1165.824,1162.34,1159.179,1156.504,1153.122,1149.892,1146.418,1143.469,1139.508,1135.343,1131.869,1127.947,1125.348,1122.324,1119.719,1116.447,1113.299,1109.451,1106.299,1103.221,1100.569,1097.282,1094.87,1091.806,1088.33,1086.347,1082.942,1080.162,1078.118,1075.12,1071.744,1069.196,1065.682,1062.015,1058.935,1056.953,1054.192,1051.868,1049.48,1047.564,1045.25,1043.742,1039.657,1036.341,1035.858,1035.139,1033.975,1033.681,1031.099,1027.283,1024.491,1021.686,1018.882,1016.094,1013.104,1010.684,1009.267,1006.098,1005.078,1005.058,1003.877,1002.655,1001.528,999.9811,998.9169,998.9697,996.946,994.9916,994.5314,993.2495,992.0549,991.4594,990.4974,989.1586,987.7861,986.8651,985.1405,983.1212,982.9647,983.2389,982.3597,981.4349,981.49,980.51,979.6435,980.2254,980.5234,977.4582,976.0824,974.565,973.2877,972.4603,971.6516,971.5818,970.3196,967.7274,965.0684,963.3924,962.9199,962.6713,964.1745,963.9689,963.2834,961.9246,963.0172,964.058,966.1267,967.5939,968.2585,971.6186,972.2066,974.5121,976.1489,977.1646,978.7917,981.512,983.6922,986.3292,988.7108,990.5063,992.2333,994.1339,997.415,999.8118,1002.344,1005.305,1006.724,1009.785,1012.48,1014.822,1017.3,1019.707,1022.351,
1683.882,1669.704,1644.054,1631.778,1623.195,1618.385,1614.774,1610.601,1615.51,1614.411,1609.795,1603.304,1595.571,1587.594,1579.416,1570.792,1567.072,1560.366,1555.499,1547.153,1540.082,1531.945,1490.197,1478.76,1478.9,1475.513,1464.778,1462.068,1459.36,1453.591,1445.665,1441.574,1439.123,1436.651,1432.879,1416.419,1415.835,1415.504,1413.856,1411.663,1408.168,1398.729,1393.496,1385.212,1379.877,1373.432,1363.849,1355.12,1348.691,1343.857,1339.923,1336.081,1331.892,1327.91,1323.997,1319.735,1315.894,1311.808,1307.659,1303.616,1296.075,1290.146,1284.604,1281.048,1277.373,1273.448,1269.636,1265.838,1262.891,1259.584,1255.656,1250.602,1248.975,1246.468,1244.367,1242.105,1240.472,1237.719,1233.798,1230.004,1226.71,1223.768,1220.028,1216.059,1212.123,1207.927,1204.147,1200.014,1196.971,1193.705,1190.457,1187.378,1183.931,1180.924,1177.521,1173.87,1170.219,1166.355,1162.677,1159.331,1156.06,1153.36,1149.777,1145.959,1142.605,1139.237,1136.099,1131.392,1128.593,1125.641,1122.135,1119.375,1116.205,1113.205,1109.479,1105.764,1103.01,1100.211,1096.077,1092.978,1090.962,1088.699,1085.464,1082.349,1079.897,1077.021,1074.549,1071.204,1068.384,1065.035,1060.997,1058.632,1056.639,1053.28,1050.687,1048.909,1046.882,1044.721,1042.662,1040.043,1036.843,1037.201,1035.017,1033.557,1033.264,1031.552,1028.321,1025.766,1022.953,1019.938,1016.634,1013.333,1011.285,1009.833,1008.379,1006.933,1005.773,1005.763,1004.247,1003.258,1001.982,1000.828,1000.583,998.7795,996.3864,995.6287,994.1754,993.1597,991.9811,990.6451,989.2371,987.2776,986.3114,984.1834,983.7535,985.1315,986.2752,984.9796,984.5369,983.4584,982.011,982.0026,983.379,982.3525,979.1219,978.285,976.8673,976.1016,974.9998,974.0078,973.3499,972.577,970.0304,968.602,967.0376,966.8534,966.8295,965.4738,965.4285,964.7899,965.4393,965.4939,966.0869,967.7997,970.354,970.9395,972.626,974.1377,975.841,977.4853,979.3448,980.8042,982.8561,985.1774,987.8756,989.7069,991.3024,994.0326,996.1132,999.3594,1001.351,1003.782,1006.589,1009.392,1010.888,1012.808,1014.852,1017.145,1019.389,1021.902,
1660.295,1640.443,1635.274,1627.304,1617.659,1610.684,1610.464,1608.004,1605.43,1606.383,1604.602,1598.207,1591.587,1584.118,1574.488,1567.044,1562.316,1558.298,1551.573,1543.649,1536.802,1519.386,1489.572,1471.606,1465.284,1464.794,1459.69,1457.831,1456.232,1446.925,1441.959,1438.859,1433.973,1426.358,1419.078,1409.514,1406.633,1405.021,1404.833,1403.783,1398.158,1392.061,1386.474,1380.238,1376.514,1370.473,1361.773,1351.318,1347.825,1343.612,1339.729,1334.823,1331.028,1327.038,1322.904,1318.337,1314.663,1309.464,1304.041,1299.395,1293.434,1288.893,1285.954,1282.54,1279.083,1274.603,1271.956,1267.933,1263.281,1259.18,1256.288,1254.008,1251.998,1249.867,1247.239,1244.077,1241.366,1238.05,1234.363,1230.849,1227.264,1224.361,1221.06,1217.993,1213.708,1208.257,1204.242,1200.374,1196.839,1193.891,1190.618,1187.233,1183.855,1181.642,1177.737,1173.809,1170.822,1166.96,1162.865,1159.595,1156.453,1152.863,1149.064,1146.037,1142.088,1139.156,1135.469,1131.997,1129.984,1125.866,1122.188,1118.944,1116.115,1112.511,1109.601,1106.373,1102.487,1100.584,1096.335,1092.861,1090.386,1088.122,1084.578,1081.296,1079.542,1077.574,1074.393,1070.486,1066.894,1063.387,1061.341,1059.388,1055.871,1052.824,1049.998,1047.897,1045.997,1044.401,1042.97,1040.859,1037.49,1036.987,1035.206,1033.197,1032.726,1031.558,1029.005,1027.048,1023.54,1020.134,1016.87,1013.622,1012.035,1010.962,1009.325,1008.432,1007.355,1006.432,1005.798,1004.778,1004.486,1003.021,1000.118,998.2015,997.5884,996.6471,995.0663,994.5074,993.0473,991.0786,989.7042,986.2275,985.1444,985.1326,984.5041,986.0722,986.3286,986.7709,987.295,986.8603,984.8896,985.4507,985.5797,983.6968,982.2529,980.7692,979.7083,978.6369,977.9149,975.4921,975.2427,973.5528,972.8029,972.1398,970.0181,969.6127,969.6539,967.6566,967.6446,968.7098,968.3782,968.9531,969.6717,972.0003,973.5829,975.3445,975.9781,976.2557,976.8014,979.3585,981.1719,982.3103,984.251,987.2185,989.8207,991.6423,995.014,996.0161,998.3492,1000.861,1004.25,1006.641,1009.437,1012.361,1013.627,1015.709,1017.129,1017.953,1020.008,1021.948,
1650.012,1644.189,1637.968,1633.356,1622.522,1611.418,1602.302,1598.494,1596.224,1597.482,1595.493,1592.465,1586.906,1579.973,1570.033,1564.354,1558.625,1554.509,1549.416,1540.869,1532.194,1503.779,1477.972,1467.147,1461.95,1460.13,1456.711,1452.78,1449.312,1442.628,1439.916,1435.977,1429.216,1417.217,1409.052,1404.564,1402.298,1400.125,1399.496,1397.148,1394.115,1388.66,1382.791,1376.146,1369.356,1362.821,1356.008,1351.302,1347.256,1343.355,1338.523,1334.202,1330.126,1325.978,1321.594,1316.449,1312.201,1306.848,1302.354,1297.251,1293.486,1289.492,1286.811,1283.739,1280.932,1277.144,1272.756,1267.714,1264.361,1261.833,1261.442,1257.485,1255.515,1251.573,1248.107,1245.692,1242.304,1238.183,1234.67,1230.951,1227.647,1224.599,1219.943,1217.35,1214.025,1208.254,1204.451,1200.693,1196.922,1194.093,1190.709,1187.366,1184.394,1181.731,1178.299,1173.855,1171.12,1167.615,1163.007,1159.685,1156.182,1152.799,1148.859,1145.288,1142.22,1138.622,1134.977,1132.021,1129.464,1125.175,1121.985,1118.606,1115.125,1111.966,1110.47,1106.974,1102.565,1099.374,1096.462,1092.484,1090.056,1087.051,1084.181,1081.903,1078.559,1076.392,1074.29,1070.575,1066.765,1062.958,1060.816,1058.218,1055.925,1052.991,1050.078,1047.819,1045.314,1043.013,1041.92,1040.235,1037.955,1037.554,1034.735,1032.66,1031.919,1031.514,1029.128,1026.362,1023.29,1019.95,1016.42,1013.346,1012.915,1012.3,1010.729,1009.647,1009.182,1007.168,1007.086,1005.749,1005.004,1002.73,1000.683,999.5785,998.6946,997.4588,996.0475,995.441,993.913,991.6514,990.3759,988.5024,986.9544,987.6944,987.098,988.006,987.2536,987.8419,988.6324,988.4534,988.0978,987.045,987.4616,986.7074,985.5113,984.1564,983.0172,981.8003,980.1259,978.7719,978.6226,976.1228,974.6756,973.675,973.0643,970.8161,970.204,969.4138,970.6412,972.6225,971.3472,971.7531,974.6174,975.6882,976.6245,977.3896,977.8915,979.4992,980.009,981.8173,983.1897,984.889,986.7628,989.079,990.5652,993.5402,995.8722,998.5323,1000.568,1004.035,1006.57,1009.447,1012.036,1015.104,1016.798,1018.4,1020.321,1021.097,1021.411,1021.838,
1654.153,1646.771,1639.89,1633.955,1627.078,1616.411,1607.71,1595.537,1589.752,1592.184,1587.312,1580.18,1583.596,1569.578,1566.09,1558.924,1557.106,1552.236,1544.366,1537.999,1506.695,1497.319,1475.031,1464.294,1459.614,1456.273,1455.353,1451.885,1442.701,1440.464,1437.563,1425.44,1417.125,1410.698,1405.58,1401.972,1399.258,1397.076,1394.606,1393.312,1390.433,1385.662,1381.522,1371.671,1365.052,1360.791,1356.423,1351.943,1347.382,1342.455,1337.801,1333.166,1328.671,1323.697,1319.615,1315.318,1310.574,1305.963,1301.441,1298.224,1294.977,1292.243,1290.212,1286.629,1281.844,1276.813,1272.746,1270.094,1267.514,1265.429,1263.395,1260.435,1256.544,1251.861,1248.552,1245.877,1242.346,1238.049,1234.632,1230.571,1226.937,1223.097,1219.286,1215.974,1213.229,1208.591,1204.638,1200.625,1197.061,1193.751,1190.651,1187.395,1183.937,1180.033,1176.717,1173.057,1170.878,1166.436,1162.614,1159.059,1155.835,1152.266,1148.996,1145.169,1142.191,1138.79,1135.457,1131.28,1128.417,1125.045,1121.733,1118.05,1114.585,1111.514,1109.855,1106.11,1102.857,1099.034,1095.342,1092.814,1090.243,1087.235,1083.775,1081.539,1078.327,1075.035,1073.493,1069.966,1066.758,1063.553,1060.635,1057.764,1056.073,1053.426,1050.869,1048.484,1045.884,1043.398,1042.623,1040.164,1038.413,1037.767,1034.727,1032.463,1031.359,1031.251,1027.226,1024.85,1022.431,1019.094,1016.261,1014.402,1013.686,1012.878,1011.935,1010.996,1010.114,1008.472,1007.934,1006.642,1004.739,1002.999,1001.606,1000.712,999.4292,998.6805,997.5496,995.4131,993.6903,992.0049,990.441,989.3522,988.9354,989.212,990.0012,989.1162,988.4488,987.9988,990.5659,989.4338,989.3582,989.5732,990.5441,990.0161,988.4266,986.9886,985.2107,984.5892,983.3701,982.2016,980.9646,977.4195,976.5679,976.1005,974.0931,973.3998,972.3265,973.4708,973.2753,974.7835,974.0925,974.5868,978.4269,977.6608,978.8895,980.8175,981.1225,982.4809,983.3911,985.2231,985.6594,987.239,989.243,990.6997,993.4432,996.6846,998.5844,1000.545,1003.091,1005.692,1008.276,1011.712,1014.272,1016.753,1019.896,1021.541,1023.805,1024.597,1024.603,1026.07,
1653.879,1647.497,1643.624,1635.792,1627.572,1622.264,1612.192,1599.731,1586.463,1586.031,1579.169,1573.993,1570.314,1563.631,1558.891,1553.892,1553.804,1548.755,1541.438,1528.168,1512.76,1499.408,1472.169,1461.392,1456.666,1450.507,1450.551,1449.192,1440.102,1437.206,1433.119,1417.37,1410.548,1406.864,1402.849,1399.804,1396.443,1393.202,1388.314,1386.437,1384.519,1381.938,1377.642,1367.711,1364.098,1360.39,1355.81,1351.258,1346.799,1342.514,1336.113,1331.3,1326.841,1322.574,1318.538,1313.638,1309.758,1306.691,1303.243,1299.836,1297.342,1294.718,1290.521,1286.499,1280.744,1277.902,1276.093,1273.339,1271.27,1268.381,1265.023,1261.356,1256.891,1253.09,1249.353,1246.09,1242.256,1238.608,1234.642,1229.849,1226.914,1221.571,1217.285,1214.165,1211.242,1208.72,1204.578,1200.286,1196.556,1193.27,1190.09,1186.272,1182.736,1179.437,1175.526,1172.725,1170.062,1166.339,1162.116,1158.283,1154.975,1151.684,1148.206,1145.368,1141.259,1138.792,1134.801,1131.08,1127.778,1125.014,1121.426,1117.821,1114.17,1110.855,1108.595,1105.526,1102.648,1098.809,1095.745,1092.75,1089.378,1086.711,1083.238,1079.949,1077.647,1074.899,1072.48,1070.018,1066.895,1063.367,1061.04,1057.55,1056.187,1053.577,1050.765,1049.755,1046.341,1043.934,1042.049,1040.275,1038.936,1038.586,1035.259,1032.42,1030.902,1030.703,1028.211,1025.246,1022.042,1018.588,1016.5,1015.527,1014.846,1014.246,1013.449,1012.151,1010.399,1009.268,1008.525,1007.774,1005.095,1004.085,1003.192,1001.723,1000.129,998.7645,997.9697,996.4725,993.9127,992.6044,991.2736,990.4069,990.3431,990.3938,991.0982,990.8976,990.8076,991.5201,991.7592,991.8206,991.8494,992.2576,992.4299,992.1076,990.6317,988.8813,986.5335,986.4131,985.7927,985.1779,983.4583,980.3724,978.57,977.1754,976.5071,975.1251,974.0002,975.0533,976.7711,977.3853,977.6905,978.6047,980.3287,980.4734,981.2178,983.2211,983.8141,985.7225,987.1838,987.8053,988.8972,990.7015,993.1809,994.601,996.3354,998.4965,1000.654,1003.172,1005.142,1007.841,1011.395,1014.354,1017.019,1019.861,1023.272,1025.16,1027.316,1028.59,1028.266,1033.575,
1650.459,1645.496,1641.913,1634.346,1627.694,1623.071,1617.537,1607.22,1592.553,1580.601,1575.245,1572.773,1568.802,1562.018,1556.862,1549.269,1547.801,1546.036,1537.757,1524.175,1505.835,1479.109,1466.127,1457.837,1448.362,1441.764,1439.605,1438.402,1431.941,1433.164,1420.373,1413.638,1408.48,1404.267,1401.072,1397.884,1394.66,1390.754,1386.334,1381.932,1379.109,1375.886,1371.368,1365.816,1361.736,1358.486,1353.58,1348.956,1344.366,1339.195,1334.936,1330.344,1325.804,1321.602,1318.084,1314.82,1311.245,1307.785,1304.955,1301.661,1298.618,1294.124,1289.911,1285.503,1283.175,1281.985,1281.885,1278.933,1274.471,1270.137,1265.866,1261.851,1257.678,1253.775,1250.286,1246.12,1242.561,1238.786,1234.302,1230.289,1226.4,1221.903,1217.405,1211.402,1208.718,1205.444,1201.841,1198.996,1195.849,1192.463,1188.711,1185.785,1181.999,1178.453,1175.168,1171.603,1168.207,1165.687,1161.609,1157.713,1154.093,1151.274,1148.155,1144.43,1140.757,1137.758,1133.981,1130.96,1127.985,1124.918,1121.416,1118.173,1114.633,1111.196,1108.283,1106.643,1102.395,1098.921,1095.834,1092.651,1089.142,1086.432,1082.992,1080.38,1077.618,1074.385,1072.186,1070.164,1066.817,1063.02,1060.94,1058.456,1056.194,1054,1051.563,1049.843,1047.098,1045.14,1043.209,1041.833,1039.651,1039.4,1036.315,1032.257,1030.542,1029.959,1028.197,1025.871,1021.683,1019.364,1017.722,1017.002,1016.12,1015.008,1014.441,1013.927,1012.247,1011.46,1009.429,1008.911,1006.455,1005.371,1004.123,1001.817,1000.895,1000.33,999.6895,999.0659,997.2937,995.2531,993.0988,991.3358,990.4299,991.7997,993.2249,992.5363,992.7256,993.7165,993.4391,993.9236,993.7043,994.8068,994.553,995.6382,993.5527,990.815,988.8951,988.3225,987.3223,985.8889,984.2446,982.6217,980.2546,979.91,978.8198,977.4823,977.9164,977.7761,979.1948,979.9984,980.3713,980.686,982.2361,983.6502,985.2064,985.025,987.4929,988.5349,991.4507,991.908,992.5765,994.3426,997.5045,998.6666,999.7357,999.9562,1002.874,1005.734,1008.135,1010.423,1013.425,1016.636,1019.786,1022.652,1025.507,1028.259,1030.851,1032.051,1032.453,1039.285,
1647.585,1642.981,1636.911,1630.992,1624.552,1619.194,1615.244,1609.423,1596.572,1579.591,1575.305,1570.856,1565.83,1561.678,1556.986,1551.86,1542.382,1537.334,1533.06,1526.88,1492.44,1472.741,1462.124,1456.46,1444.147,1438.663,1432.203,1428.079,1426.698,1419.951,1415.264,1411.104,1406.992,1402.868,1399.718,1396.224,1392.642,1389.433,1385.418,1381.078,1376.74,1372.597,1368.651,1364.383,1360.701,1355.678,1350.911,1346.702,1342.179,1337.495,1334.03,1329.774,1326.63,1323.547,1320.14,1316.572,1312.923,1309.582,1306.096,1302.581,1297.752,1294.515,1291.537,1289.426,1289.387,1287.424,1284.379,1280.707,1277.24,1272.519,1267.212,1263.108,1259.11,1255.784,1252.038,1246.826,1242.732,1238.692,1233.89,1230.3,1226.221,1221.311,1216.765,1212.106,1205.623,1203.606,1200.477,1197.068,1194.336,1191.77,1187.99,1184.39,1181.297,1177.76,1174.408,1171.704,1168.241,1164.859,1161.018,1157.966,1154.133,1150.409,1147.493,1143.535,1140.58,1137.231,1133.723,1130.591,1127.838,1124.285,1120.985,1118.42,1115.173,1111.99,1108.595,1105.086,1101.588,1098.318,1095.264,1092.207,1089.135,1086.014,1082.694,1080.342,1077.894,1075.207,1072.353,1068.454,1065.665,1062.699,1060.902,1059.123,1057.277,1054.511,1052.247,1050.157,1048.053,1046.103,1044.186,1042.377,1040.41,1039.833,1036.63,1032.863,1030.831,1029.604,1028.144,1024.359,1021.328,1019.729,1018.695,1018.338,1017.183,1016.217,1015.224,1014.257,1012.949,1011.627,1010.028,1009.865,1008.286,1006.682,1004.598,1003.191,1002.498,1002.218,1002.165,1001.323,1000.704,998.5314,996.5506,994.538,992.4193,993.3718,994.2062,994.7152,994.9078,995.5159,995.5082,996.726,998.136,997.3207,996.1216,995.4144,994.5916,991.2175,991.1497,989.2786,988.8634,987.9356,986.5173,984.8536,982.7041,982.2423,981.1478,980.0746,979.8782,980.1413,981.8422,982.734,981.84,983.1819,985.1531,986.0504,987.6151,988.7832,989.9493,991.5743,993.0062,995.4602,996.9833,997.8845,999.9251,1001.439,1002.739,1003.874,1006.107,1008.123,1010.371,1013.587,1016.444,1019.298,1022.388,1025.17,1027.726,1030.375,1033.531,1035.753,1037.033,1043.922,
1645.229,1639.986,1633.818,1627.07,1622.09,1615.861,1610.099,1605.993,1598.193,1578.278,1574.481,1569.662,1567.146,1562.519,1555.574,1551.672,1541.711,1533.782,1529.288,1522.448,1482.86,1468.756,1460.458,1451.591,1442.448,1436.859,1432.627,1428.816,1424.85,1420.755,1417.159,1413.12,1408.575,1404.394,1399.88,1395.885,1391.501,1387.767,1383.226,1379.346,1375.495,1372.161,1368.156,1364.154,1358.893,1354.659,1351.499,1346.437,1342.357,1339.766,1335.676,1331.318,1328.321,1324.655,1321.401,1318.306,1314.944,1311.497,1306.942,1303.243,1299.851,1296.894,1294.631,1294.427,1292.88,1289.949,1286.991,1283.381,1280.173,1276.64,1271.163,1269.13,1266.368,1260.679,1257.859,1255.492,1249.593,1241.738,1233.859,1228.965,1225.223,1221.518,1213.691,1207.425,1203.015,1200.789,1198.558,1195.729,1192.335,1189.629,1186.284,1183.488,1180.168,1176.89,1173.566,1171.045,1167.611,1164.036,1161.008,1157.713,1154.467,1149.831,1146.77,1143.508,1140.664,1137.711,1134.212,1130.709,1127.14,1123.569,1120.378,1117.185,1115.023,1111.88,1107.89,1104.032,1100.867,1097.714,1094.641,1091.543,1088.445,1085.57,1082.36,1079.967,1077.452,1074.328,1071.286,1068.433,1065.58,1063.404,1061.208,1058.817,1057.371,1054.571,1052.486,1050.549,1048.614,1046.791,1044.875,1042.864,1040.91,1040.168,1037.327,1032.999,1030.585,1029.128,1028.606,1025.415,1022.635,1020.655,1019.561,1018.996,1018.237,1017.046,1016.076,1014.911,1013.849,1012.583,1010.991,1010.437,1010.445,1007.86,1005.291,1004.578,1003.815,1004.178,1003.692,1003.517,1004.201,1002.575,1001.019,998.8435,994.197,994.7842,995.8668,996.4282,997.0847,997.5712,997.5641,998.0533,998.8199,998.6341,997.0924,995.8951,995.6752,993.6485,993.3775,992.1172,991.1546,989.5358,988.1172,986.4031,983.7811,982.0145,982.5298,983.5547,984.132,983.1949,983.9969,984.0081,984.444,985.2864,987.9197,989.1586,990.7273,992.7458,993.2699,995.5206,996.7219,997.7255,1000.062,1000.929,1003.27,1004.317,1005.853,1007.684,1009.166,1010.242,1012.955,1015.747,1018.561,1021.08,1024.341,1027.436,1029.934,1032.996,1036.179,1038.822,1041.292,1045.602,
1647.251,1646.599,1644.526,1630.152,1619.074,1612.153,1604.524,1602.083,1593.254,1576.783,1571.767,1566.249,1564.267,1562.348,1554.492,1548.392,1539.507,1533.232,1527.417,1494.954,1474.264,1465.414,1456.194,1446.115,1439.655,1434.599,1430.845,1427.512,1423.495,1419.856,1416.202,1413.189,1408.804,1404.648,1400.251,1396.174,1392.084,1387.96,1383.481,1379.748,1375.841,1371.88,1368.033,1363.832,1359.989,1356.154,1352.212,1348.084,1344.896,1342.413,1336.415,1332.852,1329.495,1326.146,1322.646,1318.588,1314.761,1311.498,1309.019,1305.358,1303.446,1301.756,1301.166,1297.905,1295.836,1292.537,1290.064,1289.564,1290.098,1285.633,1281.049,1277.727,1273.35,1268.082,1263.19,1260.016,1252.637,1241.855,1233.632,1227.034,1223.604,1219.564,1211.979,1204.549,1202.306,1198.899,1196.201,1193.848,1190.783,1187.545,1184.685,1181.384,1179.094,1175.618,1172.967,1170.006,1166.103,1163.545,1159.846,1156.654,1153.847,1150.303,1145.91,1142.512,1139.209,1136.026,1133.336,1130.054,1126.437,1122.67,1119.367,1116.676,1113.419,1110.154,1107.032,1103.158,1099.796,1097.101,1093.955,1090.853,1087.754,1084.606,1082.312,1079.796,1077.413,1073.94,1071.071,1068.753,1066.048,1063.618,1060.705,1058.285,1057.212,1054.683,1052.414,1050.465,1048.743,1046.927,1045.152,1042.45,1041.125,1040.411,1036.678,1032.807,1030.437,1029.218,1028.082,1026.738,1023.788,1021.82,1020.863,1020.117,1019.46,1017.557,1017.129,1015.747,1014.498,1013.505,1011.95,1011.258,1011.113,1011.397,1009.133,1006.92,1006.064,1005.843,1005.919,1005.846,1005.827,1005.933,1005.539,1003.82,996.0043,995.5819,996.8004,998.2607,999.2844,999.4631,999.4301,999.448,1000.15,999.9265,997.8768,996.4326,996.088,995.4841,994.6887,993.8884,992.5482,990.1436,988.5314,987.2369,985.6564,984.3408,984.1288,984.8268,984.8989,984.8967,985.2283,985.7295,987.017,988.582,990.2211,991.6981,993.62,994.7142,996.5643,998.7352,999.9463,1001.232,1002.81,1004.668,1006.205,1007.371,1008.293,1010.448,1011.533,1013.415,1015.407,1018.214,1021.045,1023.661,1026.812,1030.113,1032.955,1035.797,1038.778,1041.723,1044.778,1048.136,
1656.485,1651.215,1643.798,1634.68,1622.708,1612.825,1602.746,1595.23,1589.993,1577.478,1568.145,1564.089,1559.739,1557.384,1551.983,1543.978,1536.813,1531.545,1522.588,1489.414,1467.612,1461.312,1448.98,1444.861,1437.53,1433.295,1428.361,1424.596,1420.901,1417.104,1413.409,1409.514,1406.905,1402.662,1398.678,1394.379,1390.688,1386.547,1382.442,1378.747,1374.972,1371.308,1367.594,1364.108,1360.364,1356.728,1352.949,1349.141,1344.589,1340.811,1337.366,1333.25,1329.541,1324.896,1321.479,1319.425,1317.043,1314.089,1312.093,1311.267,1308.464,1305.602,1303.266,1301.436,1299.522,1299.285,1298.698,1298.851,1298.644,1294.654,1287.569,1282.142,1279.176,1273.983,1268.285,1260.414,1253.807,1243.777,1233.974,1224.978,1221.604,1217.094,1211.892,1206.405,1200.92,1197.167,1194.131,1192.274,1189.239,1186.008,1183.121,1179.854,1176.431,1174.378,1171.39,1168.215,1164.532,1161.752,1158.708,1155.594,1152.504,1148.851,1145.426,1141.859,1139.424,1135.631,1133.408,1129.268,1125.26,1121.386,1118.772,1115.602,1113.044,1109.336,1106.094,1103.092,1099.361,1097.054,1093.341,1090.169,1087.063,1083.894,1082.927,1079.761,1077.015,1072.929,1070.395,1067.865,1065.905,1063.058,1060.456,1058.115,1057.215,1054.784,1052.429,1050.121,1047.982,1045.576,1043.817,1041.971,1041.635,1040.425,1036.046,1032.79,1031.368,1029.89,1028.028,1027.26,1026.022,1023.57,1021.982,1021.219,1020.013,1019.169,1017.692,1016.396,1015.534,1014.403,1013.329,1012.359,1011.829,1012.054,1010.452,1009.224,1008.186,1008.488,1007.825,1007.458,1006.524,1006.985,1007.405,1003.415,997.6641,997.3179,998.3201,999.5173,1000.854,1001.373,1001.051,1001.264,1001.533,1000.761,999.6619,998.4954,997.0756,997.8545,996.5945,995.7379,993.5812,992.1454,989.4694,988.9736,987.6437,985.5319,984.4796,985.3572,985.4348,985.464,986.2405,987.0952,988.9131,990.8868,992.493,994.256,995.5854,997.7158,999.8019,1002.215,1002.892,1005.583,1007.277,1007.26,1009.697,1011.617,1012.203,1013.17,1015.063,1016.315,1018.479,1020.787,1023.094,1025.804,1028.602,1032.136,1035.202,1037.982,1041.315,1044.145,1046.949,1050.273,
1659.283,1654.687,1648.916,1635.665,1624.565,1615.995,1607.08,1591.449,1581.812,1573.124,1566.007,1561.761,1558.538,1555.276,1549.548,1540.898,1535.722,1529.084,1511.902,1483.647,1464.594,1458.293,1448.403,1446.113,1439.526,1434.614,1429.647,1425.496,1421.774,1418.12,1414.049,1410.168,1405.972,1401.189,1397.366,1393.762,1390.143,1386.083,1382.219,1378.304,1374.926,1370.897,1367.646,1363.51,1359.474,1356.097,1352.795,1347.409,1344.152,1341.241,1338.626,1334.98,1330.448,1327.198,1324.804,1321.97,1319.463,1317.778,1317.023,1314.323,1311.024,1308.027,1306.286,1305.393,1306.285,1307.021,1306.362,1305.596,1305.677,1301.256,1295.262,1293.295,1283.68,1281.039,1273.638,1267.479,1255.485,1248.522,1234.183,1224.266,1221.027,1216.659,1212.291,1205.628,1199.636,1196.588,1194.105,1190.858,1187.46,1183.553,1180.862,1177.318,1174.498,1171.876,1169.385,1166.953,1163.292,1161.129,1157.765,1154.338,1151.302,1148.484,1145.051,1141.631,1138.505,1134.718,1131.982,1129.205,1124.725,1120.926,1118.139,1114.46,1111.547,1108.855,1106.001,1102.534,1099.667,1097.113,1094.03,1089.572,1086.526,1083.927,1081.094,1078.131,1075.833,1072.552,1069.349,1067.103,1064.9,1062.232,1059.438,1058.133,1056.781,1054.801,1051.801,1049.914,1047.915,1045.909,1043.7,1042.411,1041.814,1039.252,1035.768,1033.617,1032.282,1030.657,1028.977,1027.12,1026.377,1025.593,1023.806,1022.286,1021.222,1019.923,1018.78,1017.637,1016.479,1015.419,1014.511,1013.701,1012.918,1012.608,1012.541,1010.924,1009.811,1009.274,1009.451,1008.115,1006.958,1006.835,1006.071,1002.348,1000.388,1000.13,1000.73,1002.095,1002.832,1002.644,1002.319,1002.838,1002.115,1002.759,1002.663,1002.005,1000.068,999.6432,999.4778,997.3295,995.236,993.0924,991.737,991.2845,989.0859,987.503,986.7649,987.3551,986.9902,988.4178,989.0917,989.0086,990.9783,992.976,994.841,996.6176,998.2694,1000.396,1003.603,1004.281,1005.871,1008.817,1009.991,1011.307,1012.917,1014.338,1016.546,1017.384,1018.977,1020.492,1021.033,1023.969,1025.476,1027.874,1031.183,1034.084,1037.371,1040.234,1043.498,1046.681,1049.548,1052.055,
1660.608,1654.979,1646.936,1637.997,1626.868,1619.685,1611.339,1586.65,1577.95,1571.461,1563.74,1560.317,1556.584,1553.333,1548.596,1538.489,1533.28,1514.041,1506.226,1479.93,1465.531,1457.484,1451.934,1449.713,1441.393,1436.85,1432.807,1425.56,1422.558,1418.783,1414.983,1410.3,1406.573,1402.53,1398.279,1394.579,1389.586,1385.978,1383.762,1380.197,1376.352,1372.631,1368.789,1365.519,1361.66,1358.037,1354.719,1351.556,1349.565,1345.154,1341.284,1336.466,1333.112,1330.473,1327.498,1325.278,1324.016,1322.485,1319.44,1316.806,1314.526,1312.28,1311.268,1308.912,1310.106,1310.576,1310.651,1310.139,1309.761,1308.045,1303.966,1300.112,1291.496,1283.84,1282.08,1276.384,1265.325,1253.187,1233.16,1225.147,1220.268,1214.571,1210.81,1206.56,1200.241,1195.968,1192.512,1189.501,1185.98,1182.112,1179.065,1175.571,1172.889,1169.974,1167.311,1164.385,1161.588,1159.276,1156.068,1152.452,1149.902,1147.187,1144.89,1141.053,1137.741,1134.621,1131.133,1128.28,1124.567,1120.792,1117.47,1114.146,1111.221,1108.401,1105.46,1102.522,1099.255,1095.803,1092.552,1089.058,1086.557,1084.097,1080.762,1077.823,1075.374,1072.641,1069.125,1066.397,1064.125,1061.901,1059.625,1057.881,1056.891,1054.968,1052.481,1050.223,1047.8,1045.651,1043.831,1042.881,1041.804,1039.074,1035.922,1034.785,1033.425,1031.92,1030.292,1028.429,1026.656,1025.467,1025.026,1024.271,1022.825,1021.177,1019.686,1018.257,1017.368,1016.575,1015.487,1014.781,1014.315,1013.299,1013.638,1013.005,1011.432,1010.916,1010.131,1008.535,1007.383,1005.003,1004.038,1003.177,1001.733,1003.322,1003.534,1005.919,1006.187,1004.935,1004.647,1003.788,1002.995,1002.705,1002.696,1002.522,1003.509,1002.573,1000.858,999.1204,995.4269,993.7633,992.5192,991.6691,990.2903,988.2294,988.3182,988.0797,987.9952,991.0673,992.2105,992.2556,994.3815,996.0421,997.4905,999.2266,1001.406,1004.119,1006.252,1007.455,1009.684,1012.256,1013.756,1015.599,1016.307,1018.032,1019.402,1021.951,1022.642,1024.164,1024.144,1026.201,1027.862,1030.008,1033.188,1035.821,1039,1042.252,1045.678,1049.234,1052.067,1054.43,
1661.917,1657.539,1650.607,1640.901,1630.619,1622.81,1615.54,1592.919,1580.683,1567.513,1561.157,1557.455,1555.625,1552.289,1545.585,1537.521,1531.234,1510.564,1504.78,1476.13,1467.19,1460.721,1455.158,1452.113,1447.912,1440.222,1433.197,1425.352,1422.273,1417.85,1414.085,1411.014,1407.823,1403.619,1399.902,1396.617,1392.803,1389.673,1385.61,1381.324,1377.64,1374.532,1371.657,1368.077,1365.164,1363.354,1358.033,1354.279,1349.656,1345.524,1341.4,1338.441,1336.19,1334.038,1331.507,1330.15,1328.96,1328.268,1326.204,1323.819,1323.838,1321.095,1320.228,1317.224,1319.793,1319.715,1318.855,1318.595,1316.108,1315.685,1312.158,1304.497,1297.951,1291.868,1288.636,1284.836,1275.282,1263.481,1250.077,1233.803,1225.065,1214.615,1210.48,1207.238,1201.845,1194.981,1191.563,1189.383,1185.334,1180.453,1177.735,1173.718,1170.13,1168.292,1164.511,1162.386,1159.501,1156.377,1154.608,1152.139,1148.811,1145.765,1142.944,1139.663,1136.781,1133.896,1130.824,1127.525,1124.538,1121.121,1117.804,1114.587,1111.193,1108.054,1105.13,1101.979,1098.552,1095.331,1092.879,1090.107,1087.461,1084.331,1080.892,1077.546,1075.01,1072.647,1070.05,1067.467,1064.315,1062.991,1060.797,1058.611,1057.161,1054.906,1053.3,1051.481,1048.51,1045.591,1043.878,1043.1,1041.631,1039.326,1037.137,1035.864,1034.672,1033.075,1031.557,1029.672,1028.043,1026.513,1025.132,1024.142,1023.575,1022.909,1021.555,1019.797,1018.692,1017.461,1016.276,1015.679,1015.446,1014.552,1013.918,1014.448,1013.856,1011.155,1010.263,1009.129,1008.24,1007.317,1006.012,1005.67,1005.189,1005.592,1006.637,1006.12,1007.044,1007.444,1005.796,1004.654,1004.097,1003.648,1003.625,1003.714,1003.911,1003.6,1002.008,999.9854,997.0103,996.305,994.2987,992.5858,990.459,989.7322,990.1631,990.5388,990.6607,993.3182,994.1506,997.105,997.3239,998.4551,1000.078,1001.842,1004.614,1007.221,1009.413,1010.654,1012.573,1015.286,1017.081,1018.934,1020.461,1021.596,1023.475,1025.386,1026.215,1027.432,1028.114,1028.949,1030.599,1032.001,1034.786,1038.133,1041.399,1044.219,1047.296,1050.926,1054.409,1057.033,
1663.126,1659.232,1654.29,1644.702,1633.656,1622.602,1615.53,1596.749,1588.466,1569.206,1559.858,1555.66,1552.187,1548.654,1543.037,1537.269,1518.919,1507.192,1492.091,1475.005,1468.696,1460.546,1456.375,1452.004,1446.453,1443.061,1432.732,1427.707,1423.493,1420.51,1416.364,1414.109,1410.453,1406.617,1402.962,1399.739,1396.387,1393.075,1389.081,1384.436,1381.356,1379.236,1374.918,1371.359,1367.397,1363.805,1358.49,1354.122,1350.684,1346.721,1343.638,1341.844,1341.324,1338.82,1335.66,1334.949,1334.448,1334.397,1333.929,1332.641,1332.426,1331.942,1328.941,1328.775,1330.214,1328.669,1328.935,1329.421,1327.812,1326.094,1322.184,1315.901,1308.979,1300.026,1296.003,1291.233,1285.566,1273.342,1260.224,1246.434,1235.219,1230.898,1215.245,1207.934,1203.046,1196.959,1190.819,1187.026,1183.535,1179.004,1175.655,1171.792,1168.74,1166.549,1162.933,1159.685,1157.69,1154.007,1152.564,1150.896,1147.212,1144.238,1141.161,1138.223,1135.361,1132.491,1129.863,1127.064,1124.31,1121.505,1118.15,1114.714,1110.722,1107.7,1104.941,1101.662,1098.726,1095.781,1093.085,1091.289,1087.591,1084.626,1081.532,1078.129,1075.508,1073.3,1071.023,1067.165,1065.206,1063.101,1061.654,1059.595,1057.293,1055.143,1052.832,1050.513,1048.271,1045.268,1044.221,1042.783,1041.368,1040.098,1038.4,1037.18,1035.807,1034.153,1032.534,1030.947,1029.559,1028.109,1026.273,1024.89,1023.879,1022.871,1022.044,1021.589,1020.52,1019.063,1017.668,1017.083,1017.348,1016.251,1015.069,1014.534,1015.313,1013.034,1010.982,1010.055,1008.551,1008.673,1008.428,1008.689,1009.042,1009.106,1008.734,1008.175,1008.733,1009.328,1008.227,1006.057,1004.475,1004.099,1004.485,1005.05,1005.331,1004.858,1003.517,1001.268,998.689,997.2761,995.5849,994.0826,992.369,993.2162,993.54,992.6845,994.2424,995.8344,996.78,997.6532,999.7162,1001.096,1003.063,1004.855,1007.789,1009.946,1012.822,1014.272,1016.705,1018.679,1020.63,1021.893,1024.163,1025.556,1027.533,1028.966,1029.794,1031.198,1032.545,1033.143,1033.732,1034.938,1038.624,1041.967,1044.62,1046.508,1051.424,1053.383,1055.32,1059.199,
1663.595,1659.342,1654.187,1647.341,1638.13,1626.2,1612.71,1599.004,1593.161,1573.578,1562.292,1555.585,1551.959,1547.681,1544.207,1532.01,1510.136,1498.801,1479.776,1472.358,1468.594,1462.84,1459.14,1454.727,1450.123,1445.881,1436.35,1429.26,1425.84,1422.779,1420.491,1417.322,1413.721,1410.38,1406.553,1402.734,1398.86,1394.922,1390.958,1386.889,1383.615,1380.073,1376.256,1371.941,1367.238,1362.874,1359.514,1355.722,1351.736,1348.5,1348.456,1346.744,1345.615,1343.181,1340.815,1339.403,1338.415,1339.375,1338.762,1338.806,1337.006,1334.769,1334.688,1335.107,1336.52,1337.654,1338.453,1339.768,1338.545,1331.57,1327.478,1322.112,1318.09,1312.307,1304.907,1299.356,1294.305,1284.963,1273.615,1263.049,1254.008,1246.347,1228.753,1216.841,1207.648,1199.791,1192.8,1185.642,1181.118,1177.89,1174.76,1170.756,1167.322,1165.098,1161.898,1158.138,1155.574,1151.903,1150.402,1148.497,1145.615,1142.656,1139.479,1136.471,1133.739,1131.218,1128.849,1125.916,1122.93,1120.603,1118.209,1114.383,1110.524,1108.164,1105.096,1102.033,1098.965,1096.304,1093.54,1090.776,1087.846,1084.878,1081.979,1078.81,1076.756,1074.158,1071.235,1068.332,1066.152,1063.875,1062.451,1060.13,1057.455,1055.526,1052.708,1049.062,1045.631,1045.023,1044.813,1043.423,1042.179,1040.712,1039.59,1038.228,1036.616,1035.308,1033.65,1032.273,1030.927,1029.18,1027.724,1026.151,1024.827,1023.972,1022.93,1021.662,1020.805,1020.147,1019.265,1018.674,1018.854,1017.92,1015.48,1015.697,1015.622,1014.074,1011.795,1010.357,1009.376,1010.088,1010.775,1011.211,1011.533,1010.761,1009.822,1008.591,1009.347,1009.926,1009.571,1007.323,1005.063,1004.874,1005.507,1005.718,1007.237,1006.416,1005.393,1003.327,1000.548,999.0707,997.2117,995.7385,994.2132,994.5832,994.4635,995.0428,996.9504,997.3043,998.0953,999.8124,1001.988,1003.409,1005.566,1008.61,1010.617,1013.314,1015.305,1017.599,1019.94,1022.265,1023.949,1025.608,1027.659,1029.256,1031.61,1032.588,1033.52,1034.638,1036.386,1036.64,1036.849,1038.473,1044.962,1047.354,1051.978,1054.502,1060.584,1066.818,1079.067,1070.858,
1662.765,1659.379,1654.802,1649.814,1639.021,1627.665,1607.541,1598.25,1591.109,1586.209,1563.846,1556.061,1552.35,1548.771,1546.379,1530.866,1508.56,1498.058,1477.991,1471.91,1468.895,1463.682,1461.708,1458.375,1454.394,1449.76,1440.102,1432.621,1427.744,1425.116,1422.27,1418.645,1415.383,1411.484,1407.55,1403.565,1399.502,1395.637,1391.706,1387.94,1384.323,1380.314,1376.326,1371.099,1367.45,1363.937,1359.966,1356.719,1354.659,1353.749,1352.938,1351.116,1349.532,1347.101,1346.667,1343.561,1343.336,1343.59,1344.368,1344.186,1340.467,1339.276,1340.395,1340.119,1340.273,1341.776,1342.322,1342.206,1339.84,1335.22,1329.639,1326.553,1322.733,1317.7,1311.467,1307.889,1302.756,1293.498,1283.834,1274.298,1264.23,1254.11,1241.017,1229.013,1218.94,1210.957,1204.448,1197.583,1185.79,1178.487,1173.434,1170.262,1164.461,1161.737,1158.181,1155.766,1153.109,1151.305,1147.813,1145.953,1143.293,1140.391,1137.875,1135.126,1132.602,1130.084,1127.77,1124.812,1121.579,1119.439,1116.917,1113.018,1109.645,1107.29,1104.786,1101.881,1098.801,1096.48,1093.455,1091.108,1088.324,1085.762,1082.73,1079.768,1077.231,1074.078,1071.564,1068.775,1067.074,1065.128,1063.111,1060.22,1057.469,1056.599,1053.626,1048.887,1045.786,1045.381,1045.74,1044.27,1042.882,1041.659,1040.314,1039.309,1037.788,1036.237,1034.775,1033.55,1032.086,1030.318,1028.869,1027.177,1025.774,1024.901,1024.12,1022.921,1021.968,1020.751,1019.316,1019.005,1018.997,1018.99,1016.271,1016.061,1016.094,1014.942,1012.59,1012.521,1011.734,1012.479,1013.299,1012.993,1012.511,1011.794,1010.748,1009.42,1009.794,1010.724,1009.667,1008.747,1006.697,1006.183,1007.115,1008.245,1008.291,1007.919,1006.799,1004.599,1002.514,999.8594,997.9801,996.89,995.4695,996.126,996.0712,997.7513,997.8653,998.2626,1000.15,1002.344,1004.473,1005.803,1007.904,1010.508,1013.465,1016.404,1018.145,1020.351,1022.802,1025.517,1027.143,1029.05,1030.875,1032.944,1034.189,1036.121,1037.541,1038.7,1040.04,1040.669,1043.686,1048.491,1053.052,1057.173,1058.856,1062.618,1079.281,1089.168,1092.332,1096.481,
1660.108,1656.718,1651.259,1643.631,1631.725,1622.67,1603.656,1613.412,1598.281,1593.574,1568.016,1555.466,1551.578,1548.381,1544.427,1535.07,1507.434,1495.982,1479.279,1473.248,1469.429,1466.267,1463.317,1460.384,1458.206,1455.449,1449.411,1446.437,1430.747,1426.748,1423.387,1419.624,1415.822,1412.051,1408.083,1403.979,1399.935,1396.201,1392.416,1388.231,1384.215,1380.119,1375.823,1371.528,1368.253,1365.053,1363.332,1361.266,1359.824,1357.73,1356.072,1354.88,1352.996,1350.913,1348.968,1348.681,1348.704,1349.406,1351.253,1355.099,1347.003,1342.822,1343.64,1343.047,1343.603,1345.056,1345.495,1344.252,1340.805,1338,1333.904,1329.592,1324.616,1319.192,1313.694,1309.788,1304.986,1299.926,1292.395,1286.577,1274.619,1258.131,1248.614,1236.83,1223.257,1214.585,1212.239,1210.643,1201.935,1185.33,1181.488,1174.469,1164.78,1160.667,1156.399,1153.277,1151.224,1148.833,1145.561,1143.323,1140.758,1138.688,1136.279,1133.902,1131.423,1129.093,1126.695,1123.459,1120.229,1118.146,1115.492,1111.701,1108.786,1105.883,1103.625,1101.17,1098.605,1095.971,1093.437,1090.713,1088.068,1085.754,1082.105,1079.854,1077.141,1074.02,1070.989,1069.076,1068.113,1066.21,1063.479,1060.726,1057.55,1056.085,1053.573,1049.241,1046.25,1045.995,1045.646,1045.001,1043.633,1042.497,1041.393,1040.324,1038.742,1037.464,1035.878,1034.512,1032.874,1031.412,1029.77,1028.261,1026.625,1026.036,1025.028,1023.793,1022.616,1021.782,1021.277,1019.139,1018.126,1017.647,1017.32,1016.662,1017.582,1015.442,1014.19,1015.059,1014.236,1013.825,1014.809,1014.394,1013.694,1012.263,1011.221,1010.377,1009.744,1010.904,1010.483,1009.186,1007.364,1007.03,1008.363,1007.937,1008.403,1009.011,1006.882,1004.862,1002.735,1000.718,999.4822,997.5636,996.4727,996.7059,997.001,998.0954,999.387,1001.269,1003.273,1005.603,1007.526,1009.375,1011.169,1013.486,1016.176,1019.016,1020.811,1023.894,1027.099,1028.936,1030.602,1032.856,1034.781,1036.425,1038.103,1039.727,1041.025,1042.41,1043.518,1044.091,1056.456,1057.141,1059.016,1066.085,1083.409,1086.577,1092.772,1096.874,1098.324,1104.348,
1657.2,1651.29,1645.178,1636.9,1626.438,1613.96,1604.342,1602.41,1589.211,1580.574,1571.506,1564.468,1549.524,1545.682,1538.621,1533.833,1517.991,1491.969,1479.812,1475.146,1472.513,1469.164,1465.777,1463.136,1460.339,1456.732,1452.649,1443.675,1432.123,1427.875,1424.273,1420.577,1416.372,1412.406,1408.762,1404.727,1401.475,1397.851,1392.46,1388.102,1384.567,1379.949,1376.266,1375.399,1372.392,1372.222,1368.223,1366.126,1363.504,1361.377,1359.535,1358.418,1356.478,1354.964,1354.427,1355.6,1354.602,1355.079,1361.209,1361.982,1357.08,1347.248,1346.014,1345.735,1348.221,1349.053,1348.966,1347.255,1342.858,1339.306,1335.658,1331.398,1326.219,1321.451,1316.864,1311.303,1306.406,1300.887,1294.375,1288.071,1280.406,1266.129,1253.661,1240.504,1229.255,1222.764,1218.208,1216.271,1210.763,1200.714,1195.127,1184.431,1173.601,1167.477,1159.612,1150.479,1148.178,1145.96,1143.767,1141.625,1138.131,1136.753,1134.139,1132.075,1130.039,1127.527,1125.381,1122.041,1118.774,1116.875,1114.037,1110.858,1107.924,1104.955,1102.446,1099.979,1097.876,1096.058,1093.314,1090.596,1087.879,1085.245,1082.189,1079.125,1076.9,1074.171,1071.208,1068.96,1067.412,1064.676,1062.317,1059.657,1057.138,1054.462,1052.295,1050.559,1046.97,1046.664,1046.246,1045.418,1044.157,1043.414,1042.237,1041.109,1039.729,1038.268,1036.923,1035.297,1033.762,1032.136,1030.731,1029.055,1027.758,1026.938,1025.665,1024.751,1023.358,1023.336,1023.28,1021.61,1020.148,1018.348,1018.088,1018.419,1018.05,1017.295,1016.523,1016.676,1016.149,1015.74,1015.906,1015.503,1014.968,1013.139,1012.106,1011.64,1010.183,1011.554,1012.467,1011.673,1010.75,1010.706,1010.672,1010.226,1009.89,1009.8,1008.982,1007.542,1005.446,1003.61,1001.593,998.0995,997.3121,997.8609,998.3958,999.374,1001.084,1003.468,1006.518,1007.457,1010.92,1012.326,1013.737,1015.796,1018.775,1021.404,1023.629,1026.45,1029.089,1031.804,1034.013,1036.137,1038.314,1040.239,1042.012,1043.684,1044.66,1046.133,1047.653,1049.247,1054.881,1062.119,1064.54,1075.168,1091.264,1097.597,1100.23,1107.159,1108.525,1109.862,
1651.115,1644.349,1638.79,1632.468,1623.94,1611.106,1602.992,1597.647,1590.812,1586.004,1574.786,1557.864,1546.087,1540.944,1535.49,1532.787,1500.682,1490.18,1484.888,1478.59,1476.234,1472.165,1468.22,1465.128,1461.691,1457.739,1453.419,1447.119,1433.841,1429.753,1425.829,1420.286,1416.87,1413.196,1409.588,1405.684,1402.281,1398.38,1393.842,1389.107,1384.294,1383.095,1380.871,1379.217,1377.963,1376.11,1372.439,1370.455,1368.249,1365.607,1363.728,1362.209,1360.509,1359.828,1359.584,1359.892,1359.135,1358.62,1364.891,1366.277,1363.643,1350.625,1349.146,1349.475,1352.197,1352.586,1351.942,1350.729,1344.564,1341.28,1337.22,1332.711,1329.529,1322.09,1317.216,1313.775,1307.987,1303.93,1296.802,1290.286,1283.221,1271.701,1260.245,1249.144,1238.894,1232.501,1226.145,1222.073,1218.151,1210.927,1205.629,1197.331,1186.737,1182.279,1177.664,1167.53,1147.361,1144.19,1141.587,1139.139,1136.17,1133.961,1131.961,1130.683,1128.59,1125.81,1123.433,1120.841,1117.714,1115.453,1112.935,1109.996,1107.061,1104.115,1101.254,1099.247,1096.969,1094.645,1092.322,1090.13,1087.34,1084.374,1081.882,1078.917,1076.715,1074.058,1071.121,1068.767,1066.847,1064.519,1061.954,1059.447,1057.123,1054.096,1051.777,1049.817,1047.722,1047.224,1046.739,1045.709,1045.2,1044.095,1043.009,1041.772,1040.578,1039.031,1037.468,1035.895,1034.171,1032.924,1031.406,1029.9,1028.717,1027.882,1026.47,1025.643,1024.258,1023.562,1023.06,1022.165,1021.285,1020.468,1019.833,1018.727,1019.31,1018.946,1018.426,1018.631,1018.535,1017.78,1017.317,1016.84,1016.357,1015.3,1014.298,1013.156,1013.611,1013.203,1013.019,1012.83,1012.338,1012.283,1012.738,1012.223,1012.009,1010.134,1009.278,1008.064,1006.495,1003.831,1002.619,1000.488,999.6661,999.5552,999.9655,1000.871,1002.202,1004.298,1005.973,1009.137,1010.748,1013.479,1015.934,1018.553,1021.153,1023.955,1026.614,1029.367,1031.813,1034.796,1036.888,1039.249,1041.025,1043.35,1045.288,1046.565,1048.469,1050.372,1052.346,1053.234,1056.035,1062.296,1069.963,1083.266,1100.809,1104.174,1109.682,1111.706,1113.713,1114.446,
1645.931,1640.043,1633.937,1625.518,1619.074,1610.438,1602.219,1597.395,1592.17,1585.711,1574.752,1555.615,1543.174,1538.596,1534.441,1530.612,1507.077,1497.188,1492.542,1485.293,1479.991,1476.291,1471.795,1467.216,1463.91,1461.311,1455.719,1452.063,1436.258,1430.979,1425.418,1422.313,1418.191,1413.377,1409.474,1406.255,1402.693,1398.511,1394.117,1390.749,1388.608,1386.046,1384.505,1384.065,1382.344,1379.767,1376.697,1374.766,1372.059,1369.797,1367.674,1366.388,1364.747,1364.177,1365.646,1364.885,1363.419,1365.239,1369.112,1368.884,1366.403,1355.503,1353.094,1353.972,1355.897,1356.52,1354.914,1352.392,1347.319,1344.306,1340.128,1335.102,1331.581,1324.326,1318.607,1314.704,1310.651,1305.784,1299.605,1290.631,1283.117,1272.968,1264.622,1253.054,1244.479,1238.641,1232.781,1227.328,1222.006,1216.764,1208.556,1200.501,1195.61,1189.517,1186.578,1179.551,1174.802,1145.58,1141.516,1138.465,1134.818,1132.064,1130.431,1128.161,1125.228,1123.427,1121.824,1119.677,1117.474,1114.491,1111.534,1108.949,1106.101,1103.094,1100.594,1097.845,1095.427,1093.047,1090.666,1088.318,1086.061,1083.352,1080.928,1078.498,1076.48,1073.766,1071.028,1068.564,1066.328,1064.032,1061.705,1059.464,1056.794,1054.099,1051.523,1049.619,1047.944,1047.647,1047.146,1046.398,1045.81,1045.037,1043.728,1042.44,1041.149,1039.869,1038.209,1036.515,1035.127,1033.422,1032.128,1030.777,1029.758,1028.901,1027.667,1026.292,1025.296,1024.573,1023.65,1022.796,1022.37,1021.799,1020.474,1020.462,1021.249,1021.334,1020.751,1020.534,1019.892,1019.095,1018.834,1018.398,1017.819,1016.106,1015.034,1015.423,1015.407,1015.428,1015.55,1015.083,1013.44,1013.394,1014.699,1014.155,1012.699,1011.187,1011.055,1008.454,1006.629,1005.055,1002.924,1002.11,1000.604,1000.505,1001.919,1001.961,1004.45,1004.667,1006.81,1009.423,1011.544,1015.124,1018.324,1020.593,1023.4,1026.354,1029.209,1031.978,1034.479,1037.745,1040.45,1042.343,1044.35,1046.81,1049.067,1050.311,1051.966,1054.564,1056.067,1057.003,1058.412,1065.141,1083.235,1101.366,1110.972,1114.951,1117.388,1118.021,1118.149,1119.202,
1647.363,1642.53,1637.877,1629.743,1621.854,1613.652,1605.878,1599.001,1594.711,1588.547,1577.518,1563.959,1541.191,1536.578,1532.829,1526.276,1521.699,1508.5,1500.025,1490.39,1486.238,1482.463,1477.011,1469.709,1466.097,1463.535,1457.569,1453.821,1446.154,1433.911,1428.37,1425.089,1419.454,1414.081,1409.663,1405.87,1402.196,1400.367,1396.402,1394.405,1392.058,1390.821,1389.911,1387.952,1386.064,1384.528,1381.231,1379.013,1376.685,1373.569,1371.81,1370.017,1369.105,1368.89,1369.494,1369.055,1367.63,1368.601,1372.304,1372.655,1368.195,1359.466,1357,1358.788,1360.914,1361.353,1359.067,1354.733,1351.443,1347.684,1342.989,1337.318,1332.111,1327.26,1321.584,1316.606,1312.671,1307.307,1302.044,1294.773,1282.653,1273.621,1266.418,1255.472,1244.773,1239.019,1233.938,1230.12,1224.339,1220.211,1211.547,1200.855,1194.778,1189.861,1187.103,1181.462,1179.32,1170.396,1152.626,1143.974,1134.577,1130.495,1129.695,1126.837,1123.662,1120.827,1118.993,1117.687,1115.984,1112.847,1109.773,1107.093,1104.379,1102.032,1100.406,1096.953,1094.433,1092.049,1089.748,1087.265,1084.891,1082.582,1080.268,1077.345,1074.484,1072.576,1070.639,1068.259,1065.686,1063.325,1061.031,1058.984,1057.057,1054.35,1052.339,1050.122,1048.548,1047.851,1047.365,1046.901,1046.243,1045.575,1044.316,1043.043,1041.849,1040.304,1038.87,1037.261,1035.809,1034.388,1032.828,1031.855,1031.052,1029.902,1028.785,1027.449,1027.125,1026.253,1025.584,1023.582,1023.805,1023.894,1022.815,1022.665,1023.244,1022.979,1021.904,1021.72,1021.272,1020.204,1020.234,1020.045,1019.321,1016.775,1015.698,1016.171,1016.589,1016.818,1017.289,1016.06,1015.378,1015.445,1015.994,1015.748,1014.599,1012.979,1012.546,1009.872,1008.1,1006.265,1004.662,1003.033,1002.574,1003.685,1003.864,1003.226,1004.474,1005.342,1007.715,1010.219,1012.552,1015.735,1019.315,1022.414,1025.506,1028.303,1031.58,1034.266,1036.799,1039.875,1043.046,1045.545,1048.362,1050.449,1052.48,1054.192,1055.799,1057.687,1059.684,1060.946,1062.099,1075.068,1096.193,1115.782,1120.255,1122.036,1123.056,1123.79,1124.323,1125.485,
1648.411,1644.121,1638.799,1633.348,1628.545,1618.673,1608.34,1603.948,1595.781,1588.174,1578.819,1559.639,1542.321,1534.197,1529.218,1524.257,1516.407,1511.789,1502.689,1495.343,1490.433,1485.39,1481.906,1472.827,1467.903,1463.96,1457.902,1453.391,1445.128,1436.833,1433.101,1428.547,1422.456,1416.86,1410.475,1407.512,1405.062,1403.293,1400.055,1398.182,1396.356,1395.236,1394.35,1391.948,1390.943,1388.481,1385.031,1382.547,1379.957,1377.55,1376.497,1374.121,1373.467,1374.15,1373.953,1373.755,1373.12,1375.647,1376.516,1379.022,1372.247,1362.674,1360.901,1363.465,1365.423,1365.753,1362.616,1357.855,1354.617,1350.556,1345.929,1340.428,1336.285,1330.8,1324.775,1320.254,1314.894,1308.912,1303.215,1295.392,1285.005,1275.131,1267.031,1255.426,1244.016,1240.551,1235.988,1230.58,1225.07,1220.907,1211.819,1200.997,1192.436,1188.485,1184.919,1180.856,1176.802,1172.041,1154.121,1148.142,1136.751,1129.715,1128.108,1125.688,1121.312,1119.901,1116.781,1114.745,1113.579,1111.288,1108.487,1105.918,1103.61,1101.387,1099.03,1096.297,1093.923,1091.601,1088.974,1086.367,1084.446,1082.074,1079.278,1076.346,1073.442,1071.591,1069.639,1066.783,1064.602,1062.561,1060.523,1058.367,1056.334,1054.889,1052.888,1050.326,1049.032,1048.606,1047.569,1047.285,1046.638,1046.01,1045.074,1043.985,1042.867,1040.896,1039.435,1038.286,1036.769,1035.336,1033.932,1032.861,1032.172,1031.06,1029.898,1028.648,1029.031,1027.701,1026.433,1025.586,1025.477,1025.488,1025.446,1025.075,1025.233,1025.69,1025.132,1023.384,1023.739,1022.971,1021.506,1021.431,1020.83,1018.908,1017.924,1017.066,1017.295,1018.854,1018.879,1018.799,1018.194,1017.184,1017.067,1017.265,1016.338,1015.713,1014.33,1011.647,1009.909,1009.2,1006.8,1004.529,1004.629,1006.089,1005.922,1005.244,1005.744,1007.655,1009.715,1011.257,1014.661,1017.799,1020,1024.019,1026.567,1030.663,1033.726,1036.565,1039.724,1042.689,1045.436,1048.628,1051.336,1054.834,1056.067,1058.217,1059.849,1061.096,1063.539,1064.665,1065.627,1076.655,1096.715,1118.246,1124.711,1126.734,1127.866,1128.963,1129.058,1128.711,
1649.034,1644.607,1641.624,1636.505,1630.449,1623.638,1618.791,1606.453,1603.269,1598.438,1582.562,1573.297,1542.583,1533.904,1530.276,1535.634,1519.528,1513.069,1505.086,1498.214,1491.359,1484.5,1479.994,1472.046,1467.549,1462.936,1457.46,1453.166,1445.562,1439.417,1435.656,1430.831,1424.663,1418.718,1415.629,1412.48,1409.197,1406.719,1404.451,1402.317,1401.061,1400.016,1398,1396.437,1396.163,1393.988,1389.812,1386.242,1383.64,1381.766,1379.768,1378.434,1378.199,1378.309,1378.191,1377.825,1376.967,1380.063,1382.919,1384.946,1384.838,1374.516,1378.156,1375.794,1375.08,1372.323,1367.439,1362.137,1359.842,1357.092,1349.458,1343.993,1339.404,1334.337,1327.943,1322.032,1317.304,1310.632,1304.832,1297.178,1285.039,1277.07,1267.116,1256.521,1246.819,1241.524,1236.306,1230.96,1223.952,1220.608,1213.937,1204.254,1192.832,1186.283,1181.621,1177.495,1173.953,1167.836,1161.348,1139.539,1135.89,1130.61,1126.584,1123.062,1119.372,1117.598,1114.299,1112.196,1111.058,1110,1107.371,1104.975,1102.717,1100.519,1098.276,1095.941,1093.528,1091.088,1088.572,1085.978,1083.961,1081.356,1078.513,1075.844,1072.961,1070.22,1068.209,1065.874,1063.976,1061.649,1059.552,1057.828,1055.416,1054.439,1052.821,1050.313,1049.582,1049.015,1048.2,1047.631,1047.044,1046.425,1045.787,1044.582,1043.015,1042.011,1040.373,1039.223,1037.74,1036.243,1034.92,1034.286,1033.45,1032.241,1031.025,1030.346,1029.742,1028.089,1027.829,1026.895,1026.991,1026.582,1026.593,1026.256,1025.881,1026.324,1025.78,1024.569,1024.473,1023.965,1023.45,1022.81,1021.579,1021.214,1019.897,1019.282,1019.223,1020.754,1020.301,1020.386,1019.967,1019.651,1019.182,1017.989,1017.173,1016.458,1015.044,1013.864,1012.765,1010.287,1007.067,1006.253,1005.99,1007.789,1007.723,1007.144,1007.503,1008.106,1010.771,1011.901,1015.158,1017.975,1021.531,1024.89,1028.276,1032.372,1035.445,1038.693,1042.034,1044.815,1049.065,1051.39,1053.913,1057.206,1060.559,1062.438,1064.328,1064.961,1066.68,1068.785,1070.372,1077.683,1095.067,1111.847,1127.075,1129.303,1131.356,1132.835,1132.93,1132.804,
1648.757,1645.414,1642.401,1640.531,1643.574,1625.817,1633.809,1607.162,1601.613,1604.745,1586.653,1583.161,1558.93,1540.153,1546.939,1555.297,1517.529,1511.228,1506.968,1501.023,1492.816,1485.229,1476.941,1471.258,1466.617,1461.885,1457.328,1452.715,1445.935,1441.334,1437.443,1432.18,1426.953,1422.423,1420.74,1419.975,1418.782,1413.693,1409.128,1407.028,1405.633,1403.614,1401.719,1400.892,1400.869,1398.305,1394.172,1390.052,1387.931,1386.111,1383.668,1382.781,1382.602,1382.826,1382.477,1382.083,1382.219,1385.905,1387.435,1389.978,1391.945,1391.378,1387.165,1384.323,1379.107,1376.694,1371.21,1366.787,1371.127,1363.221,1353.684,1347.652,1343.232,1336.905,1332.125,1325.525,1319.665,1312.841,1306.526,1296.729,1287.868,1276.687,1268.706,1255.628,1244.725,1239.507,1235.653,1229.165,1220.82,1218.566,1211.179,1204.126,1192.488,1183.295,1178.282,1174.986,1170.083,1165.247,1158.907,1142.556,1136.331,1133.048,1127.572,1121.588,1117.885,1115.199,1112.779,1111.499,1109.727,1108.104,1106.663,1104.046,1101.817,1099.692,1097.452,1095.02,1092.7,1090.07,1087.784,1085.52,1082.73,1080.385,1077.618,1075.134,1072.955,1069.503,1066.717,1064.488,1063.37,1061.613,1059.349,1057.653,1055.408,1054.136,1052.853,1051.151,1050.357,1050.092,1049.255,1048.444,1047.67,1047.037,1046.157,1045.142,1044.032,1042.279,1041.282,1039.713,1038.309,1037.098,1036,1035.105,1034.014,1033.378,1032.499,1031.644,1030.315,1029.381,1029.351,1028.727,1027.18,1026.582,1026.8,1026.639,1027.211,1027.514,1026.864,1025.696,1025.88,1025.483,1025.136,1024.648,1023.57,1022.825,1022.733,1021.804,1021.173,1021.968,1022.224,1021.738,1021.741,1021.158,1020.095,1019.442,1019.182,1016.901,1015.992,1014.813,1012.114,1009.42,1008.31,1008.505,1008.022,1008.901,1009.631,1009.811,1010.015,1010.458,1011.85,1013.895,1016.235,1018.982,1022.334,1024.788,1028.276,1032.405,1036.835,1039.977,1043.652,1046.905,1051.574,1054.156,1057.137,1060.531,1063.231,1065.113,1067.427,1069.087,1070.408,1072.223,1074.064,1079.301,1090.425,1099.286,1127.559,1131.863,1134.51,1135.254,1136.453,1137.17,
1647.011,1644.22,1640.415,1643.295,1644.25,1623.579,1623.228,1605.299,1611.813,1592.119,1583.286,1571.903,1560.594,1553.029,1555.607,1546.183,1517.253,1509.87,1503.584,1497.842,1493.292,1485.927,1474.609,1470.182,1465.59,1461.226,1456.287,1451.558,1446.222,1442.27,1438.036,1435.155,1429.735,1427.341,1426.408,1424.894,1422.937,1418.168,1413.236,1411.502,1409.333,1407.317,1405.671,1405.425,1404.177,1401.61,1398.344,1394.463,1391.946,1390.18,1388.09,1387.033,1387.268,1387.193,1386.585,1386.057,1386.67,1394.321,1395.066,1396.933,1397.817,1397.375,1392.244,1388.045,1382.521,1380.622,1378.084,1379.334,1380.145,1373.384,1367.196,1357.204,1353.566,1341.749,1334.911,1328.393,1321.206,1313.955,1300.305,1295.092,1290.502,1281.351,1272.273,1259.771,1241.558,1238.799,1234.726,1227.189,1218.531,1211.479,1202.43,1193.589,1184.537,1180.126,1175.661,1171.748,1167.915,1163.501,1158.809,1149.753,1137.932,1134.025,1128.354,1121.926,1117.41,1113.989,1112.764,1110.468,1108.891,1107.221,1105.839,1102.955,1100.282,1097.803,1096.011,1093.968,1091.812,1089.247,1087.229,1084.247,1081.982,1079.925,1077.598,1075.143,1072.846,1070.086,1067.002,1064.697,1062.755,1061.198,1059.146,1057.864,1056.003,1054.938,1053.712,1051.843,1051.031,1051.337,1050.339,1049.32,1048.608,1047.733,1047.057,1045.541,1044.46,1043.261,1041.858,1040.358,1039.122,1037.648,1036.295,1036.304,1035.74,1034.779,1033.833,1032.686,1031.167,1031.32,1030.462,1030.308,1029.314,1026.868,1026.965,1026.973,1027.682,1028.228,1028.247,1027.736,1028.266,1027.875,1027.066,1026.448,1026.213,1026.29,1024.328,1023.601,1023.423,1022.751,1023.224,1023.459,1023.504,1022.561,1021.998,1021.432,1020.283,1018.594,1016.766,1015.388,1012.834,1011.05,1010.345,1010.697,1010.352,1010.766,1011.679,1011.587,1011.652,1013.271,1014.649,1015.968,1018.11,1020.723,1022.046,1024.962,1028.845,1033.665,1039.042,1042.587,1045.909,1049.956,1053.673,1056.328,1060.155,1063.75,1066.159,1068.268,1070.084,1072.616,1074.533,1076.105,1078.058,1081.613,1084.804,1098.772,1126.309,1135.176,1138.786,1139.749,1140.556,1144.664,
1644.299,1640.139,1639.413,1647.367,1635.699,1618.087,1609.75,1608.642,1604.763,1594.655,1589.735,1586.544,1559.948,1544.827,1550.994,1543.295,1515.95,1510.587,1505.633,1500.322,1495.329,1483.059,1472.78,1468.828,1464.759,1459.864,1455.821,1451.522,1447.415,1443.276,1441.283,1440.245,1436.544,1433.975,1430.781,1430.263,1428.422,1424.958,1417.581,1415.239,1412.95,1411,1410.036,1409.735,1408.383,1405.589,1402.564,1399.501,1397.031,1394.205,1392.276,1391.996,1391.618,1391.519,1391.94,1392.523,1395.976,1399.465,1401.772,1402.841,1403.408,1404.821,1403.744,1395.927,1386.089,1386.273,1387.444,1388.798,1390.894,1380.877,1380.001,1370.241,1366.677,1351.125,1336.402,1331.701,1325.182,1315.342,1305.386,1291.973,1288.018,1281.993,1271.403,1255.416,1240.276,1236.965,1233.677,1226.52,1215.659,1203.344,1195.234,1185.651,1182.048,1177.669,1174.199,1170.579,1167.261,1162.879,1158.635,1152.337,1139.323,1134.553,1128.77,1122.992,1116.116,1113.813,1112.038,1110.343,1109.087,1106.022,1104.337,1101.882,1099.029,1096.832,1094.823,1092.393,1089.945,1087.909,1085.593,1083.17,1080.838,1078.937,1076.969,1074.886,1072.698,1070.284,1067.464,1064.851,1062.874,1060.901,1059.09,1057.64,1057.2,1055.711,1054.596,1052.999,1051.74,1051.819,1051.303,1050.3,1049.464,1048.275,1047.356,1046.124,1044.745,1043.786,1042.642,1041.307,1039.616,1038.098,1037.749,1037.186,1036.487,1035.508,1034.411,1033.355,1032.455,1031.932,1032.117,1031.557,1029.949,1028.199,1028.298,1028.808,1028.547,1029.364,1029.251,1029.025,1029.325,1029.553,1029.154,1028.226,1027.018,1028.74,1027.068,1025.78,1024.874,1024.417,1024.732,1024.994,1025.796,1023.426,1023.956,1022.258,1020.948,1019.247,1017.422,1016.211,1014.383,1011.771,1011.591,1011.668,1011.833,1012,1012.551,1012.749,1013.497,1016.167,1017.575,1019.362,1021.823,1023.632,1023.618,1026.363,1040.65,1042.834,1046.017,1050.127,1052.274,1052.872,1056.313,1059.574,1062.672,1067.048,1069.358,1071.49,1073.258,1075.285,1077.861,1081.2,1083.29,1084.762,1089.047,1105.865,1124.263,1136.786,1143.425,1145.564,1149.721,1154.083,
1641.302,1633.42,1633.91,1630.635,1634.156,1627.022,1609.155,1597.328,1590.532,1583.098,1581.462,1577.454,1543.061,1537.325,1540.575,1533.126,1514.391,1509.937,1506.087,1501.253,1494.367,1480.784,1472.9,1468.599,1464.176,1460.22,1456.56,1453.031,1448.74,1446.089,1444.523,1444.606,1444.365,1438.442,1435.597,1434.929,1432.839,1427.851,1421.356,1418.681,1416.621,1415.099,1414.25,1413.632,1411.415,1409.158,1406.02,1402.99,1401.166,1398.573,1396.553,1396.318,1395.936,1399.174,1401.56,1403.313,1403.268,1403.832,1405.541,1406.662,1407.997,1409.186,1410.525,1410.201,1400.913,1390.511,1391.888,1393.741,1395.092,1388.177,1377.761,1369.571,1365.807,1348.111,1334.474,1329.908,1322.848,1313.966,1294.646,1289.104,1283.886,1279.308,1270.31,1256.335,1238.484,1235.631,1231.716,1224.144,1211.833,1199.698,1186.959,1184.057,1179.635,1175.577,1172.939,1169.049,1165.619,1162.12,1156.968,1151.121,1139.332,1134.199,1127.658,1122.455,1117.152,1115.085,1112.365,1110.532,1108.785,1105.331,1102.897,1100.881,1098.174,1096.885,1094.709,1092.006,1089.212,1086.479,1084.088,1083.151,1080.271,1077.668,1075.597,1073.903,1071.935,1069.77,1067.561,1064.592,1062.485,1060.689,1059.557,1058.558,1057.504,1056.725,1055.662,1054.123,1052.358,1052.375,1051.837,1050.98,1050.206,1048.941,1047.692,1046.574,1045.468,1044.375,1043.82,1042.335,1040.792,1039.274,1038.843,1038.327,1037.81,1036.439,1035.404,1034.827,1033.671,1033.146,1032.151,1032.208,1031.909,1031.871,1031.724,1031.34,1031.954,1031.978,1031.13,1030.089,1031.003,1031.157,1030.669,1029.999,1029.301,1029.585,1029.716,1028.907,1026.997,1025.609,1026.253,1026.872,1027.971,1025.876,1025.087,1023.241,1022.111,1021.273,1020.022,1017.46,1016.251,1014.835,1013.363,1013.23,1012.702,1012.841,1013.952,1014.707,1016.529,1018.207,1020.314,1022.826,1025.334,1027.37,1027.478,1038.496,1047.833,1050.17,1052.986,1055.782,1056.479,1056.233,1058.523,1061.376,1064.471,1067.98,1071.399,1073.787,1076.931,1078.997,1081.486,1085.029,1087.552,1088.83,1091.073,1118.286,1121.679,1126.55,1138.28,1146.458,1156.827,1161.635,
1646.454,1628.607,1627.009,1623.354,1616.114,1624.104,1614.188,1604.498,1586.848,1579.749,1572.427,1566.953,1544.494,1524.522,1520.077,1515.258,1511.254,1506.857,1503.381,1500.09,1494.288,1485.854,1474.141,1469.152,1465.465,1462.042,1458.505,1453.945,1451.799,1449.43,1448.287,1448.807,1448.876,1445.395,1441.719,1439.351,1433.041,1427.715,1424.863,1422.212,1420.738,1419.45,1418.264,1417.472,1415.222,1413.473,1413.143,1410.85,1406.104,1403.851,1401.134,1400.467,1401.031,1404.426,1408.674,1410.189,1408.183,1408.876,1409.576,1411.409,1412.36,1414.414,1417.123,1417.509,1409.419,1399.203,1397.034,1398.522,1396.856,1390.175,1381.803,1368.996,1364.352,1351.58,1331.756,1325.844,1319.774,1309.744,1289.661,1286.483,1280.229,1274.484,1261.181,1252.881,1237.819,1232.39,1228.653,1222.692,1211.311,1198.51,1185.067,1181.047,1177.735,1173.893,1170.822,1167.636,1163.54,1160.202,1155.441,1147.659,1140.87,1131.802,1126.697,1122.599,1119.53,1118.057,1114.569,1112.462,1110.245,1107.118,1103.865,1099.625,1097.688,1097,1094.676,1091.797,1089.555,1087.058,1084.413,1081.433,1079.032,1076.65,1074.274,1072.449,1071.363,1069.417,1067.674,1065.059,1063.164,1061.724,1060.715,1059.443,1058.468,1057.941,1057.111,1055.446,1052.908,1052.853,1052.542,1051.683,1050.444,1049.367,1048.368,1047.259,1046.189,1045.363,1044.552,1043.593,1042.124,1040.665,1039.947,1039.354,1038.622,1037.629,1036.478,1035.11,1034.37,1033.427,1033.021,1032.859,1033.353,1033.512,1034.194,1035.095,1033.955,1033.566,1032.604,1032.72,1032.786,1031.504,1032.053,1031.458,1030.454,1030.358,1030.674,1030.927,1029.468,1027.618,1028.839,1029.196,1028.851,1028.972,1026.944,1024.96,1023.207,1022.675,1021.5,1019.033,1017.871,1017.349,1015.55,1014.581,1014.158,1014.504,1015.326,1015.634,1017.206,1019.786,1022.793,1025.483,1028.998,1031.277,1031.799,1051.729,1053.211,1054.84,1057.402,1059.365,1060.514,1060.569,1060.871,1063.235,1066.767,1070.464,1074.216,1076.26,1079.133,1082.05,1084.639,1087.94,1091.85,1093.025,1095.995,1112.486,1116.17,1118.215,1126.842,1141.467,1153.762,1162.281,
1643.008,1633.191,1620.794,1618.727,1612.439,1606.064,1612.11,1590.852,1583.44,1576.613,1571.179,1562.985,1553.264,1519.908,1517.079,1513.601,1509.114,1504.779,1500.292,1496.615,1491.834,1487.241,1476.87,1470.703,1466.527,1462.64,1459.264,1456.795,1454.587,1454.052,1454.495,1453.587,1452.401,1449.161,1445.512,1439.442,1433.911,1431.135,1428.05,1426.036,1425.21,1424.532,1422.101,1420.952,1419.641,1418.848,1418.547,1414.304,1409.836,1407.475,1405.268,1406.226,1410.015,1411.244,1413.198,1414.653,1412.973,1414.455,1415.873,1416.555,1419.175,1420.969,1420.473,1418.366,1413.475,1406.568,1402.843,1402.73,1399.449,1393.191,1377.749,1367.033,1360.634,1347.133,1335.756,1320.416,1311.557,1304.68,1287.564,1281.561,1275.153,1263.012,1253.525,1246.164,1234.719,1229.138,1224.293,1219.453,1209.822,1196.014,1183.728,1179.243,1175.96,1173.122,1169.03,1166.438,1162.614,1158.282,1151.158,1144.115,1136.455,1131.55,1127.546,1124.552,1121.65,1119.399,1116.16,1113.736,1111.464,1109.899,1106.718,1102.014,1098.419,1096.601,1094.335,1091.734,1089.665,1087.205,1084.404,1081.77,1078.613,1076.423,1074.323,1072.47,1070.7,1068.626,1067.065,1065.181,1063.4,1063.114,1061.88,1060.955,1059.986,1059.337,1057.791,1055.038,1053.452,1053.444,1053.521,1052.283,1050.927,1050.037,1048.889,1048.124,1047.33,1046.715,1045.456,1044.325,1043.221,1041.841,1041.097,1040.402,1039.416,1037.877,1036.969,1036.539,1035.481,1035.355,1034.833,1035.308,1036.115,1036.134,1037.232,1037.262,1035.297,1035.196,1034.187,1033.681,1034.142,1033.379,1033.445,1033.691,1031.955,1032.054,1031.722,1031.569,1030.634,1029.795,1031.674,1030.853,1029.727,1029.212,1028.384,1026.805,1024.609,1024.408,1023.128,1021.546,1018.583,1018.261,1017.693,1016.867,1016.758,1015.431,1016.1,1015.864,1017.606,1020.954,1023.854,1027.224,1030.87,1034.582,1036.135,1046.963,1054.391,1056.678,1060.392,1061.804,1063.253,1064.763,1065.663,1067.632,1071.56,1074.672,1076.29,1078.801,1081.715,1084.643,1087.336,1090.322,1093.85,1096.894,1098.881,1104.081,1110.812,1114.497,1123.806,1135.591,1150.122,1161.842,
1628.907,1623.681,1615.032,1612.715,1615.503,1603.261,1606.637,1587.582,1580.033,1574.281,1569.916,1570.204,1537.821,1519.422,1515.909,1511.336,1507.49,1503.699,1499.541,1495.477,1490.894,1483.803,1477.649,1472.9,1469.787,1466.097,1462.798,1459.512,1457.721,1458.253,1458.575,1458.008,1455.395,1451.513,1446.958,1441.708,1438.541,1437.783,1437.515,1431.231,1429.959,1428.76,1426.406,1425.048,1424.34,1423.092,1420.9,1416.781,1413.858,1411.21,1411.54,1414.138,1415.991,1416.321,1416.978,1418.565,1417.662,1418.892,1421.225,1423.543,1422.611,1422.119,1418.886,1416.798,1413.749,1409.459,1406.053,1401.934,1396.122,1390.644,1372.145,1364.811,1356.612,1348.466,1333.657,1326.423,1305.387,1297.256,1283.215,1278.314,1270.421,1254.289,1247.943,1241.745,1231.352,1225.718,1220.458,1215.051,1209.423,1196.517,1181.788,1178.411,1175.553,1172.682,1168.051,1164.473,1161.688,1156.681,1151.614,1143.779,1136.526,1133.039,1130.133,1127.349,1125.956,1121.345,1118.48,1115.274,1112.307,1109.926,1107.635,1103.159,1100.069,1096.845,1094.443,1092.413,1090.183,1087.563,1085.031,1082.409,1079.133,1076.664,1074.536,1072.416,1070.375,1068.83,1066.769,1065.423,1064.849,1063.901,1063.293,1062.251,1061.429,1059.647,1057.827,1056.929,1055.676,1054.256,1054.19,1053.113,1051.676,1050.825,1049.956,1049.258,1048.474,1047.691,1046.397,1045.071,1043.633,1042.56,1042.134,1041.432,1040.099,1038.169,1038.259,1037.318,1037.373,1036.914,1037.031,1037.248,1037.29,1037.198,1038.565,1038.278,1037.527,1037.337,1036.274,1035.875,1036.546,1035.922,1035.301,1035.194,1032.814,1032.536,1032.864,1032.07,1031.631,1032.524,1033.016,1031.929,1030.274,1030.746,1029.341,1028.394,1027.232,1026.62,1026.668,1022.42,1020.375,1019.163,1018.051,1018.215,1017.5,1017.124,1017.233,1017.55,1019.5,1022.642,1025.455,1029.127,1032.999,1036.926,1040.163,1043.449,1049.712,1053.747,1059.067,1061.832,1064.591,1068.136,1070.721,1073.783,1076.603,1077.674,1079.482,1081.832,1084.617,1087.758,1090.505,1093.44,1095.815,1100.258,1103.12,1106.951,1110.015,1112.81,1123.241,1130.26,1148.388,1167.129,
1629.097,1615,1610.327,1609.462,1606.163,1600.2,1603.363,1584.553,1578.728,1571.766,1562.808,1546.234,1532.55,1518.476,1514.427,1509.751,1505.785,1502.095,1497.457,1492.98,1487.781,1483.149,1479.381,1476.583,1473.725,1472.382,1470.776,1465.809,1464.214,1463.354,1462.358,1461.047,1458.535,1454.457,1450.963,1448.265,1444.865,1443.447,1442.995,1439.171,1434.174,1432.733,1434.893,1429.761,1428.928,1426.93,1424.116,1421.998,1418.925,1417.141,1416.146,1417.777,1419.893,1420.927,1422.153,1423.668,1422.902,1423.182,1425.222,1426.49,1424.036,1420.523,1417.749,1414.499,1410.49,1407.272,1403.839,1399.646,1393.588,1384.176,1371.338,1364.267,1354.394,1345.12,1330.849,1327.557,1306.915,1294.768,1280.683,1273.766,1257.748,1249.553,1243.639,1237.494,1229.737,1223.866,1219.619,1214.272,1209.392,1202.988,1188.638,1178.372,1174.392,1170.755,1167.088,1163.13,1158.9,1154.964,1151.161,1145.377,1138.258,1135.546,1131.483,1128.737,1126.191,1123.573,1120.988,1117.919,1114.137,1110.948,1108.159,1104.797,1100.598,1097.09,1095.184,1093.403,1090.304,1087.693,1085.415,1083.117,1080.495,1078.199,1075.905,1073.192,1070.4,1068.99,1067.934,1067.019,1065.815,1065.146,1064.759,1063.664,1062.06,1060.549,1058.873,1057.974,1057.627,1056.323,1054.818,1054.367,1052.988,1051.962,1051.08,1050.288,1049.311,1048.244,1047.313,1046.199,1044.834,1043.308,1043.329,1042.435,1040.963,1039.302,1038.973,1037.651,1037.944,1036.885,1037.438,1037.927,1038.252,1038.626,1039.74,1040.439,1039.904,1039.157,1037.581,1037.813,1037.788,1037.205,1036.646,1035.494,1034.782,1035.707,1036.016,1033.981,1034.374,1035.529,1034.302,1033.207,1033.07,1032.032,1029.669,1030.307,1028.896,1028.014,1027.105,1025.449,1022.773,1020.117,1019.448,1018.661,1018.304,1018.87,1019.604,1019.982,1021,1022.944,1026.403,1030.125,1033.594,1037.223,1041.64,1045.158,1049.908,1053.945,1057.969,1060.961,1065.434,1069.631,1074.358,1077.832,1081.09,1081.136,1082.864,1084.602,1086.958,1089.917,1093.112,1096.346,1099.285,1102.722,1106.551,1109.342,1112.297,1115.03,1118.876,1123.37,1137.271,1172.294,
1631.003,1616.015,1604.903,1603.483,1598.784,1612.207,1598.476,1584.539,1576.058,1574.237,1564.529,1547.467,1533.992,1518.925,1513.606,1509.523,1505.021,1500.659,1496.382,1492.913,1488.399,1485.904,1482.219,1479.905,1478.469,1478.191,1479.688,1474.292,1469.952,1469.37,1469.12,1468.09,1466.096,1463,1454.033,1452.291,1450.265,1448.62,1448.852,1448.138,1443.375,1438.473,1437.729,1437.189,1433.779,1431.257,1427.599,1427.128,1425.445,1421.688,1420.875,1421.87,1423.401,1425.539,1426.959,1428.635,1429.861,1429.222,1426.836,1425.51,1422.525,1419.243,1415.324,1411.904,1407.861,1404.556,1401.75,1397.448,1389.826,1380.036,1367.134,1360.273,1349.152,1337.445,1328.373,1322.259,1314.283,1284.661,1275.948,1263.19,1252.717,1247.807,1243.767,1237.917,1231.684,1224.292,1219.204,1215.573,1210.218,1201.786,1192.505,1178.292,1173.782,1169.583,1165.159,1161.726,1157.796,1153.859,1150.114,1144.001,1139.507,1136.387,1133.617,1130.476,1127.919,1125.469,1121.907,1118.426,1115.413,1112.433,1109.556,1105.546,1101.94,1099.698,1097.29,1094.057,1090.581,1088.824,1086.87,1082.34,1081.567,1078.831,1076.62,1072.617,1071.201,1070.638,1069.828,1068.827,1067.423,1067.317,1066.542,1064.7,1063.129,1061.573,1060.427,1059.861,1058.63,1057.219,1055.421,1055.523,1054.6,1053.584,1052.399,1051.023,1050.362,1049.352,1048.755,1047.281,1045.989,1044.693,1044.156,1044.575,1041.391,1040.123,1039.872,1039.335,1037.803,1038.066,1039.264,1038.922,1039.296,1039.244,1040.357,1041.01,1040.776,1039.402,1039.127,1039.505,1039.106,1038.533,1037.991,1036.212,1034.966,1037.069,1037.374,1034.945,1036.581,1037.253,1036.004,1034.353,1034.504,1033.747,1031.661,1031.728,1031.242,1030.218,1028.693,1026.259,1022.148,1021.465,1021.586,1020.168,1020.061,1020.457,1021.149,1021.442,1021.78,1023.263,1026.168,1030.196,1033.678,1038.505,1043.616,1046.539,1050.508,1055.621,1059.474,1062.396,1067.387,1073.072,1078.27,1081.323,1084.256,1085.169,1085.741,1087.551,1089.603,1092.677,1095.525,1098.32,1101.139,1104.049,1107.547,1111.049,1114.491,1117.072,1120.405,1123.652,1132.863,1167.483,
1628.95,1618.195,1600.786,1598.815,1595.992,1597.829,1599.144,1584.802,1573.907,1564.863,1552.639,1540.716,1528.993,1519.362,1513.831,1509.563,1505.951,1502.387,1498.614,1495.318,1492.646,1490.435,1486.358,1484.417,1484.635,1484.574,1484.65,1482.746,1480.52,1482.228,1478.073,1474.071,1474.235,1472.915,1460.873,1456.054,1454.762,1454.221,1453.454,1451.639,1449.147,1448.806,1446.031,1441.673,1440.124,1437.699,1432.751,1431.76,1430.171,1427.372,1425.329,1427.104,1429.586,1430.793,1431.652,1432.712,1431.935,1427.85,1424.547,1423.266,1420.428,1414.441,1412.199,1409.507,1405.515,1401.602,1398.949,1394.187,1384.959,1376.204,1363.661,1356.872,1345.818,1333.016,1327.304,1317.706,1306.881,1283.039,1271.674,1260.374,1254.152,1250.055,1245.635,1240.128,1235.066,1226.894,1217.798,1214.678,1210.039,1202.083,1186.442,1178.924,1173.853,1167.732,1164.514,1160.97,1156.969,1154.031,1149.125,1144.335,1141.002,1137.443,1134.446,1131.41,1128.441,1125.786,1122.054,1118.57,1115.485,1113.213,1110.384,1107.525,1103.742,1100.579,1097.513,1093.849,1091.973,1088.959,1085.638,1081.98,1080.011,1078.078,1075.81,1073.378,1072.078,1072.676,1071.507,1069.805,1070.095,1068.819,1067.84,1066.154,1064.226,1063.456,1062.541,1061.492,1059.961,1059.151,1056.778,1056.231,1056.279,1055.246,1054.019,1052.762,1051.798,1050.824,1050.311,1048.77,1046.973,1045.602,1045.259,1045.101,1043.51,1041.348,1040.883,1039.66,1038.898,1040.092,1040.639,1040.535,1040.819,1041.01,1041.33,1042.37,1042.332,1041.514,1042.512,1041.509,1040.655,1039.97,1039.381,1038.339,1037.131,1037.264,1036.933,1036.885,1037.565,1037.864,1037.02,1035.768,1035.204,1034.848,1033.868,1033.209,1033.138,1031.949,1029.828,1026.821,1022.869,1022.787,1022.702,1021.142,1021.299,1021.326,1022.3,1023.408,1023.03,1027.66,1030.654,1033.92,1038.057,1043.631,1048.902,1053.571,1058.788,1061.014,1064.408,1066.868,1071.218,1076.71,1083.082,1085.263,1087.468,1089.303,1089.358,1090.241,1092.152,1094.473,1097.313,1099.992,1103.289,1106.298,1109.502,1112.978,1115.292,1119.49,1122.362,1124.385,1132.776,1161.823,
1624.879,1612.184,1596.796,1593.675,1592.665,1596.025,1590.46,1579.738,1571.092,1561.005,1546.958,1534.833,1528.047,1521.748,1515.915,1512.219,1508.374,1504.527,1501.182,1498.295,1496.63,1495.592,1493.865,1491.233,1491.538,1489.322,1488.823,1487.784,1487.833,1488.506,1486.529,1481.912,1480.021,1479.089,1474.497,1462.605,1457.998,1457.91,1457.77,1456.038,1454.774,1453.533,1451.876,1447.616,1446.632,1442.927,1439.142,1436.948,1435.076,1432.096,1431.63,1433.409,1434.767,1434.725,1436.062,1434.429,1430.424,1425.886,1423.369,1418.63,1415.757,1413.015,1409.769,1406.621,1403.313,1400.716,1396.094,1389.281,1379.934,1365.307,1361.984,1353.454,1337.654,1329.425,1323.628,1309.162,1296.317,1282.398,1270.815,1263.462,1257.513,1252.856,1248.033,1241.819,1234.674,1228.568,1218.41,1214.091,1211.954,1203.202,1185.165,1177.379,1173.183,1166.891,1163.201,1160.39,1157.184,1154.237,1149.445,1145.665,1142.257,1139.201,1135.1,1132.618,1130.255,1126.116,1122.911,1119.244,1116.301,1114.23,1110.433,1107.81,1105.932,1102.536,1098.604,1094.815,1091.299,1089.55,1087.505,1083.744,1080.753,1078.708,1076.814,1074.13,1073.654,1073.791,1073.386,1073.157,1071.648,1070.314,1068.966,1068.528,1066.489,1065.58,1064.818,1063.212,1061.434,1060.699,1058.368,1057.641,1057.056,1056.55,1055.683,1054.074,1053.229,1051.858,1050.892,1050.076,1047.822,1046.745,1046.073,1045.485,1044.397,1042.498,1041.95,1040.493,1040.096,1041.984,1042.26,1041.875,1042.449,1041.739,1042.182,1042.881,1043.629,1042.727,1043.068,1042.811,1043.431,1042.581,1041.815,1040.494,1040.632,1040.121,1038.944,1038.27,1039.141,1038.652,1037.456,1036.099,1035.668,1035.362,1034.447,1034.88,1034.617,1033.216,1030.624,1027.933,1024.456,1023.164,1022.64,1023.197,1022.701,1023.539,1024.549,1025.789,1025.514,1029.357,1038.315,1041.65,1047.399,1053.654,1059.394,1063.209,1064.044,1066.313,1071.111,1074.281,1077.983,1084.009,1088.515,1089.797,1091.706,1092.943,1093.934,1097.627,1097.796,1097.378,1100.735,1101.891,1104.671,1110.435,1111.908,1114.82,1116.511,1121.639,1124.2,1127.828,1137.229,1172.045,
1619.945,1603.807,1593.307,1589.346,1586.204,1582.384,1577.271,1569.853,1560.812,1550.139,1541.699,1534.985,1529.546,1524.525,1520.464,1516.282,1510.978,1506.625,1503.949,1502.521,1500.962,1499.62,1498.394,1498.187,1496.217,1493.805,1493.759,1493.706,1492.916,1492.301,1492.191,1491.364,1489.043,1483.546,1479.801,1475.692,1473.471,1467.841,1462.712,1461.534,1459.663,1457.243,1455.697,1453.263,1451.252,1447.232,1444.753,1441.626,1439.621,1437.095,1436.851,1438.08,1438.178,1438.559,1436.13,1432.361,1429.053,1424.492,1420.938,1417.769,1414.093,1410.531,1407.701,1405.113,1402.259,1398.872,1394.355,1381.187,1373.501,1364.339,1356.584,1344.274,1331.569,1325.954,1316.772,1300.296,1291.101,1280.331,1275.594,1266.945,1259.761,1254.727,1249.98,1243.914,1236.298,1229.946,1219.091,1214.509,1210.401,1202.816,1184.615,1177.071,1172.131,1167.863,1163.281,1159.982,1156.688,1153.086,1149.413,1145.637,1142.562,1138.981,1135.282,1133.443,1130.71,1126.442,1123.883,1120.948,1118.058,1115.052,1110.687,1108.608,1106.867,1102.485,1100.669,1097.708,1092.611,1089.245,1086.68,1084.697,1081.427,1079.928,1079.613,1077.323,1076.908,1076.385,1075.049,1074.654,1072.682,1072.064,1070.564,1069.688,1068.929,1067.737,1066.378,1064.536,1062.826,1060.716,1058.416,1058.067,1057.534,1057.871,1057.856,1056.061,1054.408,1053.399,1051.803,1049.919,1048.14,1047.573,1046.717,1046.071,1046.083,1045.217,1043.91,1042.617,1042.126,1043.119,1044.802,1044.713,1044.439,1043.694,1044.008,1044.327,1044.849,1043.864,1044.79,1045.777,1045.191,1044.5,1042.872,1042.51,1041.47,1040.884,1040.337,1040.017,1041.602,1039.797,1037.83,1036.395,1036.453,1036.207,1036.762,1036.782,1036.497,1034.766,1031.424,1028.62,1024.946,1024.231,1025.153,1024.975,1024.319,1025.609,1027.559,1028.713,1030.012,1037.718,1043.066,1047.672,1052.415,1058.682,1065.098,1067.46,1069.239,1071.701,1079.678,1081.549,1086.026,1091.648,1093.514,1094.245,1095.489,1098.16,1101.827,1103.486,1104.383,1105.023,1106.065,1107.39,1114.554,1118.134,1117.273,1119.541,1123.535,1126.906,1128.632,1132.374,1154.332,1183.12
};

#pragma warning( pop )

/*ncols,,,,,,,,,230
nrows,,,,,,,,,144
xllcorner,,,,,732900
yllcorner,,,,,215880
cellsize,,,,,,5
NODATA_value,,-9999*/

static const char* gDesc_Avalanche = "Avalanche. Rendering gets expensive in this one, consider disabling the wireframe pass (press X) or switching render models.";
#include "MyConvex.h"

class Avalanche : public TestBase
{
			ComboBoxPtr		mComboBox_ConvexIndex;
			EditBoxPtr		mEditBox_Size;
	public:
							Avalanche()								{								}
	virtual					~Avalanche()							{								}
	virtual	const char*		GetName()						const	{ return "Avalanche";			}
	virtual	const char*		GetDescription()				const	{ return gDesc_Avalanche;		}
	virtual	TestCategory	GetCategory()					const	{ return CATEGORY_PERFORMANCE;	}
	virtual	float			GetRenderData(Point& center)	const
	{
		center = GetCameraPos();
		return 2000.0f;
	}

	virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-449.34f, 1483.83f, 276.89f), Point(0.43f, -0.79f, -0.44f));
		desc.mCamera[1] = PintCameraPose(Point(11.32f, 1168.64f, 97.75f), Point(-0.92f, 0.29f, 0.28f));
		desc.mCamera[2] = PintCameraPose(Point(-485.35f, 1562.39f, 273.39f), Point(0.41f, -0.90f, -0.16f));
		desc.mCamera[3] = PintCameraPose(Point(5.05f, 1114.73f, 122.78f), Point(-0.83f, 0.41f, 0.37f));
		SetDefEnv(desc, false);
	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 256;
		WD.mHeight	= 160;
		WD.mLabel	= "Avalanche";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 80;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			y += YStep;
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Convex:", &UIElems);
			mComboBox_ConvexIndex = CreateConvexObjectComboBox(UI, 4+OffsetX, y, true);
			mComboBox_ConvexIndex->Add("Real rock data");
			mComboBox_ConvexIndex->Select(CONVEX_INDEX_13);
			RegisterUIElement(mComboBox_ConvexIndex);
			y += YStep;
		}

		{
			y += YStep;
			{
				helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Nb convexes:", &UIElems);
				mEditBox_Size = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "10000", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
				y += YStep;
			}
		}

		y += YStep;
		AddResetButton(UI, 4, y, 256-16);

		return null;
	}

	virtual bool			CommonSetup()
	{
		IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
		bool status = IS->MakePlane(230, 144);
		ASSERT(status);

		Point* V = (Point*)IS->GetVerts();

		const float Coeff = 5.0f/40.0f;
		const udword ncols = 230;
		const udword nrows = 144;
		const float* h = Heights;
		for(udword i=0;i<ncols*nrows;i++)
		{
			V[i].x *= Coeff;
			V[i].y = h[i];
			V[i].z *= Coeff;
		}

//		IS->Scale(Point(0.1f, 0.1f, 0.1f));
		IS->Flip();

		RegisterSurface(IS);

		return TestBase::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportConvexes)
			return false;

		const PINT_MATERIAL_CREATE MatDesc(0.5f, 0.5f, 0.0f);
		if(!CreateMeshesFromRegisteredSurfaces(pint, caps, &MatDesc))
			return false;

		ConvexIndex Index = CONVEX_INDEX_0;
		if(mComboBox_ConvexIndex)
			Index = ConvexIndex(mComboBox_ConvexIndex->GetSelectedIndex());

		PINT_CONVEX_CREATE ConvexCreate;
		MyConvex C;
		if(Index==CONVEX_INDEX_13+1)
		{
			const udword NbPts = ICE_ARRAYSIZE(MyConvexData)/3;
			const Point* Pts = (const Point*)MyConvexData;
			ConvexCreate.mNbVerts	= NbPts;
			ConvexCreate.mVerts		= Pts;
			ConvexCreate.mRenderer	= CreateConvexRenderer(NbPts, Pts);
		}
		else
		{
			C.LoadFile(Index);
//			C.LoadFile(13);
			ConvexCreate.mNbVerts	= C.mNbVerts;
			ConvexCreate.mVerts		= C.mVerts;
			ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);
		}

//		const Point SpawnPt(-4226.25f, 1799.95f, 258.09f);
//		const Point SpawnPt(-449.34f, 1483.83f, 276.89f);
//		const Point SpawnPt(-449.34f, 1500.0f, 276.89f);
		const Point SpawnPt(-445.0f, 1500.0f, 250.0f);

		const udword NbConvexes = GetInt(1000, mEditBox_Size);

		BasicRandom Rnd(42);
		for(udword i=0;i<NbConvexes;i++)
		{
			const float x = Rnd.RandomFloat() * 50.0f;
//			const float y = Rnd.RandomFloat() * 20.0f;
			const float y = Rnd.RandomFloat() * 40.0f;
			const float z = Rnd.RandomFloat() * 50.0f;
			PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
//			ObjectDesc.mPosition	= Point(0.0f, 1080.0f, 0.0f);
			ObjectDesc.mPosition	= SpawnPt + Point(x, y, z);
//			ObjectDesc.mPosition	= SpawnPt;
			ObjectDesc.mMass		= 1.0f;
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

}Avalanche;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_JoltPerformanceTest = "Jolt Convex Performance Test scene.";

START_TEST(JoltPerformanceTest, CATEGORY_PERFORMANCE, gDesc_JoltPerformanceTest)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(87.89f, 84.29f, 85.00f), Point(-0.48f, -0.75f, -0.45f));
		SetDefEnv(desc, false);
	}

	virtual	float	GetRenderData(Point& center)	const	{ return 400.0f;	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes || !caps.mSupportConvexes)
			return false;

		// Create terrain
		{
			const int n = 100;
			const float cell_size = 3.0f;
			const float max_height = 5.0f;
			float center = n * cell_size / 2;

			// Create vertices
			const int num_vertices = (n + 1) * (n + 1);
			Point* vertices = ICE_NEW(Point)[num_vertices];
			for (int x = 0; x <= n; ++x)
				for (int z = 0; z <= n; ++z)
				{
					float height = sin(float(x) * 50.0f / n) * cos(float(z) * 50.0f / n);
					vertices[z * (n + 1) + x] = Point(cell_size * x, max_height * height, cell_size * z);
				}

			// Create regular grid of triangles
			const int num_triangles = n * n * 2;
			udword* indices = new udword[num_triangles * 3];
			udword *next = indices;
			for (int x = 0; x < n; ++x)
				for (int z = 0; z < n; ++z)
				{
					int start = (n + 1) * z + x;

					*next++ = start;
					*next++ = start + n + 1;
					*next++ = start + 1;

					*next++ = start + 1;
					*next++ = start + n + 1;
					*next++ = start + n + 2;
				}

			IndexedSurface Surface;
			Surface.Init(num_triangles, num_vertices, vertices, (const IndexedTriangle*)indices, false);

			PINT_MESH_CREATE MeshDesc;
			MeshDesc.SetSurfaceData(Surface.GetSurfaceInterface());
			MeshDesc.mRenderer	= CreateMeshRenderer(MeshDesc.GetSurface());

			PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
			ObjectDesc.mPosition	= Point(-center, max_height, -center);
			ObjectDesc.mMass		= 0.0f;
			PintActorHandle h = CreatePintObject(pint, ObjectDesc);

			delete [] vertices;
			delete [] indices;
		}

		// Create dynamic
		{
			const Point Verts[] = { Point(0, 1, 0), Point(1, 0, 0), Point(-1, 0, 0), Point(0, 0, 1), Point(0, 0, -1) };

			const int num_shapes = 4;

			PINT_BOX_CREATE BoxDesc(0.5f, 0.75f, 1.0f);
			BoxDesc.mRenderer = CreateBoxRenderer(BoxDesc.mExtents);

			PINT_SPHERE_CREATE SphereDesc(0.5f);
			SphereDesc.mRenderer = CreateSphereRenderer(SphereDesc.mRadius);

			PINT_CAPSULE_CREATE CapsuleDesc(0.5f, 0.75f);
			CapsuleDesc.mRenderer = CreateCapsuleRenderer(CapsuleDesc.mRadius, CapsuleDesc.mHalfHeight*2.0f);

			PINT_CONVEX_CREATE ConvexDesc(5, Verts);
			ConvexDesc.mRenderer = CreateConvexRenderer(5, Verts);

			// Construct bodies
			for (int x = -10; x <= 10; ++x)
			{
				for (int y = 0; y < num_shapes; ++y)
				{
					for (int z = -10; z <= 10; ++z)
					{
						const Point Pos(7.5f * x, 15.0f + 2.0f * y, 7.5f * z);

						if(y==0)
							CreateDynamicObject(pint, &BoxDesc, Pos);
						else if(y==1)
							CreateDynamicObject(pint, &SphereDesc, Pos);
						else if(y==2)
							CreateDynamicObject(pint, &CapsuleDesc, Pos);
						else if(y==3)
							CreateDynamicObject(pint, &ConvexDesc, Pos);
					}
				}
			}
		}
		return true;
	}

END_TEST(JoltPerformanceTest)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_JoltPerformanceTest2 = "(Larger) Jolt Convex Performance Test scene.";

START_TEST(JoltPerformanceTest2, CATEGORY_PERFORMANCE, gDesc_JoltPerformanceTest2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(87.89f, 84.29f, 85.00f), Point(-0.48f, -0.75f, -0.45f));
		SetDefEnv(desc, false);
	}

	virtual	float	GetRenderData(Point& center)	const	{ return 400.0f;	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes || !caps.mSupportConvexes)
			return false;

		// Create terrain
		{
			const int n = 100;
			const float cell_size = 3.0f;
			const float max_height = 5.0f;
			float center = n * cell_size / 2;

			// Create vertices
			const int num_vertices = (n + 1) * (n + 1);
			Point* vertices = ICE_NEW(Point)[num_vertices];
			for (int x = 0; x <= n; ++x)
				for (int z = 0; z <= n; ++z)
				{
					float height = sin(float(x) * 50.0f / n) * cos(float(z) * 50.0f / n);
					vertices[z * (n + 1) + x] = Point(cell_size * x, max_height * height, cell_size * z);
				}

			// Create regular grid of triangles
			const int num_triangles = n * n * 2;
			udword* indices = new udword[num_triangles * 3];
			udword *next = indices;
			for (int x = 0; x < n; ++x)
				for (int z = 0; z < n; ++z)
				{
					int start = (n + 1) * z + x;

					*next++ = start;
					*next++ = start + n + 1;
					*next++ = start + 1;

					*next++ = start + 1;
					*next++ = start + n + 1;
					*next++ = start + n + 2;
				}

			IndexedSurface Surface;
			Surface.Init(num_triangles, num_vertices, vertices, (const IndexedTriangle*)indices, false);

			PINT_MESH_CREATE MeshDesc;
			MeshDesc.SetSurfaceData(Surface.GetSurfaceInterface());
			MeshDesc.mRenderer	= CreateMeshRenderer(MeshDesc.GetSurface());

			PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
			ObjectDesc.mPosition	= Point(-center, max_height, -center);
			ObjectDesc.mMass		= 0.0f;
			PintActorHandle h = CreatePintObject(pint, ObjectDesc);

			delete [] vertices;
			delete [] indices;
		}

		// Create dynamic
		{
			const Point Verts[] = { Point(0, 1, 0), Point(1, 0, 0), Point(-1, 0, 0), Point(0, 0, 1), Point(0, 0, -1) };

			const int num_shapes = 4;

			PINT_BOX_CREATE BoxDesc(0.5f, 0.75f, 1.0f);
			BoxDesc.mRenderer = CreateBoxRenderer(BoxDesc.mExtents);

			PINT_SPHERE_CREATE SphereDesc(0.5f);
			SphereDesc.mRenderer = CreateSphereRenderer(SphereDesc.mRadius);

			PINT_CAPSULE_CREATE CapsuleDesc(0.5f, 0.75f);
			CapsuleDesc.mRenderer = CreateCapsuleRenderer(CapsuleDesc.mRadius, CapsuleDesc.mHalfHeight*2.0f);

			PINT_CONVEX_CREATE ConvexDesc(5, Verts);
			ConvexDesc.mRenderer = CreateConvexRenderer(5, Verts);

			// Construct bodies
			const int range = 16;
			for (int x = -range; x <= range; ++x)
			{
				for (int k = 0; k < 2; ++k)
				for (int y = 0; y < num_shapes; ++y)
				{
					for (int z = -range; z <= range; ++z)
					{
						const Point Pos(7.5f * x, 15.0f + 2.0f * y + 10.0f * k, 7.5f * z);

						if(y==0)
							CreateDynamicObject(pint, &BoxDesc, Pos);
						else if(y==1)
							CreateDynamicObject(pint, &SphereDesc, Pos);
						else if(y==2)
							CreateDynamicObject(pint, &CapsuleDesc, Pos);
						else if(y==3)
							CreateDynamicObject(pint, &ConvexDesc, Pos);
					}
				}
			}
		}
		return true;
	}

END_TEST(JoltPerformanceTest2)
