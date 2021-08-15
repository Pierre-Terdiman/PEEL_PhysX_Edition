///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Cylinder.h"
#include "PintShapeRenderer.h"
#include "TestScenes_VehiclesBase.h"
#include "TestScenesHelpers.h"
#include "PintObjectsManager.h"
#include "Loader_Bin.h"
#include "ProceduralTrack.h"
#include "MyConvex.h"
#include "GUI_Helpers.h"

///////////////////////////////////////////////////////////////////////////////

//#define TEST_ROD_OBJECT

static const bool gDriveVehicle = true;

namespace
{
/*	class PintCustomWheelRenderer : public PintShapeRenderer
	{
		public:

							PintCustomWheelRenderer()	{}
		virtual				~PintCustomWheelRenderer()	{}

		virtual	void		Render(const PR& pose)						{}
	};

	struct VehicleRender
	{
		VehicleRender() : mRenderer(null)	{}
		PintCustomWheelRenderer*	mRenderer;
	};*/

	struct WheelData : public Allocateable
	{
		WheelData() : mParent(null), mWheel(null)
		{
		}

		PintActorHandle	mParent;
		PintActorHandle	mWheel;
	};

	struct ChassisData : public Allocateable
	{
		ChassisData() : mChassis(null)//, mNbWheels(0)
		{
			for(udword i=0;i<2;i++)
				mFrontAxleObject[i] = null;
#ifdef TEST_ROD_OBJECT
			mRod = null;
#endif
		}
		PintActorHandle	mChassis;
//		udword				mNbWheels;
		WheelData			mFront[2];
		WheelData			mRear[2];
		PintActorHandle	mFrontAxleObject[2];
#ifdef TEST_ROD_OBJECT
		PintActorHandle	mRod;
#endif
	};

	// Experimental drive modes
	enum DriveMode
	{
		DRIVE_MODE_NONE,
		DRIVE_MODE_SET_ANGULAR_VELOCITY,
		DRIVE_MODE_ADD_TORQUE,
		DRIVE_MODE_MOTOR,
	};
//	const DriveMode DM = DRIVE_MODE_MOTOR;

	class VehicleBase : public VehicleInput
	{
		public:
									VehicleBase();
		virtual						~VehicleBase();

		virtual	void				Close(Pint& pint);
		virtual	udword				Update(Pint& pint, float dt);
		virtual	bool				CommonSetup();

//				PlanarMeshHelper	mPMH;
//				VehicleRender		mRender;
				DriveMode			mDriveMode;
				float				mAcceleration;
				float				mMaxAngularVelocity;
				float				mSteeringForce;
				//
				bool				mClampAngularVelocity;
				bool				mLoadTerrain;

				void				CreateTestScene(Pint& pint, const PINT_MATERIAL_CREATE* material);
				bool				ClampAngularVelocity(Pint& pint, const ChassisData& vehicle_data);
	};
}

VehicleBase::VehicleBase() :
	mDriveMode				(DRIVE_MODE_NONE),
	mAcceleration			(0.0f),
	mMaxAngularVelocity		(0.0f),
	mSteeringForce			(0.0f),
	mClampAngularVelocity	(false),
	mLoadTerrain			(false)
{
}

VehicleBase::~VehicleBase()
{
}

void VehicleBase::Close(Pint& pint)
{
	ChassisData* UserData = (ChassisData*)pint.mUserData;
	DELETESINGLE(UserData);
	pint.mUserData = null;

	VehicleInput::Close(pint);
}

bool VehicleBase::CommonSetup()
{
	VehicleInput::CommonSetup();
	if(mLoadTerrain)
	{
		LoadMeshesFromFile_(*this, "Terrain.bin", null, false, 0);
	}

	mPlanarMeshHelper.Generate(200, 0.1f);

	return true;
}

bool VehicleBase::ClampAngularVelocity(Pint& pint, const ChassisData& vehicle_data)
{
	bool CanAccelerate = true;
	if(mClampAngularVelocity)
	{
		const float MaxAngularVelocity = mMaxAngularVelocity;
		for(udword i=0;i<2;i++)
		{
			if(vehicle_data.mFront[i].mWheel && vehicle_data.mFront[i].mParent)
			{
//				Point AngularVelocity = pint.GetAngularVelocity(vehicle_data.mFront[i].mWheel);
//				printf("Angular velocity %d: %f %f %f\n", i, AngularVelocity.x, AngularVelocity.y, AngularVelocity.z);

				if(::ClampAngularVelocity(pint, vehicle_data.mFront[i].mParent, vehicle_data.mFront[i].mWheel, MaxAngularVelocity))
					CanAccelerate = false;
			}
		}
		for(udword i=0;i<2;i++)
		{
			if(vehicle_data.mRear[i].mWheel && vehicle_data.mRear[i].mParent)
			{
//				Point AngularVelocity = pint.GetAngularVelocity(vehicle_data.mRear[i].mWheel);
//				printf("Angular velocity %d: %f %f %f\n", i, AngularVelocity.x, AngularVelocity.y, AngularVelocity.z);

				if(::ClampAngularVelocity(pint, vehicle_data.mRear[i].mParent, vehicle_data.mRear[i].mWheel, MaxAngularVelocity))
					CanAccelerate = false;
			}
		}
	}
	return CanAccelerate;
}

udword VehicleBase::Update(Pint& pint, float dt)
{
	if(!gDriveVehicle)
		return 0;

	ChassisData* UserData = (ChassisData*)pint.mUserData;
	if(!UserData)
		return 0;

	if(UserData->mFrontAxleObject[0] && UserData->mFrontAxleObject[1])
	{
		const float Steering = mSteeringForce;
#ifdef TEST_ROD_OBJECT
		const PR Pose = pint.GetWorldTransform(UserData->mRod);
		const Matrix3x3 M(Pose.mRot);
#endif
		if(mInput.mKeyboard_Right)
		{
#ifdef TEST_ROD_OBJECT
			pint.AddWorldImpulseAtWorldPos(UserData->mRod, -M[2]*Steering, Pose.mPos);
#else
			pint.AddLocalTorque(UserData->mFrontAxleObject[0], Point(0.0f, -Steering, 0.0f));
			pint.AddLocalTorque(UserData->mFrontAxleObject[1], Point(0.0f, -Steering, 0.0f));
#endif
		}
		if(mInput.mKeyboard_Left)
		{
#ifdef TEST_ROD_OBJECT
			pint.AddWorldImpulseAtWorldPos(UserData->mRod, M[2]*Steering, Pose.mPos);
#else
			pint.AddLocalTorque(UserData->mFrontAxleObject[0], Point(0.0f, Steering, 0.0f));
			pint.AddLocalTorque(UserData->mFrontAxleObject[1], Point(0.0f, Steering, 0.0f));
#endif
		}
	}

	bool CanAccelerate = ClampAngularVelocity(pint, *UserData);

	const bool FWD = true;
	const bool RWD = true;

	if(mDriveMode==DRIVE_MODE_SET_ANGULAR_VELOCITY)
	{
		const float Coeff = mMaxAngularVelocity;
		{
			if(mInput.mKeyboard_Accelerate)
			{
				if(FWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mFront[i].mWheel)
							pint.SetAngularVelocity(UserData->mFront[i].mWheel, Point(0.0f, 0.0f, -Coeff));
					}
				}
				if(RWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mRear[i].mWheel)
							pint.SetAngularVelocity(UserData->mRear[i].mWheel, Point(0.0f, 0.0f, -Coeff));
					}
				}
			}
			if(mInput.mKeyboard_Brake)
			{
				if(FWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mFront[i].mWheel)
							pint.SetAngularVelocity(UserData->mFront[i].mWheel, Point(0.0f, 0.0f, 0.0f));
					}
				}
				if(RWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mRear[i].mWheel)
							pint.SetAngularVelocity(UserData->mRear[i].mWheel, Point(0.0f, 0.0f, 0.0f));
					}
				}
			}
		}
	}
	
	if(mDriveMode==DRIVE_MODE_ADD_TORQUE)
	{
		const float Coeff = mAcceleration;
		if(mInput.mKeyboard_Accelerate /*&& CanAccelerate*/)
		{
			if(FWD)
			{
				for(udword i=0;i<2;i++)
				{
					if(UserData->mFront[i].mWheel)
						pint.AddLocalTorque(UserData->mFront[i].mWheel, Point(0.0f, 0.0f, -Coeff));
				}
			}
			if(RWD)
			{
				for(udword i=0;i<2;i++)
				{
					if(UserData->mRear[i].mWheel)
						pint.AddLocalTorque(UserData->mRear[i].mWheel, Point(0.0f, 0.0f, -Coeff));
				}
			}
		}
		if(mInput.mKeyboard_Brake)
		{
			if(FWD)
			{
				for(udword i=0;i<2;i++)
				{
					if(UserData->mFront[i].mWheel)
						pint.AddLocalTorque(UserData->mFront[i].mWheel, Point(0.0f, 0.0f, Coeff));
				}
			}
			if(RWD)
			{
				for(udword i=0;i<2;i++)
				{
					if(UserData->mRear[i].mWheel)
						pint.AddLocalTorque(UserData->mRear[i].mWheel, Point(0.0f, 0.0f, Coeff));
				}
			}
		}
	}

	if(mDriveMode==DRIVE_MODE_MOTOR)
	{
		struct Local
		{
			static void UpdateMotorData(Pint& pint, PintActorHandle h, float velocity, bool use_motor=true)
			{
				PINT_ARTICULATED_MOTOR_CREATE Motor;
				Motor.mExternalCompliance = 1.0f;
				Motor.mInternalCompliance = 1.0f;
				Motor.mDamping = use_motor ? 1000.0f : 0.0f;
				Motor.mStiffness = 0.0f;
				Motor.mTargetVelocity = Point(velocity, 0.0f, 0.0f);
				pint.SetArticulatedMotor(h, Motor);
			}
		};

		if(0)
		{
			static float TargetVelocity = 0.0f;
			const float Acceleration = dt * 10.0f;
			if(!mInput.mKeyboard_Accelerate && !mInput.mKeyboard_Brake)
			{
	//			TargetVelocity -= Acceleration;
				TargetVelocity *= 0.99f;
			}
			else if(mInput.mKeyboard_Accelerate)
				TargetVelocity += Acceleration;
			else if(mInput.mKeyboard_Brake)
				TargetVelocity -= Acceleration;

			if(TargetVelocity>mMaxAngularVelocity)
				TargetVelocity = mMaxAngularVelocity;
			if(TargetVelocity<-mMaxAngularVelocity)
				TargetVelocity = -mMaxAngularVelocity;
	//		printf("TargetVelocity: %f\n", TargetVelocity);

			const float Coeff = TargetVelocity;
			{
				if(FWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mFront[i].mWheel)
							Local::UpdateMotorData(pint, UserData->mFront[i].mWheel, -Coeff);
					}
				}
				if(RWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mRear[i].mWheel)
							Local::UpdateMotorData(pint, UserData->mRear[i].mWheel, -Coeff);
					}
				}
			}
		}
		else
		{
			static float TargetVelocity = 0.0f;
			const float Acceleration = dt * 10.0f;
			bool UseMotor = false;
			if(!mInput.mKeyboard_Accelerate && !mInput.mKeyboard_Brake)
			{
				TargetVelocity *= 0.99f;
			}
			else if(mInput.mKeyboard_Accelerate)
			{
				TargetVelocity += Acceleration;
				UseMotor = true;
			}
			else if(mInput.mKeyboard_Brake)
			{
				TargetVelocity -= Acceleration;
				UseMotor = true;
			}

			if(TargetVelocity>mMaxAngularVelocity)
				TargetVelocity = mMaxAngularVelocity;
			if(TargetVelocity<-mMaxAngularVelocity)
				TargetVelocity = -mMaxAngularVelocity;
	//		printf("TargetVelocity: %f\n", TargetVelocity);

			const float Coeff = TargetVelocity;
			{
				if(FWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mFront[i].mWheel)
							Local::UpdateMotorData(pint, UserData->mFront[i].mWheel, -Coeff, UseMotor);
					}
				}
				if(RWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mRear[i].mWheel)
							Local::UpdateMotorData(pint, UserData->mRear[i].mWheel, -Coeff, UseMotor);
					}
				}
			}
		}

#ifdef REMOVED
//		const float Coeff = mMaxAngularVelocity;
		const float Coeff = TargetVelocity;
		if(mInput.mAccelerate /*&& CanAccelerate*/)
		{
//			mInput.mAccelerate = false;
			if(FWD)
			{
				for(udword i=0;i<2;i++)
				{
					if(UserData->mFront[i].mWheel)
						Local::UpdateMotorData(pint, UserData->mFront[i].mWheel, -Coeff);
				}
			}
			if(RWD)
			{
				for(udword i=0;i<2;i++)
				{
					if(UserData->mRear[i].mWheel)
						Local::UpdateMotorData(pint, UserData->mRear[i].mWheel, -Coeff);
				}
			}
		}

		if(mInput.mBrake)
		{
			if(FWD)
			{
				for(udword i=0;i<2;i++)
				{
					if(UserData->mFront[i].mWheel)
//						Local::UpdateMotorData(pint, UserData->mFront[i].mWheel, 0.0f);
						Local::UpdateMotorData(pint, UserData->mFront[i].mWheel, -Coeff);
				}
			}
			if(RWD)
			{
				for(udword i=0;i<2;i++)
				{
					if(UserData->mRear[i].mWheel)
//						Local::UpdateMotorData(pint, UserData->mRear[i].mWheel, 0.0f);
						Local::UpdateMotorData(pint, UserData->mRear[i].mWheel, -Coeff);
				}
			}
		}
#endif
	}

#ifdef REMOVED
	if(0)
	{
		pint.SetWorldTransform(UserData->mRearWheel, pint.GetWorldTransform(UserData->mRearWheel));

		for(udword i=0;i<2;i++)
		{
			const PR AxlePose = pint.GetWorldTransform(UserData->mFrontAxleObject[i]);
			const Matrix3x3 AxleM(AxlePose.mRot);
			const udword UpAxis = 2;
			const udword OtherAxis1 = 0;
			const udword OtherAxis2 = 1;
			const Point Up = AxleM[UpAxis];

			PR WheelPose = pint.GetWorldTransform(UserData->mFront[i].mWheel);
			Matrix3x3 WheelM(WheelPose.mRot);
			WheelM[UpAxis] = Up;

//	vec[i] *= invSqrt; // normalize the first axis
float dotij = WheelM[UpAxis].Dot(WheelM[OtherAxis1]);
float dotik = WheelM[UpAxis].Dot(WheelM[OtherAxis2]);
//	magnitude[i] += PxAbs(dotij) + PxAbs(dotik); // elongate the axis by projection of the other two
WheelM[OtherAxis1] -= WheelM[UpAxis] * dotij;                    // orthogonize the two remaining axii relative to vec[i]
WheelM[OtherAxis2] -= WheelM[UpAxis] * dotik;

//	magnitude[OtherAxis1] = WheelM[OtherAxis1].normalize();
WheelM[OtherAxis1].Normalize();
float dotjk = WheelM[OtherAxis1].Dot(WheelM[OtherAxis2]);
//	magnitude[OtherAxis1] += PxAbs(dotjk); // elongate the axis by projection of the other one
WheelM[OtherAxis2] -= WheelM[OtherAxis1] * dotjk;     // orthogonize vec[k] relative to vec[j]

//	magnitude[OtherAxis2] = WheelM[OtherAxis2].normalize();
WheelM[OtherAxis2].Normalize();

//				WheelPose.mRot = AxlePose.mRot;
			WheelPose.mRot = WheelM;
			WheelPose.mRot.Normalize();
			pint.SetWorldTransform(UserData->mFront[i].mWheel, WheelPose);
		}
	}
#endif

	// Camera
	UpdateCamera(pint, UserData->mChassis);

	return 0;
}

void VehicleBase::CreateTestScene(Pint& pint, const PINT_MATERIAL_CREATE* material)
{
	//##########
/*	if(0)
	{
		IndexedSurface* IS;
		if(GetNbSurfaces())
		{
			IS = GetFirstSurface();
		}
		else
		{
			IS = CreateManagedSurface();

			RaceTrack RT;
			RT.Generate();

			IS->Init(RT.mNbTris, RT.mNbVerts, RT.mVerts, (const IndexedTriangle*)RT.mIndices);

			Point Center;
			IS->GetFace(0)->Center(IS->GetVerts(), Center);

			IS->Translate(-Center);
		}

		PINT_MESH_CREATE MeshDesc;
		MeshDesc.mSurface	= IS->GetSurfaceInterface();
		MeshDesc.mRenderer	= CreateMeshRenderer(MeshDesc.mSurface);
		MeshDesc.mMaterial	= material;

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes		= &MeshDesc;
		ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
		ObjectDesc.mMass		= 0.0f;
		CreatePintObject(pint, ObjectDesc);
		return;
	}*/

	const float Altitude = 0.0f;
	mPlanarMeshHelper.CreatePlanarMesh(pint, Altitude, null);

	if(1)
	{
		const float Spacing = 10.0f;
	//	const float Size = 0.3f;
		const float Size = 0.2f;
	//	const float Size = 1.0f;
		GenerateGroundObstacles(pint, Spacing, Size, false);
	}
}

static void SetupMotor(PINT_ARTICULATED_BODY_CREATE& desc)
{
	desc.mUseMotor = true;
	desc.mMotor.mExternalCompliance = 1.0f;
	desc.mMotor.mInternalCompliance = 1.0f;
	desc.mMotor.mDamping = 1000.0f;
	desc.mMotor.mStiffness = 0.0f;
	desc.mMotor.mTargetVelocity = Point(0.0f, 0.0f, 0.0f);
}

