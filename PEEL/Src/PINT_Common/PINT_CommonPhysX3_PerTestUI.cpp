///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX3 plug-ins, so put only the code here that is "the same" for all versions.

// We cannot tweak the PhysX-specific parameters per-scene, since we would have to expose all of them to the shared PINT API.
// So instead we tweak them here based on the test names. This gives us the same flexibility as for regular PhysX/Bullet/etc samples,
// but in the context of PEEL.
//
// We can also create specific per-test per-engine UI dialogs to clearly show which parameters are important/useful for any
// combination of test & physics engine. It's getting complicated but that's a pretty powerful feature.

#include "stdafx.h"
#include "PINT_Common.h"
#include "PINT_CommonPhysX3.h"
#include "PINT_CommonPhysX3_PerTestUI.h"

///////////////////////////////////////////////////////////////////////////////

// Only one test will be active at any time so we can actually share these objects.
static IceCheckBox*	gCheckBox_Override = null;
static IceCheckBox*	gCheckBox_Generic0 = null;
static IceCheckBox*	gCheckBox_Generic1 = null;
static IceCheckBox*	gCheckBox_Generic2 = null;
static IceEditBox* gEditBox_Generic0 = null;
static IceEditBox* gEditBox_Generic1 = null;
static IceSlider* gSlider_Generic0 = null;
static IceLabel* gLabel_Generic0 = null;

void PhysXPlugIn::CloseTestGUI()
{
	gCheckBox_Override = null;
	gCheckBox_Generic0 = null;
	gCheckBox_Generic1 = null;
	gCheckBox_Generic2 = null;
	gEditBox_Generic0 = null;
	gEditBox_Generic1 = null;
	gSlider_Generic0 = null;
	gLabel_Generic0 = null;
}

static const sdword OffsetX = 90;
static const sdword EditBoxWidth = 60;
static const sdword LabelOffsetY = 2;
static const sdword YStep = 20;
static const sdword LabelWidth = 90;
static const sdword EditBoxX = 100;
static const udword CheckBoxWidth = 200;

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateTabWindow(IceWidget* parent, Widgets& owner)
{
	WindowDesc WD;
	WD.mParent	= parent;
	WD.mX		= 0;
	WD.mY		= 0;
//	WD.mWidth	= WD.mWidth;
//	WD.mHeight	= TCD.mHeight;
//	WD.mLabel	= "Tab";
	WD.mType	= WINDOW_DIALOG;
	IceWindow* TabWindow = ICE_NEW(IceWindow)(WD);
	owner.Register(TabWindow);
//	TabWindow->SetVisible(true);
	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef REMOVED
namespace
{
	enum UIElemState
	{
		UI_ELEM_DISABLED,
		UI_ELEM_ENABLED_CHECKED,
		UI_ELEM_ENABLED_UNCHECKED,
	};

	struct UICREATE
	{
		UICREATE() :
			mTGS	(UI_ELEM_DISABLED)
		{
		}

		UIElemState	mTGS;

/*		IceCheckBox** GetCheckBox()
		{
			if(!gCheckBox_Generic0)
				return &gCheckBox_Generic0;
			if(!gCheckBox_Generic1)
				return &gCheckBox_Generic1;
			if(!gCheckBox_Generic2)
				return &gCheckBox_Generic2;
			ASSERT(0);
			return null;
		}*/
	};
}
static IceWindow* CreateUI_Generic(IceWidget* parent, PintGUIHelper& helper, Widgets& owner, const UICREATE& create)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void Generic(const IceCheckBox& check_box, bool checked, void* user_data)
	{
#if PHYSX_SUPPORT_TGS
		gCheckBox_Generic0->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::Generic, null);
	y += YStep;

#if PHYSX_SUPPORT_TGS
	if(create.mTGS!=UI_ELEM_DISABLED)
	{
		IceCheckBox* CB = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Enable TGS", &owner, create.mTGS==UI_ELEM_ENABLED_CHECKED, null, null);

		gCheckBox_Generic0 = 
		y += YStep;
	}
#endif
	return TabWindow;
}
#endif

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_TGS(IceWidget* parent, PintGUIHelper& helper, Widgets& owner, bool enabled)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void TGS(const IceCheckBox& check_box, bool checked, void* user_data)
	{
#if PHYSX_SUPPORT_TGS
		gCheckBox_Generic0->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::TGS, null);
	y += YStep;

