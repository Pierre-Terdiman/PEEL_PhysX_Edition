///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PEEL.h"
#include "PintShapeRenderer.h"
#include "Camera.h"
#include "TestScenes_VehiclesBase.h"
#include "TestScenesHelpers.h"
#include "PintObjectsManager.h"
#include "Cylinder.h"
#include "ProceduralTrack.h"
#include "Loader_Bin.h"
#include "GUI_Helpers.h"
#include "GLFontRenderer.h"
#include "LoopTheLoop.h"
#include "TerrainRegionManager.h"
#include "TerrainStreamingUI.h"

static const bool gTrySound = false;

extern float gZCB2_RenderScale;
extern String gVehicleFilename;

enum VehicleScene
{
	SCENE_FACTORY,
	SCENE_FLAT,
	SCENE_PLAYGROUND,
	SCENE_PLAYGROUND_BRIDGE,
	SCENE_PLAYGROUND_LOGS,
	SCENE_SINE,
	SCENE_TERRAIN,
	SCENE_RACE_TRACK,
	SCENE_STRAIGHT_ROAD,
	SCENE_ARCHIPELAGO,
	SCENE_KP,
	SCENE_TIRE_TEST,
	SCENE_BOXES,
	SCENE_BOX,
	SCENE_TERRAIN_STREAMING,
};

static const bool gCreateDefaultEnv[] = 
{
	false,	//SCENE_FACTORY
	false,	//SCENE_FLAT
	false,	//SCENE_PLAYGROUND
	false,	//SCENE_PLAYGROUND_BRIDGE
	false,	//SCENE_PLAYGROUND_LOGS
	false,	//SCENE_SINE
	false,	//SCENE_TERRAIN
	false,	//SCENE_RACE_TRACK
	false,	//SCENE_STRAIGHT_ROAD
	false,	//SCENE_ARCHIPELAGO
	false,	//SCENE_KP
	false,	//SCENE_TIRE_TEST
	true,	//SCENE_BOXES
	true,	//SCENE_BOX
	false,	//SCENE_TERRAIN_STREAMING
};

const Quat gZtoX = ShortestRotation(Point(0.0f, 0.0f, 1.0f), Point(1.0f, 0.0f, 0.0f));
const Quat gZtoNZ = ShortestRotation(Point(0.0f, 0.0f, 1.0f), Point(0.0f, 0.0f, -1.0f));

static const PR gStartPoses[] = {
	PR(Point(0.0f, 6.0f, 0.0f), Quat(Idt)),	// SCENE_FACTORY
	PR(Point(0.0f, 6.0f, 0.0f), Quat(Idt)),	// SCENE_FLAT
//	PR(Point(60.5f, 5.9f, 40.1f), ShortestRotation(Point(0.0f, 0.0f, 1.0f), Point(1.0f, 0.0f, 0.0f))),	// SCENE_PLAYGROUND
	PR(Point(0.0f, 6.0f, 0.0f), Quat(Idt)),	// SCENE_PLAYGROUND
	PR(Point(-150.0f, 6.0f, 0.0f), gZtoX),	// SCENE_PLAYGROUND_BRIDGE
	PR(Point(20.0f, 6.0f, 29.0f), gZtoNZ),	// SCENE_PLAYGROUND_LOGS
	PR(Point(0.0f, 6.0f, 0.0f), Quat(Idt)),	// SCENE_SINE
	PR(Point(0.0f, 6.0f, 0.0f), Quat(Idt)),	// SCENE_TERRAIN
	PR(Point(0.0f, 6.0f, 0.0f), Quat(Idt)),	// SCENE_RACE_TRACK
	PR(Point(0.0f, 6.0f, 0.0f), Quat(Idt)),	// SCENE_STRAIGHT_ROAD
	PR(Point(5.0f, 2.0f, 6.0f), Quat(Idt)),	// SCENE_ARCHIPELAGO
//	PR(Point(-64.0f, 673.0f, 6.0f), Quat(Idt)),	// SCENE_KP
	PR(Point(-6.4f, 67.3f, 0.6f), Quat(Idt)),	// SCENE_KP
	PR(Point(0.0f, 6.0f, 0.0f), Quat(Idt)),	// SCENE_TIRE_TEST
	PR(Point(0.0f, 6.0f, 0.0f), Quat(Idt)),	// SCENE_BOXES
	PR(Point(0.0f, 6.0f, 0.0f), Quat(Idt)),	// SCENE_BOX
	PR(Point(0.0f, 6.0f, 0.0f), Quat(Idt)),	// SCENE_TERRAIN_STREAMING
};

static const float gDefaultSteerVsForwardSpeedData[8]=
{
	0.0f*3.6f,		1.0f,
	5.0f*3.6f,		0.75f,
	30.0f*3.6f,		0.125f,
	120.0f*3.6f,	0.1f,
};

static const float gDefaultSmoothingData[] = {	//				Rise		Fall
												// Keyboard
												/*Accel*/		3.0f,		5.0f,
												/*Brake*/		3.0f,		5.0f,
												/*Handbrake*/	10.0f,		10.0f,
												/*Steering*/	2.5f,		5.0f,
												// Gamepad
												/*Accel*/		6.0f,		10.0f,
												/*Brake*/		6.0f,		10.0f,
												/*Handbrake*/	12.0f,		12.0f,
												/*Steering*/	2.5f,		5.0f,
												};

///////////////////////////////////////////////////////////////////////////////

	struct VehicleSDKData : public Allocateable
	{
		VehicleSDKData(Pint& owner) :
			mOwner				(owner),
			mVehicle			(null),
			mKinematicActors	(null),
			mStreamer			(null),
			mStreamInterface	(null),
			mDeformManager		(null)
		{
		}
		~VehicleSDKData()
		{
			ICE_FREE(mKinematicActors);
			DELETESINGLE(mDeformManager);
			if(mStreamer)
			{
				class RegionRemoval : public RegionIterator
				{
					public:
					inline_	RegionRemoval(StreamInterface& streamer) : mStreamer(streamer)	{}
					virtual	void	ProcessRegion(const StreamRegion& region)
					{
						mStreamer.RemoveRegion(region);
					}
					StreamInterface&	mStreamer;
				};

				mStreamer->Iterate(RegionRemoval(*mStreamInterface));
			}
			DELETESINGLE(mStreamer);
			DELETESINGLE(mStreamInterface);
		}

		inline_	Streamer*				GetStreamer()					{ return mStreamer;			}
		inline_	const Streamer*			GetStreamer()			const	{ return mStreamer;			}
		inline_	StreamInterface*		GetStreamInterface()			{ return mStreamInterface;	}
		inline_	const StreamInterface*	GetStreamInterface()	const	{ return mStreamInterface;	}
		inline_	void					InitStreamer(const TerrainRegionManager::Params& params, udword tiles_per_side, bool deform_terrain)
										{
											//mStreamer = ICE_NEW(Streamer);
											mStreamer = ICE_NEW(Streamer)(params.mWorldSize, tiles_per_side);

											//mStreamInterface = ICE_NEW(MinimalRegionManager)(pint);
											mStreamInterface = ICE_NEW(TerrainRegionManager)(mOwner, NULL, params);

											if(deform_terrain)
												mDeformManager = ICE_NEW(DeformManager)(mOwner);
										}

		Pint&							mOwner;
		PintVehicleHandle				mVehicle;
		PintVehicleData					mData;
		//
		PintActorHandle*				mKinematicActors;
		//protected:
		Streamer*						mStreamer;
		//StreamInterface*				mStreamInterface;
		TerrainRegionManager*			mStreamInterface;
		public:
		DeformManager*					mDeformManager;
	};

static VehicleSDKData* GetVehicleData(Pint& pint)
{
	if(!pint.mUserData)
	{
		VehicleSDKData* VehicleData = ICE_NEW(VehicleSDKData)(pint);
		pint.mUserData = VehicleData;
		return VehicleData;
	}

	return reinterpret_cast<VehicleSDKData*>(pint.mUserData);
}

static void ReleaseVehicleData(Pint& pint)
{
	if(pint.mUserData)
	{
		VehicleSDKData* UserData = GetVehicleData(pint);
		DELETESINGLE(UserData);
		pint.mUserData = null;
	}
}

///////////////////////////////////////////////////////////////////////////////

static inline_ bool CoordMatch(float value, bool pos)
{
	if(pos && value>=0.0f)
		return true;
	else if(!pos && value<=0.0f)
		return true;
	return false;
}

static bool WheelMatch(const PINT_SHAPE_CREATE* wheel, bool posx, bool posz)
{
	const Point& p = wheel->mLocalPos;
	return (CoordMatch(p.x, posx) && CoordMatch(p.z, posz));
}

static const PINT_SHAPE_CREATE* FindChassis(const PINT_OBJECT_CREATE* desc, const PINT_SHAPE_CREATE** wheels)
{
	const PINT_SHAPE_CREATE* Part0 = desc->GetFirstShape();
	const PINT_SHAPE_CREATE* Part1;
	const PINT_SHAPE_CREATE* Part2;
	const PINT_SHAPE_CREATE* Part3;
	const PINT_SHAPE_CREATE* Part4;

	const PtrContainer* ExtraShapes = desc->GetExtraShapes();
	if(ExtraShapes)
	{
		PINT_SHAPE_CREATE** ExtraParts = reinterpret_cast<PINT_SHAPE_CREATE**>(ExtraShapes->GetEntries());
		Part1 = *ExtraParts++;
		Part2 = *ExtraParts++;
		Part3 = *ExtraParts++;
		Part4 = *ExtraParts++;
	}
	else
	{
		Part1 = Part0->_GetNext();
		Part2 = Part1->_GetNext();
		Part3 = Part2->_GetNext();
		Part4 = Part3->_GetNext();
	}

/*	ASSERT(Part0->mType==PINT_SHAPE_CONVEX);
	ASSERT(Part1->mType==PINT_SHAPE_CONVEX);
	ASSERT(Part2->mType==PINT_SHAPE_CONVEX);
	ASSERT(Part3->mType==PINT_SHAPE_CONVEX);
	ASSERT(Part4->mType==PINT_SHAPE_CONVEX);

	const PINT_CONVEX_SHAPE_CREATE*/

	const PINT_SHAPE_CREATE* Chassis;

	float MaxY = MIN_FLOAT;
	const float y0 = Part0->mLocalPos.y;
	const float y1 = Part1->mLocalPos.y;
	const float y2 = Part2->mLocalPos.y;
	const float y3 = Part3->mLocalPos.y;
	const float y4 = Part4->mLocalPos.y;

	if(y0>MaxY)	{ MaxY = y0; Chassis = Part0;	}
	if(y1>MaxY)	{ MaxY = y1; Chassis = Part1;	}
	if(y2>MaxY)	{ MaxY = y2; Chassis = Part2;	}
	if(y3>MaxY)	{ MaxY = y3; Chassis = Part3;	}
	if(y4>MaxY)	{ MaxY = y4; Chassis = Part4;	}

	udword NbWheels = 0;
	if(Part0!=Chassis)	{ wheels[NbWheels++] = Part0;	}
	if(Part1!=Chassis)	{ wheels[NbWheels++] = Part1;	}
	if(Part2!=Chassis)	{ wheels[NbWheels++] = Part2;	}
	if(Part3!=Chassis)	{ wheels[NbWheels++] = Part3;	}
	if(Part4!=Chassis)	{ wheels[NbWheels++] = Part4;	}
	ASSERT(NbWheels==4);

	return Chassis;
}

static const PINT_SHAPE_CREATE* FindWheel(const PINT_SHAPE_CREATE** wheels, bool posx, bool posz)
{
	const PINT_SHAPE_CREATE* Wheel0 = wheels[0];
	const PINT_SHAPE_CREATE* Wheel1 = wheels[1];
	const PINT_SHAPE_CREATE* Wheel2 = wheels[2];
	const PINT_SHAPE_CREATE* Wheel3 = wheels[3];

	if(WheelMatch(Wheel0, posx, posz))
		return Wheel0;

	if(WheelMatch(Wheel1, posx, posz))
		return Wheel1;

	if(WheelMatch(Wheel2, posx, posz))
		return Wheel2;

	if(WheelMatch(Wheel3, posx, posz))
		return Wheel3;

	ASSERT(0);
	return null;
}

/*static const PINT_SHAPE_CREATE* FindWheel(const PINT_SHAPE_CREATE* chassis, bool posx, bool posz)
{
	const PINT_SHAPE_CREATE* Wheel0 = chassis->mNext;
	const PINT_SHAPE_CREATE* Wheel1 = Wheel0->mNext;
	const PINT_SHAPE_CREATE* Wheel2 = Wheel1->mNext;
	const PINT_SHAPE_CREATE* Wheel3 = Wheel2->mNext;

	if(WheelMatch(Wheel0, posx, posz))
		return Wheel0;

	if(WheelMatch(Wheel1, posx, posz))
		return Wheel1;

	if(WheelMatch(Wheel2, posx, posz))
		return Wheel2;

	if(WheelMatch(Wheel3, posx, posz))
		return Wheel3;

	ASSERT(0);
	return null;
}*/

///////////////////////////////////////////////////////////////////////////////

#define	NB_RACE_TRACK_WIDGETS	8

static const char* gDesc_VehicleSDK = "Basic test for the Vehicle SDK. Mainly based on the PhysX API & plugin for now. \
Use the arrow keys to drive the vehicle (the render window must have the focus). Switch between raycast-based and sweep-based vehicles in the \
PhysX UI options panel, for versions that expose/support both. Use the X key (wireframe overlay rendering) to better visualize the ground if needed. \
Use the PageUp & PageDown keys to change the camera. \
This is a work-in-progress, many parameters are hardcoded, not fine-tuned, not exposed to the UI, etc.";

static void gButtonCallback(IceButton& button, void* user_data);

class VehicleSDK : public VehicleInput
{
	public:
							VehicleSDK(VehicleScene default_scene=SCENE_RACE_TRACK) :
								//
								mDefaultScene	(default_scene),
								mLocalTime		(0.0f),
								mNbX			(0),
								mNbY			(0),
								mScaleX			(0.0f),
								mScaleY			(0.0f),
								mKinePositions	(null)
														{
															for(udword i=0;i<4;i++)
															{
																mEditBox_SteeringSpeed[i] = null;
																mEditBox_SteeringValue[i] = null;
															}
															for(udword i=0;i<16;i++)
																mEditBox_SmoothingData[i] = null;

															mCameraStyle = true;
														}
	virtual					~VehicleSDK()				{}
	virtual	const char*		GetName()			const	{ return "VehicleSDK";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_VehicleSDK;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_VEHICLES;	}

			VehicleScene		mDefaultScene;
			float				mLocalTime;
			udword				mNbX;
			udword				mNbY;
			float				mScaleX;
			float				mScaleY;
			Point*				mKinePositions;

			CheckBoxPtr			mCheckBox_DriveVehicle;
			CheckBoxPtr			mCheckBox_UseRenderMeshes;
			CheckBoxPtr			mCheckBox_ImmediateAutoReverse;
			EditBoxPtr			mEditBox_VehicleScale;
			EditBoxPtr			mEditBox_CameraHeight;
			EditBoxPtr			mEditBox_CameraDist;
			EditBoxPtr			mEditBox_CameraSharpnessPos;
			EditBoxPtr			mEditBox_CameraSharpnessRot;
			EditBoxPtr			mEditBox_WheelRadius;
			EditBoxPtr			mEditBox_WheelWidth;
			EditBoxPtr			mEditBox_WheelCoeffX;
			EditBoxPtr			mEditBox_WheelCoeffZ;
			EditBoxPtr			mEditBox_WheelMass;
			EditBoxPtr			mEditBox_WheelMaxBrakeTorqueFront;
			EditBoxPtr			mEditBox_WheelMaxBrakeTorqueRear;
			EditBoxPtr			mEditBox_WheelMaxHandBrakeTorqueFront;
			EditBoxPtr			mEditBox_WheelMaxHandBrakeTorqueRear;
			EditBoxPtr			mEditBox_WheelMaxSteerFront;
			EditBoxPtr			mEditBox_WheelMaxSteerRear;
			EditBoxPtr			mEditBox_AntirollBarStiffness;
			EditBoxPtr			mEditBox_TireFrictionMultiplierFront;
			EditBoxPtr			mEditBox_TireFrictionMultiplierRear;
			EditBoxPtr			mEditBox_Substeps;
			EditBoxPtr			mEditBox_ChassisX;
			EditBoxPtr			mEditBox_ChassisY;
			EditBoxPtr			mEditBox_ChassisZ;
			EditBoxPtr			mEditBox_ChassisMass;
			EditBoxPtr			mEditBox_ChassisMOICoeffY;
			EditBoxPtr			mEditBox_ChassisCMOffsetY;
			EditBoxPtr			mEditBox_ChassisCMOffsetZ;
			EditBoxPtr			mEditBox_ForceApplicationCMOffsetY;
			EditBoxPtr			mEditBox_GroundClearanceCutOffset;
			EditBoxPtr			mEditBox_EngineMOI;
			EditBoxPtr			mEditBox_EnginePeakTorque;
			EditBoxPtr			mEditBox_EngineMaxOmega;
			EditBoxPtr			mEditBox_SuspMaxCompression;
			EditBoxPtr			mEditBox_SuspMaxDroop;
			EditBoxPtr			mEditBox_SuspSpringStrength;
			EditBoxPtr			mEditBox_SuspSpringDamperRate;
			EditBoxPtr			mEditBox_SuspCamberAngleAtRest;
			EditBoxPtr			mEditBox_SuspCamberAngleAtMaxCompr;
			EditBoxPtr			mEditBox_SuspCamberAngleAtMaxDroop;
			EditBoxPtr			mEditBox_GearsSwitchTime;
			EditBoxPtr			mEditBox_ClutchStrength;
			EditBoxPtr			mEditBox_FrontRearSplit;
			EditBoxPtr			mEditBox_FrontLeftRightSplit;
			EditBoxPtr			mEditBox_RearLeftRightSplit;
			EditBoxPtr			mEditBox_CentreBias;
			EditBoxPtr			mEditBox_FrontBias;
			EditBoxPtr			mEditBox_RearBias;
			EditBoxPtr			mEditBox_SteeringSpeed[4];
			EditBoxPtr			mEditBox_SteeringValue[4];
			EditBoxPtr			mEditBox_SmoothingData[4*4];
			EditBoxPtr			mEditBox_AckermannAccuracy;
			EditBoxPtr			mEditBox_ThresholdForwardSpeed;
			ComboBoxPtr			mComboBox_Level;
			ComboBoxPtr			mComboBox_Differential;

			EditBoxPtr			mEditBox_RaceTrackSize;
			EditBoxPtr			mEditBox_RaceTrackAmplitude;
			EditBoxPtr			mEditBox_RaceTrackWidth;
			EditBoxPtr			mEditBox_RaceTrackWallHeight;
			IceWidget*			mRaceTrackWidgets[NB_RACE_TRACK_WIDGETS];

			TerrainStreamingUI	mTerrainStreamingUI;