#define START_VEHICLE_TEST(name, category, desc)										\
	class name : public VehicleBase														\
	{																					\
		public:																			\
								name()						{						}	\
		virtual					~name()						{						}	\
		virtual	const char*		GetName()			const	{ return #name;			}	\
		virtual	const char*		GetDescription()	const	{ return desc;			}	\
		virtual	TestCategory	GetCategory()		const	{ return category;		}

static const char* gDesc_Vehicle = "Vehicle";
START_VEHICLE_TEST(Vehicle, CATEGORY_WIP, gDesc_Vehicle)

	virtual	void Vehicle::GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(5.26f, 3.21f, 5.69f), Point(-0.75f, -0.41f, -0.52f));
//		desc.mCamera[0] = PintCameraPose(Point(8.53f, 2.95f, 3.00f), Point(-0.86f, -0.24f, -0.45f));
//		desc.mCamera[0] = PintCameraPose(Point(-2.61f, 2.95f, 7.80f), Point(0.30f, -0.10f, -0.95f));
		SetDefEnv(desc, false);

		mLoadTerrain = false;
	}

	virtual	bool Vehicle::CommonSetup()
	{
		if(gDriveVehicle)
		{
			mDriveMode			= DRIVE_MODE_MOTOR;
//			mDriveMode			= DRIVE_MODE_SET_ANGULAR_VELOCITY;
//			mDriveMode			= DRIVE_MODE_ADD_TORQUE;
		}
		else
			mDriveMode			= DRIVE_MODE_NONE;

		mAcceleration			= 100.0f;
		mMaxAngularVelocity		= 20.0f;
//		mMaxAngularVelocity		= 30.0f;
		mSteeringForce			= 10.0f*100.0f;
//		mSteeringForce			= 500.0f;
#ifdef TEST_ROD_OBJECT
		mSteeringForce			= 5.0f;
#endif
		mCamera.mUpOffset		= 2.0f;
		mCamera.mDistToTarget	= 6.0f;
//		mClampAngularVelocity	= false;
		mClampAngularVelocity	= mDriveMode!=DRIVE_MODE_MOTOR;
		mControlCamera			= gDriveVehicle;
		mLoadTerrain			= false;

		return VehicleBase::CommonSetup();
	}

	virtual bool Vehicle::Setup(Pint& pint, const PintCaps& caps)
	{
//		if(!caps.mSupportMCArticulations || !caps.mSupportRigidBodySimulation)
//			return false;


		if(!caps.mSupportCollisionGroups)
			return false;
		const PintDisabledGroups DG(1, 1);
		pint.SetDisabledGroups(1, &DG);
		PintCollisionGroup GroupChassis = 1;
		PintCollisionGroup GroupWheel = 1;
		PintCollisionGroup GroupStruct = 1;


		const PINT_MATERIAL_CREATE LowFrictionMaterial(0.1f, 0.1f, 0.0f);

		/////

		CreateMeshesFromRegisteredSurfaces(pint, caps, &mHighFrictionMaterial);

		// The high mass is actually important to make the vehicle go straight. With a mass of 1 the vehicle turns on its own as soon as the terrain
		// is not perfectly flat.
		const float ChassisMass = 40.0f;	// 40.0f
		const float WheelMass = 15.0f;	// 15.0f
		const float StructMass = 1.0f;
		const float RearWheelMass = StructMass+WheelMass*2.0f;

		const float y = 4.0f;
//		const float WheelRadius = 0.75f;
		const float WheelRadius = 0.5f;
//		const float WheelWidth = 0.2f;
//		const float WheelWidth = 0.3f;
		const float WheelWidth = 0.6f;
		const float WheelOffset = 0.2f;
//		const float WheelWidth = 0.4f;
		const Point Chassis(1.5f, 0.5f, 1.0f);
//		const Point Chassis(1.5f, 0.1f, 1.0f);
		const Point Axle(0.1f, 0.1f, Chassis.z + WheelOffset);
		/*const*/ Point CarPos(0.0f, y + Chassis.y, 0.0f);
		const bool UseArticulations = true;
		const bool UseSphereWheels = false;
		const bool UseMotor = mDriveMode==DRIVE_MODE_MOTOR;

		/////

	//	const udword NbPts = 64;
//		const udword NbPts = 48;
		const udword NbPts = 60;
//		const udword NbPts = 6;
//		const udword NbPts = 100;
//		const udword NbPts = 16;
		const CylinderMesh Cylinder(NbPts, WheelRadius, WheelWidth*0.5f);
		const udword TotalNbVerts = Cylinder.mNbVerts;
		const Point* Verts = Cylinder.mVerts;

		/////

		ChassisData* UserData = ICE_NEW(ChassisData);
		pint.mUserData = UserData;

		//for(udword k=0;k<2;k++)
		udword k = 1;
		{
//		PintArticHandle Articulation = UseArticulations ? pint.CreateArticulation() : null;
		PintArticHandle Articulation = k ? pint.CreateArticulation(PINT_ARTICULATION_CREATE()) : null;
		CarPos.z = float(k) * 4.0f;

		PintActorHandle ChassisObject;
		{
			PINT_BOX_CREATE BoxDesc(Chassis.x, Chassis.y, Chassis.z*0.6f);
//			BoxDesc.mExtents	= Chassis;
//			BoxDesc.mExtents	= Point(Chassis.x, Chassis.y, Chassis.z*0.75f);
//			BoxDesc.mExtents	= Point(Chassis.x, Chassis.y, Chassis.z*0.6f);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
//			BoxDesc.mRenderer	= CreateBoxRenderer(Point(0.0f, 0.0f, 0.0f));

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mPosition		= CarPos;
			ObjectDesc.mShapes			= &BoxDesc;
			ObjectDesc.mMass			= ChassisMass;
			ObjectDesc.mCOMLocalOffset	= Point(0.0f, -0.5f, 0.0f);
			ObjectDesc.mCOMLocalOffset	= Point(0.0f, -1.0f, 0.0f);
//			ObjectDesc.mCOMLocalOffset	= Point(0.0f, -2.0f, 0.0f);
			ObjectDesc.mCollisionGroup	= GroupChassis;

			if(Articulation)
			{
				PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ChassisObject = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
			}
			else
				ChassisObject = CreatePintObject(pint, ObjectDesc);

			if(0)
			{
				BoxDesc.mExtents	= Point(Chassis.x, 0.5f, Chassis.z*0.6f);
				BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

				PINT_OBJECT_CREATE ObjectDesc;
				ObjectDesc.mPosition	= CarPos + Point(0.0f, Chassis.y+BoxDesc.mExtents.y, 0.0f);
				ObjectDesc.mShapes		= &BoxDesc;
				ObjectDesc.mMass		= 4.0f;
				PintActorHandle h = CreatePintObject(pint, ObjectDesc);

				PINT_PRISMATIC_JOINT_CREATE Desc;
				Desc.mObject0			= h;
				Desc.mObject1			= ChassisObject;
				//###PRISMATIC2
				Desc.mLocalPivot0.mPos	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalAxis0		= Point(0.0f, 1.0f, 0.0f);
				Desc.mLocalAxis1		= Point(0.0f, 1.0f, 0.0f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
			}
		}
		UserData->mChassis = ChassisObject;

		PintActorHandle AxleObject;
		const Point AxleOffset = Point(-Chassis.x, -Chassis.y - Axle.y, 0.0f);
		{
			PINT_BOX_CREATE BoxDesc(Axle);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			const Point AxlePos = CarPos + AxleOffset;
			PINT_OBJECT_CREATE AxleDesc;
			AxleDesc.mPosition			= AxlePos;
			AxleDesc.mShapes			= &BoxDesc;
			AxleDesc.mMass				= RearWheelMass;
			AxleDesc.mCollisionGroup	= GroupStruct;	//### Technically we'd need 3 groups here

			PINT_CONVEX_CREATE WheelDesc;
			PINT_CONVEX_CREATE WheelDesc2;
			PINT_SPHERE_CREATE SphereWheelDesc;
			PINT_SPHERE_CREATE SphereWheelDesc2;
			if(1)
			{
				WheelDesc.mMaterial			= &mHighFrictionMaterial;
				WheelDesc2.mMaterial		= &mHighFrictionMaterial;
				SphereWheelDesc.mMaterial	= &mHighFrictionMaterial;
				SphereWheelDesc2.mMaterial	= &mHighFrictionMaterial;
			}
			else
			{
				WheelDesc.mMaterial			= &LowFrictionMaterial;
				WheelDesc2.mMaterial		= &LowFrictionMaterial;
				SphereWheelDesc.mMaterial	= &LowFrictionMaterial;
				SphereWheelDesc2.mMaterial	= &LowFrictionMaterial;
			}
			if(UseSphereWheels)
			{
				SphereWheelDesc.mRadius		= WheelRadius;
				SphereWheelDesc.mRenderer	= CreateSphereRenderer(WheelRadius);
				SphereWheelDesc.mLocalPos	= Point(0.0f, 0.0f, Axle.z);
				BoxDesc.mNext = &SphereWheelDesc;

				SphereWheelDesc2.mRadius	= WheelRadius;
				SphereWheelDesc2.mRenderer	= CreateSphereRenderer(WheelRadius);
				SphereWheelDesc2.mLocalPos	= Point(0.0f, 0.0f, -Axle.z);
				SphereWheelDesc.mNext = &SphereWheelDesc2;
			}
			else
			{
				WheelDesc.mNbVerts	= TotalNbVerts;
				WheelDesc.mVerts	= Verts;
				WheelDesc.mRenderer	= CreateConvexRenderer(WheelDesc.mNbVerts, WheelDesc.mVerts);
				WheelDesc.mLocalPos	= Point(0.0f, 0.0f, Axle.z);
				BoxDesc.mNext = &WheelDesc;

				WheelDesc2.mNbVerts		= TotalNbVerts;
				WheelDesc2.mVerts		= Verts;
				WheelDesc2.mRenderer	= CreateConvexRenderer(WheelDesc2.mNbVerts, WheelDesc2.mVerts);
				WheelDesc2.mLocalPos	= Point(0.0f, 0.0f, -Axle.z);
				WheelDesc.mNext = &WheelDesc2;
			}

			if(Articulation)
			{
				PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = ChassisObject;
				ArticulatedDesc.mLocalPivot0 = AxleOffset;
				ArticulatedDesc.mLocalPivot1 = Point(0.0f, 0.0f, 0.0f);
				ArticulatedDesc.mX = Point(0.0f, 0.0f, 1.0f);
				ArticulatedDesc.mEnableTwistLimit = false;
				ArticulatedDesc.mTwistLowerLimit = -0.01f;
				ArticulatedDesc.mTwistUpperLimit = 0.01f;
				ArticulatedDesc.mEnableSwingLimit = true;
				ArticulatedDesc.mSwingYLimit = 0.001f;//PI/6.0f;
				ArticulatedDesc.mSwingZLimit = 0.001f;//PI/6.0f;
				if(UseMotor)
					SetupMotor(ArticulatedDesc);
				AxleObject = pint.CreateArticulatedObject(AxleDesc, ArticulatedDesc, Articulation);
			}
			else
			{
				AxleObject = CreatePintObject(pint, AxleDesc);

				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mObject0		= ChassisObject;
				Desc.mObject1		= AxleObject;
				Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
				Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
				Desc.mLocalPivot0	= AxleOffset;
				Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
			UserData->mRear[0].mWheel = AxleObject;
			UserData->mRear[0].mParent = ChassisObject;
		}
		
		///
		Point WheelPt[2];
		Point WheelGlobalPt[2];
		PintActorHandle FrontAxleObject[2];
		const Point FrontAxle = Point(0.1f, 0.1f, WheelOffset);
		const float z2 = FrontAxle.z*0.5f;
		for(udword i=0;i<2;i++)
		{
			const float Coeff = i ? -1.0f : 1.0f;
			const Point FrontAxleOffset = Point(Chassis.x, -Chassis.y - Axle.y, Coeff*Chassis.z);
			{
				PINT_BOX_CREATE BoxDesc(FrontAxle);
//				BoxDesc.mExtents	= Axle;
				BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

				const Point AxlePos = CarPos + FrontAxleOffset;
				PINT_OBJECT_CREATE AxleDesc;
				AxleDesc.mPosition			= AxlePos;
				AxleDesc.mShapes			= &BoxDesc;
				AxleDesc.mMass				= StructMass;
				AxleDesc.mCollisionGroup	= GroupStruct;

				PINT_BOX_CREATE BoxDesc2(0.3f, FrontAxle.y*0.5f, z2);
				BoxDesc2.mRenderer	= CreateBoxRenderer(BoxDesc2.mExtents);
				BoxDesc2.mLocalPos	= Point(-BoxDesc.mExtents.x - BoxDesc2.mExtents.x, 0.0f, -Coeff*FrontAxle.z*0.5f);
				BoxDesc.mNext = &BoxDesc2;

/*				PINT_CONVEX_CREATE WheelDesc;
				WheelDesc.mNbVerts	= TotalNbVerts;
				WheelDesc.mVerts	= Verts;
				WheelDesc.mRenderer	= CreateConvexRenderer(WheelDesc.mNbVerts, WheelDesc.mVerts);
				WheelDesc.mLocalPos	= Point(0.0f, 0.0f, Coeff*FrontAxle.z);
				BoxDesc.mNext = &WheelDesc;*/

				if(Articulation)
				{
				const float Limit = PI/4.0f;
//				const float Limit = PI/6.0f;
//				const float Limit = 0.0f;
					PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = ChassisObject;
					ArticulatedDesc.mLocalPivot0 = FrontAxleOffset;
					ArticulatedDesc.mLocalPivot1 = Point(0.0f, 0.0f, 0.0f);
/*					ArticulatedDesc.mX = Point(0.0f, 1.0f, 0.0f);
					ArticulatedDesc.mEnableTwistLimit = true;
					ArticulatedDesc.mTwistLowerLimit = -Limit;
					ArticulatedDesc.mTwistUpperLimit = Limit;
					ArticulatedDesc.mEnableSwingLimit = true;
					ArticulatedDesc.mSwingYLimit = 0.001f;//PI/6.0f;
					ArticulatedDesc.mSwingZLimit = 0.001f;//PI/6.0f;*/
					ArticulatedDesc.mX = Point(1.0f, 0.0f, 0.0f);
					ArticulatedDesc.mEnableTwistLimit = true;
					ArticulatedDesc.mTwistLowerLimit = -0.001f;
					ArticulatedDesc.mTwistUpperLimit = 0.001f;
					ArticulatedDesc.mEnableSwingLimit = true;
					ArticulatedDesc.mSwingYLimit = 0.001f;
					ArticulatedDesc.mSwingZLimit = Limit;
					FrontAxleObject[i] = pint.CreateArticulatedObject(AxleDesc, ArticulatedDesc, Articulation);
				}
				else
				{
					FrontAxleObject[i] = CreatePintObject(pint, AxleDesc);
				}
				UserData->mFrontAxleObject[i] = FrontAxleObject[i];

				WheelPt[i] = Point(-BoxDesc.mExtents.x - BoxDesc2.mExtents.x*2.0f, 0.0f, -Coeff*FrontAxle.z*0.5f);
				WheelGlobalPt[i] = AxlePos + WheelPt[i];
			}

			if(1)
			{
				PINT_CONVEX_CREATE WheelDesc(TotalNbVerts, Verts);
				WheelDesc.mRenderer	= CreateConvexRenderer(WheelDesc.mNbVerts, WheelDesc.mVerts);
				WheelDesc.mLocalPos	= Point(0.0f, 0.0f, 0.0f);
				WheelDesc.mMaterial	= &mHighFrictionMaterial;

				const Point AxlePos = CarPos + FrontAxleOffset;
				PINT_OBJECT_CREATE AxleDesc;
				AxleDesc.mPosition			= AxlePos + Point(0.0f, 0.0f, Coeff*FrontAxle.z);
				AxleDesc.mMass				= WheelMass;
				AxleDesc.mShapes			= &WheelDesc;
				AxleDesc.mCollisionGroup	= GroupWheel;

				if(Articulation)
				{
					PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = FrontAxleObject[i];
					ArticulatedDesc.mLocalPivot0 = Point(0.0f, 0.0f, Coeff*FrontAxle.z);
					ArticulatedDesc.mLocalPivot1 = Point(0.0f, 0.0f, 0.0f);
					ArticulatedDesc.mX = Point(0.0f, 0.0f, 1.0f);
					ArticulatedDesc.mEnableTwistLimit = false;
					ArticulatedDesc.mTwistLowerLimit = -0.01f;
					ArticulatedDesc.mTwistUpperLimit = 0.01f;
					ArticulatedDesc.mEnableSwingLimit = true;
					ArticulatedDesc.mSwingYLimit = 0.001f;//PI/6.0f;
					ArticulatedDesc.mSwingZLimit = 0.001f;//PI/6.0f;
					if(UseMotor)
						SetupMotor(ArticulatedDesc);
					PintActorHandle WheelObject = pint.CreateArticulatedObject(AxleDesc, ArticulatedDesc, Articulation);
					UserData->mFront[i].mWheel = WheelObject;
					UserData->mFront[i].mParent = FrontAxleObject[i];
				}
				else
				{
					PintActorHandle WheelObject = CreatePintObject(pint, AxleDesc);
					UserData->mFront[i].mWheel = WheelObject;
					UserData->mFront[i].mParent = FrontAxleObject[i];

					PINT_HINGE_JOINT_CREATE Desc;
					Desc.mObject0		= WheelObject;
					Desc.mObject1		= FrontAxleObject[i];
					Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
					Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
					Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
					Desc.mLocalPivot1	= Point(0.0f, 0.0f, Coeff*FrontAxle.z);
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
			}

			if(!Articulation)
			{
				const float Limit = PI/6.0f;
//				const float Limit = 0.0f;
				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mObject0		= ChassisObject;
				Desc.mObject1		= FrontAxleObject[i];
				Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
				Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
				Desc.mLocalPivot0	= FrontAxleOffset;
				Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
				Desc.mMinLimitAngle	= -Limit;
				Desc.mMaxLimitAngle	= Limit;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}

		{
			const float Length = WheelGlobalPt[0].Distance(WheelGlobalPt[1]);
			if(0)
			{
				PINT_DISTANCE_JOINT_CREATE Desc;
				Desc.mObject0		= FrontAxleObject[0];
				Desc.mObject1		= FrontAxleObject[1];
				Desc.mLocalPivot0	= WheelPt[0];
				Desc.mLocalPivot1	= WheelPt[1];
				Desc.mMinDistance	= Length;
				Desc.mMaxDistance	= Length;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
			else
			{
				PINT_BOX_CREATE BoxDesc(0.05f, 0.05f, Length*0.5f/* - z2*/);
				BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

				PINT_OBJECT_CREATE ObjectDesc;
				ObjectDesc.mPosition		= (WheelGlobalPt[0] + WheelGlobalPt[1])*0.5f;
				ObjectDesc.mShapes			= &BoxDesc;
				ObjectDesc.mMass			= StructMass;
				ObjectDesc.mCollisionGroup	= GroupStruct;
				PintActorHandle RodObject = CreatePintObject(pint, ObjectDesc);
#ifdef TEST_ROD_OBJECT
				UserData->mRod = RodObject;
#endif

				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
				Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);

				Desc.mObject0		= FrontAxleObject[0];
				Desc.mObject1		= RodObject;
				Desc.mLocalPivot0	= WheelPt[0];
				Desc.mLocalPivot1	= Point(0.0f, 0.0f, BoxDesc.mExtents.z);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);

				Desc.mObject0		= FrontAxleObject[1];
				Desc.mObject1		= RodObject;
				Desc.mLocalPivot0	= WheelPt[1];
				Desc.mLocalPivot1	= Point(0.0f, 0.0f, -BoxDesc.mExtents.z);
				PintJointHandle JointHandle2 = pint.CreateJoint(Desc);
				ASSERT(JointHandle2);
			}
		}

		if(Articulation)
			pint.AddArticulationToScene(Articulation);

		}

//		CreateSeaOfStaticConvexes(pint, caps, 8, 8, 0.0f);

		if(!mLoadTerrain)
			CreateTestScene(pint, &mHighFrictionMaterial);
		return true;
	}

END_TEST(Vehicle)

///////////////////////////////////////////////////////////////////////////////

//### use articulation in that one too
//### check mass distrib
// 4 / 32
static const char* gDesc_Vehicle2 = "Vehicle2";

START_VEHICLE_TEST(Vehicle2, CATEGORY_WIP, gDesc_Vehicle2)

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("VehicleTest2", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(10.76f, 11.91f, 14.07f), Point(-0.54f, -0.49f, -0.68f));

//		desc.mNbSimulateCallsPerFrame = 4;
//		desc.mTimestep = (1.0f/60.0f)/4.0f;
		SetDefEnv(desc, false);
	}

	virtual	bool	CommonSetup()
	{
		mDriveMode				= DRIVE_MODE_ADD_TORQUE;
		mAcceleration			= 100.0f;
		mMaxAngularVelocity		= 17.0f;
//		mMaxAngularVelocity		= 20.0f;
//		mMaxAngularVelocity		= 32.0f;
//		mMaxAngularVelocity		= 170.0f;
		mSteeringForce			= 200.0f;
//		mSteeringForce			= 500.0f;
		mCamera.mUpOffset		= 4.0f;
		mCamera.mDistToTarget	= 10.0f;
		mClampAngularVelocity	= true;
		mControlCamera			= gDriveVehicle;
		mLoadTerrain			= false;

		return VehicleBase::CommonSetup();
	}

	void	CreateVehicle(Pint& pint, const Point& pos, ChassisData* UserData, const PINT_MATERIAL_CREATE& LowFrictionMaterial, PintCollisionGroup ChassisGroup, PintCollisionGroup WheelGroup)
	{
		const float MainScale = 0.25f;

//		mCreateDefaultEnvironment = true;

/*		const float ChassisMass = 4.0f;
//		const float WheelMass = 15.0f;
//		const float StructMass = 10.0f;
		const float WheelMass = 4.0f;
		const float StructMass = 4.0f;*/

		const float M = 4.0f;
		const float ChassisMass = M;
		const float WheelMass = M;
		const float StructMass = M;

//		const PintCollisionGroup ChassisGroup = 1;
//		const PintCollisionGroup WheelGroup = 2;

		//

//		const float WheelRadius = 2.0f * MainScale;
		const float WheelRadius = 4.0f * MainScale;

//		const float WheelWidth = 1.0f * MainScale;
//		const float WheelWidth = 2.0f * MainScale;
//		const float WheelWidth = 3.0f * MainScale;
		const float WheelWidth = 4.0f * MainScale;

	//	const udword NbPts = 64;
//		const udword NbPts = 48;
		const udword NbPts = 60;
//		const udword NbPts = 6;
//		const udword NbPts = 100;
//		const udword NbPts = 16;
		const CylinderMesh Cylinder(NbPts, WheelRadius, WheelWidth*0.5f);
		const udword TotalNbVerts = Cylinder.mNbVerts;
		const Point* Verts = Cylinder.mVerts;

		//

PtrContainer JointDescs;
//udword NbPrims = 0;
//PINT_PRISMATIC_JOINT_CREATE PrismDesc[64];

		const Point Extents(MainScale*8.0f, MainScale*2.0f, MainScale*4.0f);

		PintActorHandle Chassis;
//		const Point ChassisPos(0.0f, 4.0f+Extents.y*4.0f, 0.0f);
		const Point ChassisPos = pos;

		{
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes			= &BoxDesc;
			ObjectDesc.mMass			= ChassisMass;
			ObjectDesc.mPosition		= ChassisPos;
			ObjectDesc.mCollisionGroup	= ChassisGroup;
			ObjectDesc.mCOMLocalOffset	= Point(0.0f, -5.0f, 0.0f);
//			ObjectDesc.mCOMLocalOffset	= Point(0.0f, -3.0f, 0.0f);
			Chassis = CreatePintObject(pint, ObjectDesc);
			if(UserData)
			UserData->mChassis = Chassis;
		}

		const Point SuspExtents(MainScale*1.0f, MainScale*1.0f, MainScale*1.0f);
		Point LocalAnchorPt[2];
		Point GlobalAnchorPt[2];
		PintActorHandle AnchorObjects[2];
		for(udword j=0;j<2;j++)
		{
			const float z = j ? 1.0f : -1.0f;
			for(udword i=0;i<2;i++)
			{
				const float x = i ? 1.0f : -1.0f;
				const bool FrontWheels = i!=0;

				const Point Offset(x*(Extents.x-SuspExtents.x), -Extents.y-SuspExtents.y, z*(Extents.z-SuspExtents.z));
				const Point SuspPos = ChassisPos + Offset;
				PintActorHandle SuspObject;
				{
					PINT_BOX_CREATE BoxDesc(SuspExtents);
					BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

					PINT_OBJECT_CREATE ObjectDesc;
					ObjectDesc.mShapes			= &BoxDesc;
					ObjectDesc.mMass			= StructMass;
					ObjectDesc.mPosition		= SuspPos;
					ObjectDesc.mCollisionGroup	= ChassisGroup;
					SuspObject = CreatePintObject(pint, ObjectDesc);

					PINT_PRISMATIC_JOINT_CREATE Desc;
					Desc.mObject0			= Chassis;
					Desc.mObject1			= SuspObject;
					//###PRISMATIC2
					Desc.mLocalPivot0.mPos	= Offset;
					Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
					Desc.mLocalAxis0		= Point(0.0f, 1.0f, 0.0f);
					Desc.mLocalAxis1		= Point(0.0f, 1.0f, 0.0f);
					Desc.mMaxLimit			= 0.01f;
					Desc.mMinLimit			= 0.0f;
//					Desc.mSpringStiffness	= 20000.0f;
//					Desc.mSpringDamping		= 1000.0f;
//					Desc.mSpringStiffness	= 10000.0f;
//					Desc.mSpringDamping		= 500.0f;
					Desc.mSpringStiffness	= 1000.0f;
					Desc.mSpringDamping		= 100.0f;
//						Desc.mSpringStiffness	= 100.0f;
//						Desc.mSpringDamping		= 10.0f;
/*Desc.mSpringStiffness	= 100000.0f;
//Desc.mSpringStiffness	= 100.0f;
Desc.mSpringDamping		= 10.0f;
Desc.mMaxLimit			= 0.2f;
Desc.mMinLimit			= -0.2f;*/

//					PintJointHandle JointHandle = pint.CreateJoint(Desc);
//					ASSERT(JointHandle);
PINT_PRISMATIC_JOINT_CREATE* Copy = ICE_NEW(PINT_PRISMATIC_JOINT_CREATE);
*Copy = Desc;
JointDescs.AddPtr(Copy);

//PrismDesc[NbPrims++] = Desc;
				}

				// We only create the hinge for the front wheels. For the rear wheels we'll attach the wheels
				// directly to the suspension.
				PintActorHandle WheelAttachObject;
				Point WheelPosOffset;
				Point WheelJointOffset;
				if(FrontWheels)
				{
					const float Extension = -1.0f*MainScale;
					const Point HingeExtents(SuspExtents.x+fabsf(Extension), SuspExtents.y*0.5f, SuspExtents.z);
//					WheelPosOffset = WheelJointOffset = Point(0.0f, 0.0f, z*HingeExtents.z);
					WheelPosOffset = WheelJointOffset = Point(-Extension, 0.0f, z*HingeExtents.z);
					WheelPosOffset.x = 0.0f;
					{
						PINT_BOX_CREATE BoxDesc(HingeExtents);
						BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

						PINT_OBJECT_CREATE ObjectDesc;
						ObjectDesc.mPosition		= SuspPos + Point(Extension, 0.0f, 0.0f);
						ObjectDesc.mShapes			= &BoxDesc;
						ObjectDesc.mMass			= StructMass;
						ObjectDesc.mCollisionGroup	= WheelGroup;
						const PintActorHandle HingeObject = CreatePintObject(pint, ObjectDesc);
						if(UserData)
						UserData->mFrontAxleObject[j] = HingeObject;
						WheelAttachObject = HingeObject;

//						LocalAnchorPt[j] = Point(HingeExtents.x, 0.0f, -z*HingeExtents.z);
						LocalAnchorPt[j] = Point(Extension<0.0f ? -HingeExtents.x : HingeExtents.x, 0.0f, 0.0f);
						GlobalAnchorPt[j] = ObjectDesc.mPosition + LocalAnchorPt[j];
						AnchorObjects[j] = HingeObject;

						{
//							const float Limit = PI/6.0f;
							const float Limit = PI/4.0f;
							PINT_HINGE_JOINT_CREATE Desc;
							Desc.mObject0		= SuspObject;
							Desc.mObject1		= HingeObject;
							Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
							Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
							Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
							Desc.mLocalPivot1	= Point(-Extension, 0.0f, 0.0f);
//							Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
							Desc.mMinLimitAngle	= -Limit;
							Desc.mMaxLimitAngle	= Limit;
//							PintJointHandle JointHandle = pint.CreateJoint(Desc);
//							ASSERT(JointHandle);
PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE);
*Copy = Desc;
JointDescs.AddPtr(Copy);
						}
					}
				}
				else
				{
					WheelAttachObject = SuspObject;
					WheelPosOffset = WheelJointOffset = Point(0.0f, 0.0f, z*SuspExtents.z);
				}

				{
					PINT_CONVEX_CREATE WheelDesc(TotalNbVerts, Verts);
					WheelDesc.mRenderer	= CreateConvexRenderer(WheelDesc.mNbVerts, WheelDesc.mVerts);
//						PINT_SPHERE_CREATE WheelDesc(WheelRadius);
//						WheelDesc.mRenderer	= CreateConvexRenderer(TotalNbVerts, Verts);
					if(FrontWheels)
						WheelDesc.mMaterial	= &mHighFrictionMaterial;
					else
						WheelDesc.mMaterial	= &LowFrictionMaterial;					

					PINT_OBJECT_CREATE ObjectDesc;
					ObjectDesc.mPosition		= SuspPos + WheelPosOffset;
					ObjectDesc.mMass			= WheelMass;
					ObjectDesc.mShapes			= &WheelDesc;
					ObjectDesc.mCollisionGroup	= WheelGroup;

					{
						PintActorHandle WheelObject = CreatePintObject(pint, ObjectDesc);
						if(UserData)
						{
						if(FrontWheels)
						{
							UserData->mFront[j].mWheel = WheelObject;
							UserData->mFront[j].mParent = WheelAttachObject;
						}
						else
						{
							UserData->mRear[j].mWheel = WheelObject;
							UserData->mRear[j].mParent = WheelAttachObject;
						}
						}

						PINT_HINGE_JOINT_CREATE Desc;
						Desc.mObject0		= WheelObject;
						Desc.mObject1		= WheelAttachObject;
						Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
						Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
						Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
						Desc.mLocalPivot1	= WheelJointOffset;
//						PintJointHandle JointHandle = pint.CreateJoint(Desc);
//						ASSERT(JointHandle);
PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE);
*Copy = Desc;
JointDescs.AddPtr(Copy);
					}
				}



			}
		}

		//###refactor with other test code
		const float Length = GlobalAnchorPt[0].Distance(GlobalAnchorPt[1]);
		{
			PINT_BOX_CREATE BoxDesc(0.1f*MainScale, 0.1f*MainScale, Length*0.5f);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mPosition		= (GlobalAnchorPt[0] + GlobalAnchorPt[1])*0.5f;
			ObjectDesc.mShapes			= &BoxDesc;
			ObjectDesc.mMass			= StructMass;
			ObjectDesc.mCollisionGroup	= WheelGroup;
			PintActorHandle RodObject = CreatePintObject(pint, ObjectDesc);

//			PINT_HINGE_JOINT_CREATE Desc;
//			Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
//			Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
			PINT_SPHERICAL_JOINT_CREATE Desc;

			Desc.mObject0		= AnchorObjects[0];
			Desc.mObject1		= RodObject;
			Desc.mLocalPivot0	= LocalAnchorPt[0];
			Desc.mLocalPivot1	= Point(0.0f, 0.0f, -BoxDesc.mExtents.z);
//			PintJointHandle JointHandle = pint.CreateJoint(Desc);
//			ASSERT(JointHandle);
PINT_SPHERICAL_JOINT_CREATE* Copy = ICE_NEW(PINT_SPHERICAL_JOINT_CREATE);
*Copy = Desc;
JointDescs.AddPtr(Copy);

			Desc.mObject0		= AnchorObjects[1];
			Desc.mObject1		= RodObject;
			Desc.mLocalPivot0	= LocalAnchorPt[1];
			Desc.mLocalPivot1	= Point(0.0f, 0.0f, BoxDesc.mExtents.z);
//			PintJointHandle JointHandle2 = pint.CreateJoint(Desc);
//			ASSERT(JointHandle2);
PINT_SPHERICAL_JOINT_CREATE* Copy2 = ICE_NEW(PINT_SPHERICAL_JOINT_CREATE);
*Copy2 = Desc;
JointDescs.AddPtr(Copy2);
		}

//for(udword j=0;j<32;j++)
/*{
for(udword i=0;i<NbPrims;i++)
{
	PintJointHandle JointHandle = pint.CreateJoint(PrismDesc[i]);
	ASSERT(JointHandle);
}
}*/

const udword NbJoints = JointDescs.GetNbEntries();
for(udword j=0;j<32;j++)
{
for(udword i=0;i<NbJoints;i++)
{
	const PINT_JOINT_CREATE* jc = (const PINT_JOINT_CREATE*)JointDescs.GetEntry(i);
	PintJointHandle JointHandle = pint.CreateJoint(*jc);
	ASSERT(JointHandle);
}

//for(udword j=0;j<32;j++)
/*{
for(udword i=0;i<NbPrims;i++)
{
	PintJointHandle JointHandle = pint.CreateJoint(PrismDesc[i]);
	ASSERT(JointHandle);
}
}*/

}

for(udword i=0;i<NbJoints;i++)
{
	const PINT_JOINT_CREATE* jc = (const PINT_JOINT_CREATE*)JointDescs.GetEntry(i);
	DELETESINGLE(jc);
}

/*//for(udword j=0;j<32;j++)
{
for(udword i=0;i<NbPrims;i++)
{
	PintJointHandle JointHandle = pint.CreateJoint(PrismDesc[i]);
	ASSERT(JointHandle);
}
}*/
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
//		if(!caps.mSupportPrismaticJoints || !caps.mSupportRigidBodySimulation)
//			return false;

		//

		const PINT_MATERIAL_CREATE LowFrictionMaterial(0.75f, 0.75f, 0.0f);

		CreateMeshesFromRegisteredSurfaces(pint, caps, &LowFrictionMaterial);
//		CreateMeshesFromRegisteredSurfaces(pint, caps, &mHighFrictionMaterial);

		//


const PintCollisionGroup ChassisGroup = 1;
const PintCollisionGroup WheelGroup = 2;

// ### there's some weird contact creating trouble, just disable everything for now
//		const PintDisabledGroups DG(ChassisGroup, WheelGroup);
//		pint.SetDisabledGroups(1, &DG);
//		const PintDisabledGroups DG[2] = { PintDisabledGroups(1, 2), PintDisabledGroups(1, 1)	};
//		pint.SetDisabledGroups(2, DG);
const PintDisabledGroups DG[3*2] = {
	PintDisabledGroups(1, 2),
	PintDisabledGroups(1, 1),
	PintDisabledGroups(2, 2),

	PintDisabledGroups(3, 4),
	PintDisabledGroups(3, 3),
	PintDisabledGroups(4, 4)
};
pint.SetDisabledGroups(3*2, DG);

ChassisData* UserData = ICE_NEW(ChassisData);
pint.mUserData = UserData;

const float MainScale = 0.25f;
const Point Extents(MainScale*8.0f, MainScale*2.0f, MainScale*4.0f);
const Point ChassisPos(0.0f, 4.0f+Extents.y*4.0f, 0.0f);

CreateVehicle(pint, ChassisPos, UserData, LowFrictionMaterial, ChassisGroup, WheelGroup);
CreateVehicle(pint, ChassisPos + Point(10.0f, 0.0f, 0.0f), null, LowFrictionMaterial, 3, 4);


{
//	const PINT_MATERIAL_CREATE BoxMaterial(1.0f, 1.0f, 0.0f);
	const PINT_MATERIAL_CREATE BoxMaterial(0.2f, 0.2f, 0.0f);

	const float BoxExtent = 0.2f;

	PINT_BOX_CREATE BoxDesc(BoxExtent, BoxExtent, BoxExtent);
	BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
	BoxDesc.mMaterial	= &BoxMaterial;

	const udword NbStacks = 1;
	for(udword j=0;j<NbStacks;j++)
	{
		udword NbBoxes = 20;
		float BoxPosY = BoxExtent;
		while(NbBoxes)
		{
			for(udword i=0;i<NbBoxes;i++)
			{
		//		const float Coeff = float(i)/float(NbBoxes-1);
				const float Coeff = float(i) - float(NbBoxes)*0.5f;

				PINT_OBJECT_CREATE ObjectDesc;
				ObjectDesc.mShapes		= &BoxDesc;
				ObjectDesc.mPosition.x	= Coeff * BoxExtent * 2.0f;
				ObjectDesc.mPosition.y	= BoxPosY;
				ObjectDesc.mPosition.z	= float(j) * BoxExtent * 4.0f;
				ObjectDesc.mPosition.z += 10.0f;
				ObjectDesc.mMass		= 1.0f;
				ObjectDesc.mMass		= 0.1f;
				PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
				ASSERT(Handle);
			}

			NbBoxes--;
			BoxPosY += BoxExtent*2.0f;
		}
	}
}


#ifdef REMOVED

		const float MainScale = 0.25f;

//		mCreateDefaultEnvironment = true;

/*		const float ChassisMass = 4.0f;
//		const float WheelMass = 15.0f;
//		const float StructMass = 10.0f;
		const float WheelMass = 4.0f;
		const float StructMass = 4.0f;*/

		const float M = 4.0f;
		const float ChassisMass = M;
		const float WheelMass = M;
		const float StructMass = M;

		const PintCollisionGroup ChassisGroup = 1;
		const PintCollisionGroup WheelGroup = 2;

		// ### there's some weird contact creating trouble, just disable everything for now
//		const PintDisabledGroups DG(ChassisGroup, WheelGroup);
//		pint.SetDisabledGroups(1, &DG);
//		const PintDisabledGroups DG[2] = { PintDisabledGroups(1, 2), PintDisabledGroups(1, 1)	};
//		pint.SetDisabledGroups(2, DG);
		const PintDisabledGroups DG[3] = { PintDisabledGroups(1, 2), PintDisabledGroups(1, 1), PintDisabledGroups(2, 2)	};
		pint.SetDisabledGroups(3, DG);

		ChassisData* UserData = ICE_NEW(ChassisData);
		pint.mUserData = UserData;

		//

//		const float WheelRadius = 2.0f * MainScale;
		const float WheelRadius = 4.0f * MainScale;
//		const float WheelWidth = 1.0f * MainScale;
//		const float WheelWidth = 2.0f * MainScale;
//		const float WheelWidth = 3.0f * MainScale;
		const float WheelWidth = 4.0f * MainScale;

	//	const udword NbPts = 64;
//		const udword NbPts = 48;
		const udword NbPts = 60;
//		const udword NbPts = 6;
//		const udword NbPts = 100;
//		const udword NbPts = 16;
		const CylinderMesh Cylinder(NbPts, WheelRadius, WheelWidth*0.5f);
		const udword TotalNbVerts = Cylinder.mNbVerts;
		const Point* Verts = Cylinder.mVerts;

		//

Container JointDescs;
udword NbPrims = 0;
PINT_PRISMATIC_JOINT_CREATE PrismDesc[64];

		const Point Extents(MainScale*8.0f, MainScale*2.0f, MainScale*4.0f);

		PintActorHandle Chassis;
		const Point ChassisPos(0.0f, 4.0f+Extents.y*4.0f, 0.0f);

		{
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes			= &BoxDesc;
			ObjectDesc.mMass			= ChassisMass;
			ObjectDesc.mPosition		= ChassisPos;
			ObjectDesc.mCollisionGroup	= ChassisGroup;
			ObjectDesc.mCOMLocalOffset	= Point(0.0f, -5.0f, 0.0f);
			Chassis = CreatePintObject(pint, ObjectDesc);
			UserData->mChassis = Chassis;
		}

		const Point SuspExtents(MainScale*1.0f, MainScale*1.0f, MainScale*1.0f);
		Point LocalAnchorPt[2];
		Point GlobalAnchorPt[2];
		PintActorHandle AnchorObjects[2];
		for(udword j=0;j<2;j++)
		{
			const float z = j ? 1.0f : -1.0f;
			for(udword i=0;i<2;i++)
			{
				const float x = i ? 1.0f : -1.0f;
				const bool FrontWheels = i!=0;

				const Point Offset(x*(Extents.x-SuspExtents.x), -Extents.y-SuspExtents.y, z*(Extents.z-SuspExtents.z));
				const Point SuspPos = ChassisPos + Offset;
				PintActorHandle SuspObject;
				{
					PINT_BOX_CREATE BoxDesc(SuspExtents);
					BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

					PINT_OBJECT_CREATE ObjectDesc;
					ObjectDesc.mShapes			= &BoxDesc;
					ObjectDesc.mMass			= StructMass;
					ObjectDesc.mPosition		= SuspPos;
					ObjectDesc.mCollisionGroup	= ChassisGroup;
					SuspObject = CreatePintObject(pint, ObjectDesc);

					PINT_PRISMATIC_JOINT_CREATE Desc;
					Desc.mObject0			= Chassis;
					Desc.mObject1			= SuspObject;
					Desc.mLocalPivot0		= Offset;
					Desc.mLocalPivot1		= Point(0.0f, 0.0f, 0.0f);
					Desc.mLocalAxis0		= Point(0.0f, 1.0f, 0.0f);
					Desc.mLocalAxis1		= Point(0.0f, 1.0f, 0.0f);
					Desc.mMaxLimit			= 0.01f;
					Desc.mMinLimit			= 0.0f;
//					Desc.mSpringStiffness	= 20000.0f;
//					Desc.mSpringDamping		= 1000.0f;
//					Desc.mSpringStiffness	= 10000.0f;
//					Desc.mSpringDamping		= 500.0f;
					Desc.mSpringStiffness	= 1000.0f;
					Desc.mSpringDamping		= 100.0f;
/*Desc.mSpringStiffness	= 100000.0f;
//Desc.mSpringStiffness	= 100.0f;
Desc.mSpringDamping		= 10.0f;
Desc.mMaxLimit			= 0.2f;
Desc.mMinLimit			= -0.2f;*/

//					PintJointHandle JointHandle = pint.CreateJoint(Desc);
//					ASSERT(JointHandle);
PINT_PRISMATIC_JOINT_CREATE* Copy = ICE_NEW(PINT_PRISMATIC_JOINT_CREATE);
*Copy = Desc;
JointDescs.Add(udword(Copy));

//PrismDesc[NbPrims++] = Desc;
				}

				// We only create the hinge for the front wheels. For the rear wheels we'll attach the wheels
				// directly to the suspension.
				PintActorHandle WheelAttachObject;
				Point WheelPosOffset;
				Point WheelJointOffset;
				if(FrontWheels)
				{
					const float Extension = -1.0f*MainScale;
					const Point HingeExtents(SuspExtents.x+fabsf(Extension), SuspExtents.y*0.5f, SuspExtents.z);
//					WheelPosOffset = WheelJointOffset = Point(0.0f, 0.0f, z*HingeExtents.z);
					WheelPosOffset = WheelJointOffset = Point(-Extension, 0.0f, z*HingeExtents.z);
					WheelPosOffset.x = 0.0f;
					{
						PINT_BOX_CREATE BoxDesc(HingeExtents);
						BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

						PINT_OBJECT_CREATE ObjectDesc;
						ObjectDesc.mPosition		= SuspPos + Point(Extension, 0.0f, 0.0f);
						ObjectDesc.mShapes			= &BoxDesc;
						ObjectDesc.mMass			= StructMass;
						ObjectDesc.mCollisionGroup	= WheelGroup;
						const PintActorHandle HingeObject = CreatePintObject(pint, ObjectDesc);
						UserData->mFrontAxleObject[j] = HingeObject;
						WheelAttachObject = HingeObject;

//						LocalAnchorPt[j] = Point(HingeExtents.x, 0.0f, -z*HingeExtents.z);
						LocalAnchorPt[j] = Point(Extension<0.0f ? -HingeExtents.x : HingeExtents.x, 0.0f, 0.0f);
						GlobalAnchorPt[j] = ObjectDesc.mPosition + LocalAnchorPt[j];
						AnchorObjects[j] = HingeObject;

						{
//							const float Limit = PI/6.0f;
							const float Limit = PI/4.0f;
							PINT_HINGE_JOINT_CREATE Desc;
							Desc.mObject0		= SuspObject;
							Desc.mObject1		= HingeObject;
							Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
							Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
							Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
							Desc.mLocalPivot1	= Point(-Extension, 0.0f, 0.0f);
//							Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
							Desc.mMinLimitAngle	= -Limit;
							Desc.mMaxLimitAngle	= Limit;
//							PintJointHandle JointHandle = pint.CreateJoint(Desc);
//							ASSERT(JointHandle);
PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE);
*Copy = Desc;
JointDescs.Add(udword(Copy));
						}
					}
				}
				else
				{
					WheelAttachObject = SuspObject;
					WheelPosOffset = WheelJointOffset = Point(0.0f, 0.0f, z*SuspExtents.z);
				}

				{
					PINT_CONVEX_CREATE WheelDesc(TotalNbVerts, Verts);
					WheelDesc.mRenderer	= CreateConvexRenderer(WheelDesc.mNbVerts, WheelDesc.mVerts);
					if(FrontWheels)
						WheelDesc.mMaterial	= &mHighFrictionMaterial;
					else
						WheelDesc.mMaterial	= &LowFrictionMaterial;					

					PINT_OBJECT_CREATE ObjectDesc;
					ObjectDesc.mPosition		= SuspPos + WheelPosOffset;
					ObjectDesc.mMass			= WheelMass;
					ObjectDesc.mShapes			= &WheelDesc;
					ObjectDesc.mCollisionGroup	= WheelGroup;

					{
						PintActorHandle WheelObject = CreatePintObject(pint, ObjectDesc);
						if(FrontWheels)
						{
							UserData->mFront[j].mWheel = WheelObject;
							UserData->mFront[j].mParent = WheelAttachObject;
						}
						else
						{
							UserData->mRear[j].mWheel = WheelObject;
							UserData->mRear[j].mParent = WheelAttachObject;
						}

						PINT_HINGE_JOINT_CREATE Desc;
						Desc.mObject0		= WheelObject;
						Desc.mObject1		= WheelAttachObject;
						Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
						Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
						Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
						Desc.mLocalPivot1	= WheelJointOffset;
//						PintJointHandle JointHandle = pint.CreateJoint(Desc);
//						ASSERT(JointHandle);
PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE);
*Copy = Desc;
JointDescs.Add(udword(Copy));
					}
				}



			}
		}

		//###refactor with other test code
		const float Length = GlobalAnchorPt[0].Distance(GlobalAnchorPt[1]);
		{
			PINT_BOX_CREATE BoxDesc(0.1f*MainScale, 0.1f*MainScale, Length*0.5f);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mPosition		= (GlobalAnchorPt[0] + GlobalAnchorPt[1])*0.5f;
			ObjectDesc.mShapes			= &BoxDesc;
			ObjectDesc.mMass			= StructMass;
			ObjectDesc.mCollisionGroup	= WheelGroup;
			PintActorHandle RodObject = CreatePintObject(pint, ObjectDesc);

//			PINT_HINGE_JOINT_CREATE Desc;
//			Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
//			Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
			PINT_SPHERICAL_JOINT_CREATE Desc;

			Desc.mObject0		= AnchorObjects[0];
			Desc.mObject1		= RodObject;
			Desc.mLocalPivot0	= LocalAnchorPt[0];
			Desc.mLocalPivot1	= Point(0.0f, 0.0f, -BoxDesc.mExtents.z);
//			PintJointHandle JointHandle = pint.CreateJoint(Desc);
//			ASSERT(JointHandle);
PINT_SPHERICAL_JOINT_CREATE* Copy = ICE_NEW(PINT_SPHERICAL_JOINT_CREATE);
*Copy = Desc;
JointDescs.Add(udword(Copy));

			Desc.mObject0		= AnchorObjects[1];
			Desc.mObject1		= RodObject;
			Desc.mLocalPivot0	= LocalAnchorPt[1];
			Desc.mLocalPivot1	= Point(0.0f, 0.0f, BoxDesc.mExtents.z);
//			PintJointHandle JointHandle2 = pint.CreateJoint(Desc);
//			ASSERT(JointHandle2);
PINT_SPHERICAL_JOINT_CREATE* Copy2 = ICE_NEW(PINT_SPHERICAL_JOINT_CREATE);
*Copy2 = Desc;
JointDescs.Add(udword(Copy2));
		}