#if PHYSX_SUPPORT_TGS
	ASSERT(!gCheckBox_Generic0);
	gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Enable TGS", &owner, enabled, null, null);
	y += YStep;
#endif
	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_Winch(IceWidget* parent, PintGUIHelper& helper, Widgets& owner, bool enabled)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void Winch(const IceCheckBox& check_box, bool checked, void* user_data)
	{
#if PHYSX_SUPPORT_TGS
		gCheckBox_Generic0->SetEnabled(checked);
#endif
#if PHYSX_SUPPORT_SUBSTEPS
		gEditBox_Generic0->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::Winch, null);
	y += YStep;

#if PHYSX_SUPPORT_TGS
	ASSERT(!gCheckBox_Generic0);
	gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Enable TGS", &owner, enabled, null, null);
	y += YStep;
#endif
#if PHYSX_SUPPORT_SUBSTEPS
	helper.CreateLabel(TabWindow, x, y+LabelOffsetY, LabelWidth, 20, "Nb substeps:", &owner);
	ASSERT(!gEditBox_Generic0);
	gEditBox_Generic0 = helper.CreateEditBox(TabWindow, 0, x+EditBoxX, y, EditBoxWidth, 20, "2", &owner, EDITBOX_INTEGER_POSITIVE, null, null);
	y += YStep;
