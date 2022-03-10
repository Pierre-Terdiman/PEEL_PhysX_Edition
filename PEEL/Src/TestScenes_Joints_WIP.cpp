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
#include "GUI_Helpers.h"
#include "Loader_Bin.h"
#include "GLFontRenderer.h"
#include "ProceduralTrack.h"
#include "LoopTheLoop.h"
#include "Camera.h"

///////////////////////////////////////////////////////////////////////////////

namespace
{

static const bool gDriveVehicle = true;

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
		}
		PintActorHandle	mChassis;
//		udword			mNbWheels;
		WheelData		mFront[2];
		WheelData		mRear[2];
		PintActorHandle	mFrontAxleObject[2];
	};

	class VehicleBase : public VehicleInput
	{
		public:
									VehicleBase();
		virtual						~VehicleBase();

		virtual	void				Close(Pint& pint);
		virtual	udword				Update(Pint& pint, float dt);

				float				mAcceleration;
				float				mMaxAngularVelocity;
				float				mSteeringForce;
				//
				bool				mClampAngularVelocity;

				bool				ClampAngularVelocity(Pint& pint, const ChassisData& vehicle_data);
	};

VehicleBase::VehicleBase() :
	mAcceleration			(0.0f),
	mMaxAngularVelocity		(0.0f),
	mSteeringForce			(0.0f),
	mClampAngularVelocity	(false)
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
				if(::ClampAngularVelocity(pint, vehicle_data.mFront[i].mParent, vehicle_data.mFront[i].mWheel, MaxAngularVelocity))
					CanAccelerate = false;
			}
		}
		for(udword i=0;i<2;i++)
		{
			if(vehicle_data.mRear[i].mWheel && vehicle_data.mRear[i].mParent)
			{
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
		if(mInput.mKeyboard_Right)
		{
			pint.AddLocalTorque(UserData->mFrontAxleObject[0], Point(0.0f, -Steering, 0.0f));
			pint.AddLocalTorque(UserData->mFrontAxleObject[1], Point(0.0f, -Steering, 0.0f));
		}
		if(mInput.mKeyboard_Left)
		{
			pint.AddLocalTorque(UserData->mFrontAxleObject[0], Point(0.0f, Steering, 0.0f));
			pint.AddLocalTorque(UserData->mFrontAxleObject[1], Point(0.0f, Steering, 0.0f));
		}
	}

	bool CanAccelerate = ClampAngularVelocity(pint, *UserData);

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

#define START_VEHICLE_TEST(name, category, desc)										\
	class name : public VehicleBase														\
	{																					\
		public:																			\
								name()						{						}	\
		virtual					~name()						{						}	\
		virtual	const char*		GetName()			const	{ return #name;			}	\
		virtual	const char*		GetDescription()	const	{ return desc;			}	\
		virtual	TestCategory	GetCategory()		const	{ return category;		}

}

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_VehicleTest = "Vehicle test. This uses regular joints (no articulation). This is an experimental, work-in-progress test. It has not been properly tuned or \
optimized, and it has not been tested in engines other than PhysX 3.4. Use the arrow keys to control the vehicle. Use the PageUp and PageDown keys to switch between different \
camera views.";

START_VEHICLE_TEST(VehicleTest, CATEGORY_WIP, gDesc_VehicleTest)

	virtual	float			GetRenderData(Point& center)	const
	{
		center = GetCameraPos();
		return 1000.0f;
	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("VehicleTest settings", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(10.76f, 11.91f, 14.07f), Point(-0.54f, -0.49f, -0.68f));
		SetDefEnv(desc, false);
	}

	virtual	bool	CommonSetup()
	{
		mAcceleration			= 100.0f;
		mMaxAngularVelocity		= 17.0f;
		mSteeringForce			= 200.0f;
		mCamera.mUpOffset		= 4.0f;
		mCamera.mDistToTarget	= 10.0f;
		mClampAngularVelocity	= true;
		mControlCamera			= gDriveVehicle;

		mPlanarMeshHelper.Generate(100, 0.1f);

		return VehicleBase::CommonSetup();
	}

	virtual void	CreateVehicle(Pint& pint, const Point& pos, ChassisData* UserData, const PINT_MATERIAL_CREATE& LowFrictionMaterial, PintCollisionGroup ChassisGroup, PintCollisionGroup WheelGroup)
	{
		//### experimental stuff, to revisit. Suspension doesn't work yet, etc

		const float MainScale = 0.25f;

		const float M = 4.0f;
		const float ChassisMass = M;
		const float WheelMass = M;
		const float StructMass = M;

		const float WheelRadius = 4.0f * MainScale;
		const float WheelWidth = 4.0f * MainScale;

		const udword NbPts = 60;
		const CylinderMesh Cylinder(NbPts, WheelRadius, WheelWidth*0.5f);
		const udword TotalNbVerts = Cylinder.mNbVerts;
		const Point* Verts = Cylinder.mVerts;

		//

		PtrContainer JointDescs;

		const Point Extents(MainScale*8.0f, MainScale*2.0f, MainScale*4.0f);

		PintActorHandle Chassis;
		const Point ChassisPos = pos;

		{
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= ChassisMass;
			ObjectDesc.mPosition		= ChassisPos;
			ObjectDesc.mCollisionGroup	= ChassisGroup;
			ObjectDesc.mCOMLocalOffset	= Point(0.0f, -5.0f, 0.0f);
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

					PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
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
					Desc.mLimits.Set(0.0f, 0.01f);
					Desc.mSpring.mStiffness	= 1000.0f;
					Desc.mSpring.mDamping	= 100.0f;

					PINT_PRISMATIC_JOINT_CREATE* Copy = ICE_NEW(PINT_PRISMATIC_JOINT_CREATE);
					*Copy = Desc;
					JointDescs.AddPtr(Copy);
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
					WheelPosOffset = WheelJointOffset = Point(-Extension, 0.0f, z*HingeExtents.z);
					WheelPosOffset.x = 0.0f;
					{
						PINT_BOX_CREATE BoxDesc(HingeExtents);
						BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

						PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
						ObjectDesc.mPosition		= SuspPos + Point(Extension, 0.0f, 0.0f);
						ObjectDesc.mMass			= StructMass;
						ObjectDesc.mCollisionGroup	= WheelGroup;
						const PintActorHandle HingeObject = CreatePintObject(pint, ObjectDesc);
						if(UserData)
						UserData->mFrontAxleObject[j] = HingeObject;
						WheelAttachObject = HingeObject;

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
							Desc.mLimits.Set(-Limit, Limit);

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
					if(FrontWheels)
						WheelDesc.mMaterial	= &mHighFrictionMaterial;
					else
						WheelDesc.mMaterial	= &LowFrictionMaterial;					

					PINT_OBJECT_CREATE ObjectDesc(&WheelDesc);
					ObjectDesc.mPosition		= SuspPos + WheelPosOffset;
					ObjectDesc.mMass			= WheelMass;
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

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mPosition		= (GlobalAnchorPt[0] + GlobalAnchorPt[1])*0.5f;
			ObjectDesc.mMass			= StructMass;
			ObjectDesc.mCollisionGroup	= WheelGroup;
			PintActorHandle RodObject = CreatePintObject(pint, ObjectDesc);

			PINT_SPHERICAL_JOINT_CREATE Desc;

			Desc.mObject0			= AnchorObjects[0];
			Desc.mObject1			= RodObject;
			Desc.mLocalPivot0.mPos	= LocalAnchorPt[0];
			Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, -BoxDesc.mExtents.z);

			PINT_SPHERICAL_JOINT_CREATE* Copy = ICE_NEW(PINT_SPHERICAL_JOINT_CREATE);
			*Copy = Desc;
			JointDescs.AddPtr(Copy);

			Desc.mObject0			= AnchorObjects[1];
			Desc.mObject1			= RodObject;
			Desc.mLocalPivot0.mPos	= LocalAnchorPt[1];
			Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, BoxDesc.mExtents.z);

			PINT_SPHERICAL_JOINT_CREATE* Copy2 = ICE_NEW(PINT_SPHERICAL_JOINT_CREATE);
			*Copy2 = Desc;
			JointDescs.AddPtr(Copy2);
		}

		const udword NbJoints = JointDescs.GetNbEntries();
		for(udword j=0;j<32;j++)
		{
			for(udword i=0;i<NbJoints;i++)
			{
				const PINT_JOINT_CREATE* jc = (const PINT_JOINT_CREATE*)JointDescs.GetEntry(i);
				PintJointHandle JointHandle = pint.CreateJoint(*jc);
				ASSERT(JointHandle);
			}
		}
		for(udword i=0;i<NbJoints;i++)
		{
			const PINT_JOINT_CREATE* jc = (const PINT_JOINT_CREATE*)JointDescs.GetEntry(i);
			DELETESINGLE(jc);
		}
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportHingeJoints || !caps.mSupportPrismaticJoints || !caps.mSupportRigidBodySimulation)
			return false;

		const PINT_MATERIAL_CREATE LowFrictionMaterial(0.75f, 0.75f, 0.0f);

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
//		const Point ChassisPos(0.0f, 4.0f+Extents.y*4.0f, 0.0f);
		const Point ChassisPos(0.0f, 1.0f+Extents.y*4.0f, 0.0f);

		CreateVehicle(pint, ChassisPos, UserData, LowFrictionMaterial, ChassisGroup, WheelGroup);
		CreateVehicle(pint, ChassisPos + Point(10.0f, 0.0f, 0.0f), null, LowFrictionMaterial, 3, 4);

		// Box stack
		{
//			const PINT_MATERIAL_CREATE BoxMaterial(1.0f, 1.0f, 0.0f);
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

						PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
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

		const float Altitude = 0.0f;
		mPlanarMeshHelper.CreatePlanarMesh(pint, Altitude, null);

		return true;
	}

END_TEST(VehicleTest)

///////////////////////////////////////////////////////////////////////////////

/*static*/ PintActorHandle GenerateTireCompound(Pint& pint, const Point& torus_pos, const Quat& torus_rot, float mass, PintCollisionGroup group, const PINT_MATERIAL_CREATE& material, float scale, float mass_inertia_coeff)
{
	// Generate torus. We generate a torus by sweeping a small circle along a large circle, taking N samples (slices) along the way.
	const float BigRadius = 3.0f * 0.25f * scale;
	const float SmallRadius = 1.0f * 0.25f * scale;
//	const float SmallRadius = 0.75f;
//	const udword NbPtsSmallCircle = 8;
//	const udword NbSlices = 16;
	const udword NbPtsSmallCircle = 16;
//	const udword NbSlices = 32;
//	const udword NbSlices = 8;
	const udword NbSlices = 128;

	// First we generate a small vertical template circle, oriented in the XY plane.
	Point SmallCirclePts[NbPtsSmallCircle];
//	GeneratePolygon(NbPtsSmallCircle, SmallCirclePts, sizeof(Point), ORIENTATION_XY, SmallRadius);
	GeneratePolygon(NbPtsSmallCircle, SmallCirclePts, sizeof(Point), ORIENTATION_YZ, SmallRadius);

	// We'll be sweeping this initial circle along a curve (a larger circle), taking N slices along the way.
	// The final torus will use these vertices exclusively so the total #verts is:
	const udword TotalNbVerts = NbPtsSmallCircle * NbSlices;
	Point* Verts = ICE_NEW(Point)[TotalNbVerts];

	// Now we do the sweep along the larger circle.
	Point SliceCenters[NbSlices];
	{
		const Matrix3x3 TRot = torus_rot;

		udword Index = 0;
		for(udword j=0;j<NbSlices;j++)
		{
			const float Coeff = float(j)/float(NbSlices);

			// We rotate and translate the template circle to position it along the larger circle.
			Matrix3x3 Rot;
//			Rot.RotX(Coeff * TWOPI);
			Rot.RotZ(Coeff * TWOPI);

			const Point Trans = Rot[1]*BigRadius;
			for(udword i=0;i<NbPtsSmallCircle;i++)
				Verts[Index++] = (Trans + SmallCirclePts[i]*Rot)*TRot;

			SliceCenters[j] = Trans*TRot;
		}
		ASSERT(Index==TotalNbVerts);
	}
	// Here we have generated all the vertices.

	//###TODO: next loop is useless?
	// Next, we generate a convex object for each part of the torus. A part is a section connecting two of the previous slices.
	for(udword s=0;s<NbSlices;s++)
	{
		const udword SliceIndex0 = s;
		const udword SliceIndex1 = (s+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		Point ConvexPts[NbPtsSmallCircle*2];
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i];
			ConvexPts[i+NbPtsSmallCircle] = V1[i];
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}
	}

	PINT_OBJECT_CREATE ObjectDesc;
	ObjectDesc.mMass			= mass;
	ObjectDesc.mMassForInertia	= ObjectDesc.mMass*mass_inertia_coeff;
	ObjectDesc.mPosition		= torus_pos;
	ObjectDesc.mCollisionGroup	= group;

	Point* VertsCopy[NbSlices];
	PINT_CONVEX_CREATE* Shapes[NbSlices];
	PINT_SHAPE_CREATE* SC = null;
	for(udword s=0;s<NbSlices;s++)
	{
		const udword SliceIndex0 = s;
		const udword SliceIndex1 = (s+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		Point ConvexPts[NbPtsSmallCircle*2];
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i];
			ConvexPts[i+NbPtsSmallCircle] = V1[i];
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}

		// Now we create the convex object itself
//		PINT_CONVEX_CREATE ConvexCreate(NbPtsSmallCircle*2, ConvexPts);
//		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexPts);

		VertsCopy[s] = ICE_NEW(Point)[NbPtsSmallCircle*2];
		CopyMemory(VertsCopy[s], ConvexPts, sizeof(Point)*NbPtsSmallCircle*2);

//		PINT_CONVEX_CREATE* ConvexCreate = ICE_NEW(PINT_CONVEX_CREATE)(NbPtsSmallCircle*2, ConvexPts);
		PINT_CONVEX_CREATE* ConvexCreate = ICE_NEW(PINT_CONVEX_CREATE)(NbPtsSmallCircle*2, VertsCopy[s]);
//		ConvexCreate->mRenderer	= CreateConvexRenderer(ConvexCreate->mNbVerts, ConvexPts);
		ConvexCreate->mRenderer	= CreateConvexRenderer(ConvexCreate->mNbVerts, VertsCopy[s]);
		ConvexCreate->mLocalPos	= Center;
		ConvexCreate->mMaterial = &material;
		Shapes[s] = ConvexCreate;
		SC = ObjectDesc.AddShape(ConvexCreate, SC);

/*
//		Handles[s] = CreateDynamicObject(pint, &ConvexCreate, pos+Center);
		{
			PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
			ObjectDesc.mMass			= mass;
			ObjectDesc.mPosition		= torus_pos+Center;
			// Each convex has been computed from rotated small circles, and we only re-centered the verts but
			// didn't cancel the rotation. In theory we'd only need one convex object that we could translate/rotate,
			// but it was easier to create different parts with different rotations. As a result we don't need to set
			// a rotation for the object itself, it's captured in the vertices.
//			ObjectDesc.mRotation		= torus_rot;

			Handles[s] = CreatePintObject(pint, ObjectDesc);
		}
		ASSERT(Handles[s]);*/
	}

	PintActorHandle Compound = CreatePintObject(pint, ObjectDesc);

	for(udword s=0;s<NbSlices;s++)
	{
		DELETESINGLE(Shapes[s]);
		DELETEARRAY(VertsCopy[s]);
	}

	DELETEARRAY(Verts);

	return Compound;
}

// Repro for issue with TGS/64. Object cannot rotate anymore.
static void GenerateTire_Issue_TGS_64(Pint& pint, const Point& torus_pos, const Quat& torus_rot, float mass, udword nb_loops,
						 Vertices& debug_pts)
{
	// Generate torus. We generate a torus by sweeping a small circle along a large circle, taking N samples (slices) along the way.
	const float BigRadius = 3.0f;
	const float SmallRadius = 1.0f;
//	const float SmallRadius = 0.75f;
//	const udword NbPtsSmallCircle = 8;
//	const udword NbSlices = 16;
	const udword NbPtsSmallCircle = 16;
	const udword NbSlices = 32;
//	const udword NbSlices = 8;

	PintActorHandle Rim;
	if(1)
	{
		// Rims
//		PINT_SPHERE_CREATE ShapeCreate((BigRadius - SmallRadius)*1.1f);
		PINT_SPHERE_CREATE ShapeCreate((BigRadius - SmallRadius));
		ShapeCreate.mRenderer	= CreateSphereRenderer(ShapeCreate.mRadius);

		PINT_OBJECT_CREATE ObjectDesc(&ShapeCreate);
		ObjectDesc.mMass		= mass;
		ObjectDesc.mPosition	= torus_pos;

		ObjectDesc.mAngularVelocity	= Point(0.0f, 0.0f, 100.0f);

		//ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
		Rim = CreatePintObject(pint, ObjectDesc);
	}


	// First we generate a small vertical template circle, oriented in the XY plane.
	Point SmallCirclePts[NbPtsSmallCircle];
	GeneratePolygon(NbPtsSmallCircle, SmallCirclePts, sizeof(Point), ORIENTATION_XY, SmallRadius);

	if(0)
	{
		for(udword i=0;i<NbPtsSmallCircle;i++)
			debug_pts.AddVertex(SmallCirclePts[i]);
	}

	// We'll be sweeping this initial circle along a curve (a larger circle), taking N slices along the way.
	// The final torus will use these vertices exclusively so the total #verts is:
	const udword TotalNbVerts = NbPtsSmallCircle * NbSlices;
	Point* Verts = ICE_NEW(Point)[TotalNbVerts];

	// Now we do the sweep along the larger circle.
	Point SliceCenters[NbSlices];
	{
		const Matrix3x3 TRot = torus_rot;

		udword Index = 0;
		for(udword j=0;j<NbSlices;j++)
		{
			const float Coeff = float(j)/float(NbSlices);

			// We rotate and translate the template circle to position it along the larger circle.
			Matrix3x3 Rot;
			Rot.RotX(Coeff * TWOPI);

			const Point Trans = Rot[1]*BigRadius;
			for(udword i=0;i<NbPtsSmallCircle;i++)
				Verts[Index++] = (Trans + SmallCirclePts[i]*Rot)*TRot;

			SliceCenters[j] = Trans*TRot;
		}
		ASSERT(Index==TotalNbVerts);
	}
	// Here we have generated all the vertices.

	if(0)
	{
		for(udword i=0;i<NbSlices;i++)
			debug_pts.AddVertex(SliceCenters[i]);

		for(udword i=0;i<TotalNbVerts;i++)
			debug_pts.AddVertex(Verts[i]);
		return;
	}

	// Next, we generate a convex object for each part of the torus. A part is a section connecting two of the previous slices.
	PintActorHandle Handles[NbSlices];
	Point ObjectCenters[NbSlices];
	udword GroupBit = 0;

	for(udword s=0;s<NbSlices;s++)
	{
		const udword SliceIndex0 = s;
		const udword SliceIndex1 = (s+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		Point ConvexPts[NbPtsSmallCircle*2];
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i];
			ConvexPts[i+NbPtsSmallCircle] = V1[i];
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}
		ObjectCenters[s] = Center;
	}

	for(udword s=0;s<NbSlices;s++)
	{
		const udword SliceIndex0 = s;
		const udword SliceIndex1 = (s+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		Point ConvexPts[NbPtsSmallCircle*2];
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i];
			ConvexPts[i+NbPtsSmallCircle] = V1[i];
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}
		ObjectCenters[s] = Center;

		// Now we create the convex object itself
		PINT_CONVEX_CREATE ConvexCreate(NbPtsSmallCircle*2, ConvexPts);
		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexPts);