//for(udword j=0;j<32;j++)
/*{
for(udword i=0;i<NbPrims;i++)
{
	PintJointHandle JointHandle = pint.CreateJoint(PrismDesc[i]);
	ASSERT(JointHandle);
}
}*/

const udword NbJoints = JointDescs.GetNbEntries();
for(udword j=0;j<32;j++)
{
for(udword i=0;i<NbJoints;i++)
{
	const PINT_JOINT_CREATE* jc = (const PINT_JOINT_CREATE*)JointDescs.GetEntry(i);
	PintJointHandle JointHandle = pint.CreateJoint(*jc);
	ASSERT(JointHandle);
}

//for(udword j=0;j<32;j++)
{
for(udword i=0;i<NbPrims;i++)
{
	PintJointHandle JointHandle = pint.CreateJoint(PrismDesc[i]);
	ASSERT(JointHandle);
}
}

}

for(udword i=0;i<NbJoints;i++)
{
	const PINT_JOINT_CREATE* jc = (const PINT_JOINT_CREATE*)JointDescs.GetEntry(i);
	DELETESINGLE(jc);
}

/*//for(udword j=0;j<32;j++)
{
for(udword i=0;i<NbPrims;i++)
{
	PintJointHandle JointHandle = pint.CreateJoint(PrismDesc[i]);
	ASSERT(JointHandle);
}
}*/
#endif

		if(!mLoadTerrain)
			CreateTestScene(pint, &mHighFrictionMaterial);