	virtual	IceTabControl*		InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 500;
		WD.mHeight	= 500+100;
		WD.mLabel	= "VehicleSDK";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 180;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			mCheckBox_DriveVehicle = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Drive vehicle", &UIElems, true, null, null);
			y += YStep;
		}

		const udword ButtonSize = 500-16;
		//y += YStep;
		AddResetButton(UI, 4, y, ButtonSize);

		{
			y += YStep;
			ButtonDesc BD;
			BD.mUserData	= this;
//			BD.mStyle		= ;
			BD.mCallback	= gButtonCallback;
			BD.mID			= 0;
			BD.mParent		= UI;
			BD.mX			= 4;
			BD.mY			= y;
			BD.mWidth		= ButtonSize;
			BD.mHeight		= 20;
			BD.mLabel		= "Save car data";
			IceButton* B = ICE_NEW(IceButton)(BD);
			RegisterUIElement(B);
			B->SetVisible(true);
			y += YStep;

			BD.mID			= 1;
			BD.mY			= y;
			BD.mLabel		= "Load car data";
			B = ICE_NEW(IceButton)(BD);
			RegisterUIElement(B);
			B->SetVisible(true);
		}

		// Tab control
		enum TabIndex
		{
			TAB_SCENE,
			TAB_CAMERA,
			TAB_CHASSIS,
			TAB_WHEEL_TIRES,
			TAB_ENGINE_GEARBOX,
			TAB_SUSPENSION,
			TAB_DIFFERENTIAL,
			TAB_STEERING,
			TAB_CONTROLS,
			TAB_COUNT,
		};
		IceWindow* Tabs[TAB_COUNT];
		{
			TabControlDesc TCD;
			TCD.mParent	= UI;
			TCD.mX		= 4;
			TCD.mY		= 0;
			//TCD.mY		= y + 50;
			TCD.mY		= y + 40;
			TCD.mWidth	= WD.mWidth - 16;
			TCD.mHeight	= 300+100+40;
			IceTabControl* TabControl = ICE_NEW(IceTabControl)(TCD);
			RegisterUIElement(TabControl);

			for(udword i=0;i<TAB_COUNT;i++)
			{
				WindowDesc WD;
				WD.mParent	= UI;
				WD.mX		= 0;
				WD.mY		= 0;
//				WD.mWidth	= WD.mWidth;
				WD.mHeight	= TCD.mHeight;
				WD.mLabel	= "Tab";
				WD.mType	= WINDOW_DIALOG;
				IceWindow* Tab = ICE_NEW(IceWindow)(WD);
				RegisterUIElement(Tab);
				Tab->SetVisible(true);
				Tabs[i] = Tab;
			}
			TabControl->Add(Tabs[TAB_SCENE], "Scene");
			TabControl->Add(Tabs[TAB_CAMERA], "Camera");
			TabControl->Add(Tabs[TAB_CHASSIS], "Chassis");
			TabControl->Add(Tabs[TAB_WHEEL_TIRES], "Wheels/brakes");
			TabControl->Add(Tabs[TAB_ENGINE_GEARBOX], "Engine/gearbox");
			TabControl->Add(Tabs[TAB_SUSPENSION], "Suspension");
			TabControl->Add(Tabs[TAB_DIFFERENTIAL], "Differential");
			TabControl->Add(Tabs[TAB_STEERING], "Steering");
			TabControl->Add(Tabs[TAB_CONTROLS], "Controls");
		}

		// TAB_SCENE
		{
			IceWindow* TabWindow = Tabs[TAB_SCENE];
			y = 4;

			{
				class VehicleLevelComboBox : public IceComboBox
				{
					public:
									VehicleLevelComboBox(const ComboBoxDesc& desc) : IceComboBox(desc){}

					virtual	void	OnComboBoxEvent(ComboBoxEvent event)
					{
						if(event==CBE_SELECTION_CHANGED)
						{
							VehicleSDK* vt = reinterpret_cast<VehicleSDK*>(GetUserData());
							vt->OnComboBoxChange();
						}
					}
				};

				ComboBoxDesc CBBD;
				CBBD.mID		= 0;
				CBBD.mParent	= TabWindow;
				CBBD.mX			= 4;
				CBBD.mY			= y;
				CBBD.mWidth		= 150;
				CBBD.mHeight	= 20;
				CBBD.mLabel		= "Level";
				CBBD.mUserData	= this;
				IceComboBox* CB = ICE_NEW(VehicleLevelComboBox)(CBBD);
				RegisterUIElement(CB);

				CB->Add("Factory");
				CB->Add("Flat");
				CB->Add("Playground");
				CB->Add("Playground (bridge)");
				CB->Add("Playground (logs)");
				CB->Add("Sinus field");
				CB->Add("Terrain");
				CB->Add("Race track");
				CB->Add("Straight road");
				CB->Add("Archipelago");
				CB->Add("Konoko Payne");
				CB->Add("Tire Test");
				CB->Add("Boxes");
				CB->Add("Single box");
				CB->Add("Terrain test");

				CB->Select(mDefaultScene);
				CB->SetVisible(true);
				CB->SetEnabled(true);

				mComboBox_Level = CB;
				y += YStep;
				y += YStep;
			}

			mCheckBox_UseRenderMeshes = helper.CreateCheckBox(TabWindow, 0, 4, y, 400, 20, "Use render meshes", &UIElems, true, null, null);
			mCheckBox_UseRenderMeshes->SetEnabled(false);
			y += YStep;

			y += YStep;

			sdword SavedY = y;
			{
				mRaceTrackWidgets[0] = helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Race track size:", &UIElems);
				mRaceTrackWidgets[1] = mEditBox_RaceTrackSize = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "200.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
				y += YStep;

				mRaceTrackWidgets[2] = helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Race track amplitude:", &UIElems);
				mRaceTrackWidgets[3] = mEditBox_RaceTrackAmplitude = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
				y += YStep;

				mRaceTrackWidgets[4] = helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Race track width:", &UIElems);
				mRaceTrackWidgets[5] = mEditBox_RaceTrackWidth = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "6.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
				y += YStep;

				mRaceTrackWidgets[6] = helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Race track wall height:", &UIElems);
				mRaceTrackWidgets[7] = mEditBox_RaceTrackWallHeight = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "2.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
				y += YStep;
			}

			y = mTerrainStreamingUI.CreateUI(helper, UIElems, TabWindow, SavedY);

			OnComboBoxChange();
		}

		// TAB_CAMERA
		{
			IceWindow* TabWindow = Tabs[TAB_CAMERA];
			y = 4;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Camera height:", &UIElems);
			//mEditBox_CameraHeight = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "3.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_CameraHeight = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Camera distance:", &UIElems);
			mEditBox_CameraDist = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "8.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Camera sharpness (pos):", &UIElems);
			mEditBox_CameraSharpnessPos = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.4", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Camera sharpness (rot):", &UIElems);
			mEditBox_CameraSharpnessRot = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.05", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;
		}

		// TAB_CHASSIS
		{
			IceWindow* TabWindow = Tabs[TAB_CHASSIS];
			y = 4;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Vehicle scale:", &UIElems);
			mEditBox_VehicleScale = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.35", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_VehicleScale->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Width:", &UIElems);
			mEditBox_ChassisX = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Height:", &UIElems);
			mEditBox_ChassisY = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Length:", &UIElems);
			mEditBox_ChassisZ = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Mass:", &UIElems);
			mEditBox_ChassisMass = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1500.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "MOI Y coeff:", &UIElems);
			mEditBox_ChassisMOICoeffY = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "CM Y offset:", &UIElems);
			mEditBox_ChassisCMOffsetY = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "CM Z offset:", &UIElems);
			mEditBox_ChassisCMOffsetZ = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Force app. CM Y offset (~body roll):", &UIElems);
			mEditBox_ForceApplicationCMOffsetY = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.3", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Ground clearance cut offset:", &UIElems);
			mEditBox_GroundClearanceCutOffset= helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.3", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;			
		}

		// TAB_WHEEL_TIRES
		{
			IceWindow* TabWindow = Tabs[TAB_WHEEL_TIRES];
			y = 4;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Radius:", &UIElems);
			mEditBox_WheelRadius = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Width:", &UIElems);
			mEditBox_WheelWidth = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.3", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Coeff X:", &UIElems);
			mEditBox_WheelCoeffX = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.85", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Coeff Z:", &UIElems);
			mEditBox_WheelCoeffZ = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.85", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Mass:", &UIElems);
			mEditBox_WheelMass = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "20.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Max steer front:", &UIElems);
			mEditBox_WheelMaxSteerFront = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.047", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Max steer rear:", &UIElems);
			mEditBox_WheelMaxSteerRear = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Antiroll bar stiffness:", &UIElems);
			mEditBox_AntirollBarStiffness = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Front tire friction multiplier (~grip):", &UIElems);
			mEditBox_TireFrictionMultiplierFront = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.1", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Rear tire friction multiplier (~grip):", &UIElems);
			mEditBox_TireFrictionMultiplierRear = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.1", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Substeps:", &UIElems);
			mEditBox_Substeps = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "8", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;
/*		}

		// TAB_BRAKES
		{
			IceWindow* TabWindow = Tabs[TAB_BRAKES];
			y = 4;*/

			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Max brake torque front:", &UIElems);
			mEditBox_WheelMaxBrakeTorqueFront = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "150.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Max brake torque rear:", &UIElems);
			mEditBox_WheelMaxBrakeTorqueRear = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1500.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Max handbrake torque front:", &UIElems);
			mEditBox_WheelMaxHandBrakeTorqueFront = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Max handbrake torque rear:", &UIElems);
			mEditBox_WheelMaxHandBrakeTorqueRear = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "4000.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;
		}

		// TAB_ENGINE_GEARBOX
		{
			IceWindow* TabWindow = Tabs[TAB_ENGINE_GEARBOX];
			y = 4;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Engine MOI:", &UIElems);
			mEditBox_EngineMOI = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Engine peak torque:", &UIElems);
			mEditBox_EnginePeakTorque = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "500.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Engine max omega:", &UIElems);
			mEditBox_EngineMaxOmega = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "600.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Gears switch time:", &UIElems);
			mEditBox_GearsSwitchTime = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Clutch strength:", &UIElems);
			mEditBox_ClutchStrength = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "10.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			IceEditBox* tmp = helper.CreateEditBox(TabWindow, 0, 4, y+20, 250, 100, "", &UIElems, EDITBOX_TEXT, null);
			tmp->SetMultilineText("Most engine & gearbox params are currently\nhardcoded and not exposed to the UI.");
			tmp->SetReadOnly(true);
		}

		// TAB_SUSPENSION
		{
			IceWindow* TabWindow = Tabs[TAB_SUSPENSION];
			y = 4;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Max compression:", &UIElems);
			mEditBox_SuspMaxCompression = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.3", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Max droop:", &UIElems);
			mEditBox_SuspMaxDroop = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.1", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Spring strength:", &UIElems);
			mEditBox_SuspSpringStrength = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "35000.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Spring damper rate:", &UIElems);
			mEditBox_SuspSpringDamperRate = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "4500.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Camber angle at rest:", &UIElems);
			mEditBox_SuspCamberAngleAtRest = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Camber angle at max compression:", &UIElems);
			mEditBox_SuspCamberAngleAtMaxCompr = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.01", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Camber angle at max droop:", &UIElems);
			mEditBox_SuspCamberAngleAtMaxDroop = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.01", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;
		}

		// TAB_DIFFERENTIAL
		{
			IceWindow* TabWindow = Tabs[TAB_DIFFERENTIAL];
			y = 4;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Differential:", &UIElems);
			{
				ComboBoxDesc CBBD;
				CBBD.mID		= 0;
				CBBD.mParent	= TabWindow;
				CBBD.mX			= 64;
				CBBD.mY			= y;
				CBBD.mWidth		= 150;
				CBBD.mHeight	= 20;
				CBBD.mLabel		= "Differential";
				IceComboBox* CB = ICE_NEW(IceComboBox)(CBBD);
				RegisterUIElement(CB);

				CB->Add("4WD limited slip diff");
				CB->Add("FWD limited slip diff");
				CB->Add("RWD limited slip diff");
				CB->Add("4WD open diff");
				CB->Add("FWD open diff");
				CB->Add("RWD open diff");

				CB->Select(3);
				CB->SetVisible(true);

				mComboBox_Differential = CB;
			}
			y += YStep*2;

			//\note Only applied to DIFF_TYPE_LS_4WD and eDIFF_TYPE_OPEN_4WD
			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Front rear split:", &UIElems);
			mEditBox_FrontRearSplit = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.45", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			//\note Only applied to DIFF_TYPE_LS_4WD and eDIFF_TYPE_OPEN_4WD and eDIFF_TYPE_LS_FRONTWD
			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Front left right split:", &UIElems);
			mEditBox_FrontLeftRightSplit = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			//\note Only applied to DIFF_TYPE_LS_4WD and eDIFF_TYPE_OPEN_4WD and eDIFF_TYPE_LS_REARWD
			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Rear left right split:", &UIElems);
			mEditBox_RearLeftRightSplit = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			//\note Only applied to DIFF_TYPE_LS_4WD
			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Centre bias:", &UIElems);
			mEditBox_CentreBias = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.3", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			//\note Only applied to DIFF_TYPE_LS_4WD and DIFF_TYPE_LS_FRONTWD
			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Front bias:", &UIElems);
			mEditBox_FrontBias = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.3", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			//\note Only applied to DIFF_TYPE_LS_4WD and DIFF_TYPE_LS_REARWD
			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Rear bias:", &UIElems);
			mEditBox_RearBias = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.3", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;
		}

		// TAB_STEERING
		{
			IceWindow* TabWindow = Tabs[TAB_STEERING];
			y = 4;

			PINT_VEHICLE_CREATE desc;
			const sdword LocalLabelWidth = 60;
			for(udword i=0;i<4;i++)
			{
				sdword LocalX = 4;

				helper.CreateLabel(TabWindow, LocalX, y+LabelOffsetY, LocalLabelWidth, 20, _F("Speed %d:", i), &UIElems);
				LocalX += LocalLabelWidth;
				mEditBox_SteeringSpeed[i] = helper.CreateEditBox(TabWindow, 1, LocalX, y, EditBoxWidth, 20, helper.Convert(desc.mSteerVsForwardSpeedData[i*2+0]), &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
				LocalX += EditBoxWidth+20;

				helper.CreateLabel(TabWindow, LocalX, y+LabelOffsetY, LocalLabelWidth, 20, _F("Value %d:", i), &UIElems);
				LocalX += LocalLabelWidth;
				mEditBox_SteeringValue[i] = helper.CreateEditBox(TabWindow, 1, LocalX, y, EditBoxWidth, 20, helper.Convert(desc.mSteerVsForwardSpeedData[i*2+1]), &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
				y += YStep;
			}

			y += YStep;
			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Ackermann accuracy:", &UIElems);
			mEditBox_AckermannAccuracy = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			if(0)
			{
				GraphDesc Desc;
				Desc.mParent			= TabWindow;
				Desc.mX					= 4;
				Desc.mY					= y;
				Desc.mWidth				= 250;
				Desc.mHeight			= 200;
				Desc.mLabel				= "GW";
				Desc.mType				= WINDOW_DIALOG;
				Desc.mGraphStyle		= GRAPH_ANTIALIASED;
				Desc.mEnablePopupMenu	= false;

				GraphWindow* GW = ICE_NEW(GraphWindow)(Desc);
				RegisterUIElement(GW);

#define APP_TO_GW(x)	-(x)

				GW->SetKey(0, 0.0f/120.0f, APP_TO_GW(1.0f));

				Keyframe k;
				k.mX = 5.0f/120.0f;		k.mY = APP_TO_GW(0.75f);	GW->CreateKey(k);
				k.mX = 30.0f/120.0f;	k.mY = APP_TO_GW(0.125f);	GW->CreateKey(k);
				k.mX = 120.0f/120.0f;	k.mY = APP_TO_GW(0.1f);		GW->CreateKey(k);
				GW->SetVisible(true);
			}
		}

		// TAB_CONTROLS
		{
			IceWindow* TabWindow = Tabs[TAB_CONTROLS];
			y = 4;

			y += YStep;

			const char* Names[] = { "Keyboard accel:", "Keyboard brake:", "Keyboard handbrake:", "Keyboard steering:",
									"Gamepad accel:", "Gamepad brake:", "Gamepad handbrake:", "Gamepad steering:" };
			const char** NamePtr = Names;

			const float* SmoothingDataPtr = gDefaultSmoothingData;

			const sdword LocalLabelWidth = 60;

			udword k=0;
			for(udword j=0;j<2;j++)
			{
				for(udword i=0;i<4;i++)
				{
					sdword LocalX = 4;

					helper.CreateLabel(TabWindow, LocalX, y+LabelOffsetY, 120, 20, *NamePtr++, &UIElems);
					LocalX += 130;

					helper.CreateLabel(TabWindow, LocalX, y+LabelOffsetY, LocalLabelWidth, 20, "Rise rate:", &UIElems);
					LocalX += LocalLabelWidth;
					mEditBox_SmoothingData[k++] = helper.CreateEditBox(TabWindow, 1, LocalX, y, EditBoxWidth, 20, _F("%.3f", *SmoothingDataPtr++), &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
					LocalX += EditBoxWidth+20;

					helper.CreateLabel(TabWindow, LocalX, y+LabelOffsetY, LocalLabelWidth, 20, "Fall rate:", &UIElems);
					LocalX += LocalLabelWidth;
					mEditBox_SmoothingData[k++] = helper.CreateEditBox(TabWindow, 1, LocalX, y, EditBoxWidth, 20, _F("%.3f", *SmoothingDataPtr++), &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);

					y += YStep;
				}

				y += YStep;
				y += YStep;
			}
			ASSERT(k==16);

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Threshold forward speed:", &UIElems);
			mEditBox_ThresholdForwardSpeed = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.1", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			mCheckBox_ImmediateAutoReverse = helper.CreateCheckBox(TabWindow, 0, 4, y, 400, 20, "Immediate auto reverse", &UIElems, false, null, null);
			y += YStep;
		}
		return null;
	}

	void	OnComboBoxChange()
	{
		if(mComboBox_Level)
		{
			const int Selected = mComboBox_Level->GetSelectedIndex();
			const bool RaceTrackSelected = Selected == SCENE_RACE_TRACK;
			const bool StreamingSelected = Selected == SCENE_TERRAIN_STREAMING;

			for(udword i=0;i<NB_RACE_TRACK_WIDGETS;i++)
				mRaceTrackWidgets[i]->SetVisible(RaceTrackSelected);

			mTerrainStreamingUI.SetVisible(StreamingSelected);
		}
	}

	void	FillDescFromUI(PINT_VEHICLE_CREATE& VehicleDesc)	const
	{
		VehicleDesc.mStartPose.mPos					= Point(0.0f, 3.0f, 0.0f);
		VehicleDesc.mStartPose.mPos					= Point(0.0f, 6.0f, 0.0f);
		VehicleDesc.mDifferential					= PintVehicleDifferential(mComboBox_Differential->GetSelectedIndex());
		VehicleDesc.mFrontRearSplit					= GetFloat(0.45f,		mEditBox_FrontRearSplit);
		VehicleDesc.mFrontLeftRightSplit			= GetFloat(0.5f,		mEditBox_FrontLeftRightSplit);
		VehicleDesc.mRearLeftRightSplit				= GetFloat(0.5f,		mEditBox_RearLeftRightSplit);
		VehicleDesc.mCentreBias						= GetFloat(1.3f,		mEditBox_CentreBias);
		VehicleDesc.mFrontBias						= GetFloat(1.3f,		mEditBox_FrontBias);
		VehicleDesc.mRearBias						= GetFloat(1.3f,		mEditBox_RearBias);
		VehicleDesc.mChassisMass					= GetFloat(1500.0f,		mEditBox_ChassisMass);
		VehicleDesc.mChassisMOICoeffY				= GetFloat(0.8f,		mEditBox_ChassisMOICoeffY);
		VehicleDesc.mChassisCMOffsetY				= GetFloat(0.65f,		mEditBox_ChassisCMOffsetY);
		VehicleDesc.mChassisCMOffsetZ				= GetFloat(0.25f,		mEditBox_ChassisCMOffsetZ);
		VehicleDesc.mForceApplicationCMOffsetY		= GetFloat(0.3f,		mEditBox_ForceApplicationCMOffsetY);
		VehicleDesc.mWheelMass						= GetFloat(20.0f,		mEditBox_WheelMass);
		VehicleDesc.mWheelMaxBrakeTorqueFront		= GetFloat(150.0f,		mEditBox_WheelMaxBrakeTorqueFront);
		VehicleDesc.mWheelMaxBrakeTorqueRear		= GetFloat(1500.0f,		mEditBox_WheelMaxBrakeTorqueRear);
		VehicleDesc.mWheelMaxHandBrakeTorqueFront	= GetFloat(0.0f,		mEditBox_WheelMaxHandBrakeTorqueFront);
		VehicleDesc.mWheelMaxHandBrakeTorqueRear	= GetFloat(4000.0f,		mEditBox_WheelMaxHandBrakeTorqueRear);
		VehicleDesc.mWheelMaxSteerFront				= GetFloat(PI/3.0f,		mEditBox_WheelMaxSteerFront);
		VehicleDesc.mWheelMaxSteerRear				= GetFloat(0.0f,		mEditBox_WheelMaxSteerRear);
		VehicleDesc.mAntirollBarStiffness			= GetFloat(0.0f,		mEditBox_AntirollBarStiffness);
		VehicleDesc.mWheelSubsteps					= GetInt(32,			mEditBox_Substeps);
		VehicleDesc.mFrontTireFrictionMultiplier	= GetFloat(1.1f,		mEditBox_TireFrictionMultiplierFront);
		VehicleDesc.mRearTireFrictionMultiplier		= GetFloat(1.1f,		mEditBox_TireFrictionMultiplierRear);
		VehicleDesc.mEngineMOI						= GetFloat(1.0f,		mEditBox_EngineMOI);
		VehicleDesc.mEnginePeakTorque				= GetFloat(1000.0f,		mEditBox_EnginePeakTorque);
		VehicleDesc.mEngineMaxOmega					= GetFloat(1000.0f,		mEditBox_EngineMaxOmega);
		VehicleDesc.mSuspMaxCompression				= GetFloat(0.3f,		mEditBox_SuspMaxCompression);
		VehicleDesc.mSuspMaxDroop					= GetFloat(0.1f,		mEditBox_SuspMaxDroop);
		VehicleDesc.mSuspSpringStrength				= GetFloat(35000.0f,	mEditBox_SuspSpringStrength);
		VehicleDesc.mSuspSpringDamperRate			= GetFloat(4500.0f,		mEditBox_SuspSpringDamperRate);
		VehicleDesc.mSuspCamberAngleAtRest			= GetFloat(0.0f,		mEditBox_SuspCamberAngleAtRest);
		VehicleDesc.mSuspCamberAngleAtMaxCompr		= GetFloat(0.01f,		mEditBox_SuspCamberAngleAtMaxCompr);
		VehicleDesc.mSuspCamberAngleAtMaxDroop		= GetFloat(0.01f,		mEditBox_SuspCamberAngleAtMaxDroop);
		VehicleDesc.mGearsSwitchTime				= GetFloat(0.5f,		mEditBox_GearsSwitchTime);
		VehicleDesc.mClutchStrength					= GetFloat(10.0f,		mEditBox_ClutchStrength);
		VehicleDesc.mAckermannAccuracy				= GetFloat(1.0f,		mEditBox_AckermannAccuracy);
		VehicleDesc.mThresholdForwardSpeed			= GetFloat(0.1f,		mEditBox_ThresholdForwardSpeed);
		VehicleDesc.mImmediateAutoReverse			= mCheckBox_ImmediateAutoReverse ? mCheckBox_ImmediateAutoReverse->IsChecked() : false;

		const float kph_to_mps = 1.0f / 3.6f;
		for(udword i=0;i<4;i++)
		{
			VehicleDesc.mSteerVsForwardSpeedData[i*2+0] = GetFloat(VehicleDesc.mSteerVsForwardSpeedData[i*2+0], mEditBox_SteeringSpeed[i]) * kph_to_mps;
			VehicleDesc.mSteerVsForwardSpeedData[i*2+1] = GetFloat(VehicleDesc.mSteerVsForwardSpeedData[i*2+1], mEditBox_SteeringValue[i]);
		}

		for(udword i=0;i<16;i++)
			VehicleDesc.mSmoothingData[i] = GetFloat(VehicleDesc.mSmoothingData[i], mEditBox_SmoothingData[i]);

		///

		if(mComboBox_Level)
		{
			const udword Index = mComboBox_Level->GetSelectedIndex();
			VehicleDesc.mStartPose = gStartPoses[Index];
		}
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleInput::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.89f, 3.40f, 4.62f), Point(-0.71f, -0.31f, -0.63f));

		const udword Index = mComboBox_Level->GetSelectedIndex();
		SetDefEnv(desc, gCreateDefaultEnv[Index]);
	}

	virtual void	Close(Pint& pint)
	{
		DELETEARRAY(mKinePositions);
		ReleaseVehicleData(pint);
		VehicleInput::Close(pint);
	}

	virtual bool	CommonSetup()
	{
		SetCameraMode(1);

		const udword Index = mComboBox_Level->GetSelectedIndex();
		if(Index==SCENE_FACTORY)
		{
			mPlanarMeshHelper.Generate(16, 1.0f);
		}
		else if(Index==SCENE_FLAT)
		{
			mPlanarMeshHelper.Generate(200, 0.5f);

			{
				const float ex = 10.0f;
				const float ez = 10.0f;
				const float ey = 4.0f;
				const float Angle = 18.0f * DEGTORAD;
				const float Op = ey/tanf(Angle);
				const udword NbVerts = 6;
				Point Verts[NbVerts];
				Verts[0] = Point(-ex, ey, -ez);
				Verts[1] = Point(-ex, ey, ez);
				Verts[2] = Point(ex, ey, ez);
				Verts[3] = Point(ex, ey, -ez);
				Verts[4] = Point(ex+Op, 0.0f, ez);
				Verts[5] = Point(ex+Op, 0.0f, -ez);

				const udword NbTris = 4;
				IndexedTriangle Tris[NbTris];
				Tris[0].mRef[0] = 0;
				Tris[0].mRef[1] = 1;
				Tris[0].mRef[2] = 2;
				Tris[1].mRef[0] = 0;
				Tris[1].mRef[1] = 2;
				Tris[1].mRef[2] = 3;
				Tris[2].mRef[0] = 3;
				Tris[2].mRef[1] = 2;
				Tris[2].mRef[2] = 4;
				Tris[3].mRef[0] = 3;
				Tris[3].mRef[1] = 4;
				Tris[3].mRef[2] = 5;

				IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
				IS->Init(NbTris, NbVerts, Verts, Tris);

				PR Pose;
				Pose.mPos = Point(0.0f, 0.0f, 0.0f);
				Matrix3x3 M;
				M.RotY(-HALFPI);
				Pose.mRot = M;

				RegisterSurface(IS, null, &Pose, &mHighFrictionMaterial);
			}
		}
		else if(Index==SCENE_PLAYGROUND || Index==SCENE_PLAYGROUND_BRIDGE || Index==SCENE_PLAYGROUND_LOGS)
		{
			mPlanarMeshHelper.Generate(100, 1.0f);

			{
				const float LoopRadius = 20.0f;
				LoopTheLoop LTL;
				LTL.Generate(LoopRadius, 15.0f, 64, false);

				IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
				IS->Init(LTL.mNbTris, LTL.mNbVerts, LTL.mVerts, reinterpret_cast<const IndexedTriangle*>(LTL.mIndices));

				PR Pose(Idt);
				Pose.mPos = Point(-200.0f, LoopRadius, 200.0f);

				RegisterSurface(IS, null, &Pose, &mHighFrictionMaterial);
			}
		}
		else if(Index==SCENE_SINE)
		{
			mPlanarMeshHelper.Generate(200, 0.5f, 0.5f, 10.0f);
		}
		else if(Index==SCENE_TERRAIN)
		{
			const Point Scale(1.0f, 0.2f, 1.0f);
			LoadMeshesFromFile_(*this, "Terrain.bin", &Scale, false, 0);
		}
		else if(Index==SCENE_ARCHIPELAGO)
		{
			const Point Scale(1.0f, 1.0f, 1.0f);
			LoadMeshesFromFile_(*this, "Archipelago.bin", &Scale, false, 0);
		}		
		else if(Index==SCENE_KP)
		{
			const Point Scale(0.1f, 0.1f, 0.1f);
			LoadMeshesFromFile_(*this, "KP.bin", &Scale, false, 0);
		}		
		else if(Index==SCENE_RACE_TRACK)
		{
			IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);

			RaceTrack RT;

			const float RaceTrackSize = GetFloat(2000.0f, mEditBox_RaceTrackSize);
			const float RaceTrackAmplitude = GetFloat(20.0f, mEditBox_RaceTrackAmplitude);
			const float RaceTrackWidth = GetFloat(15.0f, mEditBox_RaceTrackWidth);
			const float RaceTrackWallHeight = GetFloat(2.0f, mEditBox_RaceTrackWallHeight);
			RT.Generate(RaceTrackSize, RaceTrackAmplitude, RaceTrackWallHeight, RaceTrackWidth);

			IS->Init(RT.mNbTris, RT.mNbVerts, RT.mVerts, (const IndexedTriangle*)RT.mIndices);

			Point Center;
			IS->GetFace(0)->Center(IS->GetVerts(), Center);

			IS->Translate(-Center);

			RegisterSurface(IS, null, null, &mHighFrictionMaterial);
		}
		else if(Index==SCENE_STRAIGHT_ROAD)
		{
			mPlanarMeshHelper.Generate(5, 300, Point(0.1f, 1.0f, 0.5f));
		}
		else if(Index==SCENE_TIRE_TEST)
		{
		}
		else if(Index==SCENE_BOXES)
		{
		}
		else if(Index==SCENE_BOX)
		{
		}
		else if(Index==SCENE_TERRAIN_STREAMING)
		{
			//mStreamer = ICE_NEW(Streamer);
		}