//		Handles[s] = CreateDynamicObject(pint, &ConvexCreate, pos+Center);
		{
			PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
			ObjectDesc.mMass			= mass;
			ObjectDesc.mPosition		= torus_pos+Center;
			// Each convex has been computed from rotated small circles, and we only re-centered the verts but
			// didn't cancel the rotation. In theory we'd only need one convex object that we could translate/rotate,
			// but it was easier to create different parts with different rotations. As a result we don't need to set
			// a rotation for the object itself, it's captured in the vertices.
//			ObjectDesc.mRotation		= torus_rot;
			ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;

			if(0)
			{
//				debug_pts.AddVertex(ObjectDesc.mPosition);

				Point N = Center;
				N.Normalize();
				N *= (BigRadius - SmallRadius);
				debug_pts.AddVertex(torus_pos + N);
			}

			Handles[s] = CreatePintObject(pint, ObjectDesc);
		}
		ASSERT(Handles[s]);
	}
	DELETEARRAY(Verts);

//	const bool CreateFixedJoints = false;
//	if(CreateFixedJoints && !Articulation && !RCArticulation && mass!=0.0f)
	if(1 && Rim && mass!=0.0f)
	{
		for(udword i=0;i<nb_loops;i++)
		{
//			for(udword s=0;s<NbSlices;s++)
//			for(udword s=0;s<NbSlices;s+=2)
			for(udword s=0;s<NbSlices;s+=NbSlices/2)
			{
				const udword SliceIndex0 = s;
//				const udword SliceIndex1 = (s+1)%NbSlices;

	//			debug_pts.AddVertex(ObjectDesc.mPosition);
				Point N = ObjectCenters[s];
				N.Normalize();
//				N *= (BigRadius - SmallRadius);
//				debug_pts.AddVertex(torus_pos + N);

				PINT_FIXED_JOINT_CREATE Desc;
				Desc.mObject0		= Handles[SliceIndex0];
				Desc.mObject1		= Rim;
				Desc.mLocalPivot0	= -N * SmallRadius;
				Desc.mLocalPivot1	= N * (BigRadius - SmallRadius);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
	}

	const bool CreateFixedJoints = true;
	if(CreateFixedJoints && mass!=0.0f)
	{
		for(udword i=0;i<nb_loops;i++)
		{
			for(udword s=0;s<NbSlices;s++)
			{
				const udword SliceIndex0 = s;
				const udword SliceIndex1 = (s+1)%NbSlices;
//				const Point Delta = (ObjectCenters[SliceIndex1] - ObjectCenters[SliceIndex0])*0.5f;

				PINT_FIXED_JOINT_CREATE Desc;
				Desc.mObject0		= Handles[SliceIndex0];
				Desc.mObject1		= Handles[SliceIndex1];
//				Desc.mLocalPivot0	= Delta;
//				Desc.mLocalPivot1	= -Delta;
				Desc.mLocalPivot0	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex0];
				Desc.mLocalPivot1	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex1];
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
	}
}

static void GenerateTire_Issue_Explosion(Pint& pint, const Point& torus_pos, const Quat& torus_rot, float mass, udword nb_loops,
						 Vertices& debug_pts)
{
	// Generate torus. We generate a torus by sweeping a small circle along a large circle, taking N samples (slices) along the way.
	const float BigRadius = 3.0f;
	const float SmallRadius = 1.0f;
//	const float SmallRadius = 0.3f;
//	const float SmallRadius = 0.75f;
//	const udword NbPtsSmallCircle = 8;
//	const udword NbSlices = 16;
	const udword NbPtsSmallCircle = 16;
	const udword NbSlices = 32;
//	const udword NbSlices = 8;

	PintActorHandle Rim;
	if(1)
	{
		// Rims
//		PINT_SPHERE_CREATE ShapeCreate((BigRadius - SmallRadius)*1.1f);
		PINT_SPHERE_CREATE ShapeCreate((BigRadius - SmallRadius));
		ShapeCreate.mRenderer	= CreateSphereRenderer(ShapeCreate.mRadius);

		PINT_OBJECT_CREATE ObjectDesc(&ShapeCreate);
		ObjectDesc.mMass		= mass;
		ObjectDesc.mPosition	= torus_pos;

		//ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
		Rim = CreatePintObject(pint, ObjectDesc);
	}


	// First we generate a small vertical template circle, oriented in the XY plane.
	Point SmallCirclePts[NbPtsSmallCircle];
	GeneratePolygon(NbPtsSmallCircle, SmallCirclePts, sizeof(Point), ORIENTATION_XY, SmallRadius);

	if(0)
	{
		for(udword i=0;i<NbPtsSmallCircle;i++)
			debug_pts.AddVertex(SmallCirclePts[i]);
	}

	// We'll be sweeping this initial circle along a curve (a larger circle), taking N slices along the way.
	// The final torus will use these vertices exclusively so the total #verts is:
	const udword TotalNbVerts = NbPtsSmallCircle * NbSlices;
	Point* Verts = ICE_NEW(Point)[TotalNbVerts];

	// Now we do the sweep along the larger circle.
	Point SliceCenters[NbSlices];
	{
		const Matrix3x3 TRot = torus_rot;

		udword Index = 0;
		for(udword j=0;j<NbSlices;j++)
		{
			const float Coeff = float(j)/float(NbSlices);

			// We rotate and translate the template circle to position it along the larger circle.
			Matrix3x3 Rot;
			Rot.RotX(Coeff * TWOPI);

			const Point Trans = Rot[1]*BigRadius;
			for(udword i=0;i<NbPtsSmallCircle;i++)
				Verts[Index++] = (Trans + SmallCirclePts[i]*Rot)*TRot;

			SliceCenters[j] = Trans*TRot;
		}
		ASSERT(Index==TotalNbVerts);
	}
	// Here we have generated all the vertices.

	if(0)
	{
		for(udword i=0;i<NbSlices;i++)
			debug_pts.AddVertex(SliceCenters[i]);

		for(udword i=0;i<TotalNbVerts;i++)
			debug_pts.AddVertex(Verts[i]);
		return;
	}

	// Next, we generate a convex object for each part of the torus. A part is a section connecting two of the previous slices.
	PintActorHandle Handles[NbSlices];
	Point ObjectCenters[NbSlices];
	udword GroupBit = 0;

	for(udword s=0;s<NbSlices;s++)
	{
		const udword SliceIndex0 = s;
		const udword SliceIndex1 = (s+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		Point ConvexPts[NbPtsSmallCircle*2];
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i];
			ConvexPts[i+NbPtsSmallCircle] = V1[i];
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}
		ObjectCenters[s] = Center;
	}

	for(udword s=0;s<NbSlices;s++)
	{
		const udword SliceIndex0 = s;
		const udword SliceIndex1 = (s+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		Point ConvexPts[NbPtsSmallCircle*2];
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i];
			ConvexPts[i+NbPtsSmallCircle] = V1[i];
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}
		ObjectCenters[s] = Center;

		// Now we create the convex object itself
		PINT_CONVEX_CREATE ConvexCreate(NbPtsSmallCircle*2, ConvexPts);
		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexPts);

//		Handles[s] = CreateDynamicObject(pint, &ConvexCreate, pos+Center);
		{
			PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
			ObjectDesc.mMass			= mass;
			ObjectDesc.mPosition		= torus_pos+Center;
			// Each convex has been computed from rotated small circles, and we only re-centered the verts but
			// didn't cancel the rotation. In theory we'd only need one convex object that we could translate/rotate,
			// but it was easier to create different parts with different rotations. As a result we don't need to set
			// a rotation for the object itself, it's captured in the vertices.
//			ObjectDesc.mRotation		= torus_rot;
			ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;

			if(0)
			{
//				debug_pts.AddVertex(ObjectDesc.mPosition);

				Point N = Center;
				N.Normalize();
				N *= (BigRadius - SmallRadius);
				debug_pts.AddVertex(torus_pos + N);
			}

			Handles[s] = CreatePintObject(pint, ObjectDesc);
		}
		ASSERT(Handles[s]);
	}
	DELETEARRAY(Verts);

//	const bool CreateFixedJoints = false;
//	if(CreateFixedJoints && !Articulation && !RCArticulation && mass!=0.0f)
	if(1 && Rim && mass!=0.0f)
	{
		for(udword i=0;i<nb_loops;i++)
		{
			for(udword s=0;s<NbSlices;s++)
//			for(udword s=0;s<NbSlices;s+=2)
//			for(udword s=0;s<NbSlices;s+=NbSlices/2)
			{
				const udword SliceIndex0 = s;
//				const udword SliceIndex1 = (s+1)%NbSlices;

	//			debug_pts.AddVertex(ObjectDesc.mPosition);
				Point N = ObjectCenters[s];
				N.Normalize();
//				N *= (BigRadius - SmallRadius);
//				debug_pts.AddVertex(torus_pos + N);

				PINT_FIXED_JOINT_CREATE Desc;
//				PINT_SPHERICAL_JOINT_CREATE Desc;
				Desc.mObject0		= Handles[SliceIndex0];
				Desc.mObject1		= Rim;
				Desc.mLocalPivot0	= -N * SmallRadius;
				Desc.mLocalPivot1	= N * (BigRadius - SmallRadius);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
	}

	const bool CreateFixedJoints = false;
	if(CreateFixedJoints && mass!=0.0f)
	{
		for(udword i=0;i<nb_loops;i++)
		{
			for(udword s=0;s<NbSlices;s++)
			{
				const udword SliceIndex0 = s;
				const udword SliceIndex1 = (s+1)%NbSlices;
//				const Point Delta = (ObjectCenters[SliceIndex1] - ObjectCenters[SliceIndex0])*0.5f;

				PINT_FIXED_JOINT_CREATE Desc;
				Desc.mObject0		= Handles[SliceIndex0];
				Desc.mObject1		= Handles[SliceIndex1];
//				Desc.mLocalPivot0	= Delta;
//				Desc.mLocalPivot1	= -Delta;
				Desc.mLocalPivot0	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex0];
				Desc.mLocalPivot1	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex1];
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
	}
}

static PintActorHandle GenerateTire_Prismatic(Pint& pint, const Point& torus_pos, const Quat& torus_rot, float mass, udword nb_loops, Vertices& debug_pts, float main_scale)
{
	// Generate torus. We generate a torus by sweeping a small circle along a large circle, taking N samples (slices) along the way.
	const float BigRadius = 3.0f*main_scale;
	const float SmallRadius = 1.0f*main_scale;
//	const float SmallRadius = 0.75f;
//	const udword NbPtsSmallCircle = 8;
//	const udword NbSlices = 16;
	const udword NbPtsSmallCircle = 16;
//	const udword NbPtsSmallCircle = 32;
	const udword NbSlices = 64;
//	const udword NbSlices = 128;
//	const udword NbSlices = 16;
//	const udword NbSlices = 8;

	mass = 0.5f;
//	mass = 0.25f;

	PintActorHandle Rim;
	if(1)
	{
		// Rims
//		PINT_SPHERE_CREATE ShapeCreate((BigRadius - SmallRadius)*1.1f);
		PINT_SPHERE_CREATE ShapeCreate((BigRadius - SmallRadius));
		ShapeCreate.mRenderer	= CreateSphereRenderer(ShapeCreate.mRadius);

		PINT_OBJECT_CREATE ObjectDesc(&ShapeCreate);
		ObjectDesc.mMass		= mass;
		ObjectDesc.mMass		= 100.0f;
//		ObjectDesc.mMass		= 0.0f;
//		ObjectDesc.mMassForInertia	= 10.0f;
//ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mCollisionGroup	= 1;
		ObjectDesc.mPosition	= torus_pos;

//		ObjectDesc.mAngularVelocity	= Point(1000.0f, 0.0f, 0.0f);

		//ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
		ObjectDesc.mCollisionGroup	= 1;
		Rim = CreatePintObject(pint, ObjectDesc);
	}
//return Rim;

	// First we generate a small vertical template circle, oriented in the XY plane.
	Point SmallCirclePts[NbPtsSmallCircle];
//	GeneratePolygon(NbPtsSmallCircle, SmallCirclePts, sizeof(Point), ORIENTATION_XY, SmallRadius);
	GeneratePolygon(NbPtsSmallCircle, SmallCirclePts, sizeof(Point), ORIENTATION_YZ, SmallRadius);

	if(0)
	{
		for(udword i=0;i<NbPtsSmallCircle;i++)
			debug_pts.AddVertex(SmallCirclePts[i]);
	}

	// We'll be sweeping this initial circle along a curve (a larger circle), taking N slices along the way.
	// The final torus will use these vertices exclusively so the total #verts is:
	const udword TotalNbVerts = NbPtsSmallCircle * NbSlices;
	Point* Verts = ICE_NEW(Point)[TotalNbVerts];

	// Now we do the sweep along the larger circle.
	Point SliceCenters[NbSlices];
	{
		const Matrix3x3 TRot = torus_rot;

		udword Index = 0;
		for(udword j=0;j<NbSlices;j++)
		{
			const float Coeff = float(j)/float(NbSlices);

			// We rotate and translate the template circle to position it along the larger circle.
			Matrix3x3 Rot;
//			Rot.RotX(Coeff * TWOPI);
			Rot.RotZ(Coeff * TWOPI);

			const Point Trans = Rot[1]*BigRadius;
			for(udword i=0;i<NbPtsSmallCircle;i++)
				Verts[Index++] = (Trans + SmallCirclePts[i]*Rot)*TRot;

			SliceCenters[j] = Trans*TRot;
		}
		ASSERT(Index==TotalNbVerts);
	}
	// Here we have generated all the vertices.

	if(0)
	{
		for(udword i=0;i<NbSlices;i++)
			debug_pts.AddVertex(SliceCenters[i]);

		for(udword i=0;i<TotalNbVerts;i++)
			debug_pts.AddVertex(Verts[i]);
	}

	PintArticHandle Articulation = null;
	if(0 && mass!=0.0f)
		Articulation = pint.CreateArticulation(PINT_ARTICULATION_CREATE());
	PintArticHandle RCArticulation = null;
	if(0 && mass!=0.0f)
		RCArticulation = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE());

	// Next, we generate a convex object for each part of the torus. A part is a section connecting two of the previous slices.
	PintActorHandle Handles[NbSlices];
	Point ObjectCenters[NbSlices];
	udword GroupBit = 0;

	for(udword s=0;s<NbSlices;s++)
	{
		const udword SliceIndex0 = s;
		const udword SliceIndex1 = (s+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		Point ConvexPts[NbPtsSmallCircle*2];
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i];
			ConvexPts[i+NbPtsSmallCircle] = V1[i];
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}
		ObjectCenters[s] = Center;
	}

	for(udword s=0;s<NbSlices;s++)
	{
		const udword SliceIndex0 = s;
		const udword SliceIndex1 = (s+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		Point ConvexPts[NbPtsSmallCircle*2];
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i];
			ConvexPts[i+NbPtsSmallCircle] = V1[i];
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}
		ObjectCenters[s] = Center;

		// Now we create the convex object itself
		PINT_CONVEX_CREATE ConvexCreate(NbPtsSmallCircle*2, ConvexPts);
		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexPts);

//		Handles[s] = CreateDynamicObject(pint, &ConvexCreate, pos+Center);
		{
			PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
			ObjectDesc.mMass			= mass;
//			ObjectDesc.mMassForInertia	= 10.0f;
			ObjectDesc.mPosition		= torus_pos+Center;
			// Each convex has been computed from rotated small circles, and we only re-centered the verts but
			// didn't cancel the rotation. In theory we'd only need one convex object that we could translate/rotate,
			// but it was easier to create different parts with different rotations. As a result we don't need to set
			// a rotation for the object itself, it's captured in the vertices.
//			ObjectDesc.mRotation		= torus_rot;
//			ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
			ObjectDesc.mCollisionGroup	= 1;

			if(0)
			{
//				debug_pts.AddVertex(ObjectDesc.mPosition);

				Point N = Center;
				N.Normalize();
				N *= (BigRadius - SmallRadius);
				debug_pts.AddVertex(torus_pos + N);
			}

			if(Articulation)
			{
				PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = s ? Handles[s-1] : null;

					const udword SliceIndex0 = (s-1)%NbSlices;
					const udword SliceIndex1 = (s)%NbSlices;

//					Desc.mObject0		= Handles[SliceIndex0];
//					Desc.mObject1		= Handles[SliceIndex1];
					ArticulatedDesc.mLocalPivot0	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex0];
					ArticulatedDesc.mLocalPivot1	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex1];

				if(1)
				{
//					ArticulatedDesc.mX = Point(0.0f, 0.0f, 1.0f);
				ArticulatedDesc.mEnableTwistLimit = true;
				ArticulatedDesc.mTwistLowerLimit = -0.0001f;
				ArticulatedDesc.mTwistUpperLimit = 0.0001f;
				ArticulatedDesc.mEnableSwingLimit = true;
				ArticulatedDesc.mSwingYLimit = 0.001f;
				ArticulatedDesc.mSwingZLimit = 0.001f;
				}