//			CreateTestScene(pint, &LowFrictionMaterial);

		return true;
	}

END_TEST(Vehicle2)

///////////////////////////////////////////////////////////////////////////////

/*static*/ PintActorHandle GenerateTireCompound(Pint& pint, const Point& torus_pos, const Quat& torus_rot, float mass, PintCollisionGroup group, const PINT_MATERIAL_CREATE& material, float scale, float mass_inertia_coeff);

static const char* gDesc_VehicleDebug = "VehicleDebug";

START_VEHICLE_TEST(VehicleDebug, CATEGORY_WIP, gDesc_VehicleDebug)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(2.38f, 4.01f, 4.18f), Point(-0.43f, -0.49f, -0.76f));
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		Quat torus_rot;	torus_rot.Identity();
		PintActorHandle WheelObject = GenerateTireCompound(pint, Point(0.0f, 4.0f, 0.0f), torus_rot, 4.0f, 0, mHighFrictionMaterial, 1.0f, 1.0f);

		CreateTestScene(pint, &mHighFrictionMaterial);

		return true;
	}

END_TEST(VehicleDebug)


static const char* gDesc_Vehicle2b = "Vehicle2b";

//START_TEST(Vehicle2b, CATEGORY_WIP, gDesc_Vehicle2b)

START_VEHICLE_TEST(Vehicle2b, CATEGORY_WIP, gDesc_Vehicle2b)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(10.76f, 11.91f, 14.07f), Point(-0.54f, -0.49f, -0.68f));
		SetDefEnv(desc, false);

//		desc.mNbSimulateCallsPerFrame = 4;
//		desc.mTimestep = (1.0f/60.0f)/4.0f;
	}

	virtual	bool Vehicle2b::CommonSetup()
	{
		mDriveMode				= DRIVE_MODE_ADD_TORQUE;
//		mDriveMode				= DRIVE_MODE_MOTOR;
		
		mAcceleration			= 150.0f;
		mAcceleration			= 300.0f;
		mMaxAngularVelocity		= 17.0f;
//		mMaxAngularVelocity		= 20.0f;
//		mMaxAngularVelocity		= 32.0f;
		mMaxAngularVelocity		= 1700.0f;
		mSteeringForce			= 2.0f*200.0f;
		mSteeringForce			= 20.0f*200.0f;
//		mSteeringForce			= 500.0f;
		mCamera.mUpOffset		= 4.0f;
		mCamera.mDistToTarget	= 10.0f;
		mClampAngularVelocity	= true;
//		mClampAngularVelocity	= false;
		mControlCamera			= gDriveVehicle;
		mLoadTerrain			= false;

//		mCreateDefaultEnvironment = false;

		return VehicleBase::CommonSetup();
	}