/*		else if(Index==5)
		{
			//mPlanarMeshHelper.Generate(5, 300, Point(0.1f, 1.0f, 0.5f));
		}*/
		return VehicleInput::CommonSetup();
	}

	bool CreateLevelMeshes(Pint& pint, const PintCaps& caps)
	{
		CreateMeshesFromRegisteredSurfaces(pint, caps, &mHighFrictionMaterial);

		const udword Index = mComboBox_Level->GetSelectedIndex();
		if(Index==SCENE_FACTORY || Index==SCENE_FLAT || Index==SCENE_PLAYGROUND || Index==SCENE_PLAYGROUND_BRIDGE || Index==SCENE_PLAYGROUND_LOGS || Index==SCENE_SINE || Index==SCENE_STRAIGHT_ROAD)
		{
			const float Altitude = 0.0f;
			mPlanarMeshHelper.CreatePlanarMesh(pint, Altitude, &mHighFrictionMaterial);
		}

		if(Index==SCENE_PLAYGROUND || Index==SCENE_PLAYGROUND_BRIDGE || Index==SCENE_PLAYGROUND_LOGS)
		{
			if(0)
			{
				PINT_BOX_CREATE BoxDesc(10.0f, 0.1f, 10.0f);
				BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mPosition	= Point(10.0f, 0.1f, 10.0f);
				ObjectDesc.mMass		= 1000.0f;
				CreatePintObject(pint, ObjectDesc);
			}

			const float Spacing = 10.0f;
		//	const float Size = 0.3f;
			const float Size = 0.2f;
//			const float Size = 0.5f;
		//	const float Size = 1.0f;
			GenerateGroundObstacles(pint, Spacing, Size, true);

			//

			{
				BRIDGES_CREATE Create;
				Create.mNbBridges		= 1;
				Create.mNbPlanks		= 20;
				Create.mPlankMass		= 1000.0f;
				Create.mPlankExtents	= Point(1.0f, 0.3f, 3.0f);
				//Create.mOrigin			= Point(40.0f, 5.0f, 40.0f);
				//Create.mOrigin			= Point(40.0f, 10.0f, 40.0f);
				Create.mOrigin			= Point(-100.0f, 5.0f, 0.0f);
				//Create.mInertiaCoeff	= 10.0f;
				//Create.mMultiplier		= 16;
				Create.mIncludeSupport	= true;
				bool status = CreateBridges(pint, caps, Create);

				{
					// ### TODO: find a cleaner way
					const PintDisabledGroups DG[] = { PintDisabledGroups(1, 2), PintDisabledGroups(1, 31), PintDisabledGroups(2, 31) };
					pint.SetDisabledGroups(3, DG);
				}
			}

			for(udword j=0;j<2;j++)
			{
				const float Coeff = 1.0f/float(j+1);

				PINT_CAPSULE_CREATE CapsuleDesc(0.4f*Coeff, 1.5f);
				CapsuleDesc.mRenderer	= CreateCapsuleRenderer(CapsuleDesc.mRadius, CapsuleDesc.mHalfHeight*2.0f);

				PINT_OBJECT_CREATE ObjectDesc(&CapsuleDesc);
				ObjectDesc.mRotation	= ShortestRotation(Point(0.0f, 1.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
				ObjectDesc.mMass		= 1000.0f;

				const float x = 20.0f + float(j)*CapsuleDesc.mHalfHeight*3.0f;
				for(udword i=0;i<8;i++)
				{
					ObjectDesc.mPosition	= Point(x, CapsuleDesc.mRadius, 20.0f + float(i)*CapsuleDesc.mRadius*2.1f);
					CreatePintObject(pint, ObjectDesc);
				}
			}
		}
		else if(Index==SCENE_TIRE_TEST)
		{
			if(!caps.mSupportRigidBodySimulation || !caps.mSupportKinematics)
				return false;

			const Point mBoxExtents(1.5f, 0.2f, 1.5f);
			//const Point mBoxExtents(10.0f, 0.2f, 10.0f);
			mNbX = 64;
			mNbY = 64;
			mScaleX = mBoxExtents.x * 2.0f;
			mScaleY = mBoxExtents.z * 2.0f;

			//const Point mBoxExtents(3.0f, 0.2f, 3.0f);
			PintShapeRenderer* mBoxRenderer = CreateBoxRenderer(mBoxExtents);
			mKinePositions = ICE_NEW(Point)[mNbX*mNbY];

			PINT_BOX_CREATE BoxDesc(mBoxExtents);
			BoxDesc.mRenderer	= mBoxRenderer;

			AABB Box;
			Box.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), mBoxExtents);

			Point Pts[8];
			Box.ComputePoints(Pts);

			// Mesh version a lot slower!
/*			PINT_MESH_CREATE MeshDesc;
			MeshDesc.mSurface.mNbVerts	= 8;
			MeshDesc.mSurface.mVerts	= Pts;
			MeshDesc.mSurface.mNbFaces	= 12;
			MeshDesc.mSurface.mDFaces	= Box.GetTriangles();
			MeshDesc.mRenderer			= CreateMeshRenderer(MeshDesc.mSurface);*/

			// Mesh version with box renderer is fast as well, so it's the mesh rendering?
			PINT_MESH_CREATE MeshDesc;
			MeshDesc.SetSurfaceData(8, Pts, 12, Box.GetTriangles(), null);
			MeshDesc.mRenderer = mBoxRenderer;

			PintActorHandle* Handles = (PintActorHandle*)ICE_ALLOC(sizeof(PintActorHandle)*mNbX*mNbY);

			VehicleSDKData* UserData = GetVehicleData(pint);
			ASSERT(UserData);
			UserData->mKinematicActors = Handles;

			udword Index = 0;
			for(udword y=0;y<mNbY;y++)
			{
				for(udword x=0;x<mNbX;x++)
				{
					const float xf = (float(x)-float(mNbX)*0.5f)*mScaleX;
					const float yf = (float(y)-float(mNbY)*0.5f)*mScaleY;

					PINT_OBJECT_CREATE ObjectDesc;
					ObjectDesc.SetShape(&BoxDesc);
						ObjectDesc.SetShape(&MeshDesc);
					ObjectDesc.mPosition	= Point(xf, 0.2f, yf);
					ObjectDesc.mMass		= 1.0f;
					ObjectDesc.mKinematic	= true;
	//				ObjectDesc.mMass		= 0.0f;
					PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
					ASSERT(Handle);
					Handles[Index++] = Handle;
				}
			}		
		}
		else if(Index==SCENE_BOXES)
		{
			if(!caps.mSupportRigidBodySimulation)
				return false;

		const float BoxHeight = 4.0f;
		const float BoxSide = 1.0f;
		const float BoxDepth = 20.0f;
		CreateBoxContainer(pint, BoxHeight, BoxSide, BoxDepth);

			const float BoxSize = 0.1f;
			PINT_BOX_CREATE BoxDesc(BoxSize, BoxSize, BoxSize);
			BoxDesc.mRenderer = CreateBoxRenderer(BoxDesc.mExtents);

			BasicRandom Rnd(42);

			float yy = BoxSize*2.0f;
			const float LayerInc = BoxSize*2.0f;
			const udword NbLayers = 1;
			const udword NbX = 64;
			const udword NbY = 64;

			for(udword k=0;k<NbLayers;k++)
			{
				for(udword y=0;y<NbY;y++)
				{
					const float CoeffY = 2.0f * ((float(y)/float(NbY-1)) - 0.5f);
					for(udword x=0;x<NbX;x++)
					{
						const float CoeffX = 2.0f * ((float(x)/float(NbX-1)) - 0.5f);

						const float RandomX = Rnd.RandomFloat();
						const float RandomY = Rnd.RandomFloat();

						PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
						if(0)
						{
							ObjectDesc.mCollisionGroup	= 1;
							//ObjectDesc.mMass		= 100.0f;
							ObjectDesc.mMass		= 1.0f;
						}
						else
							ObjectDesc.mMass		= 0.0f;
						ObjectDesc.mPosition.x	= RandomX*2.0f;
						ObjectDesc.mPosition.y	= yy;
						ObjectDesc.mPosition.z	= RandomY*2.0f;
					ObjectDesc.mPosition.x	= RandomX + CoeffX * (BoxDepth - BoxSize - BoxSide*2.0f);
					ObjectDesc.mPosition.y	= yy;
					ObjectDesc.mPosition.z	= RandomY + CoeffY * (BoxDepth - BoxSize - BoxSide*2.0f);

						UnitRandomQuat(ObjectDesc.mRotation, Rnd);

						CreatePintObject(pint, ObjectDesc);
					}
				}
				yy += LayerInc * 2.0f;
			}
		}
		else if(Index==SCENE_BOX)
		{
			if(!caps.mSupportRigidBodySimulation)
				return false;

			const float BoxSize = 10.0f;
			PINT_BOX_CREATE BoxDesc(BoxSize, 0.5f, BoxSize);
			BoxDesc.mRenderer = CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mPosition.x	= 0.0f;
			ObjectDesc.mPosition.y	= 0.0f;
			ObjectDesc.mPosition.z	= 0.0f;
			CreatePintObject(pint, ObjectDesc);
		}
		else if(Index==SCENE_TERRAIN_STREAMING)
		{
			VehicleSDKData* UserData = GetVehicleData(pint);
			ASSERT(UserData);

			const TerrainRegionManager::Params Params(
				mTerrainStreamingUI.GetNbAddedRegionsPerFrame(),
				mTerrainStreamingUI.GetVerticesPerSide(),
				mTerrainStreamingUI.GetWorldSize(),
				mTerrainStreamingUI.CreateAllInitialTiles()
				);

			UserData->InitStreamer(	Params,
									mTerrainStreamingUI.GetTilesPerSide(),
									mTerrainStreamingUI.DeformTerrain());
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		Pint_Vehicle* API = pint.GetVehicleAPI();
		if(!caps.mSupportVehicles || !API)
			return false;

//		CreateMeshesFromRegisteredSurfaces(pint, caps, *this, &mHighFrictionMaterial);

		// We currently have this hardcoded in the PhysX plugin so we must define the car accordingly:
		//const PxVec3 forward(0.0f, 0.0f, 1.0f);

		const float WheelRadius = GetFloat(0.5f, mEditBox_WheelRadius);
		const float WheelWidth = GetFloat(0.3f, mEditBox_WheelWidth);
		const float CoeffX = GetFloat(0.85f, mEditBox_WheelCoeffX);
		const float CoeffZ = GetFloat(0.85f, mEditBox_WheelCoeffZ);

		// Forward is along Z so the chassis must be along Z as well
		Point ChassisExtents;
		ChassisExtents.x = GetFloat(1.0f, mEditBox_ChassisX);
		ChassisExtents.y = GetFloat(0.5f, mEditBox_ChassisY);
		ChassisExtents.z = GetFloat(1.5f, mEditBox_ChassisZ);

		const udword NbPts = 60;
//		const udword NbPts = 10;
		// Forward is along Z so the cylinder axis must be along X
		const CylinderMesh Cylinder(NbPts, WheelRadius, WheelWidth*0.5f, ORIENTATION_YZ);
		const udword TotalNbVerts = Cylinder.mNbVerts;
		const Point* Verts = Cylinder.mVerts;

		PINT_VEHICLE_CREATE VehicleDesc;
		FillDescFromUI(VehicleDesc);

		if(0)
		{
			Matrix3x3 Rot;
//			Rot.RotZ(HALFPI);
			Rot.RotZ(60.0f*DEGTORAD);
			Rot.RotZ(70.0f*DEGTORAD);
			VehicleDesc.mStartPose.mRot = Rot;
		}

		PINT_CONVEX_CREATE WheelDesc(TotalNbVerts, Verts);
		WheelDesc.mRenderer	= CreateConvexRenderer(TotalNbVerts, Verts);
		VehicleDesc.mWheels[0] = &WheelDesc;
//		VehicleDesc.mWheels.mNbVerts	= TotalNbVerts;
//		VehicleDesc.mWheels.mVerts		= Verts;
//		VehicleDesc.mWheels.mRenderer	= CreateConvexRenderer(VehicleDesc.mWheels.mNbVerts, VehicleDesc.mWheels.mVerts);

		// We must create the wheels in this order:
		// eFRONT_LEFT=0,
		// eFRONT_RIGHT,
		// eREAR_LEFT,
		// eREAR_RIGHT
		// Forward is along Z so front is +z, left is +x

		VehicleDesc.mWheelOffset[0] = Point( ChassisExtents.x*CoeffX, -WheelRadius,  ChassisExtents.z*CoeffZ);
		VehicleDesc.mWheelOffset[1] = Point(-ChassisExtents.x*CoeffX, -WheelRadius,  ChassisExtents.z*CoeffZ);
		VehicleDesc.mWheelOffset[2] = Point( ChassisExtents.x*CoeffX, -WheelRadius, -ChassisExtents.z*CoeffZ);
		VehicleDesc.mWheelOffset[3] = Point(-ChassisExtents.x*CoeffX, -WheelRadius, -ChassisExtents.z*CoeffZ);

		AABB ChassisBox;
		ChassisBox.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), ChassisExtents);
		Point ChassisPts[8];
		ChassisBox.ComputePoints(ChassisPts);

		PINT_CONVEX_CREATE ChassisDesc(8, ChassisPts);
		ChassisDesc.mRenderer	= CreateConvexRenderer(ChassisDesc.mNbVerts, ChassisDesc.mVerts);
		VehicleDesc.mChassis	= &ChassisDesc;

		VehicleSDKData* VehicleData = GetVehicleData(pint);

		VehicleData->mVehicle = API->CreateVehicle(VehicleData->mData, VehicleDesc);

		/*if(0)
		{
			VehicleDesc.mStartPose.mPos.z += 10.0f;
			const PintVehicleHandle VehicleHandle2 = API->CreateVehicle(VD, VehicleDesc);
		}*/

		if(0)
		{
			PINT_BOX_CREATE BoxDesc(1.0f, 1.0f, 1.0f);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mPosition	= VehicleDesc.mStartPose.mPos;
			ObjectDesc.mPosition.y	+= ChassisExtents.y + BoxDesc.mExtents.y;
			ObjectDesc.mMass		= 10000.0f;
			PintActorHandle h = CreatePintObject(pint, ObjectDesc);

			if(1)
			{
				PINT_FIXED_JOINT_CREATE Desc;
				Desc.mObject0		= VehicleData->mData.mChassisActor;
				Desc.mObject1		= h;
				Desc.mLocalPivot0	= Point(0.0f, ChassisExtents.y, 0.0f);
				Desc.mLocalPivot1	= Point(0.0f, -BoxDesc.mExtents.y, 0.0f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}

		return CreateLevelMeshes(pint, caps);
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		Pint_Vehicle* API = pint.GetVehicleAPI();
		if(!API)
			return y;

		const VehicleSDKData* UserData = GetVehicleData(pint);

		const PintVehicleHandle VehicleHandle = UserData->mVehicle;

		PintVehicleInfo Info;
		if(VehicleHandle && API->GetVehicleInfo(VehicleHandle, Info))
		{
			// FWD speed is a regular linear velocity in meters/seconds. We need to:
			// - multiply by 3600 (seconds) to get meters/hours
			// - divide by 1000 (kms) to get kms/hours
//			renderer.print(0.0f, y, text_scale, _F("FWD speed: %f\n", Info.mForwardSpeed));
			renderer.print(0.0f, y, text_scale, _F("kph: %f\n", Info.mForwardSpeed*3.6f));
			y -= text_scale;

			renderer.print(0.0f, y, text_scale, _F("Gear: %d\n", Info.mCurrentGear));
			y -= text_scale;

			renderer.print(0.0f, y, text_scale, _F("Revs: %f\n", Info.mRevs));
			y -= text_scale;

			if(gTrySound)
			{
				void SetFreq(int f);
				const float BaseFreq = 22000.0f;
				const float MaxFreq = 44100.0f*2.0f;
				const float MaxRevs = 1000.0f;
				const float Coeff = Info.mRevs/MaxRevs;
				const float f = BaseFreq + (MaxFreq-BaseFreq)*Coeff*Coeff*Coeff;
				SetFreq(int(f));
			}

			// Convert from radians per second to revolutions per minute. We need to:
			// - multiply by 60 (seconds) to get radians per minute
			// - divide by PI*2 (radians) to get revs per minute
			renderer.print(0.0f, y, text_scale, _F("rpm: %f\n", Info.mRevs * 60.0f / TWOPI));
			y -= text_scale;

			if(0)
			{
				const Point LinVel = pint.GetLinearVelocity(UserData->mData.mChassisActor);
				renderer.print(0.0f, y, text_scale, _F("LinVel: %f %f %f\n", LinVel.x, LinVel.y, LinVel.z));
				y -= text_scale;
			}
		}
		return y;
	}

	virtual	void	CommonUpdate(float dt)
	{
		VehicleInput::CommonUpdate(dt);

		const bool AnimateTiles = true;
		if(AnimateTiles)
			mLocalTime += dt;

		const float Coeff = sinf(mLocalTime);
//		const float Amplitude = Coeff>0.0f ? Coeff*2.0f : 0.0f;
		const float Amplitude = Coeff*2.0f;

		// Compute new kinematic poses - the same for all engines
		//if(mUpdateKinematics)
		if(mKinePositions)
		{
			udword Index = 0;
			const float Coeff = 0.2f;
			for(udword y=0;y<mNbY;y++)
			{
				for(udword x=0;x<mNbX;x++)
				{
					const float xf = (float(x)-float(mNbX)*0.5f)*mScaleX;
					const float yf = (float(y)-float(mNbY)*0.5f)*mScaleY;

					const float h = sinf(mLocalTime*2.0f + float(x)*Coeff + float(y)*Coeff)*Amplitude;
					mKinePositions[Index++] = Point(xf, h + 0.2f, yf);
				}
			}
		}
	}

	udword	UpdateKinematics(Pint& pint, float dt)
	{
		//if(mUpdateKinematics)
		if(mKinePositions)
		{
			const VehicleSDKData* UserData = GetVehicleData(pint);
			ASSERT(UserData);
			const PintActorHandle* Handles = UserData->mKinematicActors;
			if(Handles)
			{
				const Point* Positions = mKinePositions;
				udword Nb = mNbX*mNbY;
				while(Nb--)
				{
					const PintActorHandle h = *Handles++;
					const Point& Pos = *Positions++;
					if(h)
						pint.SetKinematicPose(h, Pos);
				}
			}
		}
		return 0;
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		Pint_Vehicle* API = pint.GetVehicleAPI();
		if(!API)
			return 0;

		UpdateKinematics(pint, dt);

		VehicleSDKData* UserData = GetVehicleData(pint);
		if(!UserData)
			return 0;

		const PintVehicleHandle VehicleHandle = UserData->mVehicle;
		if(!VehicleHandle)
			return 0;

		if(mCheckBox_DriveVehicle && mCheckBox_DriveVehicle->IsChecked())
		{
			API->SetVehicleInput(VehicleHandle, mInput);

			// Camera
			mCamera.mUpOffset = GetFloat(3.0f, mEditBox_CameraHeight);
			mCamera.mDistToTarget = GetFloat(8.0f, mEditBox_CameraDist);
			mCamera.mSharpnessPos = GetFloat(0.2f, mEditBox_CameraSharpnessPos);
			mCamera.mSharpnessRot = GetFloat(0.2f, mEditBox_CameraSharpnessRot);

			UpdateCamera(pint, UserData->mData.mChassisActor);
			if(0 && UserData->mData.mChassisActor)
			{
				// Won't work well more more than 1 engine at a time of course
				const PR Pose = pint.GetWorldTransform(UserData->mData.mChassisActor);
		//		printf("%f\n", Pose.mPos.y);
		//		const Point CamPos = GetCameraPos();
		//		const Point Dir = (Pose.mPos - CamPos).Normalize();
		//		SetCamera(CamPos, Dir);

				const Matrix3x3 M(Pose.mRot);
	//			Point D = M[0];
				Point D = M[2];
				D.y = 0.0f;
				D.Normalize();

	float	mCameraUpOffset			= 3.0f;
	float	mCameraDistToTarget		= 8.0f;

				const Point Target = Pose.mPos;
				const Point CamPos = Pose.mPos + Point(0.0f, mCameraUpOffset, 0.0f) - D*mCameraDistToTarget;
		//		const Point CamPos = Pose.mPos + Point(0.0f, mCameraUpOffset, 0.0f) - Point(1.0f, 0.0f, 0.0f)*mCameraDistToTarget;

				const Point Dir = (Target - CamPos).Normalize();

	//			const float Sharpness = 0.02f;
				const float Sharpness = 0.2f;
				static Point FilteredPos(0.0f, 0.0f, 0.0f);
				static Point FilteredDir(0.0f, 0.0f, 0.0f);
				FeedbackFilter(CamPos.x, FilteredPos.x, Sharpness);
				FeedbackFilter(CamPos.y, FilteredPos.y, Sharpness);
				FeedbackFilter(CamPos.z, FilteredPos.z, Sharpness);
				FeedbackFilter(Dir.x, FilteredDir.x, Sharpness);
				FeedbackFilter(Dir.y, FilteredDir.y, Sharpness);
				FeedbackFilter(Dir.z, FilteredDir.z, Sharpness);

				Point Tmp = FilteredDir;
				Tmp.Normalize();

		//			SetCamera(CamPos, Dir);
				SetCamera(FilteredPos, Tmp);
			}
		}

		if(UserData->GetStreamer())
		{
			// ### is this done before or after updating the vehicle?

			PintActorHandle VehicleActor = API->GetVehicleActor(VehicleHandle);
			if(VehicleActor)
			{
				Pint_Actor* ActorAPI = pint.GetActorAPI();
				if(ActorAPI)
				{
					AABB VehicleBounds;
					if(ActorAPI->GetWorldBounds(VehicleActor, VehicleBounds))
						UserData->GetStreamer()->Update(VehicleBounds, *UserData->GetStreamInterface());
				}

				if(UserData->mDeformManager)
				{	 
					const float OverlapRadius = mTerrainStreamingUI.GetWheelOverlapSphereRadius();
					UserData->mDeformManager->Update(UserData->mData, *UserData->mStreamInterface, OverlapRadius, dt);
				}
			}
		}

		return VehicleInput::Update(pint, dt);
	}

	virtual	void	DrawDebugInfo(Pint& pint, PintRender& render)
	{
		VehicleSDKData* UserData = GetVehicleData(pint);
		if(UserData)
		{
			if(0 && UserData->mDeformManager)
			{
				udword NbTris = UserData->mDeformManager->mTouchedTris.GetNbVertices()/3;
				printf("%d tris\n", NbTris);
				const Point* V = UserData->mDeformManager->mTouchedTris.GetVertices();
				for(udword i=0;i<NbTris;i++)
				{
					const Point& v0 = *V++;
					const Point& v1 = *V++;
					const Point& v2 = *V++;
					render.DrawTriangle(v0, v1, v2, Point(0.0f, 1.0f, 0.0f));
				}
			}

			if(UserData->GetStreamer() && UserData->GetStreamInterface())
			{
				Streamer* S = UserData->GetStreamer();
				StreamInterface* SI = UserData->GetStreamInterface();

				if(mTerrainStreamingUI.DrawWheelOverlapSpheres())
				{
					Pint_Shape* ShapeAPI = pint.GetShapeAPI();
					if(ShapeAPI)
					{
						const float OverlapRadius = mTerrainStreamingUI.GetWheelOverlapSphereRadius();
						for(udword i=0;i<4;i++)
						{
							const PR WheelPose = ShapeAPI->GetWorldTransform(UserData->mData.mChassisActor, UserData->mData.mWheelShapes[i]);
							render.DrawSphere(OverlapRadius, WheelPose);
						}
					}
				}

				S->mDebugDrawStreamingBounds = mTerrainStreamingUI.DrawStreamingRegions();
				static_cast<TerrainRegionManager*>(SI)->mDebugDrawVertexNormals = mTerrainStreamingUI.DrawVertexNormals();

				// ### TODO: don't call if nothing to do
				S->RenderDebug(render, *SI);
			}
		}

		VehicleInput::DrawDebugInfo(pint, render);

		if(mComboBox_Level)
		{
			const udword Index = mComboBox_Level->GetSelectedIndex();
			/*if(Index==SCENE_TERRAIN_STREAMING)
			{
				VehicleInput::DrawDebugInfo(pint, render);

				Pint_Vehicle* VehicleAPI = pint.GetVehicleAPI();
				if(!VehicleAPI)
					return;

				Pint_Actor* ActorAPI = pint.GetActorAPI();
				if(!ActorAPI)
					return;

				const VehicleSDKData* UserData = GetVehicleData(pint);
				if(!UserData)
					return;

				const PintVehicleHandle VehicleHandle = UserData->mVehicle;
				PintActorHandle VehicleActor = VehicleAPI->GetVehicleActor(VehicleHandle);
				if(!VehicleActor)
					return;

				AABB VehicleBounds;
				if(!ActorAPI->GetWorldBounds(VehicleActor, VehicleBounds))
					return;

				render.DrawWireframeAABB(1, &VehicleBounds, Point(1.0f, 0.0f, 0.0f));

				AABB StreamingBounds;
				Point Center;
				VehicleBounds.GetCenter(Center);

				const float WorldExtent = 100.0f;
				StreamingBounds.SetCenterExtents(Center, Point(WorldExtent, 1.0f, WorldExtent));
				render.DrawWireframeAABB(1, &StreamingBounds, Point(0.0f, 0.0f, 1.0f));


				{
					const float WorldSize = WorldExtent*2.0f;
					const udword NbCellsPerSide = 4;
					const float CellSize = WorldSize/float(NbCellsPerSide);

					//const float CellSize = 2.0f;
					const float CellSizeX = CellSize;
					const float CellSizeZ = CellSize;

					const sdword x0 = sdword(floorf(StreamingBounds.mMin.x/CellSizeX));
					const sdword z0 = sdword(floorf(StreamingBounds.mMin.z/CellSizeZ));
					const sdword x1 = sdword(ceilf(StreamingBounds.mMax.x/CellSizeX));
					const sdword z1 = sdword(ceilf(StreamingBounds.mMax.z/CellSizeZ));

					for(sdword j=z0;j<z1;j++)
					{
						for(sdword i=x0;i<x1;i++)
						{
							AABB CellBounds;
							CellBounds.SetMinMax(Point(float(i)*CellSizeX, 0.0f, float(j)*CellSizeZ), Point(float(i+1)*CellSizeX, 0.0f, float(j+1)*CellSizeZ));
							render.DrawWireframeAABB(1, &CellBounds, Point(0.0f, 1.0f, 0.0f));
						}
					}
				}
			}*/
		}
	}

	virtual udword	GetFlags() const
	{
		return (mCheckBox_DriveVehicle && mCheckBox_DriveVehicle->IsChecked()) ? TEST_FLAGS_USE_CURSOR_KEYS : TEST_FLAGS_DEFAULT;
	}

	virtual bool	SpecialKeyCallback(int key, int x, int y, bool down)
	{
		if(mCheckBox_DriveVehicle && mCheckBox_DriveVehicle->IsChecked())
			return VehicleInput::SpecialKeyCallback(key, x, y, down);
		else
			return false;
	}

	void	SaveCarData(const char* filename)	const
	{
/*		FILE* fp = fopen(filename, "wb");
		if(!fp)
		{
			printf("OOPS.... fopen failed!\n");
			return;
		}*/

		const float WheelRadius = GetFloat(0.0f, mEditBox_WheelRadius);
		const float WheelWidth = GetFloat(0.0f, mEditBox_WheelWidth);
		const float WheelMass = GetFloat(0.0f, mEditBox_WheelMass);
		const float WheelCoeffX = GetFloat(0.0f, mEditBox_WheelCoeffX);
		const float WheelCoeffZ = GetFloat(0.0f, mEditBox_WheelCoeffZ);
		const float WheelMaxBrakeTorqueFront = GetFloat(0.0f, mEditBox_WheelMaxBrakeTorqueFront);
		const float WheelMaxBrakeTorqueRear = GetFloat(0.0f, mEditBox_WheelMaxBrakeTorqueRear);
		const float WheelMaxHandBrakeTorqueFront = GetFloat(0.0f, mEditBox_WheelMaxHandBrakeTorqueFront);
		const float WheelMaxHandBrakeTorqueRear = GetFloat(0.0f, mEditBox_WheelMaxHandBrakeTorqueRear);
		const float WheelMaxSteerFront = GetFloat(0.0f, mEditBox_WheelMaxSteerFront);
		const float WheelMaxSteerRear = GetFloat(0.0f, mEditBox_WheelMaxSteerRear);
		const float AntirollBarStiffness = GetFloat(0.0f, mEditBox_AntirollBarStiffness);
		const udword WheelSubsteps = GetInt(32, mEditBox_Substeps);
		const float CutOffset = GetFloat(0.3f, mEditBox_GroundClearanceCutOffset);

		Point ChassisExtents;
		ChassisExtents.x = GetFloat(0.0f, mEditBox_ChassisX);
		ChassisExtents.y = GetFloat(0.0f, mEditBox_ChassisY);
		ChassisExtents.z = GetFloat(0.0f, mEditBox_ChassisZ);
		const float ChassisMass					= GetFloat(0.0f,	mEditBox_ChassisMass);
		const float ChassisMOICoeffY			= GetFloat(0.0f,	mEditBox_ChassisMOICoeffY);
		const float ChassisCMOffsetY			= GetFloat(0.0f,	mEditBox_ChassisCMOffsetY);
		const float ChassisCMOffsetZ			= GetFloat(0.0f,	mEditBox_ChassisCMOffsetZ);
		const float ForceApplicationCMOffsetY	= GetFloat(0.0f,	mEditBox_ForceApplicationCMOffsetY);

		const float TireFrictionMultiplierFront	= GetFloat(0.0f,	mEditBox_TireFrictionMultiplierFront);
		const float TireFrictionMultiplierRear	= GetFloat(0.0f,	mEditBox_TireFrictionMultiplierRear);
		const float EngineMOI					= GetFloat(1.0f,	mEditBox_EngineMOI);
		const float EnginePeakTorque			= GetFloat(0.0f,	mEditBox_EnginePeakTorque);
		const float EngineMaxOmega				= GetFloat(0.0f,	mEditBox_EngineMaxOmega);
		const float SuspMaxCompression			= GetFloat(0.0f,	mEditBox_SuspMaxCompression);
		const float SuspMaxDroop				= GetFloat(0.0f,	mEditBox_SuspMaxDroop);
		const float SuspSpringStrength			= GetFloat(0.0f,	mEditBox_SuspSpringStrength);
		const float SuspSpringDamperRate		= GetFloat(0.0f,	mEditBox_SuspSpringDamperRate);
		const float SuspCamberAngleAtRest		= GetFloat(0.0f,	mEditBox_SuspCamberAngleAtRest);
		const float SuspCamberAngleAtMaxCompr	= GetFloat(0.0f,	mEditBox_SuspCamberAngleAtMaxCompr);
		const float SuspCamberAngleAtMaxDroop	= GetFloat(0.0f,	mEditBox_SuspCamberAngleAtMaxDroop);
		const float GearsSwitchTime				= GetFloat(0.0f,	mEditBox_GearsSwitchTime);
		const float ClutchStrength				= GetFloat(0.0f,	mEditBox_ClutchStrength);
		const float AckermannAccuracy			= GetFloat(1.0f,	mEditBox_AckermannAccuracy);

		const udword Differential				= mComboBox_Differential->GetSelectedIndex();
		const float FrontRearSplit				= GetFloat(0.45f,	mEditBox_FrontRearSplit);
		const float FrontLeftRightSplit			= GetFloat(0.5f,	mEditBox_FrontLeftRightSplit);
		const float RearLeftRightSplit			= GetFloat(0.5f,	mEditBox_RearLeftRightSplit);
		const float CentreBias					= GetFloat(1.3f,	mEditBox_CentreBias);
		const float FrontBias					= GetFloat(1.3f,	mEditBox_FrontBias);
		const float RearBias					= GetFloat(1.3f,	mEditBox_RearBias);

		float SteerVsForwardSpeedData[4];
		float SteerVsForwardSpeedValue[4];
		{
			for(udword i=0;i<4;i++)
			{
				SteerVsForwardSpeedData[i] = GetFloat(gDefaultSteerVsForwardSpeedData[i*2+0], mEditBox_SteeringSpeed[i]);
				SteerVsForwardSpeedValue[i] = GetFloat(gDefaultSteerVsForwardSpeedData[i*2+1], mEditBox_SteeringValue[i]);
			}
		}

		float SmoothingData[16];
		{
			for(udword i=0;i<16;i++)
				SmoothingData[i] = GetFloat(gDefaultSmoothingData[i], mEditBox_SmoothingData[i]);
		}

		const float ThresholdFWDSpeed	= GetFloat(0.1f,		mEditBox_ThresholdForwardSpeed);
		const bool ImmediateAutoReverse	= mCheckBox_ImmediateAutoReverse ? mCheckBox_ImmediateAutoReverse->IsChecked() : false;

		const udword Version = 8;
		CustomArray CA;
		CA.Store(udword('PVF!'))
			.Store(udword(Version))
		// Wheels
			.Store(WheelRadius)
			.Store(WheelWidth)
			.Store(WheelMass)
			.Store(WheelCoeffX)
			.Store(WheelCoeffZ)
			.Store(WheelMaxBrakeTorqueFront)
			.Store(WheelMaxBrakeTorqueRear)
			.Store(WheelMaxHandBrakeTorqueFront)
			.Store(WheelMaxHandBrakeTorqueRear)
			.Store(WheelMaxSteerFront)
			.Store(WheelMaxSteerRear)
			.Store(AntirollBarStiffness)
			.Store(WheelSubsteps)
		// Chassis
			.Store(ChassisExtents.x)
			.Store(ChassisExtents.y)
			.Store(ChassisExtents.z)
			.Store(ChassisMass)
			.Store(ChassisMOICoeffY)
			.Store(ChassisCMOffsetY)
			.Store(ChassisCMOffsetZ)
			.Store(ForceApplicationCMOffsetY)
			.Store(CutOffset)
		//
			.Store(TireFrictionMultiplierFront)
			.Store(TireFrictionMultiplierRear)
			.Store(EngineMOI)
			.Store(EnginePeakTorque)
			.Store(EngineMaxOmega)
			.Store(SuspMaxCompression)
			.Store(SuspMaxDroop)
			.Store(SuspSpringStrength)
			.Store(SuspSpringDamperRate)
			.Store(SuspCamberAngleAtRest)
			.Store(SuspCamberAngleAtMaxCompr)
			.Store(SuspCamberAngleAtMaxDroop)
			.Store(GearsSwitchTime)
			.Store(ClutchStrength)
			.Store(AckermannAccuracy)
			.Store(Differential)
			.Store(FrontRearSplit)
			.Store(FrontLeftRightSplit)
			.Store(RearLeftRightSplit)
			.Store(CentreBias)
			.Store(FrontBias)
			.Store(RearBias);

		for(udword i=0;i<4;i++)
		{
			CA.Store(SteerVsForwardSpeedData[i]);
			CA.Store(SteerVsForwardSpeedValue[i]);
		}

		for(udword i=0;i<16;i++)
			CA.Store(SmoothingData[i]);

		CA.Store(ThresholdFWDSpeed);
		CA.Store(ImmediateAutoReverse ? udword(1) : udword(0));

//		CA.ExportToDisk(fp);

//		fclose(fp);
		CA.ExportToDisk(filename, "wb");
	}

	void	LoadCarData(const char* filename)
	{
		IceFile F(filename);
		if(!F.IsValid())
		{
			printf("OOPS.... fopen failed!\n");
			return;
		}

		const udword Header = F.LoadDword();
		const udword Version = F.LoadDword();
		const float WheelRadius = F.LoadFloat();
		const float WheelWidth = F.LoadFloat();
		const float WheelMass = F.LoadFloat();
		const float WheelCoeffX = F.LoadFloat();
		const float WheelCoeffZ = F.LoadFloat();
		const float WheelMaxBrakeTorqueFront = F.LoadFloat();
		const float WheelMaxBrakeTorqueRear = F.LoadFloat();

		float WheelMaxHandBrakeTorqueFront = 0.0f;
		float WheelMaxHandBrakeTorqueRear = 4000.0f;
		if(Version>=6)
		{
			WheelMaxHandBrakeTorqueFront = F.LoadFloat();
			WheelMaxHandBrakeTorqueRear = F.LoadFloat();
		}

		const float WheelMaxSteerFront = F.LoadFloat();
		const float WheelMaxSteerRear = F.LoadFloat();
		float AntirollBarStiffness = 0.0f;
		if(Version>=2)
			AntirollBarStiffness = F.LoadFloat();
		udword WheelSubsteps = 8;
		if(Version>=3)
			WheelSubsteps = F.LoadDword();
		const float ChassisExtents_x = F.LoadFloat();
		const float ChassisExtents_y = F.LoadFloat();
		const float ChassisExtents_z = F.LoadFloat();
		const float ChassisMass = F.LoadFloat();
		const float ChassisMOICoeffY = F.LoadFloat();
		const float ChassisCMOffsetY = F.LoadFloat();
		const float ChassisCMOffsetZ = F.LoadFloat();
		const float ForceApplicationCMOffsetY = F.LoadFloat();
		float CutOffset = 0.3f;
		if(Version>=4)
			CutOffset = F.LoadFloat();
		const float TireFrictionMultiplierFront = F.LoadFloat();
		float TireFrictionMultiplierRear = TireFrictionMultiplierFront;
		if(Version>=2)
			TireFrictionMultiplierRear = F.LoadFloat();
		float EngineMOI = 1.0f;
		if(Version>=8)
			EngineMOI = F.LoadFloat();
		const float EnginePeakTorque = F.LoadFloat();
		const float EngineMaxOmega = F.LoadFloat();
		const float SuspMaxCompression = F.LoadFloat();
		const float SuspMaxDroop = F.LoadFloat();
		const float SuspSpringStrength = F.LoadFloat();
		const float SuspSpringDamperRate = F.LoadFloat();
		const float SuspCamberAngleAtRest = F.LoadFloat();
		const float SuspCamberAngleAtMaxCompr = F.LoadFloat();
		const float SuspCamberAngleAtMaxDroop = F.LoadFloat();
		const float GearsSwitchTime = F.LoadFloat();
		const float ClutchStrength = F.LoadFloat();
		float AckermannAccuracy = 1.0f;
		if(Version>=2)
			AckermannAccuracy = F.LoadFloat();
		const udword Differential = F.LoadDword();

		float FrontRearSplit		= 0.45f;
		float FrontLeftRightSplit	= 0.5f;
		float RearLeftRightSplit	= 0.5f;
		float CentreBias			= 1.3f;
		float FrontBias				= 1.3f;
		float RearBias				= 1.3f;
		if(Version>=4)
		{
			FrontRearSplit = F.LoadFloat();
			FrontLeftRightSplit = F.LoadFloat();
			RearLeftRightSplit = F.LoadFloat();
			CentreBias = F.LoadFloat();
			FrontBias = F.LoadFloat();
			RearBias = F.LoadFloat();
		}

		float SteerVsForwardSpeedData[4];
		float SteerVsForwardSpeedValue[4];
		for(udword i=0;i<4;i++)
		{
			SteerVsForwardSpeedData[i] = gDefaultSteerVsForwardSpeedData[i*2+0];
			SteerVsForwardSpeedValue[i] = gDefaultSteerVsForwardSpeedData[i*2+1];
		}
		if(Version>=5)
		{
			for(udword i=0;i<4;i++)
			{
				SteerVsForwardSpeedData[i] = F.LoadFloat();
				SteerVsForwardSpeedValue[i] = F.LoadFloat();
			}
		}

		float SmoothingData[16];
		for(udword i=0;i<16;i++)
			SmoothingData[i] = gDefaultSmoothingData[i];
		if(Version>=6)
		{
			for(udword i=0;i<16;i++)
				SmoothingData[i] = F.LoadFloat();
		}

		float ThresholdFWDSpeed = 0.1f;
		bool ImmediateAutoReverse = false;
		if(Version>=7)
		{
			ThresholdFWDSpeed = F.LoadFloat();
			ImmediateAutoReverse = F.LoadDword() ? true : false;
		}


		mEditBox_WheelRadius->SetText(_F("%.3f", WheelRadius));
		mEditBox_WheelWidth->SetText(_F("%.3f", WheelWidth));
		mEditBox_WheelMass->SetText(_F("%.3f", WheelMass));
		mEditBox_WheelCoeffX->SetText(_F("%.3f", WheelCoeffX));
		mEditBox_WheelCoeffZ->SetText(_F("%.3f", WheelCoeffZ));
		mEditBox_WheelMaxBrakeTorqueFront->SetText(_F("%.3f", WheelMaxBrakeTorqueFront));
		mEditBox_WheelMaxBrakeTorqueRear->SetText(_F("%.3f", WheelMaxBrakeTorqueRear));
		mEditBox_WheelMaxHandBrakeTorqueFront->SetText(_F("%.3f", WheelMaxHandBrakeTorqueFront));
		mEditBox_WheelMaxHandBrakeTorqueRear->SetText(_F("%.3f", WheelMaxHandBrakeTorqueRear));
		mEditBox_WheelMaxSteerFront->SetText(_F("%.3f", WheelMaxSteerFront));
		mEditBox_WheelMaxSteerRear->SetText(_F("%.3f", WheelMaxSteerRear));
		mEditBox_AntirollBarStiffness->SetText(_F("%.3f", AntirollBarStiffness));
		mEditBox_Substeps->SetText(_F("%d", WheelSubsteps));
		mEditBox_ChassisX->SetText(_F("%.3f", ChassisExtents_x));
		mEditBox_ChassisY->SetText(_F("%.3f", ChassisExtents_y));
		mEditBox_ChassisZ->SetText(_F("%.3f", ChassisExtents_z));
		mEditBox_ChassisMass->SetText(_F("%.3f", ChassisMass));
		mEditBox_ChassisMOICoeffY->SetText(_F("%.3f", ChassisMOICoeffY));
		mEditBox_ChassisCMOffsetY->SetText(_F("%.3f", ChassisCMOffsetY));
		mEditBox_ChassisCMOffsetZ->SetText(_F("%.3f", ChassisCMOffsetZ));
		mEditBox_ForceApplicationCMOffsetY->SetText(_F("%.3f", ForceApplicationCMOffsetY));
		mEditBox_GroundClearanceCutOffset->SetText(_F("%.3f", CutOffset));
		mEditBox_TireFrictionMultiplierFront->SetText(_F("%.3f", TireFrictionMultiplierFront));
		mEditBox_TireFrictionMultiplierRear->SetText(_F("%.3f", TireFrictionMultiplierRear));
		mEditBox_EngineMOI->SetText(_F("%.3f", EngineMOI));
		mEditBox_EnginePeakTorque->SetText(_F("%.3f", EnginePeakTorque));
		mEditBox_EngineMaxOmega->SetText(_F("%.3f", EngineMaxOmega));
		mEditBox_SuspMaxCompression->SetText(_F("%.3f", SuspMaxCompression));
		mEditBox_SuspMaxDroop->SetText(_F("%.3f", SuspMaxDroop));
		mEditBox_SuspSpringStrength->SetText(_F("%.3f", SuspSpringStrength));
		mEditBox_SuspSpringDamperRate->SetText(_F("%.3f", SuspSpringDamperRate));
		mEditBox_SuspCamberAngleAtRest->SetText(_F("%.3f", SuspCamberAngleAtRest));
		mEditBox_SuspCamberAngleAtMaxCompr->SetText(_F("%.3f", SuspCamberAngleAtMaxCompr));
		mEditBox_SuspCamberAngleAtMaxDroop->SetText(_F("%.3f", SuspCamberAngleAtMaxDroop));
		mEditBox_GearsSwitchTime->SetText(_F("%.3f", GearsSwitchTime));
		mEditBox_ClutchStrength->SetText(_F("%.3f", ClutchStrength));
		mEditBox_AckermannAccuracy->SetText(_F("%.3f", AckermannAccuracy));
		mComboBox_Differential->Select(Differential);
		mEditBox_FrontRearSplit->SetText(_F("%.3f", FrontRearSplit));
		mEditBox_FrontLeftRightSplit->SetText(_F("%.3f", FrontLeftRightSplit));
		mEditBox_RearLeftRightSplit->SetText(_F("%.3f", RearLeftRightSplit));
		mEditBox_CentreBias->SetText(_F("%.3f", CentreBias));
		mEditBox_FrontBias->SetText(_F("%.3f", FrontBias));
		mEditBox_RearBias->SetText(_F("%.3f", RearBias));

		for(udword i=0;i<4;i++)
		{
			mEditBox_SteeringSpeed[i]->SetText(_F("%.3f", SteerVsForwardSpeedData[i]));
			mEditBox_SteeringValue[i]->SetText(_F("%.3f", SteerVsForwardSpeedValue[i]));
		}

		for(udword i=0;i<16;i++)
			mEditBox_SmoothingData[i]->SetText(_F("%.3f", SmoothingData[i]));

		mEditBox_ThresholdForwardSpeed->SetText(_F("%.3f", ThresholdFWDSpeed));
		mCheckBox_ImmediateAutoReverse->SetChecked(ImmediateAutoReverse);
	}

}VehicleSDK_;