//				ArticulatedDesc.mLocalPivot0 = LocalPivot0;
//				ArticulatedDesc.mLocalPivot1 = LocalPivot1;
				Handles[s] = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
			}
			else if(RCArticulation)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = s ? Handles[s-1] : null;

					const udword SliceIndex0 = (s-1)%NbSlices;
					const udword SliceIndex1 = (s)%NbSlices;

//					Desc.mObject0		= Handles[SliceIndex0];
//					Desc.mObject1		= Handles[SliceIndex1];
					ArticulatedDesc.mLocalPivot0.mPos	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex0];
					ArticulatedDesc.mLocalPivot1.mPos	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex1];

/*				if(1)
				{
//					ArticulatedDesc.mX = Point(0.0f, 0.0f, 1.0f);
				ArticulatedDesc.mEnableTwistLimit = true;
				ArticulatedDesc.mTwistLowerLimit = -0.0001f;
				ArticulatedDesc.mTwistUpperLimit = 0.0001f;
				ArticulatedDesc.mEnableSwingLimit = true;
				ArticulatedDesc.mSwingYLimit = 0.001f;
				ArticulatedDesc.mSwingZLimit = 0.001f;
				}*/

//				ArticulatedDesc.mLocalPivot0 = LocalPivot0;
//				ArticulatedDesc.mLocalPivot1 = LocalPivot1;
				Handles[s] = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCArticulation);
			}
			else
				Handles[s] = CreatePintObject(pint, ObjectDesc);
		}
		ASSERT(Handles[s]);
	}
	DELETEARRAY(Verts);

	if(Articulation)
		pint.AddArticulationToScene(Articulation);
	if(RCArticulation)
		pint.AddRCArticulationToScene(RCArticulation);

	const bool CreatePrismaticJoints = true;
//	if(CreateFixedJoints && !Articulation && !RCArticulation && mass!=0.0f)
	if(CreatePrismaticJoints && Rim && mass!=0.0f)
	{
		for(udword i=0;i<nb_loops;i++)
		{
			for(udword s=0;s<NbSlices;s++)
//			for(udword s=0;s<NbSlices;s+=2)
//			for(udword s=0;s<NbSlices;s+=NbSlices/2)
			{
				const udword SliceIndex0 = s;
//				const udword SliceIndex1 = (s+1)%NbSlices;

	//			debug_pts.AddVertex(ObjectDesc.mPosition);
				Point N = ObjectCenters[s];
				N.Normalize();
//				N *= (BigRadius - SmallRadius);
//				debug_pts.AddVertex(torus_pos + N);

				PINT_PRISMATIC_JOINT_CREATE Desc;
				Desc.mObject0			= Handles[SliceIndex0];
				Desc.mObject1			= Rim;
				//###PRISMATIC2
				Desc.mLocalPivot0.mPos	= -N * SmallRadius;
				Desc.mLocalPivot1.mPos	= N * (BigRadius - SmallRadius);
				Desc.mLocalAxis0		= N;
				Desc.mLocalAxis1		= N;
				Desc.mLimits.Set(0.0f, 0.01f);
//				Desc.mMaxLimit			= 0.0f;
				Desc.mSpring.mStiffness	= 10000.0f;
				Desc.mSpring.mDamping	= 1000.0f;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
	}

	const bool CreateFixedJoints = false;
	if(CreateFixedJoints && !Articulation && !RCArticulation && mass!=0.0f)
	{
		for(udword i=0;i<nb_loops;i++)
		{
			for(udword s=0;s<NbSlices;s++)
			{
				const udword SliceIndex0 = s;
				const udword SliceIndex1 = (s+1)%NbSlices;
//				const Point Delta = (ObjectCenters[SliceIndex1] - ObjectCenters[SliceIndex0])*0.5f;

				PINT_FIXED_JOINT_CREATE Desc;
				Desc.mObject0		= Handles[SliceIndex0];
				Desc.mObject1		= Handles[SliceIndex1];
//				Desc.mLocalPivot0	= Delta;
//				Desc.mLocalPivot1	= -Delta;
				Desc.mLocalPivot0	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex0];
				Desc.mLocalPivot1	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex1];
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
	}
	return Rim;
}


static void GenerateTire(Pint& pint, const Point& torus_pos, const Quat& torus_rot, float mass, udword nb_loops,
						 Vertices& debug_pts)
{
	// Generate torus. We generate a torus by sweeping a small circle along a large circle, taking N samples (slices) along the way.
	const float BigRadius = 3.0f;
	const float SmallRadius = 1.0f;
//	const float SmallRadius = 0.75f;
//	const udword NbPtsSmallCircle = 8;
//	const udword NbSlices = 16;
	const udword NbPtsSmallCircle = 16;
	const udword NbSlices = 32;
//	const udword NbSlices = 8;

	PintActorHandle Rim;
	if(1)
	{
		// Rims
//		PINT_SPHERE_CREATE ShapeCreate((BigRadius - SmallRadius)*1.1f);
		PINT_SPHERE_CREATE ShapeCreate((BigRadius - SmallRadius));
		ShapeCreate.mRenderer	= CreateSphereRenderer(ShapeCreate.mRadius);

		PINT_OBJECT_CREATE ObjectDesc(&ShapeCreate);
		ObjectDesc.mMass		= mass;
		ObjectDesc.mPosition	= torus_pos;

		ObjectDesc.mAngularVelocity	= Point(0.0f, 0.0f, 100.0f);

		//ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
		Rim = CreatePintObject(pint, ObjectDesc);
	}


	// First we generate a small vertical template circle, oriented in the XY plane.
	Point SmallCirclePts[NbPtsSmallCircle];
	GeneratePolygon(NbPtsSmallCircle, SmallCirclePts, sizeof(Point), ORIENTATION_XY, SmallRadius);

	if(0)
	{
		for(udword i=0;i<NbPtsSmallCircle;i++)
			debug_pts.AddVertex(SmallCirclePts[i]);
	}

	// We'll be sweeping this initial circle along a curve (a larger circle), taking N slices along the way.
	// The final torus will use these vertices exclusively so the total #verts is:
	const udword TotalNbVerts = NbPtsSmallCircle * NbSlices;
	Point* Verts = ICE_NEW(Point)[TotalNbVerts];

	// Now we do the sweep along the larger circle.
	Point SliceCenters[NbSlices];
	{
		const Matrix3x3 TRot = torus_rot;

		udword Index = 0;
		for(udword j=0;j<NbSlices;j++)
		{
			const float Coeff = float(j)/float(NbSlices);

			// We rotate and translate the template circle to position it along the larger circle.
			Matrix3x3 Rot;
			Rot.RotX(Coeff * TWOPI);

			const Point Trans = Rot[1]*BigRadius;
			for(udword i=0;i<NbPtsSmallCircle;i++)
				Verts[Index++] = (Trans + SmallCirclePts[i]*Rot)*TRot;

			SliceCenters[j] = Trans*TRot;
		}
		ASSERT(Index==TotalNbVerts);
	}
	// Here we have generated all the vertices.

	if(0)
	{
		for(udword i=0;i<NbSlices;i++)
			debug_pts.AddVertex(SliceCenters[i]);

		for(udword i=0;i<TotalNbVerts;i++)
			debug_pts.AddVertex(Verts[i]);
		return;
	}

	PintArticHandle Articulation = null;
	if(0 && mass!=0.0f)
		Articulation = pint.CreateArticulation(PINT_ARTICULATION_CREATE());
	PintArticHandle RCArticulation = null;
	if(0 && mass!=0.0f)
		RCArticulation = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE());

	// Next, we generate a convex object for each part of the torus. A part is a section connecting two of the previous slices.
	PintActorHandle Handles[NbSlices];
	Point ObjectCenters[NbSlices];
	udword GroupBit = 0;

	for(udword s=0;s<NbSlices;s++)
	{
		const udword SliceIndex0 = s;
		const udword SliceIndex1 = (s+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		Point ConvexPts[NbPtsSmallCircle*2];
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i];
			ConvexPts[i+NbPtsSmallCircle] = V1[i];
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}
		ObjectCenters[s] = Center;
	}

	for(udword s=0;s<NbSlices;s++)
	{
		const udword SliceIndex0 = s;
		const udword SliceIndex1 = (s+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		Point ConvexPts[NbPtsSmallCircle*2];
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i];
			ConvexPts[i+NbPtsSmallCircle] = V1[i];
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}
		ObjectCenters[s] = Center;

		// Now we create the convex object itself
		PINT_CONVEX_CREATE ConvexCreate(NbPtsSmallCircle*2, ConvexPts);
		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexPts);

//		Handles[s] = CreateDynamicObject(pint, &ConvexCreate, pos+Center);
		{
			PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
			ObjectDesc.mMass			= mass;
			ObjectDesc.mPosition		= torus_pos+Center;
			// Each convex has been computed from rotated small circles, and we only re-centered the verts but
			// didn't cancel the rotation. In theory we'd only need one convex object that we could translate/rotate,
			// but it was easier to create different parts with different rotations. As a result we don't need to set
			// a rotation for the object itself, it's captured in the vertices.
//			ObjectDesc.mRotation		= torus_rot;
			ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;

			if(0)
			{
//				debug_pts.AddVertex(ObjectDesc.mPosition);

				Point N = Center;
				N.Normalize();
				N *= (BigRadius - SmallRadius);
				debug_pts.AddVertex(torus_pos + N);
			}

			if(Articulation)
			{
				PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = s ? Handles[s-1] : null;

					const udword SliceIndex0 = (s-1)%NbSlices;
					const udword SliceIndex1 = (s)%NbSlices;

//					Desc.mObject0		= Handles[SliceIndex0];
//					Desc.mObject1		= Handles[SliceIndex1];
					ArticulatedDesc.mLocalPivot0	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex0];
					ArticulatedDesc.mLocalPivot1	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex1];

				if(1)
				{
//					ArticulatedDesc.mX = Point(0.0f, 0.0f, 1.0f);
				ArticulatedDesc.mEnableTwistLimit = true;
				ArticulatedDesc.mTwistLowerLimit = -0.0001f;
				ArticulatedDesc.mTwistUpperLimit = 0.0001f;
				ArticulatedDesc.mEnableSwingLimit = true;
				ArticulatedDesc.mSwingYLimit = 0.001f;
				ArticulatedDesc.mSwingZLimit = 0.001f;
				}

//				ArticulatedDesc.mLocalPivot0 = LocalPivot0;
//				ArticulatedDesc.mLocalPivot1 = LocalPivot1;
				Handles[s] = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
			}
			else if(RCArticulation)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = s ? Handles[s-1] : null;

					const udword SliceIndex0 = (s-1)%NbSlices;
					const udword SliceIndex1 = (s)%NbSlices;

//					Desc.mObject0		= Handles[SliceIndex0];
//					Desc.mObject1		= Handles[SliceIndex1];
					ArticulatedDesc.mLocalPivot0.mPos	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex0];
					ArticulatedDesc.mLocalPivot1.mPos	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex1];

/*				if(1)
				{
//					ArticulatedDesc.mX = Point(0.0f, 0.0f, 1.0f);
				ArticulatedDesc.mEnableTwistLimit = true;
				ArticulatedDesc.mTwistLowerLimit = -0.0001f;
				ArticulatedDesc.mTwistUpperLimit = 0.0001f;
				ArticulatedDesc.mEnableSwingLimit = true;
				ArticulatedDesc.mSwingYLimit = 0.001f;
				ArticulatedDesc.mSwingZLimit = 0.001f;
				}*/

//				ArticulatedDesc.mLocalPivot0 = LocalPivot0;
//				ArticulatedDesc.mLocalPivot1 = LocalPivot1;
				Handles[s] = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCArticulation);
			}
			else
				Handles[s] = CreatePintObject(pint, ObjectDesc);
		}
		ASSERT(Handles[s]);
	}
	DELETEARRAY(Verts);

	if(Articulation)
		pint.AddArticulationToScene(Articulation);
	if(RCArticulation)
		pint.AddRCArticulationToScene(RCArticulation);