/*
	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		
//		desc.mCamera[0] = PintCameraPose(Point(10.76f, 11.91f, 14.07f), Point(-0.54f, -0.49f, -0.68f));
		desc.mCamera[0] = PintCameraPose(Point(3.99f, 2.38f, 4.84f), Point(-0.55f, -0.28f, -0.79f));
//		desc.mNbSimulateCallsPerFrame = 4;
//		desc.mTimestep = (1.0f/60.0f)/4.0f;
	}*/

	void	CreateVehicle(Pint& pint, const Point& pos, ChassisData* UserData, const PINT_MATERIAL_CREATE& LowFrictionMaterial, PintCollisionGroup ChassisGroup, PintCollisionGroup WheelGroup)
	{
		const float MainScale = 0.25f;

//		const float ChassisMass = 100.0f;
//		const float WheelMass = 4.0f;
//		const float StructMass = 1.0f;
		const float ChassisMass = 1000.0f;
		const float WheelMass = 20.0f;
		const float StructMass = 5.0f;
		const float MassForInertiaCoeff = 1.0f;

		//

//		const float WheelRadius = 4.0f * MainScale;
		const float WheelRadius = 3.0f * MainScale;
//		const float WheelRadius = 2.0f * MainScale;
//		const float WheelWidth = 4.0f * MainScale;
		const float WheelWidth = 2.0f * MainScale;
//		const float WheelWidth = 0.5f * MainScale;
		const udword NbPts = 60;
		const CylinderMesh Cylinder(NbPts, WheelRadius, WheelWidth*0.5f);
		const udword TotalNbVerts = Cylinder.mNbVerts;
		const Point* Verts = Cylinder.mVerts;

		//

PtrContainer JointDescs;
//udword NbPrims = 0;
//PINT_PRISMATIC_JOINT_CREATE PrismDesc[64];

//		const Point Extents(MainScale*8.0f, MainScale*2.0f, MainScale*4.0f);
//		const Point Extents(MainScale*8.0f, MainScale*2.0f, MainScale*6.0f);
		const Point Extents(MainScale*8.0f, MainScale*0.5f, MainScale*6.0f);

		PintActorHandle Chassis;
//		const Point ChassisPos(0.0f, 4.0f+Extents.y*4.0f, 0.0f);
		const Point ChassisPos = pos;

		{
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes			= &BoxDesc;
			ObjectDesc.mMass			= ChassisMass;
			ObjectDesc.mMassForInertia	= ChassisMass * MassForInertiaCoeff;
			ObjectDesc.mPosition		= ChassisPos;
			ObjectDesc.mCollisionGroup	= ChassisGroup;
//			ObjectDesc.mCOMLocalOffset	= Point(0.0f, -Extents.y, 0.0f);
//			ObjectDesc.mCOMLocalOffset	= Point(0.0f, -5.0f, 0.0f);
//			ObjectDesc.mCOMLocalOffset	= Point(0.0f, -3.0f, 0.0f);
			Chassis = CreatePintObject(pint, ObjectDesc);
			if(UserData)
				UserData->mChassis = Chassis;
		}
//		return;

		const Point SuspExtents(MainScale*1.0f, MainScale*1.0f, MainScale*1.0f);
		Point LocalAnchorPt[2];
		Point GlobalAnchorPt[2];
		PintActorHandle AnchorObjects[2];
		for(udword j=0;j<2;j++)
		{
			const float z = j ? 1.0f : -1.0f;
			for(udword i=0;i<2;i++)
			{
				const float x = i ? 1.0f : -1.0f;
				const bool FrontWheels = i!=0;

				const Point Offset(x*(Extents.x-SuspExtents.x), -Extents.y-SuspExtents.y, z*(Extents.z-SuspExtents.z));
				const Point SuspPos = ChassisPos + Offset;
				PintActorHandle SuspObject;
				{
					PINT_BOX_CREATE BoxDesc(SuspExtents);
					BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

					PINT_OBJECT_CREATE ObjectDesc;
					ObjectDesc.mShapes			= &BoxDesc;
					ObjectDesc.mMass			= StructMass;
					ObjectDesc.mMassForInertia	= StructMass * MassForInertiaCoeff;
					ObjectDesc.mPosition		= SuspPos;
					ObjectDesc.mCollisionGroup	= ChassisGroup;
					SuspObject = CreatePintObject(pint, ObjectDesc);

					PINT_PRISMATIC_JOINT_CREATE Desc;
					Desc.mObject0			= Chassis;
					Desc.mObject1			= SuspObject;
					//###PRISMATIC2
					Desc.mLocalPivot0.mPos	= Offset;
					Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
					Desc.mLocalAxis0		= Point(0.0f, 1.0f, 0.0f);
					Desc.mLocalAxis1		= Point(0.0f, 1.0f, 0.0f);
					Desc.mMaxLimit			= 0.01f;
					Desc.mMinLimit			= 0.0f;
//					Desc.mSpringStiffness	= 20000.0f;
//					Desc.mSpringDamping		= 1000.0f;
//					Desc.mSpringStiffness	= 10000.0f;
//					Desc.mSpringDamping		= 500.0f;
					Desc.mSpringStiffness	= 1000.0f;
					Desc.mSpringDamping		= 100.0f;
						Desc.mSpringStiffness	= 100.0f;
						Desc.mSpringDamping		= 10.0f;
						Desc.mSpringStiffness	= 500.0f;
						Desc.mSpringDamping		= 50.0f;

/*Desc.mSpringStiffness	= 100000.0f;
//Desc.mSpringStiffness	= 100.0f;
Desc.mSpringDamping		= 10.0f;
Desc.mMaxLimit			= 0.2f;
Desc.mMinLimit			= -0.2f;*/

//					PintJointHandle JointHandle = pint.CreateJoint(Desc);
//					ASSERT(JointHandle);
PINT_PRISMATIC_JOINT_CREATE* Copy = ICE_NEW(PINT_PRISMATIC_JOINT_CREATE);
*Copy = Desc;
JointDescs.AddPtr(Copy);

//PrismDesc[NbPrims++] = Desc;
				}

				// We only create the hinge for the front wheels. For the rear wheels we'll attach the wheels
				// directly to the suspension.
				PintActorHandle WheelAttachObject;
				Point WheelPosOffset;
				Point WheelJointOffset;
				if(FrontWheels)
				{
					const float Extension = -1.0f*MainScale;
					const Point HingeExtents(SuspExtents.x+fabsf(Extension), SuspExtents.y*0.5f, SuspExtents.z);
//					WheelPosOffset = WheelJointOffset = Point(0.0f, 0.0f, z*HingeExtents.z);
					WheelPosOffset = WheelJointOffset = Point(-Extension, 0.0f, z*HingeExtents.z);
					WheelPosOffset.x = 0.0f;
					{
						PINT_BOX_CREATE BoxDesc(HingeExtents);
						BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

						PINT_OBJECT_CREATE ObjectDesc;
						ObjectDesc.mPosition		= SuspPos + Point(Extension, 0.0f, 0.0f);
						ObjectDesc.mShapes			= &BoxDesc;
						ObjectDesc.mMass			= StructMass;
						ObjectDesc.mMassForInertia	= StructMass * MassForInertiaCoeff;
						ObjectDesc.mCollisionGroup	= WheelGroup;
						const PintActorHandle HingeObject = CreatePintObject(pint, ObjectDesc);
						if(UserData)
						UserData->mFrontAxleObject[j] = HingeObject;
						WheelAttachObject = HingeObject;

//						LocalAnchorPt[j] = Point(HingeExtents.x, 0.0f, -z*HingeExtents.z);
						LocalAnchorPt[j] = Point(Extension<0.0f ? -HingeExtents.x : HingeExtents.x, 0.0f, 0.0f);
						GlobalAnchorPt[j] = ObjectDesc.mPosition + LocalAnchorPt[j];
						AnchorObjects[j] = HingeObject;

						{
//							const float Limit = PI/6.0f;
//							const float Limit = PI/4.0f;
							const float Limit = PI/8.0f;
							PINT_HINGE_JOINT_CREATE Desc;
							Desc.mObject0		= SuspObject;
							Desc.mObject1		= HingeObject;
							Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
							Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
							Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
							Desc.mLocalPivot1	= Point(-Extension, 0.0f, 0.0f);
//							Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
							Desc.mMinLimitAngle	= -Limit;
							Desc.mMaxLimitAngle	= Limit;
//							PintJointHandle JointHandle = pint.CreateJoint(Desc);
//							ASSERT(JointHandle);
PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE);
*Copy = Desc;
JointDescs.AddPtr(Copy);
						}
					}
				}
				else
				{
					WheelAttachObject = SuspObject;
					WheelPosOffset = WheelJointOffset = Point(0.0f, 0.0f, z*SuspExtents.z);
				}

				{

PintActorHandle WheelObject;
if(FrontWheels)
{
					PINT_CONVEX_CREATE WheelDesc(TotalNbVerts, Verts);
					WheelDesc.mRenderer	= CreateConvexRenderer(WheelDesc.mNbVerts, WheelDesc.mVerts);
//						PINT_SPHERE_CREATE WheelDesc(WheelRadius);
//						WheelDesc.mRenderer	= CreateConvexRenderer(TotalNbVerts, Verts);
					if(FrontWheels)
						WheelDesc.mMaterial	= &mHighFrictionMaterial;
					else
						WheelDesc.mMaterial	= &LowFrictionMaterial;

					WheelDesc.mMaterial	= &mHighFrictionMaterial;
//const PINT_MATERIAL_CREATE MyMat(1.1f, 1.1f, 0.0f);
//WheelDesc.mMaterial	= &MyMat;

					PINT_OBJECT_CREATE ObjectDesc;
					ObjectDesc.mPosition		= SuspPos + WheelPosOffset;
					ObjectDesc.mMass			= WheelMass;
					ObjectDesc.mMassForInertia	= WheelMass * MassForInertiaCoeff;
					ObjectDesc.mShapes			= &WheelDesc;
					ObjectDesc.mCollisionGroup	= WheelGroup;

						WheelObject = CreatePintObject(pint, ObjectDesc);
}
else
{
//					PINT_CONVEX_CREATE WheelDesc(TotalNbVerts, Verts);
//					WheelDesc.mRenderer	= CreateConvexRenderer(WheelDesc.mNbVerts, WheelDesc.mVerts);
						PINT_SPHERE_CREATE WheelDesc(WheelRadius);
						WheelDesc.mRenderer	= CreateConvexRenderer(TotalNbVerts, Verts);
					if(FrontWheels)
						WheelDesc.mMaterial	= &mHighFrictionMaterial;
					else
						WheelDesc.mMaterial	= &LowFrictionMaterial;

					WheelDesc.mMaterial	= &mHighFrictionMaterial;
//const PINT_MATERIAL_CREATE MyMat(1.1f, 1.1f, 0.0f);
//WheelDesc.mMaterial	= &MyMat;

					PINT_OBJECT_CREATE ObjectDesc;
					ObjectDesc.mPosition		= SuspPos + WheelPosOffset;
					ObjectDesc.mMass			= WheelMass;
					ObjectDesc.mMassForInertia	= WheelMass * MassForInertiaCoeff;
					ObjectDesc.mShapes			= &WheelDesc;
					ObjectDesc.mCollisionGroup	= WheelGroup;

						WheelObject = CreatePintObject(pint, ObjectDesc);
}
//Quat torus_rot;	torus_rot.Identity();
//PintActorHandle WheelObject = GenerateTireCompound(pint, SuspPos + WheelPosOffset, torus_rot, WheelMass, WheelGroup, mHighFrictionMaterial);

					{
						if(UserData)
						{
						if(FrontWheels)
						{
							UserData->mFront[j].mWheel = WheelObject;
							UserData->mFront[j].mParent = WheelAttachObject;
						}
						else
						{
							UserData->mRear[j].mWheel = WheelObject;
							UserData->mRear[j].mParent = WheelAttachObject;
						}
						}

						PINT_HINGE_JOINT_CREATE Desc;
						Desc.mObject0		= WheelObject;
						Desc.mObject1		= WheelAttachObject;
						Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
						Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
						Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
						Desc.mLocalPivot1	= WheelJointOffset;

//						Desc.mUseMotor		= true;
//						Desc.mDriveVelocity	= 10.0f;

//						PintJointHandle JointHandle = pint.CreateJoint(Desc);
//						ASSERT(JointHandle);
PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE);
*Copy = Desc;
JointDescs.AddPtr(Copy);
					}
				}


			}
		}

		//###refactor with other test code
		const float Length = GlobalAnchorPt[0].Distance(GlobalAnchorPt[1]);
		{
			PINT_BOX_CREATE BoxDesc(0.1f*MainScale, 0.1f*MainScale, Length*0.5f);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mPosition		= (GlobalAnchorPt[0] + GlobalAnchorPt[1])*0.5f;
			ObjectDesc.mShapes			= &BoxDesc;
			ObjectDesc.mMass			= StructMass;
			ObjectDesc.mMassForInertia	= StructMass * MassForInertiaCoeff;
			ObjectDesc.mCollisionGroup	= WheelGroup;
			PintActorHandle RodObject = CreatePintObject(pint, ObjectDesc);

//			PINT_HINGE_JOINT_CREATE Desc;
//			Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
//			Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
			PINT_SPHERICAL_JOINT_CREATE Desc;

			Desc.mObject0		= AnchorObjects[0];
			Desc.mObject1		= RodObject;
			Desc.mLocalPivot0	= LocalAnchorPt[0];
			Desc.mLocalPivot1	= Point(0.0f, 0.0f, -BoxDesc.mExtents.z);
//			PintJointHandle JointHandle = pint.CreateJoint(Desc);
//			ASSERT(JointHandle);
PINT_SPHERICAL_JOINT_CREATE* Copy = ICE_NEW(PINT_SPHERICAL_JOINT_CREATE);
*Copy = Desc;
JointDescs.AddPtr(Copy);

			Desc.mObject0		= AnchorObjects[1];
			Desc.mObject1		= RodObject;
			Desc.mLocalPivot0	= LocalAnchorPt[1];
			Desc.mLocalPivot1	= Point(0.0f, 0.0f, BoxDesc.mExtents.z);
//			PintJointHandle JointHandle2 = pint.CreateJoint(Desc);
//			ASSERT(JointHandle2);
PINT_SPHERICAL_JOINT_CREATE* Copy2 = ICE_NEW(PINT_SPHERICAL_JOINT_CREATE);
*Copy2 = Desc;
JointDescs.AddPtr(Copy2);
		}

		//for(udword j=0;j<32;j++)
		/*{
		for(udword i=0;i<NbPrims;i++)
		{
			PintJointHandle JointHandle = pint.CreateJoint(PrismDesc[i]);
			ASSERT(JointHandle);
		}
		}*/

		const udword NbJoints = JointDescs.GetNbEntries();
		for(udword j=0;j<64;j++)
		{
			for(udword i=0;i<NbJoints;i++)
			{
				const PINT_JOINT_CREATE* jc = (const PINT_JOINT_CREATE*)JointDescs.GetEntry(i);
				PintJointHandle JointHandle = pint.CreateJoint(*jc);
				ASSERT(JointHandle);
			}

/*		//for(udword j=0;j<32;j++)
		{
		for(udword i=0;i<NbPrims;i++)
		{
			PintJointHandle JointHandle = pint.CreateJoint(PrismDesc[i]);
			ASSERT(JointHandle);
		}
		}*/

		}

		for(udword i=0;i<NbJoints;i++)
		{
			const PINT_JOINT_CREATE* jc = (const PINT_JOINT_CREATE*)JointDescs.GetEntry(i);
			DELETESINGLE(jc);
		}

		/*//for(udword j=0;j<32;j++)
		{
		for(udword i=0;i<NbPrims;i++)
		{
			PintJointHandle JointHandle = pint.CreateJoint(PrismDesc[i]);
			ASSERT(JointHandle);
		}
		}*/
	}


	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
//		if(!caps.mSupportPrismaticJoints || !caps.mSupportRigidBodySimulation)
//			return false;

		//

		const PINT_MATERIAL_CREATE LowFrictionMaterial(0.75f, 0.75f, 0.0f);

		CreateMeshesFromRegisteredSurfaces(pint, caps, &LowFrictionMaterial);
//		CreateMeshesFromRegisteredSurfaces(pint, caps, &mHighFrictionMaterial);

		//


		const PintCollisionGroup ChassisGroup = 1;
		const PintCollisionGroup WheelGroup = 2;

		// ### there's some weird contact creating trouble, just disable everything for now
		//		const PintDisabledGroups DG(ChassisGroup, WheelGroup);
		//		pint.SetDisabledGroups(1, &DG);
		//		const PintDisabledGroups DG[2] = { PintDisabledGroups(1, 2), PintDisabledGroups(1, 1)	};
		//		pint.SetDisabledGroups(2, DG);
		const PintDisabledGroups DG[3*2] = {
			PintDisabledGroups(1, 2),
			PintDisabledGroups(1, 1),
			PintDisabledGroups(2, 2),

			PintDisabledGroups(3, 4),
			PintDisabledGroups(3, 3),
			PintDisabledGroups(4, 4)
		};
		pint.SetDisabledGroups(3*2, DG);

		ChassisData* UserData = ICE_NEW(ChassisData);
		pint.mUserData = UserData;

		const float MainScale = 0.25f;
		const Point Extents(MainScale*8.0f, MainScale*2.0f, MainScale*4.0f);
		const Point ChassisPos(0.0f, 4.0f+Extents.y*4.0f, 0.0f);

		CreateVehicle(pint, ChassisPos, UserData, LowFrictionMaterial, ChassisGroup, WheelGroup);

		if(!mLoadTerrain)
			CreateTestScene(pint, &mHighFrictionMaterial);

		return true;
	}

END_TEST(Vehicle2b)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Vehicle3 = "Vehicle3";