static void gButtonCallback(IceButton& button, void* user_data)
{
	if(GetRoot())
	{
		FILESELECTCREATE Create;
		Create.mFilter			= "PEEL vehicle files (*.pvf)|*.pvf|All Files (*.*)|*.*||";
		Create.mFileName		= "DefaultCar.pvf";
		Create.mInitialDir		= GetRoot();
		Create.mCaptionTitle	= "Select file";
		Create.mDefExt			= "pvf";

		String Filename;
		if(button.GetID()==0)
		{
			if(!FileselectSave(Create, Filename, true))
				return;

			VehicleSDK* T = (VehicleSDK*)button.GetUserData();
			T->SaveCarData(Filename);
		}
		else
		{
			if(!FileselectOpenSingle(Create, Filename))
				return;

			VehicleSDK* T = (VehicleSDK*)button.GetUserData();
			T->LoadCarData(Filename);
			T->mMustResetTest = true;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

	struct VehicleSDKData2 : public Allocateable
	{
		VehicleSDKData2() :
			mVehicle0	(null),
			mVehicle1	(null),
			mChassis0	(null),
			mChassis1	(null),
			mTopObject	(null),
			mStartPose0	(Idt),
			mStartPose1	(Idt),
			mStartPose2	(Idt)
		{
		}
		PintVehicleHandle	mVehicle0;
		PintVehicleHandle	mVehicle1;
		PintActorHandle		mChassis0;
		PintActorHandle		mChassis1;
		PintActorHandle		mTopObject;
		PR					mStartPose0;
		PR					mStartPose1;
		PR					mStartPose2;
	};

static void ResetVehicle(Pint& pint, const VehicleSDKData2* data, Pint_Vehicle* api)
{
	api->ResetVehicleData(data->mVehicle0);
	api->ResetVehicleData(data->mVehicle1);

	pint.SetWorldTransform(data->mChassis0, data->mStartPose0);
	pint.SetWorldTransform(data->mChassis1, data->mStartPose1);
	pint.SetWorldTransform(data->mTopObject, data->mStartPose2);

	pint.SetLinearVelocity(data->mChassis0, Point(0.0f, 0.0f, 0.0f));
	pint.SetLinearVelocity(data->mChassis1, Point(0.0f, 0.0f, 0.0f));
	pint.SetAngularVelocity(data->mChassis0, Point(0.0f, 0.0f, 0.0f));
	pint.SetAngularVelocity(data->mChassis1, Point(0.0f, 0.0f, 0.0f));
}

/*
- drive back vehicle as well
- check scale, we might be smaller than necessary
- don't hardcode antirollbar
*/

//#define USE_EDY_VALUES

static const char* gDesc_VehicleSDKTruck = "Vehicle SDK Truck";

class VehicleSDKTruck : public VehicleInput
{
	bool				mReset;

	public:
							VehicleSDKTruck()			{}
	virtual					~VehicleSDKTruck()			{}
	virtual	const char*		GetName()			const	{ return "VehicleSDKTruck";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_VehicleSDKTruck;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_VEHICLES;		}

	virtual	bool			SpecialKeyCallback(int key, int x, int y, bool down)
	{
		if(key==GLUTX_KEY_HOME)
		{
			mReset = true;
			return true;
		}
		return VehicleInput::SpecialKeyCallback(key, x, y, down);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleInput::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(10.05f, 4.54f, 0.15f), Point(-0.92f, -0.29f, 0.27f));
		SetDefEnv(desc, false);
	}

	virtual void	Close(Pint& pint)
	{
		VehicleSDKData2* UserData = (VehicleSDKData2*)pint.mUserData;
		DELETESINGLE(UserData);
		pint.mUserData = null;

		VehicleInput::Close(pint);
	}

	virtual bool	CommonSetup()
	{
		mReset = false;

		mPlanarMeshHelper.Generate(64, 0.1f);
//		mPlanarMeshHelper.Generate(32, 0.1f);

//		const Point Scale(1.0f, 0.2f, 1.0f);
//		LoadMeshesFromFile_(*this, "Terrain.bin", &Scale, false, 0);

		return VehicleInput::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		Pint_Vehicle* API = pint.GetVehicleAPI();
		if(!caps.mSupportVehicles || !API)
			return false;

		if(!caps.mSupportMeshes || !caps.mSupportRigidBodySimulation)
			return false;

		const float Altitude = 0.0f;
		const PintActorHandle h = mPlanarMeshHelper.CreatePlanarMesh(pint, Altitude, &mHighFrictionMaterial);
		(void)h;

//		CreateMeshesFromRegisteredSurfaces(pint, caps, &mHighFrictionMaterial);

			const float Spacing = 10.0f;
		//	const float Size = 0.3f;
			const float Size = 0.15f;
//			const float Size = 0.5f;
		//	const float Size = 1.0f;
			GenerateGroundObstacles(pint, Spacing, Size, false);

		// We currently have this hardcoded in the PhysX plugin so we must define the car accordingly:
		//const PxVec3 forward(0.0f, 0.0f, 1.0f);

#ifdef USE_EDY_VALUES
		const float WheelRadius = 0.52f;
#else
		const float WheelRadius = 0.5f;
#endif
		const float WheelWidth = 0.3f;
		const float CoeffX = 0.85f;
		const float CoeffZ = 0.6f;

		// Forward is along Z so the chassis must be along Z as well
		Point ChassisExtents;
		ChassisExtents.x = 1.0f;
		ChassisExtents.y = 0.5f;
		ChassisExtents.z = 1.5f;

		const udword NbPts = 60;
//		const udword NbPts = 10;
		// Forward is along Z so the cylinder axis must be along X
		const CylinderMesh Cylinder(NbPts, WheelRadius, WheelWidth*0.5f, ORIENTATION_YZ);
		const udword TotalNbVerts = Cylinder.mNbVerts;
		const Point* Verts = Cylinder.mVerts;

		PINT_VEHICLE_CREATE VehicleDesc;
		VehicleDesc.mStartPose.mPos					= Point(0.0f, 3.0f, 0.0f);
		VehicleDesc.mDifferential					= DIFFERENTIAL_OPEN_4WD;
		VehicleDesc.mFrontRearSplit					= 0.45f;
		VehicleDesc.mFrontLeftRightSplit			= 0.5f;
		VehicleDesc.mRearLeftRightSplit				= 0.5f;
		VehicleDesc.mCentreBias						= 1.3f;
		VehicleDesc.mFrontBias						= 1.3f;
		VehicleDesc.mRearBias						= 1.3f;
		VehicleDesc.mChassisMass					= 1500.0f;
//		VehicleDesc.mChassisMass					= 5000.0f;
//		VehicleDesc.mChassisMass					= 1000.0f;
		VehicleDesc.mChassisMOICoeffY				= 0.8f;
		VehicleDesc.mChassisCMOffsetY				= 0.65f;
		VehicleDesc.mChassisCMOffsetZ				= 0.25f;
			//VehicleDesc.mChassisMOICoeffY			= 1.0f;
			//VehicleDesc.mChassisCMOffsetY			= 0.0f;
			//VehicleDesc.mChassisCMOffsetZ			= 0.0f;

		//VehicleDesc.mForceApplicationCMOffsetY		= 0.3f;
		VehicleDesc.mForceApplicationCMOffsetY		= 0.1f;
		VehicleDesc.mWheelMass						= 20.0f;
//		VehicleDesc.mWheelMass						= 100.0f;
		VehicleDesc.mWheelMaxBrakeTorqueFront		= 150.0f;
		VehicleDesc.mWheelMaxBrakeTorqueRear		= 1500.0f;
		VehicleDesc.mWheelMaxHandBrakeTorqueFront	= 0.0f;
		VehicleDesc.mWheelMaxHandBrakeTorqueRear	= 4000.0f;
		VehicleDesc.mWheelMaxSteerFront				= PI/3.0f;
		VehicleDesc.mWheelMaxSteerRear				= 0.0f;
		VehicleDesc.mWheelSubsteps					= 8;//32;
//		VehicleDesc.mTireFrictionMultiplier			= 1.1f;
//		VehicleDesc.mTireFrictionMultiplier			= 4.0f;
		VehicleDesc.mFrontTireFrictionMultiplier	= 4.0f;
		VehicleDesc.mRearTireFrictionMultiplier		= 4.0f;
		VehicleDesc.mEnginePeakTorque				= 1000.0f;
		VehicleDesc.mEngineMaxOmega					= 1000.0f;
		VehicleDesc.mEnginePeakTorque				= 600.0f;
		VehicleDesc.mEngineMaxOmega					= 600.0f;
#ifdef USE_EDY_VALUES
		VehicleDesc.mSuspMaxCompression				= 0.1125f;
		VehicleDesc.mSuspMaxDroop					= 0.0375f;
#else
		VehicleDesc.mSuspMaxCompression				= 0.3f;
		VehicleDesc.mSuspMaxDroop					= 0.1f;
#endif
		VehicleDesc.mSuspSpringStrength				= 35000.0f;
		VehicleDesc.mSuspSpringDamperRate			= 4500.0f;

		VehicleDesc.mSuspSpringStrength				= 35000.0f*2.0f;
		VehicleDesc.mSuspSpringDamperRate			= 4500.0f*2.0f;
#ifdef USE_EDY_VALUES
		VehicleDesc.mSuspSpringStrength				= 350000.0f;
		VehicleDesc.mSuspSpringDamperRate			= 12000.0f;
#endif

//		float NF = sqrtf(VehicleDesc.mSuspSpringStrength/(15000.0f/4.0f));

		if(0)
		{
			// todo: check MOI
			const float SprungMass = 1500.0f/4.0f;
//			const float SprungMass = 10000.0f/4.0f;
			const float gravitationalAcceleration = 9.81f;
			float SpringStrength = (SprungMass*gravitationalAcceleration)/VehicleDesc.mSuspMaxDroop;

			const float dampingRatio = 1.0f;
			float SpringDamperRate = dampingRatio * 2.0f * sqrtf(SpringStrength * SprungMass);

			float NaturalFrequency = sqrtf(SpringStrength/SprungMass);	// 5 ideal

			VehicleDesc.mSuspSpringStrength			= SpringStrength;
			VehicleDesc.mSuspSpringDamperRate		= SpringDamperRate;
		}

//		VehicleDesc.mSuspSpringStrength			= 35000.0f*4.0f;
//		VehicleDesc.mSuspSpringDamperRate		= 4500.0f*4.0f;

//		VehicleDesc.mSuspSpringStrength			= 35000.0f*8.0f;
//		VehicleDesc.mSuspSpringDamperRate		= 4500.0f*8.0f;

		VehicleDesc.mSuspCamberAngleAtRest		= 0.0f;
		VehicleDesc.mSuspCamberAngleAtMaxCompr	= 0.01f;
		VehicleDesc.mSuspCamberAngleAtMaxDroop	= 0.01f;
		VehicleDesc.mGearsSwitchTime			= 0.5f;
		VehicleDesc.mClutchStrength				= 10.0f;
		VehicleDesc.mAckermannAccuracy			= 1.0f;

		if(0)
		{
			Matrix3x3 Rot;
//			Rot.RotZ(HALFPI);
			Rot.RotZ(60.0f*DEGTORAD);
			Rot.RotZ(70.0f*DEGTORAD);
			VehicleDesc.mStartPose.mRot = Rot;
		}

		PINT_CONVEX_CREATE WheelDesc(TotalNbVerts, Verts);
		WheelDesc.mRenderer	= CreateConvexRenderer(TotalNbVerts, Verts);
		VehicleDesc.mWheels[0] = &WheelDesc;
//		VehicleDesc.mWheels.mNbVerts	= TotalNbVerts;
//		VehicleDesc.mWheels.mVerts		= Verts;
//		VehicleDesc.mWheels.mRenderer	= CreateConvexRenderer(VehicleDesc.mWheels.mNbVerts, VehicleDesc.mWheels.mVerts);

		// We must create the wheels in this order:
		// eFRONT_LEFT=0,
		// eFRONT_RIGHT,
		// eREAR_LEFT,
		// eREAR_RIGHT
		// Forward is along Z so front is +z, left is +x

		VehicleDesc.mWheelOffset[0] = Point( ChassisExtents.x*CoeffX, -WheelRadius,  ChassisExtents.z*CoeffZ);
		VehicleDesc.mWheelOffset[1] = Point(-ChassisExtents.x*CoeffX, -WheelRadius,  ChassisExtents.z*CoeffZ);
		VehicleDesc.mWheelOffset[2] = Point( ChassisExtents.x*CoeffX, -WheelRadius, -ChassisExtents.z*CoeffZ);
		VehicleDesc.mWheelOffset[3] = Point(-ChassisExtents.x*CoeffX, -WheelRadius, -ChassisExtents.z*CoeffZ);

		AABB ChassisBox;
		ChassisBox.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), ChassisExtents);
		Point ChassisPts[8];
		ChassisBox.ComputePoints(ChassisPts);

		PINT_CONVEX_CREATE ChassisDesc(8, ChassisPts);
		ChassisDesc.mRenderer	= CreateConvexRenderer(ChassisDesc.mNbVerts, ChassisDesc.mVerts);
		VehicleDesc.mChassis	= &ChassisDesc;

		VehicleSDKData2* VehicleData = ICE_NEW(VehicleSDKData2);
		pint.mUserData = VehicleData;

		PintVehicleData VD;
		const PintVehicleHandle VehicleHandle = API->CreateVehicle(VD, VehicleDesc);
		PintActorHandle Chassis0 = VD.mChassisActor;
		VehicleData->mChassis0 = VD.mChassisActor;
		VehicleData->mVehicle0 = VehicleHandle;
		VehicleData->mStartPose0 = VehicleDesc.mStartPose;

		const Point Pose0 = VehicleDesc.mStartPose.mPos;
		VehicleDesc.mStartPose.mPos.z -= 7.0f;
		const PintVehicleHandle VehicleHandle2 = API->CreateVehicle(VD, VehicleDesc);
		VehicleData->mChassis1 = VD.mChassisActor;
		VehicleData->mVehicle1 = VehicleHandle2;
		VehicleData->mStartPose1 = VehicleDesc.mStartPose;

		PintActorHandle Chassis1 = VD.mChassisActor;
		const Point Pose1 = VehicleDesc.mStartPose.mPos;

		Point MiddlePose = (Pose0 + Pose1)*0.5f;
		const float Offset = 1.0f;
		MiddlePose.z -= Offset;


		if(1)
		{
			PINT_BOX_CREATE BoxDesc(1.0f, 1.0f, (Pose0.z - Pose1.z + ChassisExtents.z*2.0f - Offset)*0.5f);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mPosition	= MiddlePose;
			ObjectDesc.mPosition.y	+= ChassisExtents.y + BoxDesc.mExtents.y;
			ObjectDesc.mMass		= 15000.0f;
//			ObjectDesc.mMass		= 10000.0f;
			PintActorHandle h = CreatePintObject(pint, ObjectDesc);

			// Tell system about this new actor so that it's configured to ignore wheel casts
			API->AddActor(VehicleHandle, h);

			VehicleData->mTopObject = h;
			VehicleData->mStartPose2.mPos = ObjectDesc.mPosition;

			if(1)
			{
				PINT_HINGE_JOINT_CREATE Desc;
//				PINT_SPHERICAL_JOINT_CREATE Desc;
				Desc.mObject0		= Chassis0;
				Desc.mObject1		= h;
				Desc.mLocalPivot0	= Point(0.0f, ChassisExtents.y, 0.0f);
				Desc.mLocalPivot1	= Point(0.0f, -BoxDesc.mExtents.y, Pose0.z - MiddlePose.z);
				Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
				Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
			if(1)
			{
				PINT_FIXED_JOINT_CREATE Desc;
				Desc.mObject0		= Chassis1;
				Desc.mObject1		= h;
				Desc.mLocalPivot0	= Point(0.0f, ChassisExtents.y, 0.0f);
				Desc.mLocalPivot1	= Point(0.0f, -BoxDesc.mExtents.y, Pose1.z - MiddlePose.z);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
		return true;
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		Pint_Vehicle* API = pint.GetVehicleAPI();
		if(!API)
			return 0;

		const VehicleSDKData2* UserData = (const VehicleSDKData2*)pint.mUserData;

		const PintVehicleHandle VehicleHandle = UserData->mVehicle0;
		if(!VehicleHandle)
			return 0;

		if(mReset)
		{
			mReset = false;
			ResetVehicle(pint, UserData, API);
		}

//		if(mCheckBox_DriveVehicle->IsChecked())
		{
//			pint.SetVehicleInput(UserData->mVehicle2, mInput);
			API->SetVehicleInput(VehicleHandle, mInput);

			// Camera
			mCamera.mUpOffset		= 3.0f;
			mCamera.mDistToTarget	= 10.0f;
			UpdateCamera(pint, UserData->mChassis0);
		}
		return VehicleInput::Update(pint, dt);
	}

	virtual udword	GetFlags() const
	{
		return TEST_FLAGS_USE_CURSOR_KEYS;
	}

}VehicleSDKTruck;


///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_VehicleSDKAndRope = "Vehicle SDK and rope";

class VehicleSDKAndRope : public VehicleInput
{
	bool				mReset;

	public:
							VehicleSDKAndRope()			{}
	virtual					~VehicleSDKAndRope()		{}
	virtual	const char*		GetName()			const	{ return "VehicleSDKAndRope";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_VehicleSDKAndRope;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_VEHICLES;			}

	virtual	bool			SpecialKeyCallback(int key, int x, int y, bool down)
	{
		if(key==GLUTX_KEY_HOME)
		{
			mReset = true;
			return true;
		}
		return VehicleInput::SpecialKeyCallback(key, x, y, down);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleInput::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(10.05f, 4.54f, 0.15f), Point(-0.92f, -0.29f, 0.27f));
		SetDefEnv(desc, false);
	}

	virtual void	Close(Pint& pint)
	{
		VehicleSDKData2* UserData = (VehicleSDKData2*)pint.mUserData;
		DELETESINGLE(UserData);
		pint.mUserData = null;

		VehicleInput::Close(pint);
	}

	virtual bool	CommonSetup()
	{
		mReset = false;
//		mCreateDefaultEnvironment = false;

		mPlanarMeshHelper.Generate(64, 0.1f);

//		const Point Scale(1.0f, 0.2f, 1.0f);
//		LoadMeshesFromFile_(*this, "Terrain.bin", &Scale, false, 0);

		return VehicleInput::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		Pint_Vehicle* API = pint.GetVehicleAPI();
		if(!caps.mSupportVehicles || !API)
			return false;

//		CreateMeshesFromRegisteredSurfaces(pint, caps, &mHighFrictionMaterial);
		if(!caps.mSupportMeshes || !caps.mSupportRigidBodySimulation)
			return false;

		const float Altitude = 0.0f;
		const PintActorHandle h = mPlanarMeshHelper.CreatePlanarMesh(pint, Altitude, &mHighFrictionMaterial);
		(void)h;

		// We currently have this hardcoded in the PhysX plugin so we must define the car accordingly:
		//const PxVec3 forward(0.0f, 0.0f, 1.0f);

		const float WheelRadius = 0.5f;
		const float WheelWidth = 0.3f;
		const float CoeffX = 0.85f;
		const float CoeffZ = 0.6f;

		// Forward is along Z so the chassis must be along Z as well
		Point ChassisExtents;
		ChassisExtents.x = 1.0f;
		ChassisExtents.y = 0.5f;
		ChassisExtents.z = 1.5f;

		const udword NbPts = 60;
//		const udword NbPts = 10;
		// Forward is along Z so the cylinder axis must be along X
		const CylinderMesh Cylinder(NbPts, WheelRadius, WheelWidth*0.5f, ORIENTATION_YZ);
		const udword TotalNbVerts = Cylinder.mNbVerts;
		const Point* Verts = Cylinder.mVerts;

		PINT_VEHICLE_CREATE VehicleDesc;
		VehicleDesc.mStartPose.mPos					= Point(0.0f, 3.0f, 0.0f);
		VehicleDesc.mDifferential					= DIFFERENTIAL_LS_4WD;
		VehicleDesc.mFrontRearSplit					= 0.45f;
		VehicleDesc.mFrontLeftRightSplit			= 0.5f;
		VehicleDesc.mRearLeftRightSplit				= 0.5f;
		VehicleDesc.mCentreBias						= 1.3f;
		VehicleDesc.mFrontBias						= 1.3f;
		VehicleDesc.mRearBias						= 1.3f;
		VehicleDesc.mChassisMass					= 1500.0f;
		VehicleDesc.mChassisMOICoeffY				= 0.8f;
		VehicleDesc.mChassisCMOffsetY				= 0.65f;
		VehicleDesc.mChassisCMOffsetZ				= 0.25f;
		VehicleDesc.mForceApplicationCMOffsetY		= 0.3f;
		VehicleDesc.mWheelMass						= 20.0f;
		VehicleDesc.mWheelMaxBrakeTorqueFront		= 150.0f;
		VehicleDesc.mWheelMaxBrakeTorqueRear		= 1500.0f;
		VehicleDesc.mWheelMaxHandBrakeTorqueFront	= 0.0f;
		VehicleDesc.mWheelMaxHandBrakeTorqueRear	= 4000.0f;
		VehicleDesc.mWheelMaxSteerFront				= PI/3.0f;
		VehicleDesc.mWheelMaxSteerRear				= 0.0f;
		VehicleDesc.mWheelSubsteps					= 32;
//		VehicleDesc.mTireFrictionMultiplier			= 1.1f;
//		VehicleDesc.mTireFrictionMultiplier			= 4.0f;
		VehicleDesc.mFrontTireFrictionMultiplier	= 4.0f;
		VehicleDesc.mRearTireFrictionMultiplier		= 4.0f;
		VehicleDesc.mEnginePeakTorque				= 1000.0f;
		VehicleDesc.mEngineMaxOmega					= 1000.0f;
		VehicleDesc.mSuspMaxCompression				= 0.3f;
		VehicleDesc.mSuspMaxDroop					= 0.1f;
		VehicleDesc.mSuspSpringStrength				= 35000.0f;
		VehicleDesc.mSuspSpringDamperRate			= 4500.0f;

		VehicleDesc.mSuspSpringStrength				= 35000.0f*2.0f;
		VehicleDesc.mSuspSpringDamperRate			= 4500.0f*2.0f;

		VehicleDesc.mSuspCamberAngleAtRest			= 0.0f;
		VehicleDesc.mSuspCamberAngleAtMaxCompr		= 0.01f;
		VehicleDesc.mSuspCamberAngleAtMaxDroop		= 0.01f;
		VehicleDesc.mGearsSwitchTime				= 0.5f;
		VehicleDesc.mClutchStrength					= 10.0f;
		VehicleDesc.mAckermannAccuracy				= 1.0f;

		PINT_CONVEX_CREATE WheelDesc(TotalNbVerts, Verts);
		WheelDesc.mRenderer	= CreateConvexRenderer(TotalNbVerts, Verts);
		VehicleDesc.mWheels[0] = &WheelDesc;
//		VehicleDesc.mWheels.mNbVerts	= TotalNbVerts;
//		VehicleDesc.mWheels.mVerts		= Verts;
//		VehicleDesc.mWheels.mRenderer	= CreateConvexRenderer(VehicleDesc.mWheels.mNbVerts, VehicleDesc.mWheels.mVerts);

		// We must create the wheels in this order:
		// eFRONT_LEFT=0,
		// eFRONT_RIGHT,
		// eREAR_LEFT,
		// eREAR_RIGHT
		// Forward is along Z so front is +z, left is +x

		VehicleDesc.mWheelOffset[0] = Point( ChassisExtents.x*CoeffX, -WheelRadius,  ChassisExtents.z*CoeffZ);
		VehicleDesc.mWheelOffset[1] = Point(-ChassisExtents.x*CoeffX, -WheelRadius,  ChassisExtents.z*CoeffZ);
		VehicleDesc.mWheelOffset[2] = Point( ChassisExtents.x*CoeffX, -WheelRadius, -ChassisExtents.z*CoeffZ);
		VehicleDesc.mWheelOffset[3] = Point(-ChassisExtents.x*CoeffX, -WheelRadius, -ChassisExtents.z*CoeffZ);

		AABB ChassisBox;
		ChassisBox.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), ChassisExtents);
		Point ChassisPts[8];
		ChassisBox.ComputePoints(ChassisPts);

		PINT_CONVEX_CREATE ChassisDesc(8, ChassisPts);
		ChassisDesc.mRenderer	= CreateConvexRenderer(ChassisDesc.mNbVerts, ChassisDesc.mVerts);
		VehicleDesc.mChassis	= &ChassisDesc;

		VehicleSDKData2* VehicleData = ICE_NEW(VehicleSDKData2);
		pint.mUserData = VehicleData;

		PintVehicleData VD;
		const PintVehicleHandle VehicleHandle = API->CreateVehicle(VD, VehicleDesc);
		PintActorHandle Chassis0 = VD.mChassisActor;
		VehicleData->mChassis0 = VD.mChassisActor;
		VehicleData->mVehicle0 = VehicleHandle;
		VehicleData->mStartPose0 = VehicleDesc.mStartPose;

		const Point Pose0 = VehicleDesc.mStartPose.mPos;
		VehicleDesc.mStartPose.mPos.z -= 5.0f;
		const PintVehicleHandle VehicleHandle2 = API->CreateVehicle(VD, VehicleDesc);
		VehicleData->mChassis1 = VD.mChassisActor;
		VehicleData->mVehicle1 = VehicleHandle2;
		VehicleData->mStartPose1 = VehicleDesc.mStartPose;

		PintActorHandle Chassis1 = VD.mChassisActor;
		const Point Pose1 = VehicleDesc.mStartPose.mPos;

		Point MiddlePose = (Pose0 + Pose1)*0.5f;
		const float Offset = 1.0f;
		MiddlePose.z -= Offset;

		if(1)
		{
			const PintDisabledGroups DG(1, 2);
			pint.SetDisabledGroups(1, &DG);

			const Point StartPos = VehicleData->mStartPose0.mPos - Point(0.0f, 0.0f, ChassisExtents.z);
			const Point EndPos = VehicleData->mStartPose1.mPos + Point(0.0f, 0.0f, ChassisExtents.z);
			const float RopeLength = StartPos.Distance(EndPos);
			const float Radius = 0.05f;
			const udword NbSpheres = 1+udword(RopeLength/(Radius*2.0f));
			const float Mass = 50.0f;

			const Point InitPos = StartPos + Point(0.0f, 0.0f, -Radius);
			Point Pos = InitPos;
			PintActorHandle* Handles = new PintActorHandle[NbSpheres];
			Point* Positions = ICE_NEW(Point)[NbSpheres];

			udword GroupBit = 0;
			{
				PINT_SPHERE_CREATE SphereDesc(Radius);
				SphereDesc.mRenderer = CreateSphereRenderer(Radius);
				SphereDesc.mMaterial	= &mZeroFrictionMaterial;

//				const Point Offset(Radius*2.0f, 0.0f, 0.0f);
				const Point Offset(0.0f, 0.0f, -Radius*2.0f);
				const Point LocalPivot0	= Point(0.0f, 0.0f, 0.0f);
//				const Point LocalPivot1	= Point(-Radius*2.0f, 0.0f, 0.0f);
				const Point LocalPivot1	= Point(0.0f, 0.0f, Radius*2.0f);

				for(udword i=0;i<NbSpheres;i++)
				{
					Positions[i] = Pos;

					PINT_OBJECT_CREATE ObjectDesc(&SphereDesc);
					ObjectDesc.mMass			= Mass;
					ObjectDesc.mMassForInertia	= Mass*10.0f;
					ObjectDesc.mPosition		= Pos;
					ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
					Handles[i] = CreatePintObject(pint, ObjectDesc);

					Pos += Offset;
				}

				for(udword i=0;i<NbSpheres-1;i++)
				{
					PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[i], Handles[i+1], LocalPivot0, LocalPivot1));
					ASSERT(JointHandle);
				}

				if(1/*UseDistanceConstraints*/)
				{
					for(udword i=0;i<NbSpheres;i++)
					{
						if(i+2<NbSpheres)
						{
							PINT_DISTANCE_JOINT_CREATE Desc;
							Desc.mObject0			= Handles[i];
							Desc.mObject1			= Handles[i+2];
							Desc.mLimits.mMaxValue	= Positions[i].Distance(Positions[i+2]);
							PintJointHandle JointHandle = pint.CreateJoint(Desc);
							ASSERT(JointHandle);
						}
					}
				}

				pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[0], VehicleData->mChassis0, Point(0.0f, 0.0f, Radius), Point(0.0f, 0.0f, -ChassisExtents.z)));
				pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[NbSpheres-1], VehicleData->mChassis1, Point(0.0f, 0.0f, -Radius), Point(0.0f, 0.0f, ChassisExtents.z)));
			}
	
			DELETEARRAY(Positions);
			DELETEARRAY(Handles);
		}



		if(0)
		{
			PINT_BOX_CREATE BoxDesc(1.0f, 1.0f, (Pose0.z - Pose1.z + ChassisExtents.z*2.0f - Offset)*0.5f);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mPosition	= MiddlePose;
			ObjectDesc.mPosition.y	+= ChassisExtents.y + BoxDesc.mExtents.y;
			ObjectDesc.mMass		= 15000.0f;
//			ObjectDesc.mMass		= 10000.0f;
			PintActorHandle h = CreatePintObject(pint, ObjectDesc);

			VehicleData->mTopObject = h;
			VehicleData->mStartPose2.mPos = ObjectDesc.mPosition;

/*			if(1)
			{
				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mObject0		= Chassis0;
				Desc.mObject1		= h;
				Desc.mLocalPivot0	= Point(0.0f, ChassisExtents.y, 0.0f);
				Desc.mLocalPivot1	= Point(0.0f, -BoxDesc.mExtents.y, Pose0.z - MiddlePose.z);
				Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
				Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
			if(1)
			{
				PINT_FIXED_JOINT_CREATE Desc;
				Desc.mObject0		= Chassis1;
				Desc.mObject1		= h;
				Desc.mLocalPivot0	= Point(0.0f, ChassisExtents.y, 0.0f);
				Desc.mLocalPivot1	= Point(0.0f, -BoxDesc.mExtents.y, Pose1.z - MiddlePose.z);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}*/
		}
		return true;
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		Pint_Vehicle* API = pint.GetVehicleAPI();
		if(!API)
			return 0;

		const VehicleSDKData2* UserData = (const VehicleSDKData2*)pint.mUserData;

		const PintVehicleHandle VehicleHandle = UserData->mVehicle0;
		if(!VehicleHandle)
			return 0;

		if(mReset)
		{
			mReset = false;
			ResetVehicle(pint, UserData, API);
		}