//	const bool CreateFixedJoints = false;
//	if(CreateFixedJoints && !Articulation && !RCArticulation && mass!=0.0f)
	if(1 && Rim && mass!=0.0f)
	{
		for(udword i=0;i<nb_loops;i++)
		{
//			for(udword s=0;s<NbSlices;s++)
//			for(udword s=0;s<NbSlices;s+=2)
			for(udword s=0;s<NbSlices;s+=NbSlices/2)
			{
				const udword SliceIndex0 = s;
//				const udword SliceIndex1 = (s+1)%NbSlices;

	//			debug_pts.AddVertex(ObjectDesc.mPosition);
				Point N = ObjectCenters[s];
				N.Normalize();
//				N *= (BigRadius - SmallRadius);
//				debug_pts.AddVertex(torus_pos + N);

				PINT_FIXED_JOINT_CREATE Desc;
				Desc.mObject0		= Handles[SliceIndex0];
				Desc.mObject1		= Rim;
				Desc.mLocalPivot0	= -N * SmallRadius;
				Desc.mLocalPivot1	= N * (BigRadius - SmallRadius);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
	}

	const bool CreateFixedJoints = true;
	if(CreateFixedJoints && !Articulation && !RCArticulation && mass!=0.0f)
	{
		for(udword i=0;i<nb_loops;i++)
		{
			for(udword s=0;s<NbSlices;s++)
			{
				const udword SliceIndex0 = s;
				const udword SliceIndex1 = (s+1)%NbSlices;
//				const Point Delta = (ObjectCenters[SliceIndex1] - ObjectCenters[SliceIndex0])*0.5f;

				PINT_FIXED_JOINT_CREATE Desc;
				Desc.mObject0		= Handles[SliceIndex0];
				Desc.mObject1		= Handles[SliceIndex1];
//				Desc.mLocalPivot0	= Delta;
//				Desc.mLocalPivot1	= -Delta;
				Desc.mLocalPivot0	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex0];
				Desc.mLocalPivot1	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex1];
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

static PintActorHandle CreateAggregatedPintObject(Pint& pint, PINT_OBJECT_CREATE& desc, PintAggregateHandle aggregate)
{
	desc.mAddToWorld = !aggregate;
	const PintActorHandle H = CreatePintObject(pint, desc);
	if(aggregate)
		pint.AddToAggregate(H, aggregate);
	return H;
}

// ### copied from bulldozer

static PintActorHandle CreateBoxObject(Pint& pint, PintAggregateHandle aggregate, PintArticHandle rca, const Point& extents, const Point& pos, const Quat& rotation, float mass,
										PtrContainer* joints, PintActorHandle anchor, const Point& anchor_pivot, bool top_or_bottom, const PINT_MATERIAL_CREATE* material, udword& nb_rca_links)
{
	PINT_BOX_CREATE BoxDesc(extents);
	BoxDesc.mRenderer	= CreateBoxRenderer(extents);
	BoxDesc.mMaterial	= material;

	PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
	ObjectDesc.mPosition		= pos;
	ObjectDesc.mRotation		= rotation;
	ObjectDesc.mMass			= mass;
	ObjectDesc.mCollisionGroup	= 1;

	PintActorHandle h;
	if(rca && anchor)
	{
		PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
		ArticulatedDesc.mJointType			= PINT_JOINT_HINGE;
		ArticulatedDesc.mParent				= anchor;
		ArticulatedDesc.mAxisIndex			= X_;
		ArticulatedDesc.mLocalPivot0.mPos	= anchor_pivot;
		ArticulatedDesc.mLocalPivot1.mPos	= Point(0.0f, top_or_bottom ? -extents.y : extents.y, 0.0f);
		ArticulatedDesc.mLocalPivot0.mRot	= rotation;
		ArticulatedDesc.mFrictionCoeff		= 0.0f;//###RCAFriction;		TODO
		h = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, rca);
		nb_rca_links++;
	}
	else
	{
/*		ObjectDesc.mAddToWorld	= !aggregate;
		h = CreatePintObject(pint, ObjectDesc);
		if(aggregate)
			pint.AddToAggregate(h, aggregate);*/
		h = CreateAggregatedPintObject(pint, ObjectDesc, aggregate);

		if(anchor)
		{
			PINT_HINGE_JOINT_CREATE Desc;
			Desc.mLocalAxis0	= Point(1.0f, 0.0f, 0.0f);
			Desc.mLocalAxis1	= Point(1.0f, 0.0f, 0.0f);
			Desc.mObject0		= h;
			Desc.mObject1		= anchor;
			Desc.mLocalPivot0	= Point(0.0f, top_or_bottom ? -extents.y : extents.y, 0.0f);
			Desc.mLocalPivot1	= anchor_pivot;
			if(joints)
			{
				PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE)(Desc);
				joints->AddPtr(Copy);
			}
			else
			{
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
	}
	return h;
}

static void CreatePrismatic(Pint& pint, PtrContainer* joints, const Point& p0, const Point& p1,
							PintActorHandle top_anchor, const Point& top_pivot,
							PintActorHandle bottom_anchor, const Point& bottom_pivot, float mass, float stiffness, float damping,
							PintAggregateHandle aggregate, PintArticHandle rca, const PINT_MATERIAL_CREATE* material,
							float scale, udword& nb_rca_links
							//, float mass_inertia_coeff
							)
{
	const float TotalLength = p0.Distance(p1);

	const float L = TotalLength*0.5f;

	const Point TopPrismaticExtents(scale*0.15f, L*0.5f, scale*0.15f);
	const Point BottomPrismaticExtents(scale*0.1f, L*0.5f, scale*0.1f);

	const Point Dir = (p1-p0).Normalize();

	const Quat Q = ShortestRotation(Point(0.0f, 1.0f, 0.0f), Dir);

	const Point TopPrismaticPos = p0 + Dir * L * 0.5f;
	const Point BottomPrismaticPos = TopPrismaticPos + Dir * L;

	const PintActorHandle TopPrismaticObject = CreateBoxObject(pint, aggregate, rca, TopPrismaticExtents, TopPrismaticPos, Q, mass, joints, top_anchor, top_pivot, true, material, nb_rca_links);
	const PintActorHandle BottomPrismaticObject = CreateBoxObject(pint, aggregate, rca, BottomPrismaticExtents, BottomPrismaticPos, Q, mass, joints, bottom_anchor, bottom_pivot, false, material, nb_rca_links);

/*	PINT_HINGE_JOINT_CREATE Desc;
	Desc.mLocalAxis0	= Point(1.0f, 0.0f, 0.0f);
	Desc.mLocalAxis1	= Point(1.0f, 0.0f, 0.0f);
	if(top_anchor)
	{
		Desc.mObject0		= TopPrismaticObject;
		Desc.mObject1		= top_anchor;
		Desc.mLocalPivot0	= Point(0.0f, -TopPrismaticExtents.y, 0.0f);
		Desc.mLocalPivot1	= top_pivot;
		if(joints)
		{
			PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE)(Desc);
			joints->Add(udword(Copy));
		}
		else
		{
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
	}

	if(bottom_anchor)
	{
		Desc.mObject0		= BottomPrismaticObject;
		Desc.mObject1		= bottom_anchor;
		Desc.mLocalPivot0	= Point(0.0f, BottomPrismaticExtents.y, 0.0f);
		Desc.mLocalPivot1	= bottom_pivot;
		if(joints)
		{
			PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE)(Desc);
			joints->Add(udword(Copy));
		}
		else
		{
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
	}*/

	if(1)
	{
		// RCA doesn't have soft limits so we use a regular joint
		PINT_PRISMATIC_JOINT_CREATE Desc;
		Desc.mObject0			= TopPrismaticObject;
		Desc.mObject1			= BottomPrismaticObject;
		//###PRISMATIC2
		Desc.mLocalPivot0.mPos	= Point(0.0f, 0.0f, 0.0f);
		Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
		Desc.mLocalAxis0		= Point(0.0f, 1.0f, 0.0f);
		Desc.mLocalAxis1		= Point(0.0f, 1.0f, 0.0f);
		Desc.mLimits.Set(0.0f, TopPrismaticExtents.y*2.0f);
		if(1)
		{
			Desc.mLimits.Set(TopPrismaticExtents.y*1.9f, TopPrismaticExtents.y*2.0f);
/*			Desc.mSpringStiffness	= 400.0f;
			Desc.mSpringDamping		= 10.0f;
				Desc.mSpringStiffness	= 4000.0f;
				Desc.mSpringDamping		= 100.0f;
					Desc.mSpringStiffness	= 40000.0f;
					Desc.mSpringDamping		= 1000.0f;*/
			Desc.mSpring.mStiffness	= stiffness;
			Desc.mSpring.mDamping	= damping;
		}

		if(joints)
		{
			PINT_PRISMATIC_JOINT_CREATE* Copy = ICE_NEW(PINT_PRISMATIC_JOINT_CREATE)(Desc);
			joints->AddPtr(Copy);
		}
		else
		{
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
	}
}

	class WheelCreator
	{
	public:
								WheelCreator();
								~WheelCreator();

		void					Release();
		void					GenerateTireCompound(const Quat& torus_rot, const PINT_MATERIAL_CREATE& material, float big_radius, float small_radius, udword nb_slices, udword nb_pts_small_circle);

		PINT_OBJECT_CREATE		mObjectDesc;
		PINT_SPHERE_CREATE		mSphereDesc;
		PINT_CONVEX_CREATE		mConvexDesc;

		udword					mNbSlices;
		Point**					mVertsCopy;
		PINT_CONVEX_CREATE**	mShapes;
		Point*					mVerts;
	};

WheelCreator::WheelCreator() :
	mNbSlices	(0),
	mVertsCopy	(null),
	mShapes		(null),
	mVerts		(null)
{
}

WheelCreator::~WheelCreator()
{
	Release();
}

void WheelCreator::Release()
{
	for(udword s=0;s<mNbSlices;s++)
	{
		DELETESINGLE(mShapes[s]);
		DELETEARRAY(mVertsCopy[s]);
	}
	ICE_FREE(mShapes);
	ICE_FREE(mVertsCopy);
	DELETEARRAY(mVerts);
}

void WheelCreator::GenerateTireCompound(const Quat& torus_rot, const PINT_MATERIAL_CREATE& material, float big_radius, float small_radius, udword nb_slices, udword nb_pts_small_circle)
{
	Release();

	// Generate torus. We generate a torus by sweeping a small circle along a large circle, taking N samples (slices) along the way.
	const float BigRadius = big_radius;	//3.0f * 0.25f * scale;
	const float SmallRadius = small_radius;	//1.0f * 0.25f * scale;
	const udword NbPtsSmallCircle = nb_pts_small_circle;//16;
	const udword NbSlices = nb_slices;//32;
	mNbSlices = NbSlices;

	// First we generate a small vertical template circle, oriented in the XY plane.
	Point* SmallCirclePts = (Point*)StackAlloc(NbPtsSmallCircle*sizeof(Point));
//	GeneratePolygon(NbPtsSmallCircle, SmallCirclePts, sizeof(Point), ORIENTATION_XY, SmallRadius);
	GeneratePolygon(NbPtsSmallCircle, SmallCirclePts, sizeof(Point), ORIENTATION_YZ, SmallRadius);

	// We'll be sweeping this initial circle along a curve (a larger circle), taking N slices along the way.
	// The final torus will use these vertices exclusively so the total #verts is:
	const udword TotalNbVerts = NbPtsSmallCircle * NbSlices;
	Point* Verts = ICE_NEW(Point)[TotalNbVerts];
	mVerts = Verts;

	// Now we do the sweep along the larger circle.
	Point* SliceCenters = (Point*)StackAlloc(NbSlices*sizeof(Point));
	{
		const Matrix3x3 TRot = torus_rot;

		udword Index = 0;
		for(udword j=0;j<NbSlices;j++)
		{
			const float Coeff = float(j)/float(NbSlices);

			// We rotate and translate the template circle to position it along the larger circle.
			Matrix3x3 Rot;
//			Rot.RotX(Coeff * TWOPI);
			Rot.RotZ(Coeff * TWOPI);

			const Point Trans = Rot[1]*BigRadius;
			for(udword i=0;i<NbPtsSmallCircle;i++)
				Verts[Index++] = (Trans + SmallCirclePts[i]*Rot)*TRot;

			SliceCenters[j] = Trans*TRot;
		}
		ASSERT(Index==TotalNbVerts);
	}
	// Here we have generated all the vertices.

	// Next, we generate a convex object for each part of the torus. A part is a section connecting two of the previous slices.
	Point* ConvexPts = (Point*)StackAlloc(NbPtsSmallCircle*2*sizeof(Point));
	for(udword s=0;s<NbSlices;s++)
	{
		const udword SliceIndex0 = s;
		const udword SliceIndex1 = (s+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i];
			ConvexPts[i+NbPtsSmallCircle] = V1[i];
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}
	}

//	Point* VertsCopy[NbSlices];
//	PINT_CONVEX_CREATE* Shapes[NbSlices];
	mVertsCopy = (Point**)ICE_ALLOC(sizeof(Point*)*NbSlices);
	mShapes = (PINT_CONVEX_CREATE**)ICE_ALLOC(sizeof(PINT_CONVEX_CREATE*)*NbSlices);

	PINT_SHAPE_CREATE* SC = null;
	for(udword s=0;s<NbSlices;s++)
	{
		const udword SliceIndex0 = s;
		const udword SliceIndex1 = (s+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i];
			ConvexPts[i+NbPtsSmallCircle] = V1[i];
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}

		// Now we create the convex object itself
//		PINT_CONVEX_CREATE ConvexCreate(NbPtsSmallCircle*2, ConvexPts);
//		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexPts);

		mVertsCopy[s] = ICE_NEW(Point)[NbPtsSmallCircle*2];
		CopyMemory(mVertsCopy[s], ConvexPts, sizeof(Point)*NbPtsSmallCircle*2);

//		PINT_CONVEX_CREATE* ConvexCreate = ICE_NEW(PINT_CONVEX_CREATE)(NbPtsSmallCircle*2, ConvexPts);
		PINT_CONVEX_CREATE* ConvexCreate = ICE_NEW(PINT_CONVEX_CREATE)(NbPtsSmallCircle*2, mVertsCopy[s]);
//		ConvexCreate->mRenderer	= CreateConvexRenderer(ConvexCreate->mNbVerts, ConvexPts);
		ConvexCreate->mRenderer	= CreateConvexRenderer(ConvexCreate->mNbVerts, mVertsCopy[s]);
		ConvexCreate->mLocalPos	= Center;
		ConvexCreate->mMaterial = &material;
		mShapes[s] = ConvexCreate;

		SC = mObjectDesc.AddShape(ConvexCreate, SC);
	}
}

static const char* gDesc_MultiBodyVehicle = "Test vehicle using rigid bodies, joints, articulations, aggregates.";

class MultiBodyVehicle : public VehicleInput
{
	enum WheelType
	{
		WHEEL_SPHERE_VISUAL_SPHERE,
		WHEEL_SPHERE_VISUAL_CYLINDER,
		WHEEL_CYLINDER,
		WHEEL_TORUS,
		WHEEL_PRISMATIC,
	};

	enum DriveType_
	{
		DRIVE_TYPE_4WD,
		DRIVE_TYPE_RWD,
		DRIVE_TYPE_FWD,
	};

#define MAX_NB_WHEELS	16

	struct MyData : Allocateable
	{
		MyData() : mNbWheels(0), mChassis(null), mSteeringJoint(null)
		{
/*			for(udword i=0;i<MAX_NB_WHEELS;i++)
//				mWheelJoints[i] = null;
				mRCAWheel[i].mHandle = null;*/
		}
		PintActorHandle		mChassis;
		PintJointHandle		mSteeringJoint;
//		PintJointHandle		mWheelJoints[MAX_NB_WHEELS];

		class Wheel
		{
			public:
				Wheel() : mHandle(null), mIsFront(false)	{}

				void	Init(PintActorHandle handle, bool is_front)
				{
					mHandle		= handle;
					mIsFront	= is_front;
				}

				inline_	PintActorHandle	GetHandle()	const	{ return mHandle;	}
				inline_	bool			IsFront()	const	{ return mIsFront;	}
			private:
			PintActorHandle	mHandle;
			bool			mIsFront;
		};

		void	AddWheel(PintActorHandle handle, bool is_front)
		{
			ASSERT(mNbWheels<MAX_NB_WHEELS);
			mWheel[mNbWheels++].Init(handle, is_front);
		}

		inline_	udword			GetNbWheels()	const	{ return mNbWheels;	}
		inline_	const Wheel*	GetWheels()		const	{ return mWheel;	}

		private:
		udword				mNbWheels;
		Wheel				mWheel[MAX_NB_WHEELS];
	};

			Vertices		mDebugPts;

			CheckBoxPtr		mCheckBox_DriveVehicle;
			CheckBoxPtr		mCheckBox_AttachCar;
			CheckBoxPtr		mCheckBox_AttachWheels;
			CheckBoxPtr		mCheckBox_AttachSteering;
			CheckBoxPtr		mCheckBox_UseRCA;
			CheckBoxPtr		mCheckBox_UseAggregates;
			CheckBoxPtr		mCheckBox_TweakWheelsInertia;

			EditBoxPtr		mEditBox_GlobalScale;
			EditBoxPtr		mEditBox_CameraUpOffset;
			EditBoxPtr		mEditBox_CameraDistToTarget;

			EditBoxPtr		mEditBox_ChassisWidth;
			EditBoxPtr		mEditBox_ChassisHeight;
			EditBoxPtr		mEditBox_ChassisLength;
			EditBoxPtr		mEditBox_ChassisMass;
			EditBoxPtr		mEditBox_ChassisCOMOffsetX;
			EditBoxPtr		mEditBox_ChassisCOMOffsetY;
			EditBoxPtr		mEditBox_ChassisCOMOffsetZ;
			EditBoxPtr		mEditBox_MassElements;
			EditBoxPtr		mEditBox_ChassisFriction;

			EditBoxPtr		mEditBox_WheelMass;
			EditBoxPtr		mEditBox_WheelFrontFriction;
//			EditBoxPtr		mEditBox_WheelRearFriction;
			EditBoxPtr		mEditBox_WheelRadius;
			EditBoxPtr		mEditBox_WheelWidth;
			EditBoxPtr		mEditBox_WheelTessR;
			EditBoxPtr		mEditBox_WheelTessW;

			EditBoxPtr		mEditBox_SteerStiffness;
			EditBoxPtr		mEditBox_SteerDamping;
			EditBoxPtr		mEditBox_SteerLimitAngle;

			EditBoxPtr		mEditBox_EngineTorque;

			EditBoxPtr		mEditBox_WishboneWidth;
			EditBoxPtr		mEditBox_WishboneHeight;
			EditBoxPtr		mEditBox_WishboneLength;
			EditBoxPtr		mEditBox_SuspStiffness;
			EditBoxPtr		mEditBox_SuspDamping;
			EditBoxPtr		mEditBox_SuspHeight;
			EditBoxPtr		mEditBox_SuspLimitAngle;
			EditBoxPtr		mEditBox_AttachWidth;
			EditBoxPtr		mEditBox_AttachHeight;
			EditBoxPtr		mEditBox_AttachLength;

			EditBoxPtr		mEditBox_Friction;
			EditBoxPtr		mEditBox_RCAFriction;
			EditBoxPtr		mEditBox_Speed;
			EditBoxPtr		mEditBox_InertiaCoeff;
			EditBoxPtr		mEditBox_Multiplier;

			ComboBoxPtr		mComboBox_NbWheels;
			ComboBoxPtr		mComboBox_WheelType;
			ComboBoxPtr		mComboBox_DriveType;
			ComboBoxPtr		mComboBox_Suspension;
			ComboBoxPtr		mComboBox_EngineMode;
			ComboBoxPtr		mComboBox_Level;

			SliderPtr		mSteerSlider;

			float			mCurrentVel;

	public:
							MultiBodyVehicle() : mCurrentVel(0.0f)
								{
									mCamera.mUpOffset		= 5.0f;
									mCamera.mDistToTarget	= 15.0f;
									mControlCamera			= true;
								}
	virtual					~MultiBodyVehicle()			{									}
	virtual	const char*		GetName()			const	{ return "MultiBodyVehicle";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_MultiBodyVehicle;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_RCARTICULATIONS;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 400;
		WD.mHeight	= 600;
		WD.mLabel	= "Multibody vehicle settings";
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
		y += YStep;
		AddResetButton(UI, 4, y, WD.mWidth - 16);

		// Tab control
		IceTabControl* TabControl;
		enum TabIndex
		{
			TAB_GENERAL,
			TAB_CHASSIS,
			TAB_WHEEL_TIRES,
			TAB_ENGINE_GEARBOX,
			TAB_SUSPENSION,
			TAB_DIFFERENTIAL,
			TAB_STEERING,
			TAB_COUNT,
		};
		IceWindow* Tabs[TAB_COUNT];
		{
			TabControlDesc TCD;
			TCD.mParent	= UI;
			TCD.mX		= 4;
			TCD.mY		= 0;
			TCD.mY		= y + 50;
			TCD.mWidth	= WD.mWidth - 16;
			TCD.mHeight	= 450;
			TabControl = ICE_NEW(IceTabControl)(TCD);
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
			TabControl->Add(Tabs[TAB_GENERAL], "General");
			TabControl->Add(Tabs[TAB_CHASSIS], "Chassis");
			TabControl->Add(Tabs[TAB_WHEEL_TIRES], "Wheels/tires");
			TabControl->Add(Tabs[TAB_ENGINE_GEARBOX], "Engine/gearbox");
			TabControl->Add(Tabs[TAB_SUSPENSION], "Suspension");
			TabControl->Add(Tabs[TAB_DIFFERENTIAL], "Differential");
			TabControl->Add(Tabs[TAB_STEERING], "Steering");
		}

		// TAB_GENERAL
		{
			IceWindow* TabWindow = Tabs[TAB_GENERAL];
			y = 4;

			// TODO: refactor with VehicleSDK test + others where vehicles are involved
			{
				IceComboBox* CB = CreateComboBox<IceComboBox>(TabWindow, 0, 4, y, 150, 20, "Level", &UIElems, null);
				CB->Add("Flat");
				CB->Add("Flat with box bumps");
				CB->Add("Terrain");
				CB->Add("Race track");
				CB->Select(1);
				mComboBox_Level = CB;
				y += YStep;
				y += YStep;
			}

			mCheckBox_DriveVehicle = helper.CreateCheckBox(TabWindow, 0, 4, y, 100, 20, "Drive vehicle", &UIElems, true, null, null);
			y += YStep;

			mCheckBox_AttachCar = helper.CreateCheckBox(TabWindow, 0, 4, y, 100, 20, "Attach car", &UIElems, false, null);
			y += YStep;

			mCheckBox_AttachWheels = helper.CreateCheckBox(TabWindow, 0, 4, y, 100, 20, "Attach wheels", &UIElems, true, null);
			y += YStep;

			mCheckBox_AttachSteering = helper.CreateCheckBox(TabWindow, 0, 4, y, 100, 20, "Attach steering", &UIElems, true, null);
			y += YStep;

			mCheckBox_UseRCA = helper.CreateCheckBox(TabWindow, 0, 4, y, 100, 20, "Use RCA", &UIElems, true, null);
			y += YStep;

			mCheckBox_UseAggregates = helper.CreateCheckBox(TabWindow, 0, 4, y, 100, 20, "Use aggregates", &UIElems, false, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Friction:", &UIElems);
			mEditBox_Friction = helper.CreateEditBox(TabWindow, 2, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "RCA Friction:", &UIElems);
			mEditBox_RCAFriction = helper.CreateEditBox(TabWindow, 2, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Auto speed:", &UIElems);
			mEditBox_Speed = helper.CreateEditBox(TabWindow, 2, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Inertia coeff:", &UIElems);
			mEditBox_InertiaCoeff = helper.CreateEditBox(TabWindow, 2, 4+OffsetX, y, EditBoxWidth, 20, "-1.0", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Multiplier:", &UIElems);
			mEditBox_Multiplier = helper.CreateEditBox(TabWindow, 2, 4+OffsetX, y, EditBoxWidth, 20, "1", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Global scale:", &UIElems);
			mEditBox_GlobalScale = helper.CreateEditBox(TabWindow, 2, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Camera up offset:", &UIElems);
			mEditBox_CameraUpOffset = helper.CreateEditBox(TabWindow, 2, 4+OffsetX, y, EditBoxWidth, 20, "5.0", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Camera distance:", &UIElems);
			mEditBox_CameraDistToTarget = helper.CreateEditBox(TabWindow, 2, 4+OffsetX, y, EditBoxWidth, 20, "15.0", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;
		}

		// TAB_CHASSIS
		{
			IceWindow* TabWindow = Tabs[TAB_CHASSIS];
			y = 4;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Width:", &UIElems);
			mEditBox_ChassisWidth = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Height:", &UIElems);
			mEditBox_ChassisHeight = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.2", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Length:", &UIElems);
			mEditBox_ChassisLength = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "5.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Mass chassis:", &UIElems);
			mEditBox_ChassisMass = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1500.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Friction:", &UIElems);
			mEditBox_ChassisFriction = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.2", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "COM X offset:", &UIElems);
			mEditBox_ChassisCOMOffsetX = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "COM Y offset:", &UIElems);
			mEditBox_ChassisCOMOffsetY = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "COM Z offset:", &UIElems);
			mEditBox_ChassisCOMOffsetZ = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;


/*			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "MOI Y coeff:", &UIElems);
			mEditBox_ChassisMOICoeffY = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.8", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Force app. CM Y offset (~body roll):", &UIElems);
			mEditBox_ForceApplicationCMOffsetY = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.3", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;*/
		}

		// TAB_WHEEL_TIRES
		{
			IceWindow* TabWindow = Tabs[TAB_WHEEL_TIRES];
			y = 4;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Radius:", &UIElems);
			mEditBox_WheelRadius = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Width:", &UIElems);
			mEditBox_WheelWidth = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Tessellation R:", &UIElems);
			mEditBox_WheelTessR = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "32", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Tessellation W:", &UIElems);
			mEditBox_WheelTessW = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "6", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;

/*			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Coeff X:", &UIElems);
			mEditBox_WheelCoeffX = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.85", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Coeff Z:", &UIElems);
			mEditBox_WheelCoeffZ = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.85", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;*/

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Mass:", &UIElems);
			mEditBox_WheelMass = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "16.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Friction:", &UIElems);
			mEditBox_WheelFrontFriction = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.8", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

//			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Rear friction:", &UIElems);
//			mEditBox_WheelRearFriction = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
//			y += YStep;

			mCheckBox_TweakWheelsInertia = helper.CreateCheckBox(TabWindow, 0, 4, y, 100, 20, "Tweak inertia", &UIElems, false, null);
			y += YStep;

/*			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Max brake torque front:", &UIElems);
			mEditBox_WheelMaxBrakeTorqueFront = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "150.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Max brake torque rear:", &UIElems);
			mEditBox_WheelMaxBrakeTorqueRear = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1500.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Max steer front:", &UIElems);
			mEditBox_WheelMaxSteerFront = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.047", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Max steer rear:", &UIElems);
			mEditBox_WheelMaxSteerRear = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Tire friction multiplier (~grip):", &UIElems);
			mEditBox_TireFrictionMultiplier = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.1", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;*/

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Nb wheels:", &UIElems);
			{
				ComboBoxDesc CBBD;
				CBBD.mID		= 0;
				CBBD.mParent	= TabWindow;
				CBBD.mX			= 4+OffsetX;
				CBBD.mY			= y;
				CBBD.mWidth		= 140;
				CBBD.mHeight	= 20;
				CBBD.mLabel		= "Nb wheels";
				IceComboBox* CB = ICE_NEW(IceComboBox)(CBBD);
				CB->Add("4");
				CB->Add("6");
				CB->Add("8");
				CB->Select(0);
				CB->SetVisible(true);
				mComboBox_NbWheels = CB;
			}
			RegisterUIElement(mComboBox_NbWheels);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Wheel type:", &UIElems);
			{
				ComboBoxDesc CBBD;
				CBBD.mID		= 0;
				CBBD.mParent	= TabWindow;
				CBBD.mX			= 4+OffsetX;
				CBBD.mY			= y;
				CBBD.mWidth		= 140;
				CBBD.mHeight	= 20;
				CBBD.mLabel		= "Wheel type";
				IceComboBox* CB = ICE_NEW(IceComboBox)(CBBD);
				CB->Add("Sphere (visual: sphere)");
				CB->Add("Sphere (visual: cylinder)");
				CB->Add("Cylinder");
				CB->Add("Torus");
				CB->Add("Prismatic");
				CB->Select(3);
				CB->SetVisible(true);
				mComboBox_WheelType = CB;
			}
			RegisterUIElement(mComboBox_WheelType);
			y += YStep;
		}

		// TAB_ENGINE_GEARBOX
		{
			IceWindow* TabWindow = Tabs[TAB_ENGINE_GEARBOX];
			y = 4;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Control:", &UIElems);
			{
				ComboBoxDesc CBBD;
				CBBD.mID		= 0;
				CBBD.mParent	= TabWindow;
				CBBD.mX			= 4+OffsetX;
				CBBD.mY			= y;
				CBBD.mWidth		= 140;
				CBBD.mHeight	= 20;
				CBBD.mLabel		= "Control";
				IceComboBox* CB = ICE_NEW(IceComboBox)(CBBD);
				CB->Add("AddLocalTorque");
				CB->Add("Motor");
				CB->Select(0);
				CB->SetVisible(true);
				mComboBox_EngineMode = CB;
			}
			RegisterUIElement(mComboBox_EngineMode);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Torque:", &UIElems);
			mEditBox_EngineTorque = helper.CreateEditBox(TabWindow, 2, 4+OffsetX, y, EditBoxWidth, 20, "500.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

/*			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Engine peak torque:", &UIElems);
			mEditBox_EnginePeakTorque = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1000.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Engine max omega:", &UIElems);
			mEditBox_EngineMaxOmega = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "1000.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Gears switch time:", &UIElems);
			mEditBox_GearsSwitchTime = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Clutch strength:", &UIElems);
			mEditBox_ClutchStrength = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "10.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			IceEditBox* tmp = helper.CreateEditBox(TabWindow, 0, 4, y+20, 250, 100, "", &UIElems, EDITBOX_TEXT, null);
			tmp->SetMultilineText("Most engine & gearbox params are currently\nhardcoded and not exposed to the UI.");
			tmp->SetReadOnly(true);*/
		}

		// TAB_SUSPENSION
		{
			IceWindow* TabWindow = Tabs[TAB_SUSPENSION];
			y = 4;

/*			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Suspension:", &UIElems);
			{
				ComboBoxDesc CBBD;
				CBBD.mID		= 0;
				CBBD.mParent	= TabWindow;
				CBBD.mX			= 4+OffsetX;
				CBBD.mY			= y;
				CBBD.mWidth		= 140;
				CBBD.mHeight	= 20;
				CBBD.mLabel		= "Suspension";
				IceComboBox* CB = ICE_NEW(IceComboBox)(CBBD);
				CB->Add("Type1");
				CB->Add("Type2");
				CB->Select(0);
				CB->SetVisible(true);
				mComboBox_Suspension = CB;
			}
			RegisterUIElement(mComboBox_Suspension);
			y += YStep;*/

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Mass elements:", &UIElems);
			mEditBox_MassElements = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "10.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Stiffness:", &UIElems);
			mEditBox_SuspStiffness = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "80000.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Damping:", &UIElems);
			mEditBox_SuspDamping = helper.CreateEditBox(TabWindow, 2, 4+OffsetX, y, EditBoxWidth, 20, "8000.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Height:", &UIElems);
			mEditBox_SuspHeight = helper.CreateEditBox(TabWindow, 2, 4+OffsetX, y, EditBoxWidth, 20, "2.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Limit angle:", &UIElems);
			mEditBox_SuspLimitAngle = helper.CreateEditBox(TabWindow, 2, 4+OffsetX, y, EditBoxWidth, 20, "45.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Wishbone width:", &UIElems);
			mEditBox_WishboneWidth = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.75", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Wishbone height:", &UIElems);
			mEditBox_WishboneHeight = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.1", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Wishbone length:", &UIElems);
			mEditBox_WishboneLength = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Attach width:", &UIElems);
			mEditBox_AttachWidth = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.25", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Attach height:", &UIElems);
			mEditBox_AttachHeight = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.25", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Attach length:", &UIElems);
			mEditBox_AttachLength = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.1", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;


/*			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Max compression:", &UIElems);
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
			y += YStep;*/
		}

		// TAB_DIFFERENTIAL
		{
			IceWindow* TabWindow = Tabs[TAB_DIFFERENTIAL];
			y = 4;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Drive type:", &UIElems);
			{
				ComboBoxDesc CBBD;
				CBBD.mID		= 0;
				CBBD.mParent	= TabWindow;
				CBBD.mX			= 4+OffsetX;
				CBBD.mY			= y;
				CBBD.mWidth		= 140;
				CBBD.mHeight	= 20;
				CBBD.mLabel		= "Drive type";
				IceComboBox* CB = ICE_NEW(IceComboBox)(CBBD);
				CB->Add("4WD");
				CB->Add("RWD");
				CB->Add("FWD");
				CB->Select(0);
				CB->SetVisible(true);
				mComboBox_DriveType = CB;
			}
			RegisterUIElement(mComboBox_DriveType);
//			y += YStep;
//			y += YStep;


/*			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Differential:", &UIElems);
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

				CB->Select(0);
				CB->SetVisible(true);

				mComboBox_Differential = CB;
			}*/
		}

		// TAB_STEERING
		{
			IceWindow* TabWindow = Tabs[TAB_STEERING];
			y = 4;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Stiffness:", &UIElems);
//			mEditBox_SteerStiffness = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "10000.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_SteerStiffness = helper.CreateEditBox(TabWindow, 1, 4+OffsetX, y, EditBoxWidth, 20, "30000.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Damping:", &UIElems);
			mEditBox_SteerDamping = helper.CreateEditBox(TabWindow, 2, 4+OffsetX, y, EditBoxWidth, 20, "5000.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LabelWidth, 20, "Limit angle:", &UIElems);
			mEditBox_SteerLimitAngle = helper.CreateEditBox(TabWindow, 2, 4+OffsetX, y, EditBoxWidth, 20, "40.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			y += YStep;
			y += YStep;
			SliderDesc SD;
			SD.mStyle		= SLIDER_HORIZONTAL;
			SD.mID			= 0;
			SD.mParent		= TabWindow;
			SD.mX			= 4;
			SD.mY			= y;
			SD.mWidth		= WD.mWidth - SD.mX*2 - 32;
			SD.mHeight		= 20;
			SD.mLabel		= "test";
			mSteerSlider	= ICE_NEW(IceSlider)(SD);
			mSteerSlider->SetRange(0.0f, 1.0f, 100);
			mSteerSlider->SetValue(0.5f);

//			mSteerSlider->SetIntRange(0, 100);
//			mSteerSlider->SetIntValue(50);

//			mSteerSlider->SetSteps(int line, int page);
			UIElems.Register(mSteerSlider);
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

				GW->SetKey(0, 0.0f/120.0f, APP_TO_GW(0.75f));

				Keyframe k;
				k.mX = 5.0f/120.0f;		k.mY = APP_TO_GW(0.75f);	GW->CreateKey(k);
				k.mX = 30.0f/120.0f;	k.mY = APP_TO_GW(0.125f);	GW->CreateKey(k);
				k.mX = 120.0f/120.0f;	k.mY = APP_TO_GW(0.1f);		GW->CreateKey(k);
				GW->SetVisible(true);
			}
		}

		return TabControl;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleInput::GetSceneParams(desc);
		
		desc.mCamera[0] = PintCameraPose(Point(-12.12f, 6.56f, -0.21f), Point(0.99f, -0.16f, 0.01f));
		desc.mCamera[1] = PintCameraPose(Point(-9.42f, 6.54f, 2.11f), Point(0.90f, -0.28f, -0.33f));

//		desc.mGravity.y = -9.81f * 2.0f;

		SetDefEnv(desc, false);
	}

	virtual void		Close(Pint& pint)
	{
		MyData* UserData = (MyData*)pint.mUserData;
		DELETESINGLE(UserData);
		pint.mUserData = null;

		TestBase::Close(pint);
	}

	bool	InitLevel(Pint* pint, const PintCaps* caps, const PINT_MATERIAL_CREATE* user_material)
	{
		const bool IsCommonSetup = !pint || !caps;
		const udword Index = mComboBox_Level->GetSelectedIndex();
		if(IsCommonSetup)
		{
			if(Index==0 || Index==1)
			{
				mPlanarMeshHelper.Generate(200, 0.5f);

				if(Index==1)
				{
					if(1)
					{
						const float LoopRadius = 20.0f;
						LoopTheLoop LTL;
						LTL.Generate(LoopRadius, 15.0f, 64, false);

						IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
						IS->Init(LTL.mNbTris, LTL.mNbVerts, LTL.mVerts, reinterpret_cast<const IndexedTriangle*>(LTL.mIndices));

						const PR Pose(Point(-200.0f, LoopRadius, 200.0f), Quat(Idt));
						RegisterSurface(IS, null, &Pose, null);
					}

					if(1)
					{
						const float LoopRadius = 20.0f;
						LoopTheLoop LTL;
						LTL.Generate(LoopRadius, 150.0f, 64, true);

						IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
						IS->Init(LTL.mNbTris, LTL.mNbVerts, LTL.mVerts, reinterpret_cast<const IndexedTriangle*>(LTL.mIndices));

						const PR Pose(Point(-200.0f, LoopRadius, -200.0f), Quat(Idt));
						RegisterSurface(IS, null, &Pose, null);
					}
				}
			}
			else if(Index==2)
			{
				const Point Scale(1.0f, 0.2f, 1.0f);
				LoadMeshesFromFile_(*this, "Terrain.bin", &Scale, false, 0);
			}
			else if(Index==3)
			{
				IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
				{
					RaceTrack RT;
					RT.Generate();

					IS->Init(RT.mNbTris, RT.mNbVerts, RT.mVerts, (const IndexedTriangle*)RT.mIndices);

					Point Center;
					IS->GetFace(0)->Center(IS->GetVerts(), Center);

					IS->Translate(-Center);
				}
				RegisterSurface(IS);
			}
		}
		else
		{
			if(Index==0 || Index==1)
			{
				const float Altitude = 0.0f;
				mPlanarMeshHelper.CreatePlanarMesh(*pint, Altitude, user_material);

				if(Index==1)
				{
	//				PINT_BOX_CREATE BoxDesc(10.0f, 1.0f, 50.0f);
						PINT_BOX_CREATE BoxDesc(50.0f, 0.5f, 10.0f);
					BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

					const udword NbRamps = 64;
					for(udword i=0;i<NbRamps;i++)
					{
						const float Coeff = float(i)/float(NbRamps-1);

						Matrix3x3 Rot;
		//				Rot.RotX(25.0f*DEGTORAD);
		//				Rot.RotX(20.0f*DEGTORAD);
		//					Rot.RotZ(-20.0f*DEGTORAD);
							Rot.RotZ(-Coeff*45.0f*DEGTORAD);

						PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		//				ObjectDesc.mPosition	= Point(-200.0f, 0.0f, -100.0f);
							ObjectDesc.mPosition	= Point(-600.0f, 0.0f, (float(i)-float(NbRamps/2))*20.0f);
						ObjectDesc.mRotation	= Rot;
						ObjectDesc.mMass		= 0.0f;
						CreatePintObject(*pint, ObjectDesc);

		/*					Rot.RotX(-25.0f*DEGTORAD);

							ObjectDesc.mPosition	= Point(-200.0f, 0.0f, -200.0f);
							ObjectDesc.mRotation	= Rot;
							CreatePintObject(pint, ObjectDesc);*/
					}

					if(1)
					{
						const float Spacing = 10.0f;
						const float Size = 0.4f*0.5f;
					//	const float Size = 0.3f;
					//	const float Size = 0.2f;
					//	const float Size = 1.0f;
						GenerateGroundObstacles(*pint, Spacing, Size, false);
					}
				}
			}
			return CreateMeshesFromRegisteredSurfaces(*pint, *caps, user_material);
		}
		return true;
	}

	virtual	bool		CommonSetup()
	{
		SetCameraMode(2);
		const float GlobalScale	= GetFloat(1.0f, mEditBox_GlobalScale);
		mCamera.mUpOffset		= GlobalScale*GetFloat(5.0f, mEditBox_CameraUpOffset);
		mCamera.mDistToTarget	= GlobalScale*GetFloat(15.0f, mEditBox_CameraDistToTarget);

		const bool AttachCar = mCheckBox_AttachCar ? mCheckBox_AttachCar->IsChecked() : false;
		if(!AttachCar)
			InitLevel(null, null, null);

		return VehicleInput::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
//		if(!caps.mSupportRigidBodySimulation || !caps.mSupportConvexes || !caps.mSupportFixedJoints || !caps.mSupportCollisionGroups)
//			return false;

		const PintDisabledGroups DG(1, 1);
		pint.SetDisabledGroups(1, &DG);

		MyData* UserData = ICE_NEW(MyData);
		pint.mUserData = UserData;

		/////////////////

		const float GlobalScale	= GetFloat(1.0f, mEditBox_GlobalScale);
		const float WheelRadius = GlobalScale*GetFloat(1.5f, mEditBox_WheelRadius);
		const float WheelWidth = GlobalScale*GetFloat(0.5f, mEditBox_WheelWidth);
		const udword WheelTessR = GetInt(60, mEditBox_WheelTessR);
		const udword WheelTessW = GetInt(60, mEditBox_WheelTessW);

			const udword _NbPts = WheelTessR*2;
			const CylinderMesh Cylinder(_NbPts, WheelRadius, WheelWidth*0.5f);
			const udword TotalNbVerts = Cylinder.mNbVerts;
			const Point* Verts = Cylinder.mVerts;

		const bool AttachCar = mCheckBox_AttachCar ? mCheckBox_AttachCar->IsChecked() : false;
		const bool AttachWheels = mCheckBox_AttachWheels ? mCheckBox_AttachWheels->IsChecked() : false;
		const bool AttachSteering = mCheckBox_AttachSteering ? mCheckBox_AttachSteering->IsChecked() : false;
//		const bool AttachCar = true;
//		const bool AttachWheels = false;
//		const bool AttachSteering = false;
//		mCameraMode = 0;
		const bool TweakWheelsInertia = mCheckBox_TweakWheelsInertia ? mCheckBox_TweakWheelsInertia->IsChecked() : false;	
		const float ChassisMass = GetFloat(1000.0f, mEditBox_ChassisMass);
		const float ChassisWidth = GlobalScale*GetFloat(1.0f, mEditBox_ChassisWidth);
		const float ChassisHeight = GlobalScale*GetFloat(0.2f, mEditBox_ChassisHeight);
		const float ChassisLength = GlobalScale*GetFloat(5.0f, mEditBox_ChassisLength);
		const float ElemMass = GetFloat(20.0f, mEditBox_MassElements);
		const float WheelMass = GetFloat(40.0f, mEditBox_WheelMass);
		const float WheelFrontFriction = GetFloat(1.0f, mEditBox_WheelFrontFriction);
//		const float WheelRearFriction = GetFloat(1.0f, mEditBox_WheelRearFriction);

		const float SuspStiffness = GetFloat(40000.0f, mEditBox_SuspStiffness);
		const float SuspDamping = GetFloat(1000.0f, mEditBox_SuspDamping);
		const float SuspHeight = GlobalScale*GetFloat(2.0f, mEditBox_SuspHeight);
		const float SuspLimitAngle = DEGTORAD*GetFloat(45.0f, mEditBox_SuspLimitAngle);
		const float WishboneWidth = GlobalScale*GetFloat(0.75f, mEditBox_WishboneWidth);
		const float WishboneHeight = GlobalScale*GetFloat(0.1f, mEditBox_WishboneHeight);
		const float WishboneLength = GlobalScale*GetFloat(0.5f, mEditBox_WishboneLength);
		const float AttachWidth = GlobalScale*GetFloat(0.25f, mEditBox_AttachWidth);
		const float AttachHeight = GlobalScale*GetFloat(0.25f, mEditBox_AttachHeight);
		const float AttachLength = GlobalScale*GetFloat(0.1f, mEditBox_AttachLength);

		const float Friction = GetFloat(1.0f, mEditBox_Friction);
		const float RCAFriction = GetFloat(0.0f, mEditBox_RCAFriction);	// TODO: tune per joint
		const float ChassisFriction = GetFloat(1.0f, mEditBox_ChassisFriction);

		const PINT_MATERIAL_CREATE ChassisMaterial(ChassisFriction, ChassisFriction, 0.0f);
		const PINT_MATERIAL_CREATE UserMaterial(Friction, Friction, 0.0f);
//		const PINT_MATERIAL_CREATE LowFrictionMaterial(0.2f, 0.2f, 0.0f);
		const PINT_MATERIAL_CREATE WheelFrontMaterial(WheelFrontFriction, WheelFrontFriction, 0.0f);
//		const PINT_MATERIAL_CREATE WheelRearMaterial(WheelRearFriction, WheelRearFriction, 0.0f);

		const float AutoSpeed = GetFloat(0.0f, mEditBox_Speed);
		const float MassInertiaCoeff = GetFloat(-1.0f, mEditBox_InertiaCoeff);
		const udword Multiplier = GetInt(1, mEditBox_Multiplier);

		WheelType WT = WHEEL_SPHERE_VISUAL_SPHERE;
		if(mComboBox_WheelType)
			WT = WheelType(mComboBox_WheelType->GetSelectedIndex());

		udword NbWheels = 0;
		{
			const udword Index = mComboBox_NbWheels->GetSelectedIndex();
			if(Index==0)
				NbWheels = 4;
			else if(Index==1)
				NbWheels = 6;
			else if(Index==2)
				NbWheels = 8;
		}

		DriveType_ DT = DRIVE_TYPE_4WD;
		if(mComboBox_DriveType)
			DT = DriveType_(mComboBox_DriveType->GetSelectedIndex());

		/////////////////

		const Point BaseExtents(ChassisLength, ChassisHeight, ChassisWidth);
		const Point BasePos(0.0f, 4.0f, 0.0f);

		const bool UseRCA = mCheckBox_UseRCA ? mCheckBox_UseRCA->IsChecked() : false;
		const PintArticHandle RCA = UseRCA ? pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE(AttachCar)) : null;

		const bool UseAggregates = mCheckBox_UseAggregates ? mCheckBox_UseAggregates->IsChecked() : false;
		const PintAggregateHandle Aggregate = UseAggregates ? pint.CreateAggregate(128, false) : null;	//TODO: revisit params

		udword NbLinksInRCA = 0;

		// Chassis
		PintActorHandle BaseHandle;
		{
			PINT_BOX_CREATE Desc(BaseExtents);
			Desc.mRenderer	= CreateBoxRenderer(BaseExtents);
			Desc.mMaterial	= &ChassisMaterial;

			PINT_OBJECT_CREATE ObjectDesc(&Desc);
			ObjectDesc.mMass				= (AttachCar && !RCA) ? 0.0f : ChassisMass;
			ObjectDesc.mMassForInertia		= ObjectDesc.mMass*MassInertiaCoeff;
			ObjectDesc.mPosition			= BasePos;
			ObjectDesc.mCollisionGroup		= 1;
			ObjectDesc.mCOMLocalOffset.x	= GlobalScale*GetFloat(0.0f, mEditBox_ChassisCOMOffsetX);
			ObjectDesc.mCOMLocalOffset.y	= GlobalScale*GetFloat(0.0f, mEditBox_ChassisCOMOffsetY);
			ObjectDesc.mCOMLocalOffset.z	= GlobalScale*GetFloat(0.0f, mEditBox_ChassisCOMOffsetZ);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mFrictionCoeff = RCAFriction;
				BaseHandle = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
				NbLinksInRCA++;
			}
			else
			{
/*				ObjectDesc.mAddToWorld	= !Aggregate;
				BaseHandle = CreatePintObject(pint, ObjectDesc);
				if(Aggregate)
					pint.AddToAggregate(BaseHandle, Aggregate);*/
				BaseHandle = CreateAggregatedPintObject(pint, ObjectDesc, Aggregate);
			}
			UserData->mChassis = BaseHandle;
		}

		/////////////////

//		const float WheelAttachHeight = AttachHeight;//0.25f;
		const float SideWidth = WishboneWidth;
		const Point BottomSideExtentsMaster(WishboneLength, WishboneHeight, SideWidth);
		const Point TopSideExtentsMaster(WishboneLength, WishboneHeight, SideWidth);
//		const Point BottomSideExtentsMaster(0.5f, WishboneHeight, SideWidth);
//		const Point TopSideExtentsMaster(0.1f, WishboneHeight, SideWidth);
//		const Point WheelAttachExtents(WheelAttachHeight, WheelAttachHeight, 0.1f);
		const Point WheelAttachExtents(AttachWidth, AttachHeight, AttachLength);
//		const Point WheelAttachExtents(0.5f, WheelAttachHeight, 0.1f);

		PtrContainer Joints;

		PintActorHandle H[2];
		Point Pos[2];
		Point Pivots[2];
		const Quat Q(1.0f, 0.0f, 0.0f, 0.0f);

		WheelCreator WC;
		WC.mObjectDesc.mCollisionGroup	= 1;
		WC.mObjectDesc.mAddToWorld		= !Aggregate;
		WC.mObjectDesc.mMass			= WheelMass;
		WC.mObjectDesc.mMassForInertia	= WheelMass*MassInertiaCoeff;

		if(WT==WHEEL_SPHERE_VISUAL_SPHERE || WT==WHEEL_SPHERE_VISUAL_CYLINDER)
		{
			WC.mSphereDesc.mRadius	= WheelRadius;
			if(WT==WHEEL_SPHERE_VISUAL_SPHERE)
				WC.mSphereDesc.mRenderer	= CreateSphereRenderer(WheelRadius);
			else
				WC.mSphereDesc.mRenderer	= CreateConvexRenderer(TotalNbVerts, Verts);
			WC.mSphereDesc.mMaterial		= &WheelFrontMaterial;
			WC.mObjectDesc.SetShape(&WC.mSphereDesc);
		}
		else if(WT==WHEEL_CYLINDER)
		{
			WC.mConvexDesc.mNbVerts		= TotalNbVerts;
			WC.mConvexDesc.mVerts		= Verts;
			WC.mConvexDesc.mRenderer	= CreateConvexRenderer(TotalNbVerts, Verts);
			WC.mConvexDesc.mMaterial	= &WheelFrontMaterial;
//			WC.mConvexDesc.mMaterial	= IsFront ? &UserMaterial : &LowFrictionMaterial;
			WC.mObjectDesc.SetShape(&WC.mConvexDesc);
		}
		else if(WT==WHEEL_TORUS)
		{
			const float SmallRadius = WheelWidth*0.5f;
			const float BigRadius = WheelRadius - SmallRadius;
			WC.GenerateTireCompound(Q, WheelFrontMaterial, BigRadius, SmallRadius, WheelTessR, WheelTessW);
		}

		const float Offsets4[] = {1.0f, -1.0f};
		const float Offsets6[] = {1.0f, 0.0f, -1.0f};
		const float Offsets8[] = {1.0f, 0.333f, -0.333f, -1.0f};
		const float* Offsets;
		if(NbWheels==4)
			Offsets = Offsets4;
		else if(NbWheels==6)
			Offsets = Offsets6;
		else if(NbWheels==8)
			Offsets = Offsets8;

		for(udword j=0;j<NbWheels/2;j++)
		{
			const float SideX = Offsets[j];
			const bool IsFront = j==0;

			for(udword i=0;i<2;i++)
			{
				const float SideZ = i ? -1.0f : 1.0f;

				/////////////////
				Point TopSideExtents = TopSideExtentsMaster;
				Point BottomSideExtents = BottomSideExtentsMaster;
				if(!IsFront)
				{
					// We don't create the second attachment for back wheels so we increase the sides a bit so that front & back wheels align
					BottomSideExtents.z += WheelAttachExtents.z * 2.0f;
					TopSideExtents.z += WheelAttachExtents.z * 2.0f;
				}

				const Point BottomSidePos = BasePos + Point((-BaseExtents.x + BottomSideExtents.x)*SideX, 0.0f, (BaseExtents.z + BottomSideExtents.z)*SideZ);
				PintActorHandle BottomSideHandle;
				{
					PINT_BOX_CREATE Desc(BottomSideExtents);
					Desc.mRenderer	= CreateBoxRenderer(BottomSideExtents);
					Desc.mMaterial	= &ChassisMaterial;

					PINT_OBJECT_CREATE ObjectDesc(&Desc);
					ObjectDesc.mMass			= ElemMass;
					ObjectDesc.mMassForInertia	= ObjectDesc.mMass*MassInertiaCoeff;
					ObjectDesc.mPosition		= BottomSidePos;
					ObjectDesc.mCollisionGroup	= 1;

					const float LimitAngle = SuspLimitAngle;
					const Point Pivot0((-BaseExtents.x+BottomSideExtents.x)*SideX, 0.0f, BaseExtents.z*SideZ);
					const Point Pivot1(0.0f, 0.0f, -BottomSideExtents.z*SideZ);

					if(RCA)
					{
						PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
						ArticulatedDesc.mJointType			= PINT_JOINT_HINGE;
						ArticulatedDesc.mParent				= BaseHandle;
						ArticulatedDesc.mAxisIndex			= X_;
						ArticulatedDesc.mLocalPivot0.mPos	= Pivot0;
						ArticulatedDesc.mLocalPivot1.mPos	= Pivot1;
						ArticulatedDesc.mMinLimit			= -LimitAngle;
						ArticulatedDesc.mMaxLimit			= LimitAngle;
						ArticulatedDesc.mFrictionCoeff		= RCAFriction;
						BottomSideHandle = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
						NbLinksInRCA++;
					}
					else
					{
/*						ObjectDesc.mAddToWorld	= !Aggregate;
						BottomSideHandle = CreatePintObject(pint, ObjectDesc);
						if(Aggregate)
							pint.AddToAggregate(BottomSideHandle, Aggregate);*/
						BottomSideHandle = CreateAggregatedPintObject(pint, ObjectDesc, Aggregate);

						{
							PINT_HINGE_JOINT_CREATE fjc;
							fjc.mObject0		= BaseHandle;
							fjc.mObject1		= BottomSideHandle;
							fjc.mLocalAxis0		= Point(1.0f, 0.0f, 0.0f);
							fjc.mLocalAxis1		= Point(1.0f, 0.0f, 0.0f);
							fjc.mLocalPivot0	= Pivot0;
							fjc.mLocalPivot1	= Pivot1;
							fjc.mLimits.Set(-LimitAngle, LimitAngle);
		//					PintJointHandle j0 = pint.CreateJoint(fjc);
								PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE)(fjc);
								Joints.AddPtr(Copy);
						}
					}
				}

//				if(RCA)
//					continue;

				/////////////////

/*				const float h = WheelAttachHeight*2.0f;
				const Point TopSidePos = BottomSidePos + Point(0.0f, h, 0.0f);
				PintActorHandle TopSideHandle;
				{
					PINT_BOX_CREATE Desc(TopSideExtents);
					Desc.mRenderer	= CreateBoxRenderer(TopSideExtents);

					PINT_OBJECT_CREATE ObjectDesc(&Desc);
					ObjectDesc.mMass			= ElemMass;
					ObjectDesc.mMassForInertia	= ObjectDesc.mMass*MassInertiaCoeff;
					ObjectDesc.mPosition		= TopSidePos;
					ObjectDesc.mCollisionGroup	= 1;

					const Point Pivot0((-BaseExtents.x+BottomSideExtents.x)*SideX, h, BaseExtents.z*SideZ);
					const Point Pivot1(0.0f, 0.0f, -TopSideExtents.z*SideZ);

					if(RCA && 0)	// Break the loop here
					{
						PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
						ArticulatedDesc.mJointType			= PINT_JOINT_HINGE;
						ArticulatedDesc.mParent				= BaseHandle;
						ArticulatedDesc.mAxisIndex			= X_;
						ArticulatedDesc.mLocalPivot0.mPos	= Pivot0;
						ArticulatedDesc.mLocalPivot1.mPos	= Pivot1;
						ArticulatedDesc.mFrictionCoeff		= RCAFriction;
						TopSideHandle = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
					}
					else
					{
						TopSideHandle = CreatePintObject(pint, ObjectDesc);

						{
							PINT_HINGE_JOINT_CREATE fjc;
							fjc.mObject0		= BaseHandle;
							fjc.mObject1		= TopSideHandle;
							fjc.mLocalAxis0		= Point(1.0f, 0.0f, 0.0f);
							fjc.mLocalAxis1		= Point(1.0f, 0.0f, 0.0f);
							fjc.mLocalPivot0	= Pivot0;
							fjc.mLocalPivot1	= Pivot1;

		//					PintJointHandle j0 = pint.CreateJoint(fjc);
								PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE)(fjc);
								Joints.Add(udword(Copy));
						}
					}
				}
*/
				/////////////////

				const Point WheelAttachPos = BottomSidePos + Point(0.0f, WheelAttachExtents.y, (BottomSideExtents.z)*SideZ);
				PintActorHandle AttachHandle;
				{
					PINT_BOX_CREATE Desc(WheelAttachExtents);
					Desc.mRenderer	= CreateBoxRenderer(WheelAttachExtents);
					Desc.mMaterial	= &ChassisMaterial;

					PINT_OBJECT_CREATE ObjectDesc(&Desc);
					ObjectDesc.mMass			= ElemMass;
					ObjectDesc.mMassForInertia	= ObjectDesc.mMass*MassInertiaCoeff;
					ObjectDesc.mPosition		= WheelAttachPos;
					ObjectDesc.mCollisionGroup	= 1;

					const Point Pivot0(0.0f, 0.0f, BottomSideExtents.z*SideZ);
					const Point Pivot1(0.0f, -WheelAttachExtents.y, 0.0f);

					if(RCA)
					{
						PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
						ArticulatedDesc.mJointType			= PINT_JOINT_HINGE;
						ArticulatedDesc.mParent				= BottomSideHandle;
						ArticulatedDesc.mAxisIndex			= X_;
						ArticulatedDesc.mLocalPivot0.mPos	= Pivot0;
						ArticulatedDesc.mLocalPivot1.mPos	= Pivot1;
						ArticulatedDesc.mFrictionCoeff		= RCAFriction;
						AttachHandle = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
						NbLinksInRCA++;
					}
					else
					{
/*						ObjectDesc.mAddToWorld	= !Aggregate;
						AttachHandle = CreatePintObject(pint, ObjectDesc);
						if(Aggregate)
							pint.AddToAggregate(AttachHandle, Aggregate);*/
						AttachHandle = CreateAggregatedPintObject(pint, ObjectDesc, Aggregate);

						{
							PINT_HINGE_JOINT_CREATE fjc;
							fjc.mObject0		= BottomSideHandle;
							fjc.mObject1		= AttachHandle;
							fjc.mLocalAxis0		= Point(1.0f, 0.0f, 0.0f);
							fjc.mLocalAxis1		= Point(1.0f, 0.0f, 0.0f);
							fjc.mLocalPivot0	= Pivot0;
							fjc.mLocalPivot1	= Pivot1;

		//					PintJointHandle j0 = pint.CreateJoint(fjc);
								PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE)(fjc);
								Joints.AddPtr(Copy);
						}
					}
				}

//				if(RCA)
//					continue;

				/////////////////

//				const float h = WheelAttachHeight*2.0f;
				const float h = AttachHeight*2.0f;
				const Point TopSidePos = BottomSidePos + Point(0.0f, h, 0.0f);
				PintActorHandle TopSideHandle;
				{
					PINT_BOX_CREATE Desc(TopSideExtents);
					Desc.mRenderer	= CreateBoxRenderer(TopSideExtents);
					Desc.mMaterial	= &ChassisMaterial;

					PINT_OBJECT_CREATE ObjectDesc(&Desc);
					ObjectDesc.mMass			= ElemMass;
					ObjectDesc.mMassForInertia	= ObjectDesc.mMass*MassInertiaCoeff;
					ObjectDesc.mPosition		= TopSidePos;
					ObjectDesc.mCollisionGroup	= 1;

					const Point Pivot0(0.0f, 0.0f, TopSideExtents.z*SideZ);
					const Point Pivot1(0.0f, WheelAttachExtents.y, 0.0f);

					if(RCA)
					{
						PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
						ArticulatedDesc.mJointType			= PINT_JOINT_HINGE;
						ArticulatedDesc.mParent				= AttachHandle;
						ArticulatedDesc.mAxisIndex			= X_;
						ArticulatedDesc.mLocalPivot0.mPos	= Pivot1;
						ArticulatedDesc.mLocalPivot1.mPos	= Pivot0;
						ArticulatedDesc.mFrictionCoeff		= RCAFriction;
						TopSideHandle = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
						NbLinksInRCA++;
					}
					else
					{
/*						ObjectDesc.mAddToWorld	= !Aggregate;
						TopSideHandle = CreatePintObject(pint, ObjectDesc);
						if(Aggregate)
							pint.AddToAggregate(TopSideHandle, Aggregate);*/
						TopSideHandle = CreateAggregatedPintObject(pint, ObjectDesc, Aggregate);

						{
							PINT_HINGE_JOINT_CREATE fjc;
							fjc.mObject0		= TopSideHandle;
							fjc.mObject1		= AttachHandle;
							fjc.mLocalAxis0		= Point(1.0f, 0.0f, 0.0f);
							fjc.mLocalAxis1		= Point(1.0f, 0.0f, 0.0f);
							fjc.mLocalPivot0	= Pivot0;
							fjc.mLocalPivot1	= Pivot1;

		//					PintJointHandle j0 = pint.CreateJoint(fjc);
								PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE)(fjc);
								Joints.AddPtr(Copy);
						}
					}
				}