START_VEHICLE_TEST(Vehicle3, CATEGORY_WIP, gDesc_Vehicle3)

	CylinderMesh mCylinder;
	CylinderMesh mCylinder2;

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(10.76f, 11.91f, 14.07f), Point(-0.54f, -0.49f, -0.68f));
		SetDefEnv(desc, false);
	}

	virtual	bool	CommonSetup()
	{
		mDriveMode				= DRIVE_MODE_NONE;
		mAcceleration			= 100.0f;
		mMaxAngularVelocity		= 10.0f;
//		mMaxAngularVelocity		= 5.0f;
		mSteeringForce			= 10.0f*100.0f;
		mCamera.mUpOffset		= 4.0f;
		mCamera.mDistToTarget	= 15.0f;
		mClampAngularVelocity	= true;
		mControlCamera			= gDriveVehicle;
		mLoadTerrain			= true;

		const float HalfHeight = 0.5f;
		const float Radius = 0.6f;
		const udword NbCirclePts = 16;
		mCylinder.Generate(NbCirclePts, Radius, HalfHeight);
		RegisterRenderer(CreateConvexRenderer(mCylinder.mNbVerts, mCylinder.mVerts));

		const float HalfHeight2 = 0.15f;
		const float Radius2 = 1.0f;
//		const float Radius2 = 1.1f;
		mCylinder2.Generate(NbCirclePts, Radius2, HalfHeight2);
		RegisterRenderer(CreateConvexRenderer(mCylinder2.mNbVerts, mCylinder2.mVerts));

		return VehicleBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		mCylinder.Reset();
		mCylinder2.Reset();
		VehicleBase::CommonRelease();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
//		if(!caps.mSupportPrismaticJoints || !caps.mSupportRigidBodySimulation)
//			return false;

		CreateMeshesFromRegisteredSurfaces(pint, caps, &mHighFrictionMaterial);


		const float Altitude = 4.0f;

		const float D = 6.0f;
		//
		CaterpillarTrackObjects TO0;
		CreateCaterpillarTrack_OLD(pint, TO0, &mHighFrictionMaterial, Point(0.0f, 2.0f+Altitude, 0.0f), mCylinder, GetRegisteredRenderers()[0], mCylinder2, GetRegisteredRenderers()[1]);

		CaterpillarTrackObjects TO1;
		CreateCaterpillarTrack_OLD(pint, TO1, &mHighFrictionMaterial, Point(0.0f, 2.0f+Altitude, D), mCylinder, GetRegisteredRenderers()[0], mCylinder2, GetRegisteredRenderers()[1]);

		ChassisData* UserData = ICE_NEW(ChassisData);
		pint.mUserData = UserData;
//		UserData->mChassis = TO0.mChassis;
		UserData->mFront[0].mWheel = TO0.mGears[0];
		UserData->mFront[1].mWheel = TO1.mGears[0];
		UserData->mFront[0].mParent = TO0.mChassis;
		UserData->mFront[1].mParent = TO0.mChassis;
		UserData->mRear[0].mWheel = TO0.mGears[1];
		UserData->mRear[1].mWheel = TO1.mGears[1];
		UserData->mRear[0].mParent = TO0.mChassis;
		UserData->mRear[1].mParent = TO0.mChassis;
//		UserData->mFrontAxleObject[0] = TO.mChassis;
//		UserData->mFrontAxleObject[1] = TO.mChassis;
		{
			PINT_FIXED_JOINT_CREATE fjc;
			fjc.mObject0 = TO0.mChassis;
			fjc.mObject1 = TO1.mChassis;
			fjc.mLocalPivot0 = Point(0.0f, 0.0f, D*0.5f);
			fjc.mLocalPivot1 = Point(0.0f, 0.0f, -D*0.5f);
			pint.CreateJoint(fjc);
		}
		{
			PINT_BOX_CREATE BoxDesc(3.0f, 2.0f, D*0.2f);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mPosition		= Point(0.0f, 4.0f+Altitude, D*0.5f);
			ObjectDesc.mShapes			= &BoxDesc;
			ObjectDesc.mMass			= 1.0f;
//			ObjectDesc.mCollisionGroup	= WheelGroup;
			PintActorHandle Object = CreatePintObject(pint, ObjectDesc);
			UserData->mChassis = Object;

			PINT_FIXED_JOINT_CREATE fjc;
			fjc.mObject0 = Object;
			fjc.mObject1 = TO0.mChassis;
			fjc.mLocalPivot0 = Point(0.0f, -2.0f, -BoxDesc.mExtents.x);
			fjc.mLocalPivot1 = Point(0.0f, 0.0f, D*0.5f-BoxDesc.mExtents.x);
			pint.CreateJoint(fjc);

			fjc.mObject0 = Object;
			fjc.mObject1 = TO1.mChassis;
			fjc.mLocalPivot0 = Point(0.0f, -2.0f, BoxDesc.mExtents.x);
			fjc.mLocalPivot1 = Point(0.0f, 0.0f, -D*0.5f+BoxDesc.mExtents.x);
			pint.CreateJoint(fjc);
		}

		CreateTestScene(pint, &mHighFrictionMaterial);

		return true;
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		if(!gDriveVehicle)
			return 0;

/*		ChassisData* UserData = (ChassisData*)pint.mUserData;
		if(!UserData)
			return 0;

		ChassisData Copy = *UserData;

		if(mInput.mRight)
		{
			UserData->mFront[1].mWheel = null;
			UserData->mRear[1].mWheel = null;
		}
		if(mInput.mLeft)
		{
			UserData->mFrontWheel[0] = null;
			UserData->mRear[0].mWheel = null;
		}

		const udword Val = VehicleBase::Update(pint);

		*UserData = Copy;

		return Val;*/


		ChassisData* UserData = (ChassisData*)pint.mUserData;
		if(!UserData)
			return 0;

		bool CanAccelerate = ClampAngularVelocity(pint, *UserData);

		{
			const float Coeff = mAcceleration;
			if(mInput.mKeyboard_Right)
			{
				pint.AddLocalTorque(UserData->mFront[0].mWheel, Point(0.0f, 0.0f, -Coeff));
				pint.AddLocalTorque(UserData->mRear[0].mWheel, Point(0.0f, 0.0f, -Coeff));
				pint.AddLocalTorque(UserData->mFront[1].mWheel, Point(0.0f, 0.0f, Coeff));
				pint.AddLocalTorque(UserData->mRear[1].mWheel, Point(0.0f, 0.0f, Coeff));
			}
			if(mInput.mKeyboard_Left)
			{
				pint.AddLocalTorque(UserData->mFront[1].mWheel, Point(0.0f, 0.0f, -Coeff));
				pint.AddLocalTorque(UserData->mRear[1].mWheel, Point(0.0f, 0.0f, -Coeff));
				pint.AddLocalTorque(UserData->mFront[0].mWheel, Point(0.0f, 0.0f, Coeff));
				pint.AddLocalTorque(UserData->mRear[0].mWheel, Point(0.0f, 0.0f, Coeff));
			}
		}


		const bool FWD = true;
		const bool RWD = true;
		{
			const float Coeff = mAcceleration;
			if(mInput.mKeyboard_Accelerate /*&& CanAccelerate*/)
			{
				if(FWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mFront[i].mWheel)
							pint.AddLocalTorque(UserData->mFront[i].mWheel, Point(0.0f, 0.0f, -Coeff));
					}
				}
				if(RWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mRear[i].mWheel)
							pint.AddLocalTorque(UserData->mRear[i].mWheel, Point(0.0f, 0.0f, -Coeff));
					}
				}
			}
			if(mInput.mKeyboard_Brake)
			{
				if(FWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mFront[i].mWheel)
							pint.AddLocalTorque(UserData->mFront[i].mWheel, Point(0.0f, 0.0f, Coeff));
					}
				}
				if(RWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mRear[i].mWheel)
							pint.AddLocalTorque(UserData->mRear[i].mWheel, Point(0.0f, 0.0f, Coeff));
					}
				}
			}
		}

		// Camera
		UpdateCamera(pint, UserData->mChassis);

		return 0;
	}

END_TEST(Vehicle3)

///////////////////////////////////////////////////////////////////////////////

static PintJointHandle CreateHinge(Pint& pint, PintActorHandle obj0, PintActorHandle obj1,
								   const Point& local_pivot0, const Point& local_pivot1,
								   const Point& local_axis0, const Point& local_axis1,
								   float min_limit = MIN_FLOAT, float max_limit = MAX_FLOAT)
{
	PINT_HINGE_JOINT_CREATE Desc;
	Desc.mObject0		= obj0;
	Desc.mObject1		= obj1;
	Desc.mLocalAxis0	= local_axis0;
	Desc.mLocalAxis1	= local_axis1;
	Desc.mLocalPivot0	= local_pivot0;
	Desc.mLocalPivot1	= local_pivot1;
	Desc.mMinLimitAngle	= min_limit;
	Desc.mMaxLimitAngle	= max_limit;
	return pint.CreateJoint(Desc);
}

static PintActorHandle CreateDebugSphere(Pint& pint, const Point& pos, float radius)
{
	PINT_SPHERE_CREATE Desc(radius);
	Desc.mRenderer = CreateSphereRenderer(Desc.mRadius);

	PINT_OBJECT_CREATE ObjectDesc;
	ObjectDesc.mShapes		= &Desc;
	ObjectDesc.mMass		= 0.0f;
	ObjectDesc.mPosition	= pos;
	return CreatePintObject(pint, ObjectDesc);
}

static PintActorHandle CreateGear(	Pint& pint, const PINT_MATERIAL_CREATE* material, const Point& pos, float gear_mass,
									const CylinderMesh& inside_cylinder, PintShapeRenderer* inside_cylinder_renderer,
									const CylinderMesh& outside_cylinder, PintShapeRenderer* outside_cylinder_renderer)
{
	PINT_CONVEX_CREATE ConvexCreate(inside_cylinder.mNbVerts, inside_cylinder.mVerts);
	ConvexCreate.mRenderer	= inside_cylinder_renderer;
	ConvexCreate.mMaterial	= material;

	PINT_CONVEX_CREATE ConvexCreate2(outside_cylinder.mNbVerts, outside_cylinder.mVerts);
	ConvexCreate2.mRenderer	= outside_cylinder_renderer;
	ConvexCreate2.mMaterial	= material;
	ConvexCreate2.mLocalPos	= Point(0.0f, 0.0f, inside_cylinder.mHalfHeight + outside_cylinder.mHalfHeight);
	ConvexCreate.mNext = &ConvexCreate2;

	PINT_CONVEX_CREATE ConvexCreate3 = ConvexCreate2;
	ConvexCreate3.mLocalPos	= Point(0.0f, 0.0f, - inside_cylinder.mHalfHeight - outside_cylinder.mHalfHeight);
	ConvexCreate2.mNext = &ConvexCreate3;

	PINT_OBJECT_CREATE ObjectDesc;
	ObjectDesc.mShapes		= &ConvexCreate;
	ObjectDesc.mMass		= gear_mass;
	ObjectDesc.mPosition	= pos;
	return CreatePintObject(pint, ObjectDesc);
}

/*
	struct CaterpillarTrackObjects
	{
		udword				mNbGears;
		PintActorHandle	mGears[4];
		PintActorHandle	mChassis;
	};*/

void CreateConvexHull2D(Vertices& verts);

static PINT_SHAPE_CREATE* CreateGearHolderShapes(PINT_SHAPE_CREATE*& StartShape, PINT_SHAPE_CREATE*& CurrentShape,
									const Point& pos, const Point& gear_center, udword nb_gears, const Point* gear_offsets,
									const CylinderMesh& inside_cylinder, const PINT_MATERIAL_CREATE* material, const Point& local_offset)
{
//	PINT_SHAPE_CREATE* StartShape = null;
//	PINT_SHAPE_CREATE* CurrentShape = null;
	for(udword i=0;i<nb_gears;i++)
	{
		const Point GearPos = pos + gear_offsets[i];
		{
			const Point GearOffset = GearPos - gear_center;
			const float LinkLength = GearOffset.Magnitude();
			const Point LinkExtents(LinkLength*0.5f, 0.1f, inside_cylinder.mHalfHeight*0.5f);

			PINT_BOX_CREATE* BoxDesc = ICE_NEW(PINT_BOX_CREATE)(LinkExtents);
			BoxDesc->mRenderer	= CreateBoxRenderer(BoxDesc->mExtents);
			BoxDesc->mMaterial	= material;
			BoxDesc->mLocalPos	= local_offset + GearOffset*0.5f;

			const float dx = GearOffset.x;
			const float dy = GearOffset.y;
			const float Angle = atan2f(dy, dx);

			Matrix3x3 Rot;
			Rot.RotZ(Angle);
			BoxDesc->mLocalRot = Rot;

			if(!StartShape)
				StartShape = BoxDesc;

			if(CurrentShape)
				CurrentShape->mNext = BoxDesc;

			CurrentShape = BoxDesc;
		}
	}
	return StartShape;
}

