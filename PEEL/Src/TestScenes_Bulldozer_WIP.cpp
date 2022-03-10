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
#include "MyConvex.h"
#include "Camera.h"

///////////////////////////////////////////////////////////////////////////////

void CreateConvexHull2D(Vertices& verts);

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
		ChassisData() : mChassis(null)
		{
			for(udword i=0;i<2;i++)
				mFrontAxleObject[i] = null;
		}
		PintActorHandle	mChassis;
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

#define START_VEHICLE_TEST(name, category, desc)										\
	class name : public VehicleBase														\
	{																					\
		public:																			\
								name()						{						}	\
		virtual					~name()						{						}	\
		virtual	const char*		GetName()			const	{ return #name;			}	\
		virtual	const char*		GetDescription()	const	{ return desc;			}	\
		virtual	TestCategory	GetCategory()		const	{ return category;		}

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
	Desc.mLimits.Set(min_limit, max_limit);
	return pint.CreateJoint(Desc);
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
	ConvexCreate.SetNext(&ConvexCreate2);

	PINT_CONVEX_CREATE ConvexCreate3 = ConvexCreate2;
	ConvexCreate3.mLocalPos	= Point(0.0f, 0.0f, - inside_cylinder.mHalfHeight - outside_cylinder.mHalfHeight);
	ConvexCreate2.SetNext(&ConvexCreate3);

	PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
	ObjectDesc.mMass		= gear_mass;
	ObjectDesc.mPosition	= pos;
	return CreatePintObject(pint, ObjectDesc);
}

static PINT_SHAPE_CREATE* CreateGearHolderShapes(PINT_SHAPE_CREATE*& StartShape, PINT_SHAPE_CREATE*& CurrentShape,
									const Point& pos, const Point& gear_center, udword nb_gears, const Point* gear_offsets,
									const CylinderMesh& inside_cylinder, const PINT_MATERIAL_CREATE* material, const Point& local_offset)
{
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
				CurrentShape->SetNext(BoxDesc);

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
	// We tweak the incoming cylinders so that:
	// - they have a large amount of vertices. That way we get a smoother curve.
	// - they take the height of the track into account. That way the system doesn't start jittering when we increase the number of solver iterations.
	const float TrackHeight = 0.1f;

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
			PINT_OBJECT_CREATE ObjectDesc(StartShape);
			ObjectDesc.mMass		= 10.0f;
			ObjectDesc.mPosition	= GearCenter;

			GearHolder = CreatePintObject(pint, ObjectDesc);

			DeletePintObjectShapes(ObjectDesc);
		}
	}
	objects.mChassis = GearHolder;

	Vertices V;
	objects.mNbGears = nb_gears;
	for(udword i=0;i<nb_gears;i++)
	{
		const Point GearPos = pos + gear_offsets[i];
		PintActorHandle CurrentGear = CreateGear(pint, material, GearPos, gear_mass, inside_cylinder, inside_cylinder_renderer, outside_cylinder, outside_cylinder_renderer);
		objects.mGears[i] = CurrentGear;

		PintJointHandle h = CreateHinge(pint, CurrentGear, GearHolder,
									Point(0.0f, 0.0f, 0.0f), gear_holder_offset ? (*gear_holder_offset) + GearPos - GearCenter : GearPos - GearCenter,
								   Point(0.0f, 0.0f, 1.0f), Point(0.0f, 0.0f, 1.0f));
		(void)h;

		const udword NbPtsInCircle = TweakedOutsideCylinder.mNbVerts/2;
		for(udword j=0;j<NbPtsInCircle;j++)
		{
			Point p = TweakedOutsideCylinder.mVerts[j];
			p.z = 0.0f;	//#### hardcoded
			p += GearPos;
			V.AddVertex(p);
		}
	}
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
		BoxDesc.SetNext(&BoxDesc2);

		// Upper box that will contact with the ground
		const float BumpSize = 0.05f;
		PINT_BOX_CREATE BoxDesc3(BumpSize, BumpSize, Extents.z*0.9f);
		BoxDesc3.mRenderer	= CreateBoxRenderer(BoxDesc3.mExtents);
		BoxDesc3.mMaterial	= material;
		BoxDesc3.mLocalPos	= Point(0.0f, Extents.y, 0.0f);
		Matrix3x3 Rot45;
		Rot45.RotZ(PI/4.0f);
		BoxDesc3.mLocalRot	= Rot45;
		BoxDesc2.SetNext(&BoxDesc3);

	PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
	ObjectDesc.mMass	= 1.0f;

	PintActorHandle Objects[128];

	PintActorHandle Parent = null;
	for(udword i=0;i<NbLinks;i++)
	{
		const udword ii = (i)%NbLinks;

		const float Coeff = float(ii)/float(NbLinks);

		Point pts[2];
		C.GetPoint(ObjectDesc.mPosition, Coeff, pts);

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
					Desc.mObject0			= Objects[ii];
					Desc.mObject1			= Objects[jj];
					Desc.mLimits.mMaxValue	= Extents.x*2.0f*2.0f;
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
			}
		}
	}
	return GearHolder;
}