//				if(RCA)
//					continue;

				// Regular joint to avoid RCA loops
				if(1)
				{
					const Point Pivot0((-BaseExtents.x+BottomSideExtents.x)*SideX, h, BaseExtents.z*SideZ);
					const Point Pivot1(0.0f, 0.0f, -TopSideExtents.z*SideZ);

					PINT_HINGE_JOINT_CREATE fjc;
					fjc.mObject0		= BaseHandle;
					fjc.mObject1		= TopSideHandle;
					fjc.mLocalAxis0		= Point(1.0f, 0.0f, 0.0f);
					fjc.mLocalAxis1		= Point(1.0f, 0.0f, 0.0f);
					fjc.mLocalPivot0	= Pivot0;
					fjc.mLocalPivot1	= Pivot1;

//					PintJointHandle j0 = pint.CreateJoint(fjc);
						PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE)(fjc);
						Joints.AddPtr(Copy);
				}

//				if(RCA)
//					continue;

				/////////////////

				// Shock absorber
				if(1)
				{
					// TODO: don't make Ext1 depend on BaseExtents.y
					const Point Ext1((-BaseExtents.x+BottomSideExtents.x)*SideX, BaseExtents.y+SuspHeight, BaseExtents.z*SideZ);
//					const Point Ext1((-BaseExtents.x+BottomSideExtents.x)*SideX, SuspHeight, BaseExtents.z*SideZ);
					const Point p1 = BasePos + Ext1;

					const Point Ext0(0.0f, BottomSideExtents.y, BottomSideExtents.z*SideZ);
					const Point p0 = BottomSidePos + Ext0;

					CreatePrismatic(pint, &Joints,	p0, p1,	BottomSideHandle, Ext0,
													BaseHandle, Ext1, ElemMass, SuspStiffness, SuspDamping, Aggregate, RCA, &ChassisMaterial,
													GlobalScale, NbLinksInRCA
													/*,MassInertiaCoeff*/);
				}

				/////////////////