//		if(mCheckBox_DriveVehicle->IsChecked())
		{
//			pint.SetVehicleInput(UserData->mVehicle2, mInput);
			API->SetVehicleInput(VehicleHandle, mInput);

			// Camera
			mCamera.mUpOffset		= 3.0f;
			mCamera.mDistToTarget	= 10.0f;
			UpdateCamera(pint, UserData->mChassis0);
		}
		return VehicleInput::Update(pint, dt);
	}

	virtual udword	GetFlags() const
	{
		return TEST_FLAGS_USE_CURSOR_KEYS;
	}

}VehicleSDKAndRope;

///////////////////////////////////////////////////////////////////////////////

#include ".\ZCB\PINT_ZCB2.h"

class LegoVehicle : public VehicleSDK
{
			const PINT_SHAPE_CREATE*	mChassis;
			const PINT_SHAPE_CREATE*	mWheel0;
			const PINT_SHAPE_CREATE*	mWheel1;
			const PINT_SHAPE_CREATE*	mWheel2;
			const PINT_SHAPE_CREATE*	mWheel3;

	protected:
			const char*		mVehicleFile;
	public:
							LegoVehicle(VehicleScene default_scene) :
								VehicleSDK	(default_scene),
								mChassis	(null),
								mWheel0		(null),
								mWheel1		(null),
								mWheel2		(null),
								mWheel3		(null),
								mVehicleFile(null)
							{
							}
	virtual					~LegoVehicle()						{}
	virtual	TestCategory	GetCategory()				const	{ return CATEGORY_VEHICLES;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleSDK::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(10.05f, 4.54f, 0.15f), Point(-0.92f, -0.29f, 0.27f));