static PintActorHandle CreateCaterpillarTrackWIP(Pint& pint, CaterpillarTrackObjects& objects,
									const PINT_MATERIAL_CREATE* material, const Point& pos,
									const CylinderMesh& inside_cylinder, PintShapeRenderer* inside_cylinder_renderer,
									const CylinderMesh& outside_cylinder, PintShapeRenderer* outside_cylinder_renderer,
									udword nb_gears, const Point* gear_offsets, float gear_mass, PintActorHandle gear_holder=null, const Point* gear_holder_offset=null
									)
{
	/*
	TODO:
	- needs "32" passes to be cool
	- need larger contact offset & 4 threads

	- replace large debris with smaller ones à la Asteroid when they get crushed by the track

	- revisit all masses
	- fix angle for good
	- extra distance constraints
	- extra planar constraints
	*/

	// We tweak the incoming cylinders so that:
	// - they have a large amount of vertices. That way we get a smoother curve.
	// - they take the height of the track into account. That way the system doesn't start jittering when we increase the number of solver iterations.
	const float TrackHeight = 0.1f;

//	CylinderMesh TweakedInsideCylinder;
//	TweakedInsideCylinder.Generate(256, inside_cylinder.mRadius, inside_cylinder.mHalfHeight, inside_cylinder.mOrientation);

	CylinderMesh TweakedOutsideCylinder;
	TweakedOutsideCylinder.Generate(256, outside_cylinder.mRadius+TrackHeight, outside_cylinder.mHalfHeight, outside_cylinder.mOrientation);

	Point GearCenter(0.0f, 0.0f, 0.0f);
	for(udword i=0;i<nb_gears;i++)
	{
		const Point GearPos = pos + gear_offsets[i];
		GearCenter += GearPos;
	}
	GearCenter /= float(nb_gears);

	// Gear holder
	PintActorHandle GearHolder = gear_holder;
	if(!GearHolder)
	{
		PINT_SHAPE_CREATE* StartShape = null;
		PINT_SHAPE_CREATE* CurrentShape = null;
		StartShape = CreateGearHolderShapes(StartShape, CurrentShape, pos, GearCenter, nb_gears, gear_offsets, inside_cylinder, material, Point(0.0f, 0.0f, 0.0f));

		if(StartShape)
		{
			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes		= StartShape;
			ObjectDesc.mMass		= 10.0f;
			ObjectDesc.mPosition	= GearCenter;

			GearHolder = CreatePintObject(pint, ObjectDesc);

			while(StartShape)
			{
				PINT_SHAPE_CREATE* NextShape = const_cast<PINT_SHAPE_CREATE*>(StartShape->mNext);
				DELETESINGLE(StartShape);
				StartShape = NextShape;
			}
		}
	}
	objects.mChassis = GearHolder;
//	return;

	Vertices V;
	objects.mNbGears = nb_gears;
	for(udword i=0;i<nb_gears;i++)
	{
		const Point GearPos = pos + gear_offsets[i];
		PintActorHandle CurrentGear = CreateGear(pint, material, GearPos, gear_mass, inside_cylinder, inside_cylinder_renderer, outside_cylinder, outside_cylinder_renderer);
		objects.mGears[i] = CurrentGear;

//		PintJointHandle h = CreateHinge(pint, CurrentGear, null,
//								   Point(0.0f, 0.0f, 0.0f), GearPos,
//								   Point(0.0f, 0.0f, 1.0f), Point(0.0f, 0.0f, 1.0f));

		PintJointHandle h = CreateHinge(pint, CurrentGear, GearHolder,
									Point(0.0f, 0.0f, 0.0f), gear_holder_offset ? (*gear_holder_offset) + GearPos - GearCenter : GearPos - GearCenter,
								   Point(0.0f, 0.0f, 1.0f), Point(0.0f, 0.0f, 1.0f));
		(void)h;

		if(0)
		{
			const Point GearOffset = GearPos - GearCenter;
			const float LinkLength = GearOffset.Magnitude();
			const Point LinkExtents(LinkLength*0.5f, 0.1f, inside_cylinder.mHalfHeight*0.5f);
			PINT_BOX_CREATE BoxDesc(LinkExtents);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
			BoxDesc.mMaterial	= material;

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes		= &BoxDesc;
			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mPosition	= GearCenter + GearOffset*0.5f;

				const float dx = GearOffset.x;
				const float dy = GearOffset.y;
				const float Angle = atan2f(dy, dx);

				Matrix3x3 Rot;
				Rot.RotZ(Angle);
				ObjectDesc.mRotation = Rot;

			CreatePintObject(pint, ObjectDesc);
		}

//		const udword NbPtsInCircle = outside_cylinder.mNbVerts/2;
		const udword NbPtsInCircle = TweakedOutsideCylinder.mNbVerts/2;
		for(udword j=0;j<NbPtsInCircle;j++)
		{
			Point p = TweakedOutsideCylinder.mVerts[j];
//			Point p = outside_cylinder.mVerts[j];
			p.z = 0.0f;	//#### hardcoded
			p += GearPos;
			V.AddVertex(p);
//			CreateDebugSphere(pint, p);
		}
	}
//return GearHolder;
	/////

	CreateConvexHull2D(V);

	/////

	const udword NbVertices = V.GetNbVertices();
	Curve C;
	C.Init(NbVertices);
	C.mClosed = true;
	for(udword i=0;i<NbVertices;i++)
	{
		const Point& p = V.GetVertices()[NbVertices - i - 1];
//		CreateDebugSphere(pint, p, float(i+1)*0.01f);
		C.SetVertex(p.x, p.y, pos.z, i);
	}

	const float Length = C.ComputeLength();

	/////

	const udword NbLinks = 32;
	const float D = Length*0.5f/float(NbLinks);
	const float TrackWidth = inside_cylinder.mHalfHeight + outside_cylinder.mHalfHeight*2.0f;
	const Point Extents(D, TrackHeight, TrackWidth);

	// Main plank
	PINT_BOX_CREATE BoxDesc(Extents);
	BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
	BoxDesc.mMaterial	= material;

		// Smaller inside box to catch the gear
		const float AvailableSpaceY = 0.5f*(outside_cylinder.mRadius - inside_cylinder.mRadius);
		const float AvailableSpaceZ = inside_cylinder.mHalfHeight;
		const float LengthUsed = 0.75f;
		PINT_BOX_CREATE BoxDesc2(Extents.x*0.2f, AvailableSpaceY*LengthUsed, AvailableSpaceZ*LengthUsed);
		BoxDesc2.mRenderer	= CreateBoxRenderer(BoxDesc2.mExtents);
		BoxDesc2.mMaterial	= material;
		BoxDesc2.mLocalPos	= Point(0.0f, -Extents.y - BoxDesc2.mExtents.y, 0.0f);
		BoxDesc.mNext		= &BoxDesc2;

		// Upper box that will contact with the ground
		const float BumpSize = 0.05f;
		PINT_BOX_CREATE BoxDesc3(BumpSize, BumpSize, Extents.z*0.9f);
		BoxDesc3.mRenderer	= CreateBoxRenderer(BoxDesc3.mExtents);
		BoxDesc3.mMaterial	= material;
		BoxDesc3.mLocalPos	= Point(0.0f, Extents.y, 0.0f);
		Matrix3x3 Rot45;
		Rot45.RotZ(PI/4.0f);
		BoxDesc3.mLocalRot	= Rot45;
		BoxDesc2.mNext		= &BoxDesc3;

	PINT_OBJECT_CREATE ObjectDesc;
	ObjectDesc.mShapes		= &BoxDesc;
	ObjectDesc.mMass		= 1.0f;//#####LinkMass;

	PintActorHandle Objects[128];
//	Point Positions[128];

	PintActorHandle Parent = null;
	for(udword i=0;i<NbLinks;i++)
	{
		const udword ii = (i)%NbLinks;

		const float Coeff = float(ii)/float(NbLinks);

		Point pts[2];
		C.GetPoint(ObjectDesc.mPosition, Coeff, pts);

//		ObjectDesc.mPosition += pos;

		// #### that angle is not quite correct yet
		const float dx = pts[1].x - pts[0].x;
		const float dy = pts[1].y - pts[0].y;
		const float Angle = atan2f(dy, dx);

		Matrix3x3 Rot;
		Rot.RotZ(Angle);
		ObjectDesc.mRotation = Rot;

		Parent = CreatePintObject(pint, ObjectDesc);
		Objects[i] = Parent;
	}

	/////

	{
		const bool UseExtraDistanceConstraints = true;
		for(udword j=0;j<32;j++)
//		for(udword j=0;j<4;j++)
		{
			for(udword i=0;i<NbLinks;i++)
			{
				const udword ii = (i)%NbLinks;
				const udword jj = (i+1)%NbLinks;

				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mObject0		= Objects[ii];
				Desc.mObject1		= Objects[jj];
				Desc.mLocalPivot0	= Point(D, 0.0f, 0.0f);
				Desc.mLocalPivot1	= Point(-D, 0.0f, 0.0f);
				Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
				Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
			if(UseExtraDistanceConstraints)
			{
				for(udword i=0;i<NbLinks;i++)
				{
					const udword ii = (i)%NbLinks;
					const udword jj = (i+2)%NbLinks;

					PINT_DISTANCE_JOINT_CREATE Desc;
					Desc.mObject0		= Objects[ii];
					Desc.mObject1		= Objects[jj];
	//				Desc.mMaxDistance	= Positions[ii].Distance(Positions[jj]);
					Desc.mMaxDistance	= Extents.x*2.0f*2.0f;
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
			}
		}
	}
	return GearHolder;


#ifdef REMOVED
	const float Radius = 4.0f;
//	const float Altitude = 2.0f;
//	const float Altitude = pos.y;
	const float LinkMass = 1.0f;
	const float GearMass = 10.0f;
	const float SideMass = 4.0f;

	////

	Curve2 C;
//	const udword NbCurvePts = 4;
	const udword NbCurvePts = 1024;
//	const udword NbCurvePts = 64;
//	const udword NbCurvePts = 16;
	C.Init(NbCurvePts);
	C.mClosed = true;
	for(udword i=0;i<NbCurvePts;i++)
	{
		const float Coeff = float(i)/float(NbCurvePts);
		const float Angle = Coeff*TWOPI;
		const float x = sinf(Angle) * Radius;
		/*const*/ float y = cosf(Angle) * Radius * 0.5f;
		const float LimitP = 0.5f*Radius;
//		const float LimitP = 0.5f*Radius*0.5f;
		const float LimitN = 0.5f*Radius*0.5f;
		if(y>LimitP)
			y=LimitP;
		if(y<-LimitN)
			y=-LimitN;
		C.SetVertex(x, y/*+Altitude*/, 0.0f, i);
	}
	const float Length = C.ComputeLength();

	////

//	const udword NbLinks = 48;
//	const udword NbLinks = 32*2;
	const udword NbLinks = 32;	//
//	const udword NbLinks = 64;
//	const udword NbLinks = 16;
//	const udword NbLinks = 128;
//	const float D = PI*Radius/float(NbLinks);
	const float D = Length*0.5f/float(NbLinks);
//	const Point Extents(D*0.8f, 0.1f, 1.0f);
	const Point Extents(D, 0.1f, 1.0f);
//	const Point ArticulationPos(0.0f, Altitude + Extents.y, 0.0f);
//	const Point ArticulationPos = pos + Point(0.0f, Extents.y, 0.0f);

	//

	// Main plank
	PINT_BOX_CREATE BoxDesc(Extents);
	BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
	BoxDesc.mMaterial	= material;

		// Smaller inside box to catch the gear
		PINT_BOX_CREATE BoxDesc2(Extents.x*0.2f, 0.2f, Extents.z*0.4f);
//		BoxDesc2.mExtents	= Point(Extents.x*0.2f, 0.15f, Extents.z*0.4f);
//		BoxDesc2.mExtents	= Point(Extents.x*0.2f, 0.2f, Extents.z*0.4f);
		BoxDesc2.mRenderer	= CreateBoxRenderer(BoxDesc2.mExtents);
		BoxDesc2.mMaterial	= material;
		BoxDesc2.mLocalPos	= Point(0.0f, -Extents.y - BoxDesc2.mExtents.y, 0.0f);
		BoxDesc.mNext		= &BoxDesc2;

		// Left & right upper boxes (will touch the ground)
		PINT_BOX_CREATE BoxDesc3(Extents.x, 0.4f, 0.1f);
//		BoxDesc3.mExtents	= Point(Extents.x, 0.15f, 0.1f);
//		BoxDesc3.mExtents	= Point(Extents.x, 0.3f, 0.1f);
//			BoxDesc3.mExtents	= Point(Extents.x, 0.4f, 0.1f);
//BoxDesc3.mExtents	= Point(Extents.x, 0.3f, 0.1f);
		BoxDesc3.mRenderer	= CreateBoxRenderer(BoxDesc3.mExtents);
		BoxDesc3.mMaterial	= material;
		BoxDesc3.mLocalPos	= Point(0.0f, BoxDesc3.mExtents.y - Extents.y, Extents.z + BoxDesc3.mExtents.z);
			BoxDesc3.mLocalPos	= Point(0.0f, 0.0f, Extents.z + BoxDesc3.mExtents.z);
//		BoxDesc3.mLocalPos	= Point(0.0f, -BoxDesc3.mExtents.y, Extents.z + BoxDesc3.mExtents.z);
//BoxDesc3.mLocalPos	= Point(0.0f, - BoxDesc3.mExtents.y + Extents.y, Extents.z + BoxDesc3.mExtents.z);
		BoxDesc2.mNext		= &BoxDesc3;

		PINT_BOX_CREATE BoxDesc4(BoxDesc3.mExtents);
		BoxDesc4.mRenderer	= BoxDesc3.mRenderer;
		BoxDesc4.mMaterial	= material;
		BoxDesc4.mLocalPos	= Point(0.0f, BoxDesc3.mLocalPos.y, -BoxDesc3.mLocalPos.z);
		BoxDesc3.mNext		= &BoxDesc4;

	PINT_OBJECT_CREATE ObjectDesc;
	ObjectDesc.mShapes		= &BoxDesc;
	ObjectDesc.mMass		= LinkMass;
//	ObjectDesc.mPosition	= ArticulationPos;

	//
//	PintArticHandle Articulation = pint.CreateArticulation();
	PintArticHandle Articulation = null;

	float Angles[128];
	for(udword i=0;i<NbLinks;i++)
	{
		const float Coeff = float(i)/float(NbLinks);
		Point p0,p1,tmp;
		C.GetPointEx(tmp, Coeff, p0, p1);
		const float dx = p1.x - p0.x;
		const float dy = p1.y - p0.y;
		Angles[i] = atan2f(dy, dx);
	}

	PintActorHandle Objects[128];
	Point Positions[128];
	PintActorHandle Parent = null;
	for(udword i=0;i<NbLinks;i++)
	{
		const udword ii = (i)%NbLinks;

		const float Coeff = float(ii)/float(NbLinks);

		Point p0,p1;
		C.GetPointEx(ObjectDesc.mPosition, Coeff, p0, p1);

		ObjectDesc.mPosition += pos;

		float Angle2;
		if(0)
		{
			const udword jj = (i+1)%NbLinks;
			const float NextCoeff = float(jj)/float(NbLinks);
			Point NextPt;
			C.GetPoint(NextPt, NextCoeff);
			const float dx = NextPt.x - ObjectDesc.mPosition.x;
			const float dy = NextPt.y - ObjectDesc.mPosition.y;
			Angle2 = atan2f(dy, dx);
		}
		if(1)
		{
			const float dx = p1.x - p0.x;
			const float dy = p1.y - p0.y;
			Angle2 = atan2f(dy, dx);
		}
		if(0)
		{
			const udword j = (i-1)%NbLinks;
			const udword k = (i+1)%NbLinks;
			Angle2 = (Angles[i] + Angles[j] + Angles[k])/3.0f;
		}

		Matrix3x3 Rot;
//		Rot.RotZ(-Angle);
		Rot.RotZ(Angle2);
		ObjectDesc.mRotation = Rot;

		if(Articulation)
		{
			PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
			if(Parent)
			{
				ArticulatedDesc.mParent = Parent;
				ArticulatedDesc.mLocalPivot0 = Point(D, 0.0f, 0.0f);
				ArticulatedDesc.mLocalPivot1 = Point(-D, 0.0f, 0.0f);
			}
			if(0)
			{
				ArticulatedDesc.mX = Point(0.0f, 0.0f, 1.0f);
				ArticulatedDesc.mEnableTwistLimit = false;
				ArticulatedDesc.mTwistLowerLimit = -0.001f;
				ArticulatedDesc.mTwistUpperLimit = 0.001f;
				ArticulatedDesc.mEnableSwingLimit = true;
				ArticulatedDesc.mSwingYLimit = 0.001f;
				ArticulatedDesc.mSwingZLimit = 0.001f;
			}
			else
			{
				ArticulatedDesc.mEnableTwistLimit = true;
				ArticulatedDesc.mTwistLowerLimit = -0.001f;
				ArticulatedDesc.mTwistUpperLimit = 0.001f;
				ArticulatedDesc.mEnableSwingLimit = true;
				ArticulatedDesc.mSwingYLimit = 0.001f;
				ArticulatedDesc.mSwingZLimit = 0.001f;
				ArticulatedDesc.mSwingYLimit = FLT_MAX;
			}
			Parent = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
		}
		else
		{
			Parent = CreatePintObject(pint, ObjectDesc);
		}
		Objects[i] = Parent;
		Positions[i] = ObjectDesc.mPosition;
	}
	if(Articulation)
		pint.AddArticulationToScene(Articulation);

	if(Articulation)
	{
		PINT_HINGE_JOINT_CREATE Desc;
		Desc.mObject0		= Objects[0];
		Desc.mObject1		= Objects[NbLinks-1];
		Desc.mLocalPivot0	= Point(-D, 0.0f, 0.0f);
		Desc.mLocalPivot1	= Point(D, 0.0f, 0.0f);
		Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
		Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
		PintJointHandle JointHandle = pint.CreateJoint(Desc);
		ASSERT(JointHandle);
	}
	else
	{
		const bool UseExtraDistanceConstraints = false;
//		for(udword j=0;j<4;j++)
		{
			for(udword i=0;i<NbLinks;i++)
			{
				const udword ii = (i)%NbLinks;
				const udword jj = (i+1)%NbLinks;

				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mObject0		= Objects[ii];
				Desc.mObject1		= Objects[jj];
				Desc.mLocalPivot0	= Point(D, 0.0f, 0.0f);
				Desc.mLocalPivot1	= Point(-D, 0.0f, 0.0f);
				Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
				Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
			if(UseExtraDistanceConstraints)
			{
				for(udword i=0;i<NbLinks;i++)
				{
					const udword ii = (i)%NbLinks;
					const udword jj = (i+2)%NbLinks;

					PINT_DISTANCE_JOINT_CREATE Desc;
					Desc.mObject0		= Objects[ii];
					Desc.mObject1		= Objects[jj];
	//				Desc.mMaxDistance	= Positions[ii].Distance(Positions[jj]);
					Desc.mMaxDistance	= Extents.x*2.0f*2.0f;
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
			}
		}
	}

	const float NbGears = 3;
	objects.mNbGears = NbGears;
//	const float x = Radius - mCylinder.mRadius - 0.005f;
//	const float x = Radius - mCylinder2.mRadius - 0.1f;
	const float x = Radius - mCylinder2.mRadius - 0.075f;
//	const float x = Radius - mCylinder2.mRadius - 0.05f;
	PintActorHandle GearObject[3];
	const float xs[3] = { x, -x, 0.0f };
//	const float ys[3] = { 0.0f, 0.0f, 0.0f };
	const float ys[3] = { 0.0f, 0.0f, 1.5f };
	for(udword i=0;i<NbGears;i++)
	{
		PINT_CONVEX_CREATE ConvexCreate;
		ConvexCreate.mNbVerts	= mCylinder.mNbVerts;
		ConvexCreate.mVerts		= mCylinder.mVerts;
		ConvexCreate.mRenderer	= cylinder_renderer;
		ConvexCreate.mMaterial	= material;

		PINT_CONVEX_CREATE ConvexCreate2;
		ConvexCreate2.mNbVerts	= mCylinder2.mNbVerts;
		ConvexCreate2.mVerts	= mCylinder2.mVerts;
		ConvexCreate2.mRenderer	= cylinder_renderer2;
		ConvexCreate2.mMaterial	= material;
		ConvexCreate2.mLocalPos	= Point(0.0f, 0.0f, mCylinder.mHalfHeight+mCylinder2.mHalfHeight);
		ConvexCreate.mNext = &ConvexCreate2;

		PINT_CONVEX_CREATE ConvexCreate3 = ConvexCreate2;
		ConvexCreate3.mLocalPos	= Point(0.0f, 0.0f, -mCylinder.mHalfHeight-mCylinder2.mHalfHeight);
		ConvexCreate2.mNext = &ConvexCreate3;

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes		= &ConvexCreate;
		ObjectDesc.mMass		= GearMass;
//		ObjectDesc.mPosition	= Point(xs[i], Altitude, 0.0f);
		ObjectDesc.mPosition	= pos + Point(xs[i], ys[i], 0.0f);
		GearObject[i] = CreatePintObject(pint, ObjectDesc);
		ASSERT(GearObject[i]);
		objects.mGears[i] = GearObject[i];
	}
	{
		const float Gap = 0.1f;
		PINT_BOX_CREATE SideBoxDesc(x, 0.3f, 0.1f);
		SideBoxDesc.mRenderer	= CreateBoxRenderer(SideBoxDesc.mExtents);
		const float z = mCylinder.mHalfHeight + mCylinder2.mHalfHeight*2.0f + SideBoxDesc.mExtents.z + Gap;
		SideBoxDesc.mLocalPos	= Point(0.0f, 0.0f, z);

		PINT_BOX_CREATE SideBoxDesc2 = SideBoxDesc;
		SideBoxDesc2.mLocalPos	= Point(0.0f, 0.0f, -z);
		SideBoxDesc.mNext = &SideBoxDesc2;

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes		= &SideBoxDesc;
		ObjectDesc.mMass		= SideMass;
//		ObjectDesc.mPosition	= Point(0.0f, Altitude, 0.0f);
		ObjectDesc.mPosition	= pos;
		PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
		ASSERT(Handle);
		objects.mChassis = Handle;

		for(udword i=0;i<NbGears;i++)
		{
			PINT_HINGE_JOINT_CREATE Desc;
			Desc.mObject0		= GearObject[i];
			Desc.mObject1		= Handle;
			Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
			Desc.mLocalPivot1	= Point(xs[i], ys[i], 0.0f);
			Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
			Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
	}

	if(0)
	{
		PINT_CONVEX_CREATE ConvexCreate;
		ConvexCreate.mNbVerts	= mCylinder.mNbVerts;
		ConvexCreate.mVerts		= mCylinder.mVerts;
		ConvexCreate.mRenderer	= cylinder_renderer;
		ConvexCreate.mLocalPos	= Point(x, 0.0f, 0.0f);

		PINT_CONVEX_CREATE ConvexCreate2;
		ConvexCreate2.mNbVerts	= mCylinder.mNbVerts;
		ConvexCreate2.mVerts	= mCylinder.mVerts;
		ConvexCreate2.mRenderer	= cylinder_renderer;
		ConvexCreate2.mLocalPos	= Point(-x, 0.0f, 0.0f);
		ConvexCreate.mNext = &ConvexCreate2;

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes		= &ConvexCreate;
		ObjectDesc.mMass		= 20.0f;
//		ObjectDesc.mPosition	= Point(0.0f, Altitude, 0.0f);
		ObjectDesc.mPosition	= pos;
		PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
		ASSERT(Handle);
	}
#endif
}


static const char* gDesc_Bulldozer = "Bulldozer";

static PintActorHandle CreateBoxObject(Pint& pint, const Point& extents, const Point& pos, float mass, const Quat* rotation=null)
{
	PINT_BOX_CREATE BoxDesc(extents);
	BoxDesc.mRenderer	= CreateBoxRenderer(extents);

	PINT_OBJECT_CREATE ObjectDesc;
	ObjectDesc.mPosition	= pos;
	ObjectDesc.mShapes		= &BoxDesc;
	ObjectDesc.mMass		= mass;
	if(rotation)
		ObjectDesc.mRotation = *rotation;
	return CreatePintObject(pint, ObjectDesc);
}

static void CreatePrismatic(Pint& pint, const Point& p0, const Point& p1,
							PintActorHandle top_anchor, const Point& top_pivot,
							PintActorHandle bottom_anchor, const Point& bottom_pivot
							)
{
	const float TotalLength = p0.Distance(p1);

	const float L = TotalLength*0.5f;

	const Point TopPrismaticExtents(0.15f, L*0.5f, 0.15f);
	const Point BottomPrismaticExtents(0.1f, L*0.5f, 0.1f);

	const Point Dir = (p1-p0).Normalize();

/*	const float PrismaticRotation = 45.0f;
	Matrix3x3 R;
	R.RotZ(DEGTORAD*PrismaticRotation);
	const Quat Q = R;*/
	const Quat Q = ShortestRotation(Point(0.0f, 1.0f, 0.0f), Dir);

	const Point TopPrismaticPos = p0 + Dir * L * 0.5f;
	const Point BottomPrismaticPos = TopPrismaticPos + Dir * L;

	const float PrismaticMass = 1.0f;
	PintActorHandle TopPrismaticObject = CreateBoxObject(pint, TopPrismaticExtents, TopPrismaticPos, PrismaticMass, &Q);
	PintActorHandle BottomPrismaticObject = CreateBoxObject(pint, BottomPrismaticExtents, BottomPrismaticPos, PrismaticMass, &Q);

	{
		PINT_PRISMATIC_JOINT_CREATE Desc;
		Desc.mObject0			= TopPrismaticObject;
		Desc.mObject1			= BottomPrismaticObject;
		//###PRISMATIC2
		Desc.mLocalPivot0.mPos	= Point(0.0f, 0.0f, 0.0f);
		Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
		Desc.mLocalAxis0		= Point(0.0f, 1.0f, 0.0f);
		Desc.mLocalAxis1		= Point(0.0f, 1.0f, 0.0f);
//		Desc.mMaxLimit			= 0.0f;
//		Desc.mMinLimit			= -TopPrismaticExtents.y*2.0f;
		Desc.mMaxLimit			= TopPrismaticExtents.y*2.0f;
		Desc.mMinLimit			= 0.0f;
//		Desc.mMaxLimit			= 0.0f;
//		Desc.mMinLimit			= 0.0f;
		if(1)
		{
			Desc.mMaxLimit			= TopPrismaticExtents.y*2.0f;
			Desc.mMinLimit			= TopPrismaticExtents.y*1.9f;
			Desc.mSpringStiffness	= 400.0f;
			Desc.mSpringDamping		= 10.0f;
		}
		PintJointHandle JointHandle = pint.CreateJoint(Desc);
		ASSERT(JointHandle);
	}

	PINT_HINGE_JOINT_CREATE Desc;
	//### hardcoded
	Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
	Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
	if(top_anchor)
	{
		Desc.mObject0		= TopPrismaticObject;
		Desc.mObject1		= top_anchor;
		Desc.mLocalPivot0	= Point(0.0f, -TopPrismaticExtents.y, 0.0f);
		Desc.mLocalPivot1	= top_pivot;
		PintJointHandle JointHandle = pint.CreateJoint(Desc);
		ASSERT(JointHandle);
	}

	if(bottom_anchor)
	{
		Desc.mObject0		= BottomPrismaticObject;
		Desc.mObject1		= bottom_anchor;
		Desc.mLocalPivot0	= Point(0.0f, BottomPrismaticExtents.y, 0.0f);
		Desc.mLocalPivot1	= bottom_pivot;
		PintJointHandle JointHandle = pint.CreateJoint(Desc);
		ASSERT(JointHandle);
	}
}

START_VEHICLE_TEST(Bulldozer, CATEGORY_WIP, gDesc_Bulldozer)

	CylinderMesh mInsideCylinder;
	CylinderMesh mOutsideCylinder;

	virtual	void Bulldozer::GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(8.54f, 4.57f, 7.70f), Point(-0.66f, -0.28f, -0.69f));
		SetDefEnv(desc, false);
	}

	virtual	bool Bulldozer::CommonSetup()
	{
		mDriveMode				= DRIVE_MODE_NONE;
		mAcceleration			= 200.0f;
//		mAcceleration			= 100.0f;
//		mMaxAngularVelocity		= 17.0f;
		mMaxAngularVelocity		= 10.0f;
//		mMaxAngularVelocity		= 15.0f;
		mSteeringForce			= 10.0f*100.0f;
//		mCameraUpOffset			= 4.0f;
		mCamera.mUpOffset		= 10.0f;
		mCamera.mDistToTarget	= 10.0f;
		mClampAngularVelocity	= true;
		mControlCamera			= true;
		mLoadTerrain			= false;



		const float HalfHeight = 0.5f;
		const float Radius = 0.6f;
		const udword NbCirclePts = 16;
//		const udword NbCirclePts = 32;
		mInsideCylinder.Generate(NbCirclePts, Radius, HalfHeight);
		RegisterRenderer(CreateConvexRenderer(mInsideCylinder.mNbVerts, mInsideCylinder.mVerts));

		const float HalfHeight2 = 0.15f;
		const float Radius2 = 1.0f;
		mOutsideCylinder.Generate(NbCirclePts, Radius2, HalfHeight2);
		RegisterRenderer(CreateConvexRenderer(mOutsideCylinder.mNbVerts, mOutsideCylinder.mVerts));

		//

		return VehicleBase::CommonSetup();
	}

	virtual	void Bulldozer::CommonRelease()
	{
		mInsideCylinder.Reset();
		mOutsideCylinder.Reset();
		VehicleBase::CommonRelease();
	}

	virtual bool Bulldozer::Setup(Pint& pint, const PintCaps& caps)
	{
		CreateMeshesFromRegisteredSurfaces(pint, caps, &mHighFrictionMaterial);

		ChassisData* UserData = ICE_NEW(ChassisData);
		pint.mUserData = UserData;

		const float TrackWidth = mInsideCylinder.mHalfHeight + mOutsideCylinder.mHalfHeight*2.0f;
		const float TrackGap = 0.1f;

//		const Point ChassisExtents(2.0f, 2.0f, 1.0f);
		const Point ChassisExtents(3.0f, 2.0f, 1.0f);

		const float GroundClearance = 0.5f;
		const Point ChassisPos(0.0f, ChassisExtents.y+GroundClearance, 0.0f);
		const float ChassisMass = 100.0f;
//		const float ChassisMass = 0.0f;

		const float ChassisGap = (TrackWidth+TrackGap)*2.0f;

		const float SideDistToGround = 0.5f*2.0f;
//		const Point SideExtents(3.0f, 0.2f, 0.2f);
		const Point SideExtents(4.0f, 0.2f, 0.2f);
		const Point LeftSidePos(SideExtents.x-ChassisExtents.x, SideExtents.y+SideDistToGround, -ChassisExtents.z-ChassisGap-SideExtents.z);
		const Point RightSidePos(SideExtents.x-ChassisExtents.x, SideExtents.y+SideDistToGround, ChassisExtents.z+ChassisGap+SideExtents.z);
		const float SideMass = 1.0f;

//		PintActorHandle ChassisObject = CreateBoxObject(pint, ChassisExtents, ChassisPos, ChassisMass);
			PINT_BOX_CREATE ChassisBoxDesc(ChassisExtents);
			ChassisBoxDesc.mRenderer	= CreateBoxRenderer(ChassisExtents);
//			ChassisBoxDesc.mLocalPos	= ChassisPos;
			PintActorHandle ChassisObject = null;

		if(1)
		{
			// Caterpillar tracks
			const udword NbGears = 3;
			Point GearOffsets[3];
			const float GearMass = 10.0f;
			GearOffsets[0] = Point(0.0f, 2.0f, 0.0f);
			GearOffsets[1] = Point(-2.0f, 0.0f, 0.0f);
			GearOffsets[2] = Point(2.0f, 0.0f, 0.0f);
			if(0)
			{
				CaterpillarTrackObjects CTO;

				for(udword i=0;i<2;i++)
				{
					const float Coeff = i ? -1.0f : 1.0f;
					const Point Pos(0.0f, 1.5f, Coeff*(ChassisExtents.z+TrackGap+TrackWidth));

			//		CreateCaterpillarTrackWIP(pint, &mZeroFrictionMaterial, Pos,
					CreateCaterpillarTrackWIP(pint, CTO, &mHighFrictionMaterial, Pos,
											mInsideCylinder, GetRegisteredRenderers()[0], mOutsideCylinder, GetRegisteredRenderers()[1],
											NbGears, GearOffsets, GearMass
						);
				}
			}
			else
			{
				PINT_SHAPE_CREATE* StartShape = null;
				PINT_SHAPE_CREATE* CurrentShape = null;
//				PINT_SHAPE_CREATE* StartShape = &ChassisBoxDesc;
//				PINT_SHAPE_CREATE* CurrentShape = &ChassisBoxDesc;
				for(udword i=0;i<2;i++)
				{
					const float Coeff = i ? -1.0f : 1.0f;
					/*const*/ Point Pos(0.0f, 1.5f, Coeff*(ChassisExtents.z+TrackGap+TrackWidth));
					Pos -= ChassisPos;

					{
						Point GearCenter(0.0f, 0.0f, 0.0f);
						for(udword i=0;i<NbGears;i++)
						{
							const Point GearPos = Pos + GearOffsets[i];
							GearCenter += GearPos;
						}
						GearCenter /= float(NbGears);

						StartShape = CreateGearHolderShapes(StartShape, CurrentShape, Pos, GearCenter, NbGears, GearOffsets, mInsideCylinder, &mHighFrictionMaterial, GearCenter);

	/*					PINT_SHAPE_CREATE* TmpShape = StartShape;
						while(TmpShape)
						{
							TmpShape->mLocalPos += Pos;
							TmpShape = const_cast<PINT_SHAPE_CREATE*>(TmpShape->mNext);
						}*/

					}
				}

				PintActorHandle GearHolder = null;
				if(StartShape)
				{
					if(1)
					{
						ChassisBoxDesc.mNext = StartShape;

						PINT_OBJECT_CREATE ChassisObjectDesc;
//						ChassisObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
						ChassisObjectDesc.mPosition	= ChassisPos;
						ChassisObjectDesc.mShapes	= &ChassisBoxDesc;
						ChassisObjectDesc.mMass		= ChassisMass;
						GearHolder = CreatePintObject(pint, ChassisObjectDesc);
						ChassisObject = GearHolder;
					}
					else
					{
						PINT_OBJECT_CREATE ObjectDesc;
						ObjectDesc.mShapes		= StartShape;
						ObjectDesc.mMass		= 10.0f;
		//				ObjectDesc.mPosition	= GearCenter;
						ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
						GearHolder = CreatePintObject(pint, ObjectDesc);
					}

					while(StartShape)
					{
						PINT_SHAPE_CREATE* NextShape = const_cast<PINT_SHAPE_CREATE*>(StartShape->mNext);
						DELETESINGLE(StartShape);
						StartShape = NextShape;
					}
				}
//	return true;
				if(1)
				{
					UserData->mChassis = GearHolder;
//					UserData->mNbWheels = NbGears;
/*					for(udword i=0;i<NbGears;i++)
					{
						UserData->mFront[i].mParent = GearHolder;
						UserData->mRear[i].mParent = GearHolder;
					}*/
					UserData->mFront[0].mParent = GearHolder;
					UserData->mFront[1].mParent = GearHolder;
					UserData->mRear[0].mParent = GearHolder;
					UserData->mRear[1].mParent = GearHolder;

					for(udword i=0;i<2;i++)
					{
						const float Coeff = i ? -1.0f : 1.0f;
						/*const*/ Point Pos(0.0f, 1.5f, Coeff*(ChassisExtents.z+TrackGap+TrackWidth));
//						Pos -= ChassisPos;

							Point GearCenter(0.0f, 0.0f, 0.0f);
							for(udword j=0;j<NbGears;j++)
							{
								const Point GearPos = Pos + GearOffsets[j];
								GearCenter += GearPos;
							}
							GearCenter /= float(NbGears);
							GearCenter -= ChassisPos;

							CaterpillarTrackObjects CTO;
					//		CreateCaterpillarTrackWIP(pint, &mZeroFrictionMaterial, Pos,
							CreateCaterpillarTrackWIP(pint, CTO, &mHighFrictionMaterial, Pos,
													mInsideCylinder, GetRegisteredRenderers()[0], mOutsideCylinder, GetRegisteredRenderers()[1],
													NbGears, GearOffsets, GearMass, GearHolder, &GearCenter);

//							for(udword j=0;j<NbGears;j++)
							{
								UserData->mFront[i].mWheel = CTO.mGears[1];
								UserData->mRear[i].mWheel = CTO.mGears[2];
							}
					}
				}
			}
//			return true;
		}

		PintActorHandle LeftSideObject = CreateBoxObject(pint, SideExtents, LeftSidePos, SideMass);
		PintActorHandle RightSideObject = CreateBoxObject(pint, SideExtents, RightSidePos, SideMass);

		const Point HingeAxis(0.0f, 0.0f, 1.0f);
		CreateHinge(pint, ChassisObject, LeftSideObject, Point(-ChassisExtents.x, -ChassisExtents.y+SideExtents.y, -ChassisExtents.z-ChassisGap), Point(-SideExtents.x, 0.0f, SideExtents.z), HingeAxis, HingeAxis);
		CreateHinge(pint, ChassisObject, RightSideObject, Point(-ChassisExtents.x, -ChassisExtents.y+SideExtents.y, ChassisExtents.z+ChassisGap), Point(-SideExtents.x, 0.0f, -SideExtents.z), HingeAxis, HingeAxis);

		// Blade
		if(1)
		{
			const float BladeMinSize = ChassisExtents.z + ChassisGap + SideExtents.z * 2.0f;
			const float BladeSize = BladeMinSize + 0.5f;

			const Point BladeExtents(0.1f, 0.75f, BladeSize);
//			const Point BladeExtents(0.2f, 3.0f, BladeSize);
			const Point BladePos(	LeftSidePos.x + SideExtents.x + BladeExtents.x,
//									LeftSidePos.y + BladeExtents.y,
									SideDistToGround + BladeExtents.y,
									0.0f);
			const float BladeMass = 1.0f;
			PintActorHandle BladeObject = CreateBoxObject(pint, BladeExtents, BladePos, BladeMass);

			const float dz = fabsf(BladePos.z - LeftSidePos.z);
			CreateHinge(pint, BladeObject, LeftSideObject, Point(-BladeExtents.x, -BladeExtents.y+SideExtents.y, -dz), Point(SideExtents.x, 0.0f, 0.0f), HingeAxis, HingeAxis);
			CreateHinge(pint, BladeObject, RightSideObject, Point(-BladeExtents.x, -BladeExtents.y+SideExtents.y, dz), Point(SideExtents.x, 0.0f, 0.0f), HingeAxis, HingeAxis);

			{
				const Point Ext1(-BladeExtents.x, BladeExtents.y, dz);
				const Point p1 = BladePos + Ext1;

				const Point Ext0(0.0f, SideExtents.y, 0.0f);
				const Point p0 = RightSidePos + Ext0;

				CreatePrismatic(pint, p0, p1, RightSideObject, Ext0, BladeObject, Ext1);
			}

			{
				const Point Ext1(-BladeExtents.x, BladeExtents.y, -dz);
				const Point p1 = BladePos + Ext1;

				const Point Ext0(0.0f, SideExtents.y, 0.0f);
				const Point p0 = LeftSidePos + Ext0;

				CreatePrismatic(pint, p0, p1, LeftSideObject, Ext0, BladeObject, Ext1);
			}

			for(udword i=0;i<2;i++)
			{
				const float Coeff = i ? -1.0f : 1.0f;
				const float Offset = Coeff*0.5f;

				const Point Ext0(ChassisExtents.x, ChassisExtents.y, Offset);
				const Point p0 = ChassisPos + Ext0;

				const Point Ext1(-BladeExtents.x, 0.0f, Offset);
				const Point p1 = BladePos + Ext1;
				CreatePrismatic(pint, p0, p1, ChassisObject, Ext0, BladeObject, Ext1);
			}
		}

		CreateTestScene(pint, &mHighFrictionMaterial);

		if(0)
		{
			PINT_CONVEX_CREATE ConvexCreate[14];
			MyConvex C[14];
			for(udword i=0;i<14;i++)
			{
				C[i].LoadFile(i);
//				C[i].Scale(0.5f);
//				C[i].Scale(2.5f);
				ConvexCreate[i].mNbVerts	= C[i].mNbVerts;
				ConvexCreate[i].mVerts		= C[i].mVerts;
				ConvexCreate[i].mRenderer	= CreateConvexRenderer(ConvexCreate[i].mNbVerts, ConvexCreate[i].mVerts);
			}

			const float Amplitude = 1.5f;
			const udword NbLayers = 12;
			const udword NbX = 12+4;
			const udword NbY = 12+4;
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

						const Point pos = Point(xf+40.0f, Amplitude + (Amplitude * 2.0f * float(j)), yf);

//						const udword Index = 13-1;//Rnd.Randomize() % 14;
						const udword Index = 10 + Rnd.Randomize() % 4;

						PintActorHandle Handle = CreateSimpleObject(pint, &ConvexCreate[Index], 0.1f, pos);
						ASSERT(Handle);
					}
				}
			}
		}
//		pint.SetGravity(Point(0.0f, -20.0f, 0.0f));
		return true;
	}

	virtual udword Bulldozer::Update(Pint& pint, float dt)
	{
		ChassisData* UserData = (ChassisData*)pint.mUserData;
		if(!UserData)
			return 0;

		bool CanAccelerate = ClampAngularVelocity(pint, *UserData);

		{
			const float Coeff = mAcceleration;
			const float ForwardCoeff = -Coeff;
			const float BackCoeff = 0.0f;
//			const float BackCoeff = Coeff*0.1f;
			if(mInput.mKeyboard_Left)
			{
				pint.AddLocalTorque(UserData->mFront[0].mWheel, Point(0.0f, 0.0f, ForwardCoeff));
				pint.AddLocalTorque(UserData->mRear[0].mWheel, Point(0.0f, 0.0f, ForwardCoeff));
				pint.AddLocalTorque(UserData->mFront[1].mWheel, Point(0.0f, 0.0f, BackCoeff));
				pint.AddLocalTorque(UserData->mRear[1].mWheel, Point(0.0f, 0.0f, BackCoeff));
			}
			if(mInput.mKeyboard_Right)
			{
				pint.AddLocalTorque(UserData->mFront[0].mWheel, Point(0.0f, 0.0f, BackCoeff));
				pint.AddLocalTorque(UserData->mRear[0].mWheel, Point(0.0f, 0.0f, BackCoeff));
				pint.AddLocalTorque(UserData->mFront[1].mWheel, Point(0.0f, 0.0f, ForwardCoeff));
				pint.AddLocalTorque(UserData->mRear[1].mWheel, Point(0.0f, 0.0f, ForwardCoeff));
			}
		}

		const bool FWD = true;
		const bool RWD = true;
		{
			const float Coeff = mAcceleration;
			if(mInput.mKeyboard_Accelerate /*&& CanAccelerate*/)
			{
				if(FWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mFront[i].mWheel)
							pint.AddLocalTorque(UserData->mFront[i].mWheel, Point(0.0f, 0.0f, -Coeff));
					}
				}
				if(RWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mRear[i].mWheel)
							pint.AddLocalTorque(UserData->mRear[i].mWheel, Point(0.0f, 0.0f, -Coeff));
					}
				}
			}
			if(mInput.mKeyboard_Brake)
			{
				if(FWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mFront[i].mWheel)
							pint.AddLocalTorque(UserData->mFront[i].mWheel, Point(0.0f, 0.0f, Coeff));
					}
				}
				if(RWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mRear[i].mWheel)
							pint.AddLocalTorque(UserData->mRear[i].mWheel, Point(0.0f, 0.0f, Coeff));
					}
				}
			}
		}

		// Camera
		UpdateCamera(pint, UserData->mChassis);

		return 0;
//		return VehicleBase::Update(pint, dt);
	}

END_TEST(Bulldozer)

///////////////////////////////////////////////////////////////////////////////