//				const Point WheelAttachPos2 = WheelAttachPos + Point(0.0f, 0.0f, 2.0f*WheelAttachExtents.z*SideZ);
				Point WheelAttachPos2 = WheelAttachPos;
				const Point Extents(IsFront ? GlobalScale*0.5f : 0.0f, 0.0f, 0.0f);
/*				PintActorHandle AttachHandle2;
				{
					PINT_BOX_CREATE Desc(WheelAttachExtents + Extents);
					Desc.mRenderer	= CreateBoxRenderer(WheelAttachExtents + Extents);

					PINT_OBJECT_CREATE ObjectDesc(&Desc);
					ObjectDesc.mMass			= ElemMass;
					ObjectDesc.mMassForInertia	= ObjectDesc.mMass*MassInertiaCoeff;
					ObjectDesc.mPosition		= WheelAttachPos2;
					ObjectDesc.mCollisionGroup	= 1;
					AttachHandle2 = CreatePintObject(pint, ObjectDesc);
				}*/

				PintActorHandle AttachHandle2;
				if(IsFront)
				{
					WheelAttachPos2 += Point(0.0f, 0.0f, 2.0f*WheelAttachExtents.z*SideZ);

					{
						PINT_BOX_CREATE Desc(WheelAttachExtents + Extents);
						Desc.mRenderer	= CreateBoxRenderer(WheelAttachExtents + Extents);
						Desc.mMaterial	= &ChassisMaterial;

						PINT_OBJECT_CREATE ObjectDesc(&Desc);
						ObjectDesc.mMass			= ElemMass;
						ObjectDesc.mMassForInertia	= ObjectDesc.mMass*MassInertiaCoeff;
						ObjectDesc.mPosition		= WheelAttachPos2;
						ObjectDesc.mCollisionGroup	= 1;

						const Point Pivot0(0.0f, 0.0f, WheelAttachExtents.z*SideZ);
						const Point Pivot1(0.0f, 0.0f, -WheelAttachExtents.z*SideZ);
						const float LimitAngle = GetFloat(0.0f, mEditBox_SteerLimitAngle) * DEGTORAD;

						if(RCA)
						{
							PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
							ArticulatedDesc.mJointType			= PINT_JOINT_HINGE;
							ArticulatedDesc.mParent				= AttachHandle;
							ArticulatedDesc.mAxisIndex			= Y_;
							ArticulatedDesc.mLocalPivot0.mPos	= Pivot0;
							ArticulatedDesc.mLocalPivot1.mPos	= Pivot1;
							ArticulatedDesc.mMinLimit			= -LimitAngle;
							ArticulatedDesc.mMaxLimit			= LimitAngle;
							ArticulatedDesc.mFrictionCoeff		= RCAFriction;
							AttachHandle2 = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
							NbLinksInRCA++;
						}
						else
						{
/*							ObjectDesc.mAddToWorld	= !Aggregate;
							AttachHandle2 = CreatePintObject(pint, ObjectDesc);
							if(Aggregate)
								pint.AddToAggregate(AttachHandle2, Aggregate);*/
							AttachHandle2 = CreateAggregatedPintObject(pint, ObjectDesc, Aggregate);

							PINT_HINGE_JOINT_CREATE fjc;
							fjc.mObject0		= AttachHandle;
							fjc.mObject1		= AttachHandle2;
							fjc.mLocalAxis0		= Point(0.0f, 1.0f, 0.0f);
							fjc.mLocalAxis1		= Point(0.0f, 1.0f, 0.0f);
							fjc.mLocalPivot0	= Pivot0;
							fjc.mLocalPivot1	= Pivot1;
							fjc.mLimits.Set(-LimitAngle, LimitAngle);

		//					PintJointHandle j0 = pint.CreateJoint(fjc);
								PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE)(fjc);
								Joints.AddPtr(Copy);
						}
					}

					H[i] = AttachHandle2;
					Pos[i] = WheelAttachPos2;
					Pivots[i] = Point(-(WheelAttachExtents.x + Extents.x), 0.0f, 0.0f);
				}
				else
				{
					AttachHandle2 = AttachHandle;
/*					PINT_FIXED_JOINT_CREATE fjc;
					fjc.mObject0 = AttachHandle;
					fjc.mObject1 = AttachHandle2;
					fjc.mLocalPivot0 = Point(0.0f, 0.0f, WheelAttachExtents.z*SideZ);
					fjc.mLocalPivot1 = Point(0.0f, 0.0f, -WheelAttachExtents.z*SideZ);

//					PintJointHandle j0 = pint.CreateJoint(fjc);
						PINT_FIXED_JOINT_CREATE* Copy = ICE_NEW(PINT_FIXED_JOINT_CREATE)(fjc);
						Joints.Add(udword(Copy));*/
				}

				/////////////////

				if(AttachWheels)
				{
					Vertices mDebugPts;

					PintActorHandle Tire;
					Point Pivot1(0.0f, 0.0f, 0.0f);

/*					WheelCreator WC;
					WC.mObjectDesc.mCollisionGroup	= 1;
					WC.mObjectDesc.mAddToWorld		= !Aggregate;
					WC.mObjectDesc.mMass			= WheelMass;
					WC.mObjectDesc.mMassForInertia	= WheelMass*MassInertiaCoeff;*/
					WC.mObjectDesc.mPosition		= WheelAttachPos2;

/*					if(WT==WHEEL_SPHERE_VISUAL_SPHERE || WT==WHEEL_SPHERE_VISUAL_CYLINDER)
					{
						WC.mSphereDesc.mRadius	= _WheelRadius;
						if(WT==WHEEL_SPHERE_VISUAL_SPHERE)
							WC.mSphereDesc.mRenderer	= CreateSphereRenderer(_WheelRadius);
						else
							WC.mSphereDesc.mRenderer	= CreateConvexRenderer(TotalNbVerts, Verts);
						WC.mSphereDesc.mMaterial		= &UserMaterial;
						WC.mObjectDesc.SetShape(&WC.mSphereDesc);
					}
					else if(WT==WHEEL_CYLINDER)
					{
						WC.mConvexDesc.mNbVerts		= TotalNbVerts;
						WC.mConvexDesc.mVerts		= Verts;
						WC.mConvexDesc.mRenderer	= CreateConvexRenderer(TotalNbVerts, Verts);
						WC.mConvexDesc.mMaterial	= &UserMaterial;
//						WC.mConvexDesc.mMaterial	= IsFront ? &UserMaterial : &LowFrictionMaterial;
						WC.mObjectDesc.SetShape(&WC.mConvexDesc);
					}
					else if(WT==WHEEL_TORUS)
					{
						const float SmallRadius = WheelWidth*0.5f;
						const float BigRadius = WheelRadius - SmallRadius;
						WC.GenerateTireCompound(Q, UserMaterial, BigRadius, SmallRadius, WheelTessR, WheelTessW);
					}*/

					if(RCA)
					{
/*						PINT_SPHERE_CREATE WheelDesc(_WheelRadius);
//						WheelDesc.mRenderer	= CreateSphereRenderer(_WheelRadius);
							WheelDesc.mRenderer	= CreateConvexRenderer(TotalNbVerts, Verts);
*/
/*						PINT_CONVEX_CREATE WheelDesc(TotalNbVerts, Verts);
						WheelDesc.mRenderer	= CreateConvexRenderer(WheelDesc.mNbVerts, WheelDesc.mVerts);
						WheelDesc.mMaterial	= &UserMaterial;

						WC.mObjectDesc.SetShape(&WheelDesc);*/

						PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
						ArticulatedDesc.mJointType			= PINT_JOINT_HINGE;
//						ArticulatedDesc.mParent				= AttachHandle;
						ArticulatedDesc.mParent				= AttachHandle2;
						ArticulatedDesc.mAxisIndex			= Z_;
						ArticulatedDesc.mLocalPivot0.mPos	= Point(0.0f, 0.0f, 0.0f);
						ArticulatedDesc.mLocalPivot1.mPos	= Pivot1;
						ArticulatedDesc.mFrictionCoeff		= RCAFriction;

						if(AutoSpeed!=0.0f)
						{
							if(DT == DRIVE_TYPE_4WD || (DT == DRIVE_TYPE_RWD && !IsFront) || (DT == DRIVE_TYPE_FWD && IsFront))
							{
								ArticulatedDesc.mUseMotor			= true;
								//TODO: revisit these
								ArticulatedDesc.mMotor.mStiffness	= 0.0f;
								ArticulatedDesc.mMotor.mDamping		= 1000.0f;
								ArticulatedDesc.mTargetVel			= AutoSpeed;
							}
						}

						Tire = pint.CreateRCArticulatedObject(WC.mObjectDesc, ArticulatedDesc, RCA);
						NbLinksInRCA++;

//						UserData->mRCAWheel[j*2+i].Init(Tire, IsFront);
					}
					else
					{
/*						if(WT==WHEEL_SPHERE_VISUAL_SPHERE || WT==WHEEL_SPHERE_VISUAL_CYLINDER)
						{
							PINT_SPHERE_CREATE WheelDesc(_WheelRadius);
							if(WT==WHEEL_SPHERE_VISUAL_SPHERE)
								WheelDesc.mRenderer	= CreateSphereRenderer(_WheelRadius);
							else
								WheelDesc.mRenderer	= CreateConvexRenderer(TotalNbVerts, Verts);
							WheelDesc.mMaterial		= &UserMaterial;
							WC.mObjectDesc.SetShape(&WheelDesc);
							Tire = CreatePintObject(pint, WC.mObjectDesc);
							if(Aggregate)
								pint.AddToAggregate(Tire, Aggregate);
						}
						else if(WT==WHEEL_CYLINDER)
						{
							PINT_CONVEX_CREATE WheelDesc(TotalNbVerts, Verts);
							WheelDesc.mRenderer		= CreateConvexRenderer(WheelDesc.mNbVerts, WheelDesc.mVerts);
							WheelDesc.mMaterial		= &UserMaterial;
							WC.mObjectDesc.SetShape(&WheelDesc);
							Tire = CreatePintObject(pint, WC.mObjectDesc);
							if(Aggregate)
								pint.AddToAggregate(Tire, Aggregate);
						}
						else if(WT==WHEEL_TORUS)
						{
//							Tire = GenerateTireCompound(pint, WheelAttachPos2, Q, WheelMass, 1, UserMaterial, 1.5f, MassInertiaCoeff);

							WC.GenerateTireCompound(Q, UserMaterial, 1.5f);
							Tire = CreatePintObject(pint, WC.mObjectDesc);
							if(Aggregate)
								pint.AddToAggregate(Tire, Aggregate);
						}*/
						if(WT>=WHEEL_SPHERE_VISUAL_SPHERE && WT<=WHEEL_TORUS)
						{
							Tire = CreatePintObject(pint, WC.mObjectDesc);
							if(Aggregate)
								pint.AddToAggregate(Tire, Aggregate);
						}
						else if(WT==WHEEL_PRISMATIC)
						{
					const float PrismaticWheelRadius = GlobalScale*0.5f;
	//				const Point TirePos = WheelAttachPos + Point(0.0f, 0.0f, PrismaticWheelRadius*2.0f*SideZ);
					const Point TirePos = WheelAttachPos2 + Point(0.0f, 0.0f, PrismaticWheelRadius*2.0f*SideZ);

							//WheelMass, UserMaterial, remove the sphere and the extra z offset
							Tire = GenerateTire_Prismatic(pint, TirePos, Q, 1.0f, 1, mDebugPts, PrismaticWheelRadius);
							Pivot1 = Point(0.0f, 0.0f, -PrismaticWheelRadius*2.0f*SideZ);
						}

						{
							PINT_HINGE_JOINT_CREATE fjc;
		//					fjc.mObject0 = AttachHandle;
							fjc.mObject0 = AttachHandle2;
							fjc.mObject1 = Tire;
							fjc.mLocalAxis0 = Point(0.0f, 0.0f, 1.0f);
							fjc.mLocalAxis1 = Point(0.0f, 0.0f, 1.0f);
							fjc.mLocalPivot0 = Point(0.0f, 0.0f, 0.0f);
	//						fjc.mLocalPivot1 = Point(0.0f, 0.0f, -WheelRadius*2.0f*SideZ);
							fjc.mLocalPivot1 = Pivot1;

		//					if(1 && !IsFront)
							if(AutoSpeed!=0.0f)
							{
								fjc.mUseMotor	= true;
								fjc.mDriveVelocity	= AutoSpeed;
							}
	//						PintJointHandle j0 = pint.CreateJoint(fjc);
								PINT_HINGE_JOINT_CREATE* Copy = ICE_NEW(PINT_HINGE_JOINT_CREATE)(fjc);
								Joints.AddPtr(Copy);

							//UserData->mWheelJoints[]
						}
					}

					if(Tire)
						UserData->AddWheel(Tire, IsFront);

					if(Tire && TweakWheelsInertia)
					{
						Pint_Actor* API = pint.GetActorAPI();
						if(API)
						{
							Point LocalInertia;
							bool status = API->GetLocalInertia(Tire, LocalInertia);
							ASSERT(status);
							const float MaxValue = LocalInertia.Max();
							API->SetLocalInertia(Tire, Point(MaxValue, MaxValue, MaxValue));
//							API->SetLocalInertia(Tire, LocalInertia*0.1f);
//							API->SetLocalInertia(Tire, Point(LocalInertia.x*0.1f, LocalInertia.y, LocalInertia.z));
						}
					}
				}
			}
		}

		// Steering mechanism
		if(AttachSteering)
		{
			//for(udword j=0;j<2;j++)
			udword j=1;
			{
				const float SideX = j ? -1.0f : 1.0f;

				if(1)
				{
					const Point P0 = Pos[0] + Pivots[0]*SideX;
					const Point P1 = Pos[1] + Pivots[1]*SideX;
					const float Length = P0.Distance(P1);
					Point N = P1-P0;
					N.Normalize();
					const Point P0b = P0 + N*Length*0.25f;
					const Point P1b = P1 - N*Length*0.25f;

					const Point RackExtents(GlobalScale*0.05f, GlobalScale*0.05f, 0.5f*P0b.Distance(P1b));
					PintActorHandle SteeringRackHandle;
					{
						PINT_BOX_CREATE Desc(RackExtents);
						Desc.mRenderer	= CreateBoxRenderer(RackExtents);
						Desc.mMaterial	= &ChassisMaterial;

						PINT_OBJECT_CREATE ObjectDesc(&Desc);
						ObjectDesc.mMass			= ElemMass;
						ObjectDesc.mPosition		= (P0 + P1)*0.5f;
						ObjectDesc.mCollisionGroup	= 1;
/*						ObjectDesc.mAddToWorld		= !Aggregate;
						SteeringRackHandle = CreatePintObject(pint, ObjectDesc);
						if(Aggregate)
							pint.AddToAggregate(SteeringRackHandle, Aggregate);*/
						SteeringRackHandle = CreateAggregatedPintObject(pint, ObjectDesc, Aggregate);

						// TODO: add this to articulation and use a motor there
						{
	/*						PINT_PRISMATIC_JOINT_CREATE pjc;
							pjc.mObject0 = BaseHandle;
							pjc.mObject1 = SteeringRackHandle;
							pjc.mLocalPivot0 = Point((P0.x + P1.x)*0.5f - BasePos.x, BaseExtents.y, 0.0f);
							pjc.mLocalPivot1 = Point(0.0f, -RodExtents.y, 0.0f);
							pjc.mLocalAxis0 = Point(0.0f, 0.0f, 1.0f);
							pjc.mLocalAxis1 = Point(0.0f, 0.0f, 1.0f);
							PintJointHandle j0 = pint.CreateJoint(pjc);*/

				PINT_D6_JOINT_CREATE Desc;
				Desc.mObject0				= BaseHandle;
				Desc.mObject1				= SteeringRackHandle;
				Desc.mLocalPivot0.mPos		= Point((P0.x + P1.x)*0.5f - BasePos.x, BaseExtents.y, 0.0f);
				Desc.mLocalPivot1.mPos		= Point(0.0f, -RackExtents.y, 0.0f);
	//			Desc.mLinearLimits.mMin.x	= -Length*0.5f + CursorExtents.x;
	//			Desc.mLinearLimits.mMax.x	= Length*0.5f - CursorExtents.x;
				Desc.mLinearLimits.mMin.z	= FLT_MAX;
				Desc.mLinearLimits.mMax.z	= -FLT_MAX;

							if(1)
							{
								const float SteerStiffness = GetFloat(0.0f, mEditBox_SteerStiffness);
								const float SteerDamping = GetFloat(0.0f, mEditBox_SteerDamping);

								Desc.mMotorFlags		= PINT_D6_MOTOR_DRIVE_Z;
								Desc.mMotorStiffness	= SteerStiffness;
								Desc.mMotorDamping		= SteerDamping;
							}

							PintJointHandle j0 = pint.CreateJoint(Desc);
							UserData->mSteeringJoint = j0;
						}
					}

					const Point LeftLinkExtents(GlobalScale*0.05f, GlobalScale*0.05f, 0.5f*P0.Distance(P0b));
					PintActorHandle LeftLinkHandle;
					{
						PINT_BOX_CREATE Desc(LeftLinkExtents);
						Desc.mRenderer	= CreateBoxRenderer(LeftLinkExtents);
						Desc.mMaterial	= &ChassisMaterial;

						PINT_OBJECT_CREATE ObjectDesc(&Desc);
						ObjectDesc.mMass			= ElemMass;
						ObjectDesc.mPosition		= (P0 + P0b)*0.5f;
						ObjectDesc.mCollisionGroup	= 1;

						const Point Pivot0 = Pivots[0]*SideX;
						const Point Pivot1 = Point(0.0f, 0.0f, LeftLinkExtents.z);

						if(RCA)
						{
							PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
							ArticulatedDesc.mJointType			= PINT_JOINT_SPHERICAL;
							ArticulatedDesc.mParent				= H[0];
							ArticulatedDesc.mLocalPivot0.mPos	= Pivot0;
							ArticulatedDesc.mLocalPivot1.mPos	= Pivot1;
							ArticulatedDesc.mFrictionCoeff		= RCAFriction;
							LeftLinkHandle = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
							NbLinksInRCA++;
						}
						else
						{
/*							ObjectDesc.mAddToWorld	= !Aggregate;
							LeftLinkHandle = CreatePintObject(pint, ObjectDesc);
							if(Aggregate)
								pint.AddToAggregate(LeftLinkHandle, Aggregate);*/
							LeftLinkHandle = CreateAggregatedPintObject(pint, ObjectDesc, Aggregate);

							{
								PINT_SPHERICAL_JOINT_CREATE fjc;
								fjc.mObject0 = H[0];
								fjc.mObject1 = LeftLinkHandle;
								fjc.mLocalPivot0.mPos = Pivot0;
								fjc.mLocalPivot1.mPos = Pivot1;
		//						PintJointHandle j0 = pint.CreateJoint(fjc);
									PINT_SPHERICAL_JOINT_CREATE* Copy = ICE_NEW(PINT_SPHERICAL_JOINT_CREATE)(fjc);
									Joints.AddPtr(Copy);
							}
						}
					}

					const Point RightLinkExtents(GlobalScale*0.05f, GlobalScale*0.05f, 0.5f*P1.Distance(P1b));
					PintActorHandle RightLinkHandle;
					{
						PINT_BOX_CREATE Desc(RightLinkExtents);
						Desc.mRenderer	= CreateBoxRenderer(RightLinkExtents);
						Desc.mMaterial	= &ChassisMaterial;

						PINT_OBJECT_CREATE ObjectDesc(&Desc);
						ObjectDesc.mMass			= ElemMass;
						ObjectDesc.mPosition		= (P1 + P1b)*0.5f;
						ObjectDesc.mCollisionGroup	= 1;

						const Point Pivot0 = Pivots[1]*SideX;
						const Point Pivot1 = Point(0.0f, 0.0f, -RightLinkExtents.z);

						if(RCA)
						{
							PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
							ArticulatedDesc.mJointType			= PINT_JOINT_SPHERICAL;
							ArticulatedDesc.mParent				= H[1];
							ArticulatedDesc.mLocalPivot0.mPos	= Pivot0;
							ArticulatedDesc.mLocalPivot1.mPos	= Pivot1;
							ArticulatedDesc.mFrictionCoeff		= RCAFriction;
							RightLinkHandle = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
							NbLinksInRCA++;
						}
						else
						{
/*							ObjectDesc.mAddToWorld	= !Aggregate;
							RightLinkHandle = CreatePintObject(pint, ObjectDesc);
							if(Aggregate)
								pint.AddToAggregate(RightLinkHandle, Aggregate);*/
							RightLinkHandle = CreateAggregatedPintObject(pint, ObjectDesc, Aggregate);

							{
								PINT_SPHERICAL_JOINT_CREATE fjc;
								fjc.mObject0 = H[1];
								fjc.mObject1 = RightLinkHandle;
								fjc.mLocalPivot0.mPos = Pivot0;
								fjc.mLocalPivot1.mPos = Pivot1;
		//						PintJointHandle j0 = pint.CreateJoint(fjc);
									PINT_SPHERICAL_JOINT_CREATE* Copy = ICE_NEW(PINT_SPHERICAL_JOINT_CREATE)(fjc);
									Joints.AddPtr(Copy);
							}
						}
					}

					{
						PINT_SPHERICAL_JOINT_CREATE fjc;
						fjc.mObject0 = LeftLinkHandle;
						fjc.mObject1 = SteeringRackHandle;
						fjc.mLocalPivot0.mPos = Point(0.0f, 0.0f, -LeftLinkExtents.z);
						fjc.mLocalPivot1.mPos = Point(0.0f, 0.0f, RackExtents.z);
//						PintJointHandle j0 = pint.CreateJoint(fjc);
							PINT_SPHERICAL_JOINT_CREATE* Copy = ICE_NEW(PINT_SPHERICAL_JOINT_CREATE)(fjc);
							Joints.AddPtr(Copy);
					}

					{
						PINT_SPHERICAL_JOINT_CREATE fjc;
						fjc.mObject0 = RightLinkHandle;
						fjc.mObject1 = SteeringRackHandle;
						fjc.mLocalPivot0.mPos = Point(0.0f, 0.0f, RightLinkExtents.z);
						fjc.mLocalPivot1.mPos = Point(0.0f, 0.0f, -RackExtents.z);
//						PintJointHandle j0 = pint.CreateJoint(fjc);
							PINT_SPHERICAL_JOINT_CREATE* Copy = ICE_NEW(PINT_SPHERICAL_JOINT_CREATE)(fjc);
							Joints.AddPtr(Copy);
					}
				}
				// TODO: use this simpler setup with motors on each wheel attachment to reach a target steering angle?
/*				else
				{
					const Point RodExtents(0.05f, 0.05f, 0.5f*Pos[0].Distance(Pos[1]));
					PintActorHandle ConnectingRodHandle;
					{
						const Point P0 = Pos[0] + Pivots[0]*SideX;
						const Point P1 = Pos[1] + Pivots[1]*SideX;

						PINT_BOX_CREATE Desc(RodExtents);
						Desc.mRenderer	= CreateBoxRenderer(RodExtents);

						PINT_OBJECT_CREATE ObjectDesc(&Desc);
						ObjectDesc.mMass			= ElemMass;
						ObjectDesc.mPosition		= (P0 + P1)*0.5f;
						ObjectDesc.mCollisionGroup	= 1;
						ConnectingRodHandle = CreatePintObject(pint, ObjectDesc);
					}

					for(udword i=0;i<2;i++)
					{
						const float SideZ = i ? -1.0f : 1.0f;
						PINT_SPHERICAL_JOINT_CREATE fjc;
						fjc.mObject0 = H[i];
						fjc.mObject1 = ConnectingRodHandle;
						fjc.mLocalPivot0 = Pivots[i]*SideX;
						fjc.mLocalPivot1 = Point(0.0f, 0.0f, RodExtents.z*SideZ);
						PintJointHandle j0 = pint.CreateJoint(fjc);
					}
				}*/
			}
		}

		const udword NbJoints = Joints.GetNbEntries();
		if(NbJoints)
		{
			for(udword j=0;j<Multiplier;j++)
			{
				for(udword i=0;i<NbJoints;i++)
				{
					const PINT_JOINT_CREATE* Create = (const PINT_JOINT_CREATE*)Joints.GetEntry(i);
					PintJointHandle j0 = pint.CreateJoint(*Create);
				}
			}

			for(udword i=0;i<NbJoints;i++)
			{
				const PINT_JOINT_CREATE* Create = (const PINT_JOINT_CREATE*)Joints.GetEntry(i);
				DELETESINGLE(Create);
			}
		}

		if(Aggregate)
		{
			if(RCA)
				pint.AddRCArticulationToAggregate(RCA, Aggregate);

			pint.AddAggregateToScene(Aggregate);
		}
		else if(RCA)
			pint.AddRCArticulationToScene(RCA);

		if(RCA)
			printf("Nb RCA links: %d\n", NbLinksInRCA);

		if(AttachCar)
			return true;

		return InitLevel(&pint, &caps, &UserMaterial);
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

	virtual	udword		Update(Pint& pint, float dt)
	{
		const MyData* UserData = (const MyData*)pint.mUserData;

		const bool DriveVehicle = mCheckBox_DriveVehicle ? mCheckBox_DriveVehicle->IsChecked() : false;
		mControlCamera = DriveVehicle;

		const udword EngineMode = mComboBox_EngineMode ? mComboBox_EngineMode->GetSelectedIndex() : 0;

//		if(DriveVehicle)	// We want to enable slider control even when not driving with the keys
		{
			const PintJointHandle SteeringJoint = UserData->mSteeringJoint;
			if(SteeringJoint)
			{
				PR Pose;
				Pose.Identity();

				const float GlobalScale	= GetFloat(1.0f, mEditBox_GlobalScale);

				// Slider control & also reposition the wheels to zero automatically
				if(mSteerSlider)
				{
					float V = mSteerSlider->GetValue();
		//			printf("%f\n", V);
		//			Pose.mPos.z = (V-0.5f)*2.0f;
					Pose.mPos.z = (V-0.5f)*GlobalScale;
					pint.SetDrivePosition(SteeringJoint, Pose);
				}

				// Keyboard control
				if(DriveVehicle)
				{
					if(mInput.mKeyboard_Right)
					{
						Pose.mPos.z = 0.5f*GlobalScale;
		//				Pose.mPos.z = 0.1f;
						pint.SetDrivePosition(SteeringJoint, Pose);
					}
					if(mInput.mKeyboard_Left)
					{
						Pose.mPos.z = -0.5f*GlobalScale;
		//				Pose.mPos.z = -0.1f;
						pint.SetDrivePosition(SteeringJoint, Pose);
					}
				}
			}
		}

		if(DriveVehicle)
		{
			DriveType_ DT = DRIVE_TYPE_4WD;
			if(mComboBox_DriveType)
				DT = DriveType_(mComboBox_DriveType->GetSelectedIndex());

			if(EngineMode==1)
			{
	//			bool Reverse = false;
	//			const float MaxReverseVel = 10.0f;
				const float MaxVel = GetFloat(0.0f, mEditBox_Speed);
				const float Coeff = 0.2f;
				bool Handbrake = false;
				if(mInput.mKeyboard_Brake)
				{
					mCurrentVel -= Coeff*2.0f;
					mCurrentVel = 0.0f;
					if(mCurrentVel<0.0f)
						mCurrentVel=0.0f;
	//				Handbrake = true;
				}
				else if(mInput.mKeyboard_Accelerate)
				{
					mCurrentVel += Coeff;
					if(mCurrentVel>MaxVel)
						mCurrentVel=MaxVel;
				}
				else
				{
					mCurrentVel -= Coeff;
					if(mCurrentVel<0.0f)
						mCurrentVel=0.0f;
				}
	//			printf("%f\n", mCurrentVel);

				const udword NbWheels = UserData->GetNbWheels();
				const MyData::Wheel*	Wheels = UserData->GetWheels();
				for(udword i=0;i<NbWheels;i++)
				{
					if(!Wheels[i].GetHandle())
						continue;

					const bool IsFront = Wheels[i].IsFront();
					const float Coeff = mCurrentVel;
					if(!IsFront && Handbrake)
					{
						pint.SetRCADriveVelocity(Wheels[i].GetHandle(), 0.0f);
//						pint.SetAngularVelocity(UserData->mRCAWheel[j*2+i], Point(0.0f, 0.0f, 0.0f));
					}
					else if(DT == DRIVE_TYPE_4WD || (DT == DRIVE_TYPE_RWD && !IsFront) || (DT == DRIVE_TYPE_FWD && IsFront))
					{
						pint.SetRCADriveVelocity(Wheels[i].GetHandle(), mCurrentVel);
//						pint.SetAngularVelocity(UserData->mRCAWheel[j*2+i], Point(mCurrentVel, 0.0f, 0.0f));
					}
				}

			}

			if(EngineMode==0)
			{
				float Coeff = GetFloat(0.0f, mEditBox_EngineTorque);
				bool ApplyTorque = false;
				if(mInput.mKeyboard_Brake)
				{
					ApplyTorque = true;
					Coeff = -Coeff;
				}
				else if(mInput.mKeyboard_Accelerate)
				{
					ApplyTorque = true;
				}

				if(ApplyTorque)
				{
					const udword NbWheels = UserData->GetNbWheels();
					const MyData::Wheel*	Wheels = UserData->GetWheels();
					for(udword i=0;i<NbWheels;i++)
					{
						if(!Wheels[i].GetHandle())
							continue;

						const bool IsFront = Wheels[i].IsFront();
						if(DT == DRIVE_TYPE_4WD || (DT == DRIVE_TYPE_RWD && !IsFront) || (DT == DRIVE_TYPE_FWD && IsFront))
						{
							pint.AddLocalTorque(Wheels[i].GetHandle(), Point(0.0f, 0.0f, Coeff));
						}
					}
				}
			}

			UpdateCamera(pint, UserData->mChassis);
		}

		return VehicleInput::Update(pint, dt);
	}

	virtual	void	CommonDebugRender(PintRender& render)
	{
		const udword NbPts = mDebugPts.GetNbVertices();
		const Point* V = mDebugPts.GetVertices();
		render.DrawPoints(NbPts, V, sizeof(Point));
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const MyData* UserData = (const MyData*)pint.mUserData;
		if(UserData && UserData->mChassis)
		{
			const Point LinVel = pint.GetLinearVelocity(UserData->mChassis);
//			renderer.print(0.0f, y, text_scale, _F("Lin vel: %.3f | %.3f | %.3f\n", LinVel.x, LinVel.y, LinVel.z));
			const float Speed = LinVel.Magnitude() * 3600.0f / 1000.0f;
			renderer.print(0.0f, y, text_scale, _F("Speed: %d\n", int(Speed)));
			y -= text_scale;

#ifdef TO_REVISIT
			if(0)
			{
				for(udword i=0;i<4;i++)
				{
					if(UserData->mRCAWheel[i].GetHandle())
					{
						const Point AngVel = pint.GetLinearVelocity(UserData->mRCAWheel[i].GetHandle());
						renderer.print(0.0f, y, text_scale, _F("Ang vel: %.3f\n", AngVel.Magnitude()));
	//					renderer.print(0.0f, y, text_scale, _F("Ang vel: %.3f | %.3f | %.3f\n", AngVel.x, AngVel.y, AngVel.z));
						y -= text_scale;
					}
				}
			}
#endif
		}
		return y;
	}

	virtual	float			GetRenderData(Point& center)	const
	{
		center = GetCameraPos();
		return 200.0f;
	}

END_TEST(MultiBodyVehicle)