static const char* gDesc_BulldozerTest = "Bulldozer test. This uses regular joints (no articulation). This is an experimental, work-in-progress test. It has not been properly tuned or \
optimized, and it has not been tested in engines other than PhysX 3.4. Use the arrow keys to control the vehicle. Use the PageUp and PageDown keys to switch between different \
camera views.";

static PintActorHandle CreateBoxObject(Pint& pint, const Point& extents, const Point& pos, float mass, const Quat* rotation=null)
{
	PINT_BOX_CREATE BoxDesc(extents);
	BoxDesc.mRenderer	= CreateBoxRenderer(extents);

	PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
	ObjectDesc.mPosition	= pos;
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
		Desc.mLimits.Set(0.0f, TopPrismaticExtents.y*2.0f);
		if(1)
		{
			Desc.mLimits.Set(TopPrismaticExtents.y*1.9f, TopPrismaticExtents.y*2.0f);
			Desc.mSpring.mStiffness	= 400.0f;
			Desc.mSpring.mDamping	= 10.0f;
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
}

START_VEHICLE_TEST(BulldozerTest, CATEGORY_WIP, gDesc_BulldozerTest)

	CylinderMesh mInsideCylinder;
	CylinderMesh mOutsideCylinder;

	virtual	float	GetRenderData(Point& center)	const
	{
		center = GetCameraPos();
		return 1000.0f;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(8.54f, 4.57f, 7.70f), Point(-0.66f, -0.28f, -0.69f));
		SetDefEnv(desc, true);
	}

	virtual	bool	CommonSetup()
	{
		mAcceleration			= 200.0f;
		mMaxAngularVelocity		= 10.0f;
		mSteeringForce			= 10.0f*100.0f;
		mCamera.mUpOffset		= 10.0f;
		mCamera.mDistToTarget	= 10.0f;
		mClampAngularVelocity	= true;
		mControlCamera			= true;

		const float HalfHeight = 0.5f;
		const float Radius = 0.6f;
		const udword NbCirclePts = 16;
		mInsideCylinder.Generate(NbCirclePts, Radius, HalfHeight);
		RegisterRenderer(CreateConvexRenderer(mInsideCylinder.mNbVerts, mInsideCylinder.mVerts));

		const float HalfHeight2 = 0.15f;
		const float Radius2 = 1.0f;
		mOutsideCylinder.Generate(NbCirclePts, Radius2, HalfHeight2);
		RegisterRenderer(CreateConvexRenderer(mOutsideCylinder.mNbVerts, mOutsideCylinder.mVerts));

		return VehicleBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		mInsideCylinder.Reset();
		mOutsideCylinder.Reset();
		VehicleBase::CommonRelease();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		CreateMeshesFromRegisteredSurfaces(pint, caps, &mHighFrictionMaterial);

		ChassisData* UserData = ICE_NEW(ChassisData);
		pint.mUserData = UserData;

		const float TrackWidth = mInsideCylinder.mHalfHeight + mOutsideCylinder.mHalfHeight*2.0f;
		const float TrackGap = 0.1f;

		const Point ChassisExtents(3.0f, 2.0f, 1.0f);

		const float GroundClearance = 0.5f;
		const Point ChassisPos(0.0f, ChassisExtents.y+GroundClearance, 0.0f);
		const float ChassisMass = 100.0f;

		const float ChassisGap = (TrackWidth+TrackGap)*2.0f;

		const float SideDistToGround = 0.5f*2.0f;
		const Point SideExtents(4.0f, 0.2f, 0.2f);
		const Point LeftSidePos(SideExtents.x-ChassisExtents.x, SideExtents.y+SideDistToGround, -ChassisExtents.z-ChassisGap-SideExtents.z);
		const Point RightSidePos(SideExtents.x-ChassisExtents.x, SideExtents.y+SideDistToGround, ChassisExtents.z+ChassisGap+SideExtents.z);
		const float SideMass = 1.0f;

		PINT_BOX_CREATE ChassisBoxDesc(ChassisExtents);
		ChassisBoxDesc.mRenderer	= CreateBoxRenderer(ChassisExtents);
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
					}
				}

				PintActorHandle GearHolder = null;
				if(StartShape)
				{
					if(1)
					{
						ChassisBoxDesc.SetNext(StartShape);

						PINT_OBJECT_CREATE ChassisObjectDesc(&ChassisBoxDesc);
						ChassisObjectDesc.mPosition	= ChassisPos;
						ChassisObjectDesc.mMass		= ChassisMass;
						GearHolder = CreatePintObject(pint, ChassisObjectDesc);
						ChassisObject = GearHolder;
					}
					else
					{
						PINT_OBJECT_CREATE ObjectDesc(StartShape);
						ObjectDesc.mMass		= 10.0f;
						ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
						GearHolder = CreatePintObject(pint, ObjectDesc);
					}

					while(StartShape)
					{
						PINT_SHAPE_CREATE* NextShape = const_cast<PINT_SHAPE_CREATE*>(StartShape->_GetNext());
						DELETESINGLE(StartShape);
						StartShape = NextShape;
					}
					// ######
					// DeletePintObjectShapes(ObjectDesc);
				}

				if(1)
				{
					UserData->mChassis = GearHolder;
					UserData->mFront[0].mParent = GearHolder;
					UserData->mFront[1].mParent = GearHolder;
					UserData->mRear[0].mParent = GearHolder;
					UserData->mRear[1].mParent = GearHolder;

					for(udword i=0;i<2;i++)
					{
						const float Coeff = i ? -1.0f : 1.0f;
						/*const*/ Point Pos(0.0f, 1.5f, Coeff*(ChassisExtents.z+TrackGap+TrackWidth));

							Point GearCenter(0.0f, 0.0f, 0.0f);
							for(udword j=0;j<NbGears;j++)
							{
								const Point GearPos = Pos + GearOffsets[j];
								GearCenter += GearPos;
							}
							GearCenter /= float(NbGears);
							GearCenter -= ChassisPos;

							CaterpillarTrackObjects CTO;
							CreateCaterpillarTrackWIP(pint, CTO, &mHighFrictionMaterial, Pos,
													mInsideCylinder, GetRegisteredRenderers()[0], mOutsideCylinder, GetRegisteredRenderers()[1],
													NbGears, GearOffsets, GearMass, GearHolder, &GearCenter);

							{
								UserData->mFront[i].mWheel = CTO.mGears[1];
								UserData->mRear[i].mWheel = CTO.mGears[2];
							}
					}
				}
			}
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
			const Point BladePos(	LeftSidePos.x + SideExtents.x + BladeExtents.x,
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

		if(1)
		{
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

						const udword Index = 10 + Rnd.Randomize() % 4;

						PintActorHandle Handle = CreateSimpleObject(pint, &ConvexCreate[Index], 0.1f, pos);
						ASSERT(Handle);
					}
				}
			}
		}
		return true;
	}

	virtual udword	Update(Pint& pint, float dt)
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
	}

END_TEST(BulldozerTest)

///////////////////////////////////////////////////////////////////////////////