		const udword Index = mComboBox_Level->GetSelectedIndex();
		SetDefEnv(desc, gCreateDefaultEnv[Index]);

		if(gTrySound)
		{
			void StartSound(const char* filename, udword pos);
//			StartSound("f360.wav", 0);
//			StartSound("ford2c.wav", 0);
			StartSound("engine7.wav", 0);
//			StartSound("engine0.wav", 0);
//			StartSound("engine6.wav", 0);
		}
	}

	virtual void	Close(Pint& pint)
	{
		ReleaseVehicleData(pint);
		VehicleSDK::Close(pint);
	}

	virtual bool	CommonSetup()
	{
		if(gVehicleFilename.IsValid())
			mVehicleFile = gVehicleFilename.Get();

		const char* Filename = mVehicleFile ? FindPEELFile(mVehicleFile) : null;
//		const char* Filename = FindPEELFile("NX_Vehicle.zb2");
		if(Filename)
		{
			gZCB2_RenderScale = GetFloat(0.35f, mEditBox_VehicleScale);

			PINT_WORLD_CREATE desc;
			ImportZB2File(desc, Filename);

			if(mCheckBox_UseRenderMeshes)
				mCheckBox_UseRenderMeshes->SetEnabled(true);
			if(mEditBox_VehicleScale)
				mEditBox_VehicleScale->SetEnabled(true);

			if(mZB2Factory)
			{
				const udword NbActors = mZB2Factory->GetNbActors();
				const ZCB2Factory::ActorCreate* Actors = mZB2Factory->GetActors();

				ASSERT(NbActors==1);
				if(NbActors==1)
				{
					const PINT_OBJECT_CREATE* Create = Actors[0].mCreate;
					const PINT_SHAPE_CREATE* Wheels[4];
					mChassis = FindChassis(Create, Wheels);
					mWheel0 = FindWheel(Wheels, true, true);
					mWheel1 = FindWheel(Wheels, false, true);
					mWheel2 = FindWheel(Wheels, true, false);
					mWheel3 = FindWheel(Wheels, false, false);

					ASSERT(mWheel0->mType==PINT_SHAPE_CONVEX);
					ASSERT(mWheel1->mType==PINT_SHAPE_CONVEX);
					ASSERT(mWheel2->mType==PINT_SHAPE_CONVEX);
					ASSERT(mWheel3->mType==PINT_SHAPE_CONVEX);
					const PINT_CONVEX_CREATE* ConvexWheel0 = static_cast<const PINT_CONVEX_CREATE*>(mWheel0);
					const PINT_CONVEX_CREATE* ConvexWheel1 = static_cast<const PINT_CONVEX_CREATE*>(mWheel1);
					const PINT_CONVEX_CREATE* ConvexWheel2 = static_cast<const PINT_CONVEX_CREATE*>(mWheel2);
					const PINT_CONVEX_CREATE* ConvexWheel3 = static_cast<const PINT_CONVEX_CREATE*>(mWheel3);

					// ### Scale is now taken into account directly in the ZB2 loading code
					const float Scale = 1.0f;//gZCB2_RenderScale;
					if(Scale!=1.0f)
					{
						Point* v0 = const_cast<Point*>(ConvexWheel0->mVerts);
						for(udword i=0;i<ConvexWheel0->mNbVerts;i++)
							v0[i] *= Scale;

						Point* v1 = const_cast<Point*>(ConvexWheel1->mVerts);
						for(udword i=0;i<ConvexWheel1->mNbVerts;i++)
							v1[i] *= Scale;

						Point* v2 = const_cast<Point*>(ConvexWheel2->mVerts);
						for(udword i=0;i<ConvexWheel2->mNbVerts;i++)
							v2[i] *= Scale;

						Point* v3 = const_cast<Point*>(ConvexWheel3->mVerts);
						for(udword i=0;i<ConvexWheel3->mNbVerts;i++)
							v3[i] *= Scale;
					}

					ASSERT(mChassis->mType==PINT_SHAPE_CONVEX);
					const PINT_CONVEX_CREATE* ConvexChassis = static_cast<const PINT_CONVEX_CREATE*>(mChassis);

					AABB Bounds;
					ComputeAABB(Bounds, ConvexChassis->mVerts, ConvexChassis->mNbVerts);

					Point ChassisExtents;
					Bounds.GetExtents(ChassisExtents);
					mEditBox_ChassisX->SetText(_F("%.3f", ChassisExtents.x * Scale));	mEditBox_ChassisX->SetEnabled(false);
					mEditBox_ChassisY->SetText(_F("%.3f", ChassisExtents.y * Scale));	mEditBox_ChassisY->SetEnabled(false);
					mEditBox_ChassisZ->SetText(_F("%.3f", ChassisExtents.z * Scale));	mEditBox_ChassisZ->SetEnabled(false);

					mEditBox_WheelRadius->SetEnabled(false);
					mEditBox_WheelWidth->SetEnabled(false);
					mEditBox_WheelCoeffX->SetEnabled(false);
					mEditBox_WheelCoeffZ->SetEnabled(false);

					const float CutOffset = GetFloat(0.3f, mEditBox_GroundClearanceCutOffset);
					{
						Point* ppp = const_cast<Point*>(ConvexChassis->mVerts);
						const float limit = Bounds.mMin.y*Scale + CutOffset;
						for(udword i=0;i<ConvexChassis->mNbVerts;i++)
						{
							ppp[i] *= Scale;

							if(ppp[i].y<limit)
								ppp[i].y=limit;
						}
					}
				}
			}

			//gVehicleFilename.Reset();
		}

		return VehicleSDK::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		Pint_Vehicle* API = pint.GetVehicleAPI();
		if(!caps.mSupportVehicles || !API)
			return false;

//		CreateMeshesFromRegisteredSurfaces(pint, caps, *this, &mHighFrictionMaterial);
		if(!CreateLevelMeshes(pint, caps))
			return false;

		//CreateZB2Scene(pint, caps);

		// ### Scale is now taken into account directly in the ZB2 loading code
		const float Scale = 1.0f;//gZCB2_RenderScale;

		if(mZB2Factory)
		{
			const udword NbActors = mZB2Factory->GetNbActors();
			const ZCB2Factory::ActorCreate* Actors = mZB2Factory->GetActors();

			ASSERT(NbActors==1);
			if(NbActors==1)
			{
				const bool ShowCollisionShapes = !mCheckBox_UseRenderMeshes || !mCheckBox_UseRenderMeshes->IsChecked();

				const PINT_OBJECT_CREATE* Create = Actors[0].mCreate;
				//const PintActorHandle h = CreatePintObject(pint, *Create, false);
				//ASSERT(h);

/*				const PINT_SHAPE_CREATE* Chassis = Create->mShapes;
				const PINT_SHAPE_CREATE* Wheel0 = Chassis->mNext;
				const PINT_SHAPE_CREATE* Wheel1 = Wheel0->mNext;
				const PINT_SHAPE_CREATE* Wheel2 = Wheel1->mNext;
				const PINT_SHAPE_CREATE* Wheel3 = Wheel2->mNext;*/

				// We must create the wheels in this order:
				// eFRONT_LEFT=0,
				// eFRONT_RIGHT,
				// eREAR_LEFT,
				// eREAR_RIGHT
				// Forward is along Z so front is +z, left is +x

//				VehicleDesc.mWheelOffset[0] = Point( ChassisExtents.x*CoeffX, -WheelRadius,  ChassisExtents.z*CoeffZ);
//				VehicleDesc.mWheelOffset[1] = Point(-ChassisExtents.x*CoeffX, -WheelRadius,  ChassisExtents.z*CoeffZ);
//				VehicleDesc.mWheelOffset[2] = Point( ChassisExtents.x*CoeffX, -WheelRadius, -ChassisExtents.z*CoeffZ);
//				VehicleDesc.mWheelOffset[3] = Point(-ChassisExtents.x*CoeffX, -WheelRadius, -ChassisExtents.z*CoeffZ);
/*++
-+
+-
--
0
3
1
2*/
/*				const PINT_SHAPE_CREATE* Wheel0 = FindWheel(Create->mShapes, true, true);
				const PINT_SHAPE_CREATE* Wheel1 = FindWheel(Create->mShapes, false, true);
				const PINT_SHAPE_CREATE* Wheel2 = FindWheel(Create->mShapes, true, false);
				const PINT_SHAPE_CREATE* Wheel3 = FindWheel(Create->mShapes, false, false);*/

/*				const PINT_SHAPE_CREATE* Wheels[4];
				const PINT_SHAPE_CREATE* Chassis = FindChassis(Create->mShapes, Wheels);
				const PINT_SHAPE_CREATE* Wheel0 = FindWheel(Wheels, true, true);
				const PINT_SHAPE_CREATE* Wheel1 = FindWheel(Wheels, false, true);
				const PINT_SHAPE_CREATE* Wheel2 = FindWheel(Wheels, true, false);
				const PINT_SHAPE_CREATE* Wheel3 = FindWheel(Wheels, false, false);*/

				const PINT_SHAPE_CREATE* Chassis = mChassis;
				const PINT_SHAPE_CREATE* Wheel0 = mWheel0;
				const PINT_SHAPE_CREATE* Wheel1 = mWheel1;
				const PINT_SHAPE_CREATE* Wheel2 = mWheel2;
				const PINT_SHAPE_CREATE* Wheel3 = mWheel3;

				PINT_VEHICLE_CREATE VehicleDesc;
				VehicleDesc.mStartPose.mPos					= Point(0.0f, 3.0f, 0.0f);
				VehicleDesc.mStartPose.mPos					= Point(0.0f, 6.0f, 0.0f);
				VehicleDesc.mDifferential					= DIFFERENTIAL_LS_4WD;
				VehicleDesc.mChassisMass					= 1400.0f;
				VehicleDesc.mChassisMOICoeffY				= 0.8f;
					VehicleDesc.mChassisMOICoeffY			= 1.0f;
				VehicleDesc.mChassisCMOffsetY				= 0.65f;
				VehicleDesc.mChassisCMOffsetZ				= 0.25f;
					VehicleDesc.mChassisCMOffsetY			= 0.0f;
					VehicleDesc.mChassisCMOffsetY			= 1.0f;
					VehicleDesc.mChassisCMOffsetZ			= 0.0f;
				VehicleDesc.mForceApplicationCMOffsetY		= 0.3f;
					VehicleDesc.mForceApplicationCMOffsetY	= 0.0f;
				VehicleDesc.mWheelMass						= 20.0f;
				VehicleDesc.mWheelMaxBrakeTorqueFront		= 150.0f;
				VehicleDesc.mWheelMaxBrakeTorqueRear		= 1500.0f;
				VehicleDesc.mWheelMaxHandBrakeTorqueFront	= 0.0f;
				VehicleDesc.mWheelMaxHandBrakeTorqueRear	= 4000.0f;
				VehicleDesc.mWheelMaxSteerFront				= PI/3.0f;
				VehicleDesc.mWheelMaxSteerRear				= 0.0f;
				VehicleDesc.mWheelSubsteps					= 32;//16;
//				VehicleDesc.mWheelSubsteps					= 3;
//				VehicleDesc.mFrontTireFrictionMultiplier	= 2.0f*2.0f;
//				VehicleDesc.mRearTireFrictionMultiplier		= 1.8f*2.0f;
//					VehicleDesc.mFrontTireFrictionMultiplier	= 10.0f;
//					VehicleDesc.mRearTireFrictionMultiplier		= 10.0f;
					VehicleDesc.mFrontTireFrictionMultiplier= 1.5f;
					VehicleDesc.mRearTireFrictionMultiplier	= 1.5f;
						VehicleDesc.mFrontTireFrictionMultiplier= 5.0f;
//							VehicleDesc.mFrontTireFrictionMultiplier= 4.0f;
//							VehicleDesc.mFrontTireFrictionMultiplier= 3.0f;
						VehicleDesc.mRearTireFrictionMultiplier	= 5.0f;
//							VehicleDesc.mRearTireFrictionMultiplier	= 4.0f;
//							VehicleDesc.mRearTireFrictionMultiplier	= 3.0f;
		//		arb.mStiffness	= 50000.0f;
		//		arb.mStiffness	= desc.mAntirollBarStiffness;
				VehicleDesc.mEnginePeakTorque				= 4.0f*1000.0f;
				VehicleDesc.mEngineMaxOmega					= 4.0f*1000.0f;
					VehicleDesc.mEnginePeakTorque			= 2000.0f;
					VehicleDesc.mEngineMaxOmega				= 2000.0f;
				VehicleDesc.mSuspMaxCompression				= 0.3f;
				VehicleDesc.mSuspMaxDroop					= 0.1f;
//				VehicleDesc.mSuspSpringStrength				= 4.0f*35000.0f;
//				VehicleDesc.mSuspSpringDamperRate			= 4.0f*4500.0f;
					VehicleDesc.mSuspSpringStrength			= 35000.0f;
					VehicleDesc.mSuspSpringDamperRate		= 4500.0f;
				VehicleDesc.mSuspCamberAngleAtRest			= 0.0f;
				VehicleDesc.mSuspCamberAngleAtMaxCompr		= 0.01f;
				VehicleDesc.mSuspCamberAngleAtMaxDroop		= 0.01f;
				VehicleDesc.mGearsSwitchTime				= 0.005f;
					VehicleDesc.mGearsSwitchTime			= 0.0f;
				VehicleDesc.mClutchStrength					= 10.0f;
					VehicleDesc.mClutchStrength				= 100.0f;

				FillDescFromUI(VehicleDesc);


				ASSERT(Wheel0->mType==PINT_SHAPE_CONVEX);
				ASSERT(Wheel1->mType==PINT_SHAPE_CONVEX);
				ASSERT(Wheel2->mType==PINT_SHAPE_CONVEX);
				ASSERT(Wheel3->mType==PINT_SHAPE_CONVEX);
				const PINT_CONVEX_CREATE* ConvexWheel0 = static_cast<const PINT_CONVEX_CREATE*>(Wheel0);
				const PINT_CONVEX_CREATE* ConvexWheel1 = static_cast<const PINT_CONVEX_CREATE*>(Wheel1);
				const PINT_CONVEX_CREATE* ConvexWheel2 = static_cast<const PINT_CONVEX_CREATE*>(Wheel2);
				const PINT_CONVEX_CREATE* ConvexWheel3 = static_cast<const PINT_CONVEX_CREATE*>(Wheel3);

				PINT_CONVEX_CREATE WheelDesc0(ConvexWheel0->mNbVerts, ConvexWheel0->mVerts);
				if(ShowCollisionShapes)
					WheelDesc0.mRenderer	= CreateConvexRenderer(ConvexWheel0->mNbVerts, ConvexWheel0->mVerts);
				VehicleDesc.mWheels[0] = &WheelDesc0;
/*				{
					VehicleDesc.mWheels.mNbVerts	= ConvexWheel0->mNbVerts;
					VehicleDesc.mWheels.mVerts		= ConvexWheel0->mVerts;
					VehicleDesc.mWheels.mRenderer	= null;
					VehicleDesc.mWheels.mRenderer	= CreateConvexRenderer(VehicleDesc.mWheels.mNbVerts, VehicleDesc.mWheels.mVerts);
//					VehicleDesc.mWheels.mRenderer	= ConvexWheel0->mRenderer;
				}*/

				PINT_CONVEX_CREATE WheelDesc1(ConvexWheel1->mNbVerts, ConvexWheel1->mVerts);
				if(ShowCollisionShapes)
				{
					WheelDesc1.mRenderer	= CreateConvexRenderer(ConvexWheel1->mNbVerts, ConvexWheel1->mVerts);
//					VehicleDesc.mWheels[2] = &WheelDesc1;
					VehicleDesc.mWheels[1] = &WheelDesc1;
				}

				PINT_CONVEX_CREATE WheelDesc2(ConvexWheel2->mNbVerts, ConvexWheel2->mVerts);
				if(ShowCollisionShapes)
				{
					WheelDesc2.mRenderer	= CreateConvexRenderer(ConvexWheel2->mNbVerts, ConvexWheel2->mVerts);
//					VehicleDesc.mWheels[3] = &WheelDesc2;
					VehicleDesc.mWheels[2] = &WheelDesc2;
				}

				PINT_CONVEX_CREATE WheelDesc3(ConvexWheel3->mNbVerts, ConvexWheel3->mVerts);
				if(ShowCollisionShapes)
				{
					WheelDesc3.mRenderer	= CreateConvexRenderer(ConvexWheel3->mNbVerts, ConvexWheel3->mVerts);
//					VehicleDesc.mWheels[1] = &WheelDesc3;
					VehicleDesc.mWheels[3] = &WheelDesc3;
				}

/*				FILE* fp = fopen("D:\\tmp\\wheel_verts.bin", "wb");
				if(fp)
				{
					fwrite(VehicleDesc.mWheel.mVerts, VehicleDesc.mWheel.mNbVerts*sizeof(Point), 1, fp);
					fclose(fp);
				}*/

				// We must create the wheels in this order:
				// eFRONT_LEFT=0,
				// eFRONT_RIGHT,
				// eREAR_LEFT,
				// eREAR_RIGHT
				// Forward is along Z so front is +z, left is +x

//				VehicleDesc.mWheelOffset[0] = Point( ChassisExtents.x*CoeffX, -WheelRadius,  ChassisExtents.z*CoeffZ);
//				VehicleDesc.mWheelOffset[1] = Point(-ChassisExtents.x*CoeffX, -WheelRadius,  ChassisExtents.z*CoeffZ);
//				VehicleDesc.mWheelOffset[2] = Point( ChassisExtents.x*CoeffX, -WheelRadius, -ChassisExtents.z*CoeffZ);
//				VehicleDesc.mWheelOffset[3] = Point(-ChassisExtents.x*CoeffX, -WheelRadius, -ChassisExtents.z*CoeffZ);
/*++
-+
+-
--
0
3
1
2*/

/*
				struct Local
				{
					static Point FindWheelOffset(const Point& pt0, const Point& pt1, const Point& pt2, const Point& pt3, bool plusx, bool plusz)
					{
					}
				}*/

//				VehicleDesc.mWheelOffset[0] = Wheel0->mLocalPos * Scale;
//				VehicleDesc.mWheelOffset[1] = Wheel3->mLocalPos * Scale;
//				VehicleDesc.mWheelOffset[2] = Wheel1->mLocalPos * Scale;
//				VehicleDesc.mWheelOffset[3] = Wheel2->mLocalPos * Scale;
				VehicleDesc.mWheelOffset[0] = Wheel0->mLocalPos * Scale;
				VehicleDesc.mWheelOffset[1] = Wheel1->mLocalPos * Scale;
				VehicleDesc.mWheelOffset[2] = Wheel2->mLocalPos * Scale;
				VehicleDesc.mWheelOffset[3] = Wheel3->mLocalPos * Scale;

				if(!ShowCollisionShapes)
				{
					VehicleDesc.mWheels[0] = Wheel0;
					VehicleDesc.mWheels[1] = Wheel1;
					VehicleDesc.mWheels[2] = Wheel2;
					VehicleDesc.mWheels[3] = Wheel3;
				}

				ASSERT(Chassis->mType==PINT_SHAPE_CONVEX);
				const PINT_CONVEX_CREATE* ConvexChassis = static_cast<const PINT_CONVEX_CREATE*>(Chassis);

//				AABB ChassisBox;
//				ChassisBox.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), ChassisExtents);
//				Point ChassisPts[8];
//				ChassisBox.ComputePoints(ChassisPts);

				VehicleDesc.mChassisLocalPose.mPos	= ConvexChassis->mLocalPos * Scale;
				VehicleDesc.mChassisLocalPose.mRot	= ConvexChassis->mLocalRot;

				PINT_CONVEX_CREATE ChassisDesc(ConvexChassis->mNbVerts, ConvexChassis->mVerts);
				VehicleDesc.mChassis	= &ChassisDesc;

				if(ShowCollisionShapes)
				{
					ChassisDesc.mRenderer	= CreateConvexRenderer(ConvexChassis->mNbVerts, ConvexChassis->mVerts);
				}
				else
				{
					ChassisDesc.mRenderer	= ConvexChassis->mRenderer;
				}
//				VehicleDesc.mChassis	= &ChassisDesc;

				VehicleSDKData* VehicleData = GetVehicleData(pint);

				const PintVehicleHandle VehicleHandle = API->CreateVehicle(VehicleData->mData, VehicleDesc);
				VehicleData->mVehicle = VehicleHandle;

				/*if(0)
				{
					VehicleDesc.mStartPose.mPos.z += 20.0f;
					const PintVehicleHandle VehicleHandle2 = API->CreateVehicle(VD, VehicleDesc);
				}*/

				if(0)
				{
					PINT_MATERIAL_CREATE Zero(0.0f, 0.0f, 0.0f);
					//PINT_MATERIAL_CREATE Zero(1.0f, 1.0f, 0.0f);

					PINT_CAPSULE_CREATE Desc(0.5f, 0.7f);
					//PINT_CAPSULE_CREATE Desc(0.5f, 1.0f);
					Matrix3x3 M;
					M.RotZ(HALFPI);
					Desc.mLocalRot	= M;
					if(ShowCollisionShapes)
						Desc.mRenderer	= CreateCapsuleRenderer(Desc.mRadius, Desc.mHalfHeight*2.0f);
					Desc.mMaterial	= &Zero;
					Desc.mLocalPos	= Point(0.0f, VehicleDesc.mWheelOffset[0].y+VehicleDesc.mSuspMaxCompression, VehicleDesc.mWheelOffset[0].z);
					bool b0 = API->AddShape(VehicleHandle, Desc);
					Desc.mLocalPos	= Point(0.0f, VehicleDesc.mWheelOffset[2].y+VehicleDesc.mSuspMaxCompression, VehicleDesc.mWheelOffset[2].z);
					bool b1 = API->AddShape(VehicleHandle, Desc);
				}
			}
		}

		return true;
	}
};