#endif

	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_HighMassRatio(IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void HighMassRatio(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gEditBox_Generic0->SetEnabled(checked);
#if PHYSX_SUPPORT_TGS
		gCheckBox_Generic0->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::HighMassRatio, null);
	y += YStep;

#if PHYSX_SUPPORT_TGS
	ASSERT(!gCheckBox_Generic0);
	gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Enable TGS", &owner, true, null, null);
	y += YStep;
#endif
	helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Solver iter pos:", &owner);
	ASSERT(!gEditBox_Generic0);
	gEditBox_Generic0 = helper.CreateEditBox(TabWindow, 0, 4+EditBoxX, y, EditBoxWidth, 20, "16", &owner, EDITBOX_INTEGER_POSITIVE, null, null);
	y += YStep;

	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_InitialPenetration(IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void InitialPenetration(const IceCheckBox& check_box, bool checked, void* user_data)
	{
#if PHYSX_SUPPORT_MAX_DEPEN_VELOCITY
		gEditBox_Generic0->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::InitialPenetration, null);
	y += YStep;

#if PHYSX_SUPPORT_MAX_DEPEN_VELOCITY
	helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, 100, 20, "Max depen. velocity:", &owner);
	ASSERT(!gEditBox_Generic0);
	gEditBox_Generic0 = helper.CreateEditBox(TabWindow, 0, 4+20+EditBoxX, y, EditBoxWidth, 20, "2", &owner, EDITBOX_FLOAT_POSITIVE, null, null);
	y += YStep;
#endif
	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_FrictionRamp(IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void FrictionRamp(const IceCheckBox& check_box, bool checked, void* user_data)
	{
#if PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
		gCheckBox_Generic0->SetEnabled(checked);
#endif
#if PHYSX_SUPPORT_STABILIZATION_FLAG
		gCheckBox_Generic1->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::FrictionRamp, null);
	y += YStep;

#if PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
	ASSERT(!gCheckBox_Generic0);
	gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Improved patch friction", &owner, true, null, null);
	y += YStep;
#endif
#if PHYSX_SUPPORT_STABILIZATION_FLAG
	ASSERT(!gCheckBox_Generic1);
	gCheckBox_Generic1 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Stabilization", &owner, false, null, null);
	y += YStep;
#endif
	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_Dzhanibekov(IceWidget* parent, PintGUIHelper& helper, Widgets& owner, bool enabled)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void Gyro(const IceCheckBox& check_box, bool checked, void* user_data)
	{
#if PHYSX_SUPPORT_GYROSCOPIC_FORCES
		gCheckBox_Generic0->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::Gyro, null);
	y += YStep;

#if PHYSX_SUPPORT_GYROSCOPIC_FORCES
	ASSERT(!gCheckBox_Generic0);
	gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Enable gyroscopic forces", &owner, enabled, null, null);
	y += YStep;
#endif
	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_KinematicWalker(IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void Walker(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gEditBox_Generic0->SetEnabled(checked);
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::Walker, null);
	y += YStep;

	const float DesiredBaumgarte = 0.2f;
	const float Timestep = 1.0f/60.0f;
	const float MaxBiasCoeff = DesiredBaumgarte / Timestep;

	helper.CreateLabel(TabWindow, x, y+LabelOffsetY, LabelWidth, 20, "Max bias coeff:", &owner);
	ASSERT(!gEditBox_Generic0);
	gEditBox_Generic0 = helper.CreateEditBox(TabWindow, 0, x+EditBoxX, y, EditBoxWidth, 20, helper.Convert(MaxBiasCoeff), &owner, EDITBOX_FLOAT_POSITIVE, null, null);
	y += YStep;

	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_DoubleDominoEffect(IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void Walker(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gCheckBox_Generic0->SetEnabled(checked);
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::Walker, null);
	y += YStep;

	helper.CreateLabel(TabWindow, x, y+LabelOffsetY, LabelWidth, 20, "Enable sleeping:", &owner);
	ASSERT(!gCheckBox_Generic0);
	gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Enable sleeping", &owner, false, null, null);
	y += YStep;

	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_SolverIterPos_Substeps(IceWidget* parent, PintGUIHelper& helper, Widgets& owner, udword nb_iter, udword nb_substeps)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void Generic(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gEditBox_Generic1->SetEnabled(checked);
#if PHYSX_SUPPORT_SUBSTEPS
		gEditBox_Generic0->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::Generic, null);
	y += YStep;

	helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Solver iter pos:", &owner);
	ASSERT(!gEditBox_Generic1);
	gEditBox_Generic1 = helper.CreateEditBox(TabWindow, 0, 4+EditBoxX, y, EditBoxWidth, 20, _F("%d", nb_iter), &owner, EDITBOX_INTEGER_POSITIVE, null, null);
	y += YStep;

#if PHYSX_SUPPORT_SUBSTEPS
	helper.CreateLabel(TabWindow, x, y+LabelOffsetY, LabelWidth, 20, "Nb substeps:", &owner);
	ASSERT(!gEditBox_Generic0);
	gEditBox_Generic0 = helper.CreateEditBox(TabWindow, 0, x+EditBoxX, y, EditBoxWidth, 20, _F("%d", nb_substeps), &owner, EDITBOX_INTEGER_POSITIVE, null, null);
	y += YStep;
#endif
	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_ArticulationDriveVsStaticObstacle(IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void ArticulationDriveVsStaticObstacle(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gEditBox_Generic0->SetEnabled(checked);
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::ArticulationDriveVsStaticObstacle, null);
	y += YStep;

	helper.CreateLabel(TabWindow, x, y+LabelOffsetY, LabelWidth, 20, "Contact offset:", &owner);
	ASSERT(!gEditBox_Generic0);
	gEditBox_Generic0 = helper.CreateEditBox(TabWindow, 0, x+EditBoxX, y, EditBoxWidth, 20, "0.2", &owner, EDITBOX_FLOAT_POSITIVE, null, null);
	y += YStep;

	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_MultiBodyVehicle(IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void MultiBodyVehicle(const IceCheckBox& check_box, bool checked, void* user_data)
	{
#if PHYSX_SUPPORT_SUBSTEPS
		gEditBox_Generic0->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::MultiBodyVehicle, null);
	y += YStep;

#if PHYSX_SUPPORT_SUBSTEPS
	helper.CreateLabel(TabWindow, x, y+LabelOffsetY, LabelWidth, 20, "Nb substeps:", &owner);
	ASSERT(!gEditBox_Generic0);
	gEditBox_Generic0 = helper.CreateEditBox(TabWindow, 0, x+EditBoxX, y, EditBoxWidth, 20, "16", &owner, EDITBOX_INTEGER_POSITIVE, null, null);
	y += YStep;
#endif
	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_VehicleTest(IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void VehicleTest(const IceCheckBox& check_box, bool checked, void* user_data)
	{
#if PHYSX_SUPPORT_SUBSTEPS
		gEditBox_Generic0->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::VehicleTest, null);
	y += YStep;

#if PHYSX_SUPPORT_SUBSTEPS
	helper.CreateLabel(TabWindow, x, y+LabelOffsetY, LabelWidth, 20, "Nb substeps:", &owner);
	ASSERT(!gEditBox_Generic0);
	gEditBox_Generic0 = helper.CreateEditBox(TabWindow, 0, x+EditBoxX, y, EditBoxWidth, 20, "4", &owner, EDITBOX_INTEGER_POSITIVE, null, null);
	y += YStep;
#endif
	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

class IterSlider : public IceSlider
{
	public:
					IterSlider(const SliderDesc& desc, PhysXPlugIn* plugin) : IceSlider(desc), mPlugin(plugin)	{}
	virtual			~IterSlider()																				{}

			udword	GetNbIters()	const
					{
//						return 1 + udword(GetValue()*254.0f);
//						return 1 + udword(GetValue()*63.0f);
						return 1 + udword(GetValue()*79.0f);
					}

			void	SetNbIters(udword nb)
					{
						SetValue((nb-1)/79.0f);
					}

	virtual	void	OnSliderEvent(SliderEvent event)
	{
		const udword NbIters = GetNbIters();
//		printf("%d\n", NbIters);
		gLabel_Generic0->SetLabel(_F("%d solver iterations:", NbIters));

		Pint* pp = mPlugin->GetPint();
		if(!pp)
			return;

		SharedPhysX* p = static_cast<SharedPhysX*>(pp);
		ActorManager& AM = p->GetActorManager();
		udword NbActors = AM.GetNbDynamicActors();
		const ActorData* Actors = AM.GetDynamicActors();
		while(NbActors--)
		{
			if(Actors->mActor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
			{
				PxRigidDynamic* Dyna = static_cast<PxRigidDynamic*>(Actors->mActor);
				Dyna->setSolverIterationCounts(NbIters, 1);
				Dyna->wakeUp();
			}
			Actors++;
		}
	}

	PhysXPlugIn*	mPlugin;
};

static IceWindow* CreateUI_FixedJointsTorus(IceWidget* parent, PintGUIHelper& helper, Widgets& owner, PhysXPlugIn* plugin, udword nb_iters)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void FixedJointsTorus(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gSlider_Generic0->SetEnabled(checked);
#if PHYSX_SUPPORT_TGS
		gCheckBox_Generic0->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::FixedJointsTorus, null);
	y += YStep;

#if PHYSX_SUPPORT_TGS
	ASSERT(!gCheckBox_Generic0);
	gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Enable TGS", &owner, true, null, null);
	y += YStep;
#endif

	ASSERT(!gLabel_Generic0);
	gLabel_Generic0 = helper.CreateLabel(TabWindow, x, y+LabelOffsetY, 200, 20, "Nb iterations:", &owner);
	y += 24;

	SliderDesc SD;
	SD.mStyle		= SLIDER_HORIZONTAL;
	SD.mID			= 0;
	SD.mParent		= TabWindow;
	SD.mX			= 4;
	SD.mY			= y;
	SD.mWidth		= 250;
	SD.mHeight		= 30;
	SD.mLabel		= "Nb iters";
	ASSERT(!gSlider_Generic0);
	IterSlider* S	= ICE_NEW(IterSlider)(SD, plugin);
	owner.Register(S);
	gSlider_Generic0	= S;
	S->SetRange(0.0f, 1.0f, 256);
//	S->SetValue(0.5f);
//	S->SetValue(1.0f);
	S->SetNbIters(nb_iters);
	S->OnSliderEvent(SliderEvent());

	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_AddLocalTorque(IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void AddLocalTorque(const IceCheckBox& check_box, bool checked, void* user_data)
	{
#if PHYSX_SUPPORT_STABILIZATION_FLAG
		gCheckBox_Generic1->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::AddLocalTorque, null);
	y += YStep;

#if PHYSX_SUPPORT_STABILIZATION_FLAG
	ASSERT(!gCheckBox_Generic1);
	gCheckBox_Generic1 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Stabilization", &owner, false, null, null);
	y += YStep;
#endif

	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_BoxStacks(IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void BoxStacks(const IceCheckBox& check_box, bool checked, void* user_data)
	{
#if PHYSX_SUPPORT_TGS
		gCheckBox_Generic0->SetEnabled(checked);
#endif
#if PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
		gCheckBox_Generic1->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::BoxStacks, null);
	y += YStep;

#if PHYSX_SUPPORT_TGS
	ASSERT(!gCheckBox_Generic0);
	gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Enable TGS", &owner, true, null, null);
	y += YStep;
#endif

#if PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
	ASSERT(!gCheckBox_Generic1);
	gCheckBox_Generic1 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Improved patch friction", &owner, true, null, null);
	y += YStep;
#endif
	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_JacobsLadder(IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void JacobsLadder(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gEditBox_Generic0->SetEnabled(checked);
#if PHYSX_SUPPORT_TGS
		gCheckBox_Generic0->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::JacobsLadder, null);
	y += YStep;

#if PHYSX_SUPPORT_TGS
	ASSERT(!gCheckBox_Generic0);
	gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Enable TGS", &owner, true, null, null);
	y += YStep;
#endif
	helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Solver iter pos:", &owner);
	ASSERT(!gEditBox_Generic0);
	gEditBox_Generic0 = helper.CreateEditBox(TabWindow, 0, 4+EditBoxX, y, EditBoxWidth, 20, "16", &owner, EDITBOX_INTEGER_POSITIVE, null, null);
	y += YStep;

	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

/*static IceWindow* CreateUI_LimitedHingeJoint(IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void LimitedHingeJoint(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gCheckBox_Generic0->SetEnabled(checked);
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::LimitedHingeJoint, null);
	y += YStep;

	ASSERT(!gCheckBox_Generic0);
	gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Visualize joint limits", &owner, true, null, null);
	y += YStep;

	return TabWindow;
}*/

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCD =
"By design the CCD tests fail without CCD enabled.\n\
Check this box to make them pass using the\n\
default PhysX CCD algorithm.";

static const char* gDesc_AngularCCD =
"The default CCD doesn't support angular CCD, or\n\
CCD on kinematics. If this is needed you can use\n\
speculative contacts instead - it has pros & cons.";

static const char* gDesc_RaycastCCD =
"Another alternative is raycast-based CCD, which\n\
is not accurate but cheap and good enough when all\n\
you want is to stop small objects from tunnelling.";

static IceWindow* CreateUI_CCD(IceWidget* parent, PintGUIHelper& helper, Widgets& owner, const char* test_name)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void CCD(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gCheckBox_Generic0->SetEnabled(checked);
#if PHYSX_SUPPORT_ANGULAR_CCD
		gCheckBox_Generic1->SetEnabled(checked);
#endif
#if PHYSX_SUPPORT_RAYCAST_CCD
		gCheckBox_Generic2->SetEnabled(checked);
#endif
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::CCD, null);
	y += YStep;

	bool EnableCCD = true;
#if PHYSX_SUPPORT_ANGULAR_CCD
	const bool NeedsAngularCCD = strcmp(test_name, "CCDTest_AngularCCD")==0;
	if(NeedsAngularCCD)
		EnableCCD = false;
#endif

	const sdword x2 = x + 10;
	const sdword CheckBoxWidth2 = 220;
	{
		sdword YSaved = y;
		y += 50;

		ASSERT(!gCheckBox_Generic0);
		gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, x2, y, CheckBoxWidth2, 20, "Enable default PhysX CCD", &owner, EnableCCD, null, null);
		y += YStep;

		IceEditBox* EB = helper.CreateEditBox(TabWindow, 0, x, YSaved, 260, 80, "", &owner, EDITBOX_TEXT, null);
		EB->SetReadOnly(true);
		EB->SetMultilineText(gDesc_CCD);
		y += 20;
	}

#if PHYSX_SUPPORT_ANGULAR_CCD
	{
		sdword YSaved = y;
		y += 50;

		ASSERT(!gCheckBox_Generic1);
		gCheckBox_Generic1 = helper.CreateCheckBox(TabWindow, 0, x2, y, CheckBoxWidth2, 20, "Enable angular CCD/speculative contacts", &owner, NeedsAngularCCD, null, null);
		y += YStep;

		IceEditBox* EB = helper.CreateEditBox(TabWindow, 0, x, YSaved, 260, 80, "", &owner, EDITBOX_TEXT, null);
		EB->SetReadOnly(true);
		EB->SetMultilineText(gDesc_AngularCCD);
		y += 20;
	}
#endif

#if PHYSX_SUPPORT_RAYCAST_CCD
	{
		sdword YSaved = y;
		y += 50;

		ASSERT(!gCheckBox_Generic2);
		gCheckBox_Generic2 = helper.CreateCheckBox(TabWindow, 0, x2, y, CheckBoxWidth2, 20, "Enable raycast CCD", &owner, false, null, null);
		y += YStep;

		IceEditBox* EB = helper.CreateEditBox(TabWindow, 0, x, YSaved, 260, 80, "", &owner, EDITBOX_TEXT, null);
		EB->SetReadOnly(true);
		EB->SetMultilineText(gDesc_RaycastCCD);
		y += 20;
	}
#endif
	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

static bool IsCCDTest(const char* test_name)
{
	return strncmp(test_name, "CCDTest_", 8)==0;
}

static bool IsJointTest(const char* test_name)
{
	if(	strcmp(test_name, "BasicJointAPI")==0
	||	strcmp(test_name, "CheckCollisionBetweenJointed")==0
	||	strcmp(test_name, "LimitedHingeJoint")==0
	||	strcmp(test_name, "LimitedHingeJoint2")==0
	||	strcmp(test_name, "ConfigurableLimitedHingeJoint")==0
	||	strcmp(test_name, "LimitedPrismaticJoint")==0
	||	strcmp(test_name, "LimitedPrismaticJoint2")==0
	||	strcmp(test_name, "PrismaticSpring")==0	
	||	strcmp(test_name, "D6Joint_PointInPlane")==0
	||	strcmp(test_name, "D6Joint_PointInBox")==0
	)
		return true;
	return false;
}

IceWindow* PhysXPlugIn::InitTestGUI(const char* test_name, IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
/*	if(strcmp(test_name, "LimitedHingeJoint")==0)
		return CreateUI_LimitedHingeJoint(parent, helper, owner);*/

	if(IsCCDTest(test_name))
		return CreateUI_CCD(parent, helper, owner, test_name);

	if(	strcmp(test_name, "ArticulationDriveVsStaticObstacle")==0
	||	strcmp(test_name, "HingeJointMotorVsObstacle")==0)		
		return CreateUI_ArticulationDriveVsStaticObstacle(parent, helper, owner);

	if(	strcmp(test_name, "CompoundTowerTweaked")==0)
		return CreateUI_SolverIterPos_Substeps(parent, helper, owner, 32, 2);

	if(	strcmp(test_name, "LegoTechnicKart")==0)
		return CreateUI_SolverIterPos_Substeps(parent, helper, owner, 4, 4);

	if(	strcmp(test_name, "LegoTechnicBuggy")==0)
		return CreateUI_SolverIterPos_Substeps(parent, helper, owner, 64, 4);

	if(	strcmp(test_name, "CDStack")==0)
		return CreateUI_SolverIterPos_Substeps(parent, helper, owner, 32, 4);

	if(	strcmp(test_name, "Dzhanibekov")==0)
		return CreateUI_Dzhanibekov(parent, helper, owner, true);

	if(	strcmp(test_name, "Walker")==0)
		return CreateUI_KinematicWalker(parent, helper, owner);

	if(	strcmp(test_name, "DoubleDominoEffect")==0)
		return CreateUI_DoubleDominoEffect(parent, helper, owner);

	if(	strcmp(test_name, "ArticulatedChain_RegularJoints")==0
	||	strcmp(test_name, "ConfigurableSphericalChain")==0
	||	strcmp(test_name, "SwingSet_RegularJoints")==0
	||	strcmp(test_name, "Crane_RegularJoints")==0
	||	strcmp(test_name, "VoronoiFracture3")==0
	||	strcmp(test_name, "CardHouse")==0
		)
		return CreateUI_TGS(parent, helper, owner, true);

	if(	strcmp(test_name, "ArticulatedChain_MCArticulation")==0
	||	strcmp(test_name, "SwingSet_MCArticulation")==0
	||	strcmp(test_name, "Crane_MCArticulation")==0
	||	strcmp(test_name, "ConnectedMCArticulations")==0
		)
		return CreateUI_TGS(parent, helper, owner, false);

	if(	strcmp(test_name, "Winch")==0)
		return CreateUI_Winch(parent, helper, owner, false);

	if(strcmp(test_name, "BoxStacks")==0)
		return CreateUI_BoxStacks(parent, helper, owner);

	if(strcmp(test_name, "JacobsLadder")==0)
		return CreateUI_JacobsLadder(parent, helper, owner);

	if(strcmp(test_name, "AddLocalTorque")==0)
		return CreateUI_AddLocalTorque(parent, helper, owner);

	if(strcmp(test_name, "FixedJointsTorus")==0)
		return CreateUI_FixedJointsTorus(parent, helper, owner, this, 4);

	if(strcmp(test_name, "FixedJointCantilever")==0)
		return CreateUI_FixedJointsTorus(parent, helper, owner, this, 33);

	if(strcmp(test_name, "HighMassRatio")==0)
		return CreateUI_HighMassRatio(parent, helper, owner);

	if(strcmp(test_name, "InitialPenetration")==0)
		return CreateUI_InitialPenetration(parent, helper, owner);

	if(		strcmp(test_name, "FrictionRamp")==0
		||	strcmp(test_name, "FrictionRamp2")==0
		||	strcmp(test_name, "FrictionRamp3")==0
		)
		return CreateUI_FrictionRamp(parent, helper, owner);

	if(strcmp(test_name, "MultiBodyVehicle")==0)
		return CreateUI_MultiBodyVehicle(parent, helper, owner);

	if(strcmp(test_name, "VehicleTest")==0 || strcmp(test_name, "Vehicle2")==0)
		return CreateUI_VehicleTest(parent, helper, owner);

	return null;
}

void PhysXPlugIn::ApplyTestUIParams(const char* test_name)
{
	EditableParams& EP = const_cast<EditableParams&>(PhysX3::GetEditableParams());

	if(IsJointTest(test_name))
		EP.mForceJointLimitDebugViz = true;

	const bool ApplySettings = gCheckBox_Override ? gCheckBox_Override->IsChecked() : false;
	if(!ApplySettings)
		return;

	struct Local
	{
		static void OverrideSubsteps(EditableParams& EP)
		{
#if PHYSX_SUPPORT_SUBSTEPS
			udword NbSubsteps;
			Common_GetFromEditBox(NbSubsteps, gEditBox_Generic0);
			if(NbSubsteps>EP.mNbSubsteps)
				EP.mNbSubsteps = NbSubsteps;
#endif
		}
	};

	if(IsCCDTest(test_name))
	{
		EP.mEnableCCD = gCheckBox_Generic0->IsChecked();
#if PHYSX_SUPPORT_ANGULAR_CCD
		EP.mEnableAngularCCD = gCheckBox_Generic1->IsChecked();
#endif
#if PHYSX_SUPPORT_RAYCAST_CCD
		EP.mEnableRaycastCCDStatic = gCheckBox_Generic2->IsChecked();
		EP.mEnableRaycastCCDDynamic = gCheckBox_Generic2->IsChecked();
#endif
		return;
	}

	if(	strcmp(test_name, "ArticulatedChain_RegularJoints")==0
	||	strcmp(test_name, "ArticulatedChain_MCArticulation")==0
	||	strcmp(test_name, "ConfigurableSphericalChain")==0
	||	strcmp(test_name, "SwingSet_RegularJoints")==0
	||	strcmp(test_name, "SwingSet_MCArticulation")==0
	||	strcmp(test_name, "Crane_RegularJoints")==0
	||	strcmp(test_name, "Crane_MCArticulation")==0
	||	strcmp(test_name, "VoronoiFracture3")==0
	||	strcmp(test_name, "ConnectedMCArticulations")==0
	||	strcmp(test_name, "CardHouse")==0
	)
	{
#if PHYSX_SUPPORT_TGS
		EP.mTGS = gCheckBox_Generic0->IsChecked();
#endif
		return;
	}

	if(	strcmp(test_name, "Dzhanibekov")==0)
	{
#if PHYSX_SUPPORT_GYROSCOPIC_FORCES
		EP.mGyro = gCheckBox_Generic0->IsChecked();
#endif
		EP.mAngularDamping = 0.0f;
		return;
	}

	if(	strcmp(test_name, "Walker")==0)
	{
		Common_GetFromEditBox(EP.mMaxBiasCoeff, gEditBox_Generic0, 0.0f, MAX_FLOAT);
		return;
	}

	if(	strcmp(test_name, "DoubleDominoEffect")==0)
	{
		EP.mEnableSleeping = gCheckBox_Generic0->IsChecked();
		return;
	}

	if(	strcmp(test_name, "Winch")==0)
	{
#if PHYSX_SUPPORT_TGS
		EP.mTGS = gCheckBox_Generic0->IsChecked();
#endif
		Local::OverrideSubsteps(EP);
		return;
	}

	if(	strcmp(test_name, "FixedJointsTorus")==0
	||	strcmp(test_name, "FixedJointCantilever")==0)
	{
#if PHYSX_SUPPORT_TGS
		EP.mTGS = gCheckBox_Generic0->IsChecked();
#endif
		const udword NbIters = static_cast<IterSlider*>(gSlider_Generic0)->GetNbIters();
		if(NbIters>EP.mSolverIterationCountPos)
		{
			EP.mSolverIterationCountPos = NbIters;
			EP.mSolverIterationCountVel = 1;
		}
		return;
	}

	if(strcmp(test_name, "HighMassRatio")==0)
	{
		udword NbIters;
		Common_GetFromEditBox(NbIters, gEditBox_Generic0);
		if(NbIters>EP.mSolverIterationCountPos)
			EP.mSolverIterationCountPos = NbIters;
#if PHYSX_SUPPORT_TGS
		EP.mTGS = gCheckBox_Generic0->IsChecked();
#endif
		return;
	}

	if(strcmp(test_name, "InitialPenetration")==0)
	{
#if PHYSX_SUPPORT_MAX_DEPEN_VELOCITY
		Common_GetFromEditBox(EP.mMaxDepenVelocity, gEditBox_Generic0, 0.0f, FLT_MAX);
#endif
		return;
	}

	if(strcmp(test_name, "FrictionRamp")==0
	|| strcmp(test_name, "FrictionRamp2")==0
	|| strcmp(test_name, "FrictionRamp3")==0
		)
	{
#if PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
		EP.mImprovedPatchFriction = gCheckBox_Generic0->IsChecked();
#endif
#if PHYSX_SUPPORT_STABILIZATION_FLAG
		EP.mStabilization = gCheckBox_Generic1->IsChecked();
#endif
		return;
	}

	if(strcmp(test_name, "MultiBodyVehicle")==0)
	{
		Local::OverrideSubsteps(EP);
		return;
	}

	if(	strcmp(test_name, "VehicleTest")==0
	||	strcmp(test_name, "Vehicle2")==0
		)
	{
		Local::OverrideSubsteps(EP);
		return;
	}

	if(	strcmp(test_name, "CompoundTowerTweaked")==0
	||	strcmp(test_name, "LegoTechnicKart")==0
	||	strcmp(test_name, "LegoTechnicBuggy")==0
	||	strcmp(test_name, "CDStack")==0
		)
	{
		Local::OverrideSubsteps(EP);
		udword NbIters;
		Common_GetFromEditBox(NbIters, gEditBox_Generic1);
		if(NbIters>EP.mSolverIterationCountPos)
			EP.mSolverIterationCountPos = NbIters;
		return;
	}

	if(	strcmp(test_name, "ArticulationDriveVsStaticObstacle")==0
	||	strcmp(test_name, "HingeJointMotorVsObstacle")==0)		
	{
		float ContactOffset;
		Common_GetFromEditBox(ContactOffset, gEditBox_Generic0, 0.0f, MAX_FLOAT);
		if(ContactOffset>EP.mContactOffset)
			EP.mContactOffset = ContactOffset;
		return;
	}

	if(strcmp(test_name, "AddLocalTorque")==0)
	{
#if PHYSX_SUPPORT_STABILIZATION_FLAG
		EP.mStabilization = gCheckBox_Generic1->IsChecked();
#endif
		return;
	}

	if(strcmp(test_name, "BoxStacks")==0)
	{
#if PHYSX_SUPPORT_TGS
		EP.mTGS = gCheckBox_Generic0->IsChecked();
#endif
#if PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
		EP.mImprovedPatchFriction = gCheckBox_Generic1->IsChecked();
#endif
		return;
	}

	if(strcmp(test_name, "JacobsLadder")==0)
	{
#if PHYSX_SUPPORT_TGS
		EP.mTGS = gCheckBox_Generic0->IsChecked();
#endif
		udword NbIters;
		Common_GetFromEditBox(NbIters, gEditBox_Generic0);
		if(NbIters>EP.mSolverIterationCountPos)
			EP.mSolverIterationCountPos = NbIters;
		return;
	}

/*	if(strcmp(test_name, "SwingSet")==0)
	{
#if PHYSX_SUPPORT_TGS
		EP.mTGS = gCheckBox_Generic0->IsChecked();
#endif
		return;
	}*/
}