///////////////////////////////////////////////////////////////////////////////

#define DRIVABLE_LEGO_CAR(name, zb2, pvf, scene)									\
class name : public LegoVehicle														\
{																					\
	public:																			\
							name() : LegoVehicle(scene)	{ mVehicleFile = zb2;	}	\
	virtual					~name()						{}							\
	virtual	const char*		GetName()			const	{ return #name;			}	\
	virtual	const char*		GetDescription()	const	{ return gDesc_##name;	}	\
	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)							\
	{																				\
		IceTabControl* tc = LegoVehicle::InitUI(helper);							\
		if(pvf)																		\
		{																			\
			const char* VehicleFile = FindPEELFile(pvf);							\
			if(VehicleFile)															\
				LoadCarData(VehicleFile);											\
		}																			\
		if(mCheckBox_DriveVehicle)													\
			mCheckBox_DriveVehicle->SetChecked(scene!=SCENE_FACTORY);				\
		return tc;																	\
	}																				\
}name;

//mCheckBox_DriveVehicle

static const char* gDesc_LegoVehicleFactory = "Lego Vehicle Factory";
DRIVABLE_LEGO_CAR(LegoVehicleFactory, null, null, SCENE_FACTORY)

static const char* gDesc_LegoStuntTruckVehicle = "Lego Stunt Truck Vehicle";
DRIVABLE_LEGO_CAR(LegoStuntTruckVehicle, "Stunt_Truck_Vehicle.zb2", "StuntTruck.pvf", SCENE_PLAYGROUND_LOGS)

static const char* gDesc_Lego4x4 = "Lego 4x4 Vehicle";
DRIVABLE_LEGO_CAR(Lego4x4, "4x4_Vehicle.zb2", "4x4.pvf", SCENE_PLAYGROUND_BRIDGE)

static const char* gDesc_LegoPolicePursuitTruck = "Lego Police Pursuit Truck Vehicle";
DRIVABLE_LEGO_CAR(LegoPolicePursuitTruck, "PolicePursuit_Truck_Vehicle.zb2", "PursuitTruck.pvf", SCENE_PLAYGROUND_BRIDGE)

//static const char* gDesc_NX_Vehicle = "NX Vehicle";
//DRIVABLE_LEGO_CAR(NX_Vehicle, "NX_Vehicle.zb2", "4x4.pvf")

static const char* gDesc_LegoSpeedChampions_FordF150 = "Lego Speed Champions - Ford F-150 Raptor";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_FordF150, "Ford_F150_Vehicle.zb2", "StuntTruck.pvf", SCENE_PLAYGROUND)

static const char* gDesc_LegoSpeedChampions_F40 = "Lego Speed Champions - Ferrari F40";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_F40, "Ferrari_F40_Vehicle.zb2", "F40.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_F8 = "Lego Speed Champions - Ferrari F8 Tributo";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_F8, "Ferrari_F8_Tributo_Vehicle.zb2", "F40.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_SF16 = "Lego Speed Champions - Scuderia Ferrari SF16-H";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_SF16, "Scuderia_Ferrari_SF16H_Vehicle.zb2", "HighSpeedChase.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_McLarenSenna = "Lego Speed Champions - McLaren Senna";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_McLarenSenna, "McLaren_Senna_Vehicle.zb2", "F40.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_McLarenP1 = "Lego Speed Champions - McLaren P1";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_McLarenP1, "McLaren_P1_Vehicle.zb2", "F40.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_458 = "Lego Speed Champions - Ferrari 458 Italia GT2";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_458, "Ferrari_458_GT2_Vehicle.zb2", "458.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_488 = "Lego Speed Champions - Ferrari 488 GT3 Scuderia Corsa";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_488, "Ferrari_488_GT3_Vehicle.zb2", "458.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_FordFiesta = "Lego Speed Champions - Ford Fiesta M-Sport WRC";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_FordFiesta, "FordFiesta_Vehicle.zb2", "SportCar3.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_Mustang = "Lego Speed Champions - 1968 Ford Mustang Fastback";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_Mustang, "1968_Ford_Mustang_Fastback_Vehicle.zb2", "Mustang1968.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_Camaro = "Lego Speed Champions - Chevrolet Camaro ZL1";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_Camaro, "Chevrolet_Camaro_ZL1_Vehicle.zb2", "Camaro.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_Camaro2 = "Lego Speed Champions - Chevrolet Camaro Drag Race";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_Camaro2, "Chevrolet_Camaro_Drag_Race_Vehicle.zb2", "Camaro.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_DodgeCharger = "Lego Speed Champions - 1970 Dodge Charger RT";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_DodgeCharger, "1970_Dodge_Charger_RT_Vehicle.zb2", "DodgeCharger.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_DodgeChallenger = "Lego Speed Champions - 2018 Dodge Challenger SRT Demon";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_DodgeChallenger, "2018_Dodge_Challenger_SRT_Demon_Vehicle.zb2", "DodgeChallenger.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_911 = "Lego Speed Champions - 1974 Porsche 911";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_911, "1974_Porsche_911_Vehicle.zb2", "SportCar3.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_917 = "Lego Speed Champions - Porsche 917K";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_917, "Porsche_917_Vehicle.zb2", "SportCar3.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_919 = "Lego Speed Champions - Porsche 919 Hybrid";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_919, "Porsche_919_Hybrid_Vehicle.zb2", "Racer.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_FordGT40 = "Lego Speed Champions - 1966 Ford GT40";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_FordGT40, "1966_Ford_GT40_Vehicle.zb2", "FordGT40.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_FordGT = "Lego Speed Champions - 2016 Ford GT";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_FordGT, "2016_Ford_GT_Vehicle.zb2", "SportCar3.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoSpeedChampions_MercedesAMG = "Lego Speed Champions - Mercedes AMG GT3";
DRIVABLE_LEGO_CAR(LegoSpeedChampions_MercedesAMG, "Mercedes_AMG_GT3_Vehicle.zb2", "SportCar3.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoMadMax = "Lego - Mad Max Road Warrior";
DRIVABLE_LEGO_CAR(LegoMadMax, "MadMax_Vehicle.zb2", "SportCar3.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoDesertVehicle = "Lego Desert Vehicle";
DRIVABLE_LEGO_CAR(LegoDesertVehicle, "Desert_Vehicle.zb2", "DesertVehicle3.pvf", SCENE_PLAYGROUND_BRIDGE)

static const char* gDesc_LegoOffRoaderVehicle = "Lego Off-Roader Vehicle";
DRIVABLE_LEGO_CAR(LegoOffRoaderVehicle, "OffRoader_Vehicle.zb2", "OffRoader.pvf", SCENE_PLAYGROUND)

static const char* gDesc_LegoRacerVehicle = "Lego Racer Vehicle";
DRIVABLE_LEGO_CAR(LegoRacerVehicle, "Racer_Vehicle.zb2", "Racer.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoHighSpeedChase1 = "Lego - High Speed Chase Vehicle 1";
DRIVABLE_LEGO_CAR(LegoHighSpeedChase1, "HighSpeedChase_1_Vehicle.zb2", "HighSpeedChase.pvf", SCENE_RACE_TRACK)

static const char* gDesc_LegoHighSpeedChase2 = "Lego - High Speed Chase Vehicle 2";
DRIVABLE_LEGO_CAR(LegoHighSpeedChase2, "HighSpeedChase_2_Vehicle.zb2", "HighSpeedChase.pvf", SCENE_RACE_TRACK)

static const char* gDesc_Lego_MOC_Corvette = "Lego - MOC Corvette";
DRIVABLE_LEGO_CAR(Lego_MOC_Corvette, "MOC_Corvette_Vehicle.zb2", "SportCar3.pvf", SCENE_RACE_TRACK)

static const char* gDesc_Lego_MOC_Tentazione = "Lego - MOC Lumenairo 919 Tentazione";
DRIVABLE_LEGO_CAR(Lego_MOC_Tentazione, "MOC_Tentazione_Vehicle.zb2", "SportCar3.pvf", SCENE_RACE_TRACK)

static const char* gDesc_Lego_MOC_Tempesta = "Lego - MOC Lumenairo 728 Tempesta";
DRIVABLE_LEGO_CAR(Lego_MOC_Tempesta, "MOC_Tempesta_Vehicle.zb2", "SportCar3.pvf", SCENE_RACE_TRACK)

static const char* gDesc_Lego_Lamborghini_Centenario_Brown = "Lego - Lamborghini Centenario (Brown)";
DRIVABLE_LEGO_CAR(Lego_Lamborghini_Centenario_Brown, "Lamborghini_Centenario_(Brown)_Vehicle.zb2", "SportCar3.pvf", SCENE_RACE_TRACK)

static const char* gDesc_Lego_Lamborghini_Centenario_Blue = "Lego - Lamborghini Centenario (Blue)";
DRIVABLE_LEGO_CAR(Lego_Lamborghini_Centenario_Blue, "Lamborghini_Centenario_(Blue)_Vehicle.zb2", "SportCar3.pvf", SCENE_RACE_TRACK)

static const char* gDesc_Lego_Buggy = "Lego - Buggy";
DRIVABLE_LEGO_CAR(Lego_Buggy, "Buggy_Vehicle.zb2", "Buggy.pvf", SCENE_RACE_TRACK)

///////////////////////////////////////////////////////////////////////////////

