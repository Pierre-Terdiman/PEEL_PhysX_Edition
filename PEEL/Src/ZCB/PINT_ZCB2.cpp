///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PINT_ZCB2.h"
#include "..\PintShapeRenderer.h"
#include "..\Zcb2_RenderData.h"

CHECK_CONTAINER_ITEM(ZCB2Factory::ActorCreate);

static const bool gUseRenderScale = true;
extern float gZCB2_RenderScale;

static void Reset(SceneChunk& chunk)
{
	chunk.SetName(null);
	chunk.mDesc.Reset();
	chunk.Init(PINT_WORLD_CREATE());
}

ZCB2Factory::ZCB2Factory() :
	mCustomize			(null),
	mNbDisabledGroups	(0),
	mDisabledGroups		(null)
{
	::Reset(mScene);
}

ZCB2Factory::~ZCB2Factory()
{
	Reset();
}

void ZCB2Factory::Reset()
{
	Release();

	mNbDisabledGroups = 0;
	DELETEARRAY(mDisabledGroups);

//	DeleteOwnedObjects<PINT_AGGREGATE_CREATE>(mAggregates);
	DeleteOwnedObjects<AggregateChunk>(mAggregateChunks);
	//DeleteOwnedObjects<PINT_OBJECT_CREATE>(mActorsCreate);
	//DeleteOwnedObjects<ZCB2Factory::ActorCreate>(mActorsCreate);
	{
		const udword Nb = GetNbActors();
		ActorCreate* Actors = const_cast<ActorCreate*>(GetActors());
		for(udword i=0;i<Nb;i++)
			Actors[i].Release();
		mActorsCreate.Empty();
	}
	DeleteOwnedObjects<PINT_SHAPE_CREATE>(mShapeCreate);
	DeleteOwnedObjects<PINT_MATERIAL_CREATE>(mMaterials);
	DeleteOwnedObjects<ConvexShapeChunk>(mConvexChunks);
	DeleteOwnedObjects<MeshShapeChunk>(mMeshChunks);
	DeleteOwnedObjects<MeshDataChunk>(mMeshDataChunks);
	DeleteOwnedObjects<PINT_MESH_DATA_CREATE>(mMeshObjects);
	DeleteOwnedObjects<PINT_JOINT_CREATE>(mJoints);
	mShapeColors.Empty();
//	mJointsActorIndices.Empty();
	//mActorsFlags.Empty();
	::Reset(mScene);

	mCustomize = null;
}

void ZCB2Factory::SetupShape(PINT_SHAPE_CREATE* create, const ShapeChunk& chunk, const char* name)
{
	ASSERT(create->mType == chunk.mType);
	create->mName		= name;
	create->mLocalPos	= chunk.mLocalPos;
	create->mLocalRot	= chunk.mLocalRot;

	if(chunk.mMaterialID!=INVALID_ID)
	{
		ASSERT(chunk.mMaterialID<mMaterials.GetNbEntries());
		create->mMaterial = (const PINT_MATERIAL_CREATE*)mMaterials.GetEntry(chunk.mMaterialID);
	}
	create->SetNext(null);

	if(chunk.mRendererID!=INVALID_ID)
		create->mRenderer = GetShapeRenderer(chunk.mRendererID);
	else
		create->mRenderer = null;

	// PINT's shape structure doesn't contain a color so we store it in a temporary array.
	// It will be consumed in ImportZB2File.
	// ### actually that still doesn't work because we go through the linked list in actor
	// we really need a hashmap here I guess
	if(chunk.GetCore().IsSet(SHAPE_CHUNK_FLAG_COLOR))
		mShapeColors.Add(chunk.mColor.R).Add(chunk.mColor.G).Add(chunk.mColor.B).Add(chunk.mColor.A);
	else
		mShapeColors.Add(INVALID_ID).Add(INVALID_ID).Add(INVALID_ID).Add(INVALID_ID);

	ASSERT(mShapeCreate.GetNbEntries()==chunk.GetID());
	mShapeCreate.AddPtr(create);
}

static void SetupJoint(PINT_JOINT_CREATE* create, const JointChunk& chunk, PtrContainer& joints, /*Container& joints_actor_indices, */const udword nb_actors, const char* name)
{
	create->mName		= name;
	create->mObject0	= null;
	create->mObject1	= null;

	create->mObject0	= PintActorHandle(size_t(chunk.mObject0));
	create->mObject1	= PintActorHandle(size_t(chunk.mObject1));

	const bool ValidActor0 = chunk.mObject0<nb_actors || chunk.mObject0==INVALID_ID;
	const bool ValidActor1 = chunk.mObject1<nb_actors || chunk.mObject1==INVALID_ID;
	ASSERT(ValidActor0);
	ASSERT(ValidActor1);
	if(ValidActor0 && ValidActor1)
		joints.AddPtr(create);
	else
		joints.AddPtr((void*)null);	// Add a null joint to preserve the exported mapping for gear & rack joints
//	joints_actor_indices.Add(chunk.mObject0).Add(chunk.mObject1);
}

void ZCB2Factory::NewUnknownChunk(udword type, const char* name, const VirtualFile& file)
{
	// The names are only persistent for the lifetime of the ZCB2Importer::Import() method. This is usually good enough
	// since you can deserialize the objects on-the-fly in this function. But it's not enough in PEEL where the same object
	// can be deserialized N times in different engines. We could fix it by making the string table have the same lifetime
	// as the ZCB2Importer object itself but that requires a change in ICE. So for now, we do this:
	if(name)
	{
		String* S = mNames.AddString(name);
		name = S->Get();
	}

	switch(type)
	{
		case CollisionGroupsType:
		{
			CollisionGroupsChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			ASSERT(!mNbDisabledGroups);
			ASSERT(!mDisabledGroups);
			mNbDisabledGroups = Chunk.mGroups.GetNbEntries()/2;
			mDisabledGroups = ICE_NEW(PintDisabledGroups)[mNbDisabledGroups];
			const udword* Data = Chunk.mGroups.GetEntries();
			for(udword i=0;i<mNbDisabledGroups;i++)
			{
				const udword Group0 = *Data++;
				const udword Group1 = *Data++;
				mDisabledGroups[i].mGroup0 = ToWord(Group0);
				mDisabledGroups[i].mGroup1 = ToWord(Group1);
			}
		}
		break;

		case MaterialType:
		{
			MaterialChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_MATERIAL_CREATE* Create = ICE_NEW(PINT_MATERIAL_CREATE);
			Create->mName				= name;
			Create->mDynamicFriction	= Chunk.mDynamicFriction;
			Create->mStaticFriction		= Chunk.mStaticFriction;
			Create->mRestitution		= Chunk.mRestitution;
			ASSERT(mMaterials.GetNbEntries()==Chunk.GetID());
			mMaterials.AddPtr(Create);
			if(mCustomize)
				mCustomize->CustomizeMaterial(*Create);
		}
		break;

		case MeshType:
		{
			MeshDataChunk* Chunk = ICE_NEW(MeshDataChunk);
			Chunk->SetName(name);
			Chunk->Import(file);
			mMeshDataChunks.AddPtr(Chunk);

//			PINT_MESH_DATA_CREATE* Create = ICE_NEW(PINT_MESH_DATA_CREATE);
			PINT_MESH_DATA_CREATE* Create = new PINT_MESH_DATA_CREATE;

			Create->SetSurfaceData(
				Chunk->mVertexData.GetNbPts(),
				Chunk->mVertexData.GetPts(),
				Chunk->mTriangleData.GetNbFaces(),
				Chunk->mTriangleData.GetDFaces(),
				Chunk->mTriangleData.GetWFaces());

			ASSERT(mMeshObjects.GetNbEntries()==Chunk->GetID());
			mMeshObjects.AddPtr(Create);
			if(mCustomize)
				mCustomize->CustomizeMesh(*Create);
		}
		break;

		case SphereShapeType:
		{
			SphereShapeChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_SPHERE_CREATE* Create = ICE_NEW(PINT_SPHERE_CREATE);
			SetupShape(Create, Chunk, name);
			Create->mRadius = Chunk.mRadius;
			if(mCustomize)
				mCustomize->CustomizeShape(*Create);
		}
		break;

		case CapsuleShapeType:
		{
			CapsuleShapeChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_CAPSULE_CREATE* Create = ICE_NEW(PINT_CAPSULE_CREATE);
			SetupShape(Create, Chunk, name);
			Create->mRadius = Chunk.mRadius;
			Create->mHalfHeight = Chunk.mHalfHeight;
			if(mCustomize)
				mCustomize->CustomizeShape(*Create);
		}
		break;

		case CylinderShapeType:
		{
			CylinderShapeChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_CYLINDER_CREATE* Create = ICE_NEW(PINT_CYLINDER_CREATE);
			SetupShape(Create, Chunk, name);
			Create->mRadius = Chunk.mRadius;
			Create->mHalfHeight = Chunk.mHalfHeight;
			if(mCustomize)
				mCustomize->CustomizeShape(*Create);
		}
		break;

		case BoxShapeType:
		{
			BoxShapeChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_BOX_CREATE* Create = ICE_NEW(PINT_BOX_CREATE);
			SetupShape(Create, Chunk, name);
			Create->mExtents = Chunk.mExtents;
			if(mCustomize)
				mCustomize->CustomizeShape(*Create);
		}
		break;

		case ConvexShapeType:
		{
			ConvexShapeChunk* Chunk = ICE_NEW(ConvexShapeChunk);
			Chunk->SetName(name);
			Chunk->Import(file);
			mConvexChunks.AddPtr(Chunk);

			PINT_CONVEX_CREATE* Create = ICE_NEW(PINT_CONVEX_CREATE);
			SetupShape(Create, *Chunk, name);
			Create->mNbVerts	= Chunk->mConvexData.GetNbPts();
			Create->mVerts		= Chunk->mConvexData.GetPts();
			if(mCustomize)
				mCustomize->CustomizeShape(*Create);
		}
		break;

		case MeshShapeType:
		{
			MeshShapeChunk* Chunk = ICE_NEW(MeshShapeChunk);
			Chunk->SetName(name);
			Chunk->Import(file);
			mMeshChunks.AddPtr(Chunk);

			PINT_MESH_CREATE* Create = ICE_NEW(PINT_MESH_CREATE);
			SetupShape(Create, *Chunk, name);
			Create->SetSurfaceData(
				Chunk->mVertexData.GetNbPts(),
				Chunk->mVertexData.GetPts(),
				Chunk->mTriangleData.GetNbFaces(),
				Chunk->mTriangleData.GetDFaces(),
				Chunk->mTriangleData.GetWFaces());

			if(mCustomize)
				mCustomize->CustomizeShape(*Create);
		}
		break;

		case MeshShape2Type:
		{
			MeshShapeChunk2 Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_MESH_CREATE2* Create = ICE_NEW(PINT_MESH_CREATE2);
			SetupShape(Create, Chunk, name);

			if(Chunk.mMeshID!=INVALID_ID)
			{
				ASSERT(Chunk.mMeshID<mMeshObjects.GetNbEntries());
				Create->mTriangleMesh = PintMeshHandle(size_t(Chunk.mMeshID));
			}
			if(mCustomize)
				mCustomize->CustomizeShape(*Create);
		}
		break;

		case ActorType:
		{
			ActorChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_OBJECT_CREATE* Create = ICE_NEW(PINT_OBJECT_CREATE);
			Create->mName				= name;
			Create->mPosition			= Chunk.mPosition;
			Create->mRotation			= Chunk.mRotation;
			Create->mCOMLocalOffset		= Chunk.mCOMLocalOffset;
			Create->mLinearVelocity		= Chunk.mLinearVelocity;
			Create->mAngularVelocity	= Chunk.mAngularVelocity;
			Create->mMass				= Chunk.mMass;
			Create->mMassForInertia		= Chunk.mMassForInertia;
			Create->mCollisionGroup		= Chunk.mCollisionGroup;
			Create->mKinematic			= Chunk.IsSetKinematic();
			Create->mAddToWorld			= Chunk.IsSetAddToWorld();

			if(0)
			{
				PINT_SHAPE_CREATE* Dst = null;
				const udword NbShapes = Chunk.mShapes.GetNbEntries();
				for(udword i=0;i<NbShapes;i++)
				{
					const udword ShapeID = Chunk.mShapes.GetEntry(i);
					ASSERT(ShapeID<mShapeCreate.GetNbEntries());
					PINT_SHAPE_CREATE* CurrentShape = (PINT_SHAPE_CREATE*)mShapeCreate.GetEntry(ShapeID);
					if(!Dst)
						Create->SetShape(CurrentShape);
					else
					{
						ASSERT(!Dst->_GetNext() || Dst->_GetNext()==CurrentShape);
						Dst->SetNext(CurrentShape);
					}
					Dst = CurrentShape;
	//				ASSERT(!CurrentShape->mNext);
				}
			}
			else
			{
				const udword NbShapes = Chunk.mShapes.GetNbEntries();
				for(udword i=0;i<NbShapes;i++)
				{
					const udword ShapeID = Chunk.mShapes.GetEntry(i);
					ASSERT(ShapeID<mShapeCreate.GetNbEntries());
					PINT_SHAPE_CREATE* CurrentShape = (PINT_SHAPE_CREATE*)mShapeCreate.GetEntry(ShapeID);
					Create->AddShape(CurrentShape);
				}
			}

			ASSERT(GetNbActors()==Chunk.GetID());
			const bool Hidden = Chunk.IsSetHidden();

			ActorCreate* NewActor = AddActor(Create, udword(Hidden));

			if(mCustomize)
				mCustomize->CustomizeActor(*Create);
		}
		break;

		case FixedJointType:
		{
			FixedJointChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_FIXED_JOINT_CREATE* Create = ICE_NEW(PINT_FIXED_JOINT_CREATE);
			Create->mLocalPivot0	= Chunk.mLocalPivot0;
			Create->mLocalPivot1	= Chunk.mLocalPivot1;
			SetupJoint(Create, Chunk, mJoints, /*mJointsActorIndices, */GetNbActors(), name);
			if(mCustomize)
				mCustomize->CustomizeJoint(*Create);
		}
		break;

		case SphericalJointType:
		{
			SphericalJointChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_SPHERICAL_JOINT_CREATE* Create = ICE_NEW(PINT_SPHERICAL_JOINT_CREATE);
			Create->mLocalPivot0	= Chunk.mLocalPivot0;
			Create->mLocalPivot1	= Chunk.mLocalPivot1;
			Create->mLimits			= Chunk.mLimits;
			SetupJoint(Create, Chunk, mJoints, /*mJointsActorIndices, */GetNbActors(), name);
			if(mCustomize)
				mCustomize->CustomizeJoint(*Create);
		}
		break;

		case DistanceJointType:
		{
			DistanceJointChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_DISTANCE_JOINT_CREATE* Create = ICE_NEW(PINT_DISTANCE_JOINT_CREATE);
			Create->mLocalPivot0	= Chunk.mLocalPivot0;
			Create->mLocalPivot1	= Chunk.mLocalPivot1;
			Create->mLimits			= Chunk.mLimits;
			SetupJoint(Create, Chunk, mJoints, /*mJointsActorIndices, */GetNbActors(), name);
			if(mCustomize)
				mCustomize->CustomizeJoint(*Create);
		}
		break;

		case PrismaticJointType:
		{
			PrismaticJointChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

//			static int counter=0;
//			if(counter==1)
//			{
				PINT_PRISMATIC_JOINT_CREATE* Create = ICE_NEW(PINT_PRISMATIC_JOINT_CREATE);
				Create->mLocalPivot0		= Chunk.mLocalPivot0;
				Create->mLocalPivot1		= Chunk.mLocalPivot1;
				Create->mLocalAxis0			= Chunk.mLocalAxis0;
				Create->mLocalAxis1			= Chunk.mLocalAxis1;
				Create->mLimits				= Chunk.mLimits;
				Create->mSpring				= Chunk.mSpring;
				SetupJoint(Create, Chunk, mJoints, /*mJointsActorIndices, */GetNbActors(), name);
//			}
//			else
//			{
//				PINT_D6_JOINT_CREATE* Create = ICE_NEW(PINT_D6_JOINT_CREATE);
//				Create->mLocalPivot0			= Chunk.mLocalPivot0;
//				Create->mLocalPivot1			= Chunk.mLocalPivot1;
//				Create->mLinearLimits.mMin.z	= -0.5f;
//				Create->mLinearLimits.mMax.z	= 0.0f;
//				Create->mLinearLimits.mMin.x	= -0.8f;
//				Create->mLinearLimits.mMax.x	= 0.8f;
//				SetupJoint(Create, Chunk, mJoints, /*mJointsActorIndices, */GetNbActors(), name);
//			}
//			counter++;
			if(mCustomize)
				mCustomize->CustomizeJoint(*Create);
		}
		break;

		case HingeJointType:
		{
			HingeJointChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_HINGE_JOINT_CREATE* Create = ICE_NEW(PINT_HINGE_JOINT_CREATE);
			Create->mLocalPivot0	= Chunk.mLocalPivot0;
			Create->mLocalPivot1	= Chunk.mLocalPivot1;
			Create->mLocalAxis0		= Chunk.mLocalAxis0;
			Create->mLocalAxis1		= Chunk.mLocalAxis1;
			Create->mGlobalAnchor	= Chunk.mGlobalAnchor;
			Create->mGlobalAxis		= Chunk.mGlobalAxis;

			Create->mLimits			= Chunk.mLimits;
			Create->mDriveVelocity	= Chunk.mDriveVelocity;
			Create->mUseMotor		= Chunk.mUseMotor;

			SetupJoint(Create, Chunk, mJoints, /*mJointsActorIndices, */GetNbActors(), name);
			if(mCustomize)
				mCustomize->CustomizeJoint(*Create);
		}
		break;

		case Hinge2JointType:
		{
			Hinge2JointChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_HINGE2_JOINT_CREATE* Create = ICE_NEW(PINT_HINGE2_JOINT_CREATE);
			Create->mLocalPivot0	= Chunk.mLocalPivot0;
			Create->mLocalPivot1	= Chunk.mLocalPivot1;

			Create->mLimits			= Chunk.mLimits;
			Create->mDriveVelocity	= Chunk.mDriveVelocity;
			Create->mUseMotor		= Chunk.mUseMotor;

			//printf("CHECKPOINT\n");
			/*static int hack=0;
			if(!hack)
			{
				hack = 1;
				Create->mUseMotor = true;
				Create->mDriveVelocity = 20.0f;
			}*/

			SetupJoint(Create, Chunk, mJoints, /*mJointsActorIndices, */GetNbActors(), name);
			if(mCustomize)
				mCustomize->CustomizeJoint(*Create);
		}
		break;

		case D6JointType:
		{
			D6JointChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_D6_JOINT_CREATE* Create = ICE_NEW(PINT_D6_JOINT_CREATE);
			Create->mLocalPivot0	= Chunk.mLocalPivot0;
			Create->mLocalPivot1	= Chunk.mLocalPivot1;
			Create->mLinearLimits	= Chunk.mLinearLimits;
			Create->mMinTwist		= Chunk.mMinTwist;
			Create->mMaxTwist		= Chunk.mMaxTwist;
			Create->mMaxSwingY		= Chunk.mMaxSwingY;
			Create->mMaxSwingZ		= Chunk.mMaxSwingZ;
			Create->mMotorFlags		= Chunk.mMotorFlags;
			Create->mMotorStiffness	= Chunk.mMotorStiffness;
			Create->mMotorDamping	= Chunk.mMotorDamping;
			SetupJoint(Create, Chunk, mJoints, /*mJointsActorIndices, */GetNbActors(), name);
			if(mCustomize)
				mCustomize->CustomizeJoint(*Create);
		}
		break;

		case GearJointType:
		{
			GearJointChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_GEAR_JOINT_CREATE* Create = ICE_NEW(PINT_GEAR_JOINT_CREATE);
			Create->mHinge0			= PintJointHandle(size_t(Chunk.mHinge0));
			Create->mHinge1			= PintJointHandle(size_t(Chunk.mHinge1));
			Create->mLocalPivot0	= Chunk.mLocalPivot0;
			Create->mLocalPivot1	= Chunk.mLocalPivot1;
			Create->mGearRatio		= Chunk.mGearRatio;
			//Create->mRadius0		= Chunk.mRadius0;
			//Create->mRadius1		= Chunk.mRadius1;
			SetupJoint(Create, Chunk, mJoints, /*mJointsActorIndices, */GetNbActors(), name);
			if(mCustomize)
				mCustomize->CustomizeJoint(*Create);
		}
		break;

		case RackJointType:
		{
			RackJointChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_RACK_AND_PINION_JOINT_CREATE* Create = ICE_NEW(PINT_RACK_AND_PINION_JOINT_CREATE);
			Create->mHinge			= PintJointHandle(size_t(Chunk.mHinge));
			Create->mPrismatic		= PintJointHandle(size_t(Chunk.mPrismatic));
			Create->mLocalPivot0	= Chunk.mLocalPivot0;
			Create->mLocalPivot1	= Chunk.mLocalPivot1;
			Create->mNbRackTeeth	= Chunk.mNbRackTeeth;
			Create->mNbPinionTeeth	= Chunk.mNbPinionTeeth;
			Create->mRackLength		= Chunk.mRackLength;
			SetupJoint(Create, Chunk, mJoints, /*mJointsActorIndices, */GetNbActors(), name);
			if(mCustomize)
				mCustomize->CustomizeJoint(*Create);
		}
		break;

		case SceneType:
		{
			// TODO: make sure there's only one of these in the file....
			mScene.SetName(name);
			mScene.Import(file);
#ifdef REMOVED
			SceneChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			mScene.mGlobalBounds			= Chunk.mGlobalBounds;
			mScene.mGravity					= Chunk.mGravity;
			mScene.mNbSimulateCallsPerFrame	= Chunk.mNbSimulateCallsPerFrame;
			mScene.mTimestep				= Chunk.mTimestep;
			CopyMemory(&mScene.mCamera, &Chunk.mCamera, PINT_MAX_CAMERA_POSES*sizeof(PintCameraPose));
			mScene.mCreateDefaultEnvironment = Chunk.IsSetCreateDefaultEnv();

			class Access : public PINT_WORLD_CREATE
			{
				public:
				void SetNames(const char* name/*, const char* desc*/)
				{
					mTestName = name;
//					mTestDesc = desc;
				}
			};
			static_cast<Access&>(mScene).SetNames(name/*, Chunk.mDesc.GetName()*/);
#endif
		}
		break;

		case AggregateType:
		{
			AggregateChunk* Chunk = ICE_NEW(AggregateChunk);
			Chunk->SetName(name);
			Chunk->Import(file);
			mAggregateChunks.AddPtr(Chunk);

/*			AggregateChunk Chunk;
			Chunk.SetName(name);
			Chunk.Import(file);

			PINT_AGGREGATE_CREATE* Create = ICE_NEW(PINT_AGGREGATE_CREATE)(Chunk.mMaxSize, Chunk.GetCore().IsSet(AGGREGATE_CHUNK_FLAG_SELF_COLLISIONS));

			const udword NbActors = Chunk.mActors.GetNbEntries();
			for(udword i=0;i<NbActors;i++)
			{
				const udword ActorID = Chunk.mActors.GetEntry(i);
				ASSERT(ActorID<mActors.GetNbEntries());
				PINT_OBJECT_CREATE* CurrentActor = (PINT_OBJECT_CREATE*)mActors.GetEntry(ActorID);
			}

			ASSERT(mAggregates.GetNbEntries()==Chunk.GetID());
			mAggregates.AddPtr(Create);*/
		}
		break;

		default:
		{
			ASSERT(!"Unknown chunk type in ZB2 file");
		}
		break;
	};
}

void ZCB2Factory::ReleasePintIndependentData()
{
}

///////////////////////////////////////////////////////////////////////////////

static void Export(const String& string, CustomArray& array)
{
	array.Store(string.Get()).Store(ubyte(0));
//	array.Store(udword('PEEL'));
}

static void Export(const Point& p, CustomArray& array)
{
	array.Store(p.x).Store(p.y).Store(p.z);
}

static void Export(const AABB& bounds, CustomArray& array)
{
	array
		.Store(bounds.mMin.x).Store(bounds.mMin.y).Store(bounds.mMin.z)
		.Store(bounds.mMax.x).Store(bounds.mMax.y).Store(bounds.mMax.z);
}

static void Export(const Quat& q, CustomArray& array)
{
	array.Store(q.p.x).Store(q.p.y).Store(q.p.z).Store(q.w);
}

static void Export(const PR& pr, CustomArray& array)
{
	::Export(pr.mRot, array);
	::Export(pr.mPos, array);
}

static void Export(const Container& c, CustomArray& array)
{
	const udword Nb = c.GetNbEntries();
	array.Store(Nb);
//	for(udword i=0;i<Nb;i++)
//		array.Store(c.GetEntry(i));
	if(Nb)
		array.Store(c.GetEntries(), Nb*sizeof(udword));
}

static void Export(const PintLimits& limit, CustomArray& array)
{
	array.Store(limit.mMinValue).Store(limit.mMaxValue);
}

static void Export(const PintSpring& spring, CustomArray& array)
{
	array.Store(spring.mStiffness).Store(spring.mDamping);
}

//#include "SupportFile.h"
static void Import(String& string, const VirtualFile& file)
{
//	const VirtualFile2& file2 = (const VirtualFile2&)file;
//	string = file2.ReadString();
//	return;

	char buffer[8192];
	int offset=0;
	ubyte c;
	do{
		c = file.ReadByte();
		buffer[offset++] = c;
	}
	while(c);
	string = buffer;
//	string = file.ReadString();

//	udword Check = file.ReadDword();
//	ASSERT(Check=='PEEL');
}

static void Import(Point& p, const VirtualFile& file)
{
	p.x = file.ReadFloat();
	p.y = file.ReadFloat();
	p.z = file.ReadFloat();
}

static void Import(AABB& bounds, const VirtualFile& file)
{
	bounds.mMin.x = file.ReadFloat();
	bounds.mMin.y = file.ReadFloat();
	bounds.mMin.z = file.ReadFloat();
	bounds.mMax.x = file.ReadFloat();
	bounds.mMax.y = file.ReadFloat();
	bounds.mMax.z = file.ReadFloat();
}

static void Import(Quat& q, const VirtualFile& file)
{
	q.p.x = file.ReadFloat();
	q.p.y = file.ReadFloat();
	q.p.z = file.ReadFloat();
	q.w = file.ReadFloat();
}

static void Import(PR& pr, const VirtualFile& file)
{
	::Import(pr.mRot, file);
	::Import(pr.mPos, file);
}

static void Import(Container& c, const VirtualFile& file)
{
	c.Empty();
	const udword Nb = file.ReadDword();
	if(Nb)
	{
		udword* Buffer = c.Reserve(Nb);
//		for(udword i=0;i<Nb;i++)
//			Buffer[i] = file.ReadDword();
		file.ReadBuffer(Buffer, Nb*sizeof(udword));
	}
}

static void Import(PintLimits& limit, const VirtualFile& file)
{
	limit.mMinValue = file.ReadFloat();
	limit.mMaxValue = file.ReadFloat();
}

static void Import(PintSpring& spring, const VirtualFile& file)
{
	spring.mStiffness = file.ReadFloat();
	spring.mDamping = file.ReadFloat();
}

///////////////////////////////////////////////////////////////////////////////

#define IMPLEMENT_PEEL_CHUNK(name, base)\
name::name(){}\
name::~name(){}\
const char* name::Validate(){ return base::Validate();}

///////////////////////////////////////////////////////////////////////////////

#define COLLISION_GROUPS_VERSION	1
IMPLEMENT_PEEL_CHUNK(CollisionGroupsChunk, BaseChunk)

void CollisionGroupsChunk::Init(udword nb_groups, const PintDisabledGroups* groups)
{
	mChunkType						= CollisionGroupsType;
	mCollisionGroupsCore.mVersion	= COLLISION_GROUPS_VERSION;
	//
	udword* dst = mGroups.Reserve(nb_groups*2);
	while(nb_groups--)
	{
		const PintCollisionGroup g0 = groups->mGroup0;
		const PintCollisionGroup g1 = groups->mGroup1;
		groups++;
		*dst++ = udword(g0);
		*dst++ = udword(g1);
	}
}

CollisionGroupsChunk::CollisionGroupsChunk(udword nb_groups, const PintDisabledGroups* groups)
{
	Init(nb_groups, groups);
}

CollisionGroupsChunk::CollisionGroupsChunk(const CollisionGroupsData& data)
{
	Init(data.mNbGroups, data.mGroups);
}

bool CollisionGroupsChunk::Export(CustomArray& array)
{
	BaseChunk::Export(array);
	mCollisionGroupsCore.Export(array);
	::Export(mGroups, array);
	return true;
}

bool CollisionGroupsChunk::Import(const VirtualFile& file)
{
	BaseChunk::Import(file);
	mCollisionGroupsCore.Import(file);
	::Import(mGroups, file);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define MATERIAL_VERSION	1
IMPLEMENT_PEEL_CHUNK(MaterialChunk, BaseChunk)

void MaterialChunk::Init(const char* name, float static_friction, float dynamic_friction, float restitution)
{
	mChunkType				= MaterialType;
	mMaterialCore.mVersion	= MATERIAL_VERSION;
	//
	mStaticFriction			= static_friction;
	mDynamicFriction		= dynamic_friction;
	mRestitution			= restitution;

	if(name)
		SetName(name);
}

MaterialChunk::MaterialChunk(const PINT_MATERIAL_CREATE& create)
{
	Init(create.mName, create.mStaticFriction, create.mDynamicFriction, create.mRestitution);
}

MaterialChunk::MaterialChunk(const MaterialData& data)
{
	Init(data.mName, data.mStaticFriction, data.mDynamicFriction, data.mRestitution);

	SetID(data.mID);
}

bool MaterialChunk::Export(CustomArray& array)
{
	BaseChunk::Export(array);
	mMaterialCore.Export(array);
	array
		.Store(mStaticFriction)
		.Store(mDynamicFriction)
		.Store(mRestitution);
	return true;
}

bool MaterialChunk::Import(const VirtualFile& file)
{
	BaseChunk::Import(file);
	mMaterialCore.Import(file);
	mStaticFriction		= file.ReadFloat();
	mDynamicFriction	= file.ReadFloat();
	mRestitution		= file.ReadFloat();
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define SHAPE_VERSION	2
// Version 2: mRendererID and mColor added
IMPLEMENT_PEEL_CHUNK(ShapeChunk, BaseChunk)

void ShapeChunk::InitBase(const char* name, const Point& local_pos, const Quat& local_rot, PintShapeRenderer* renderer)
{
	mChunkType			= ShapeType;
	mShapeCore.mVersion	= SHAPE_VERSION;
	//
	mLocalRot			= local_rot;
	mLocalPos			= local_pos;
	mType				= PINT_SHAPE_UNDEFINED;
	mMaterialID			= INVALID_ID;
	mRendererID			= INVALID_ID;

	const RGBAColor* Color = null;
	if(renderer)
	{
		Color = renderer->GetColor();
		if(renderer->mRenderSource)
			mRendererID	= renderer->mRenderSource->GetID();
	}

	if(Color)
	{
		mColor			= *Color;
		GetCore().Enable(SHAPE_CHUNK_FLAG_COLOR);
	}
	else
	{
		mColor.R		= 0.0f;
		mColor.G		= 0.0f;
		mColor.B		= 0.0f;
		mColor.A		= 0.0f;
		GetCore().Disable(SHAPE_CHUNK_FLAG_COLOR);
	}

	if(name)
		SetName(name);
}

ShapeChunk::ShapeChunk(const PINT_SHAPE_CREATE& create)
{
	InitBase(create.mName, create.mLocalPos, create.mLocalRot, create.mRenderer);
}

ShapeChunk::ShapeChunk(const ShapeData& data)
{
	InitBase(data.mName, data.mLocalPos, data.mLocalRot, data.mRenderer);

	SetID(data.mID);

	if(data.mMaterial)
		mMaterialID = data.mMaterial->mID;
}

bool ShapeChunk::Export(CustomArray& array)
{
	BaseChunk::Export(array);
	mShapeCore.Export(array);
	::Export(mLocalRot, array);
	::Export(mLocalPos, array);
	array.Store(udword(mType)).Store(mMaterialID).Store(mRendererID);

	if(GetCore().IsSet(SHAPE_CHUNK_FLAG_COLOR))
		array.Store(mColor.R).Store(mColor.G).Store(mColor.B).Store(mColor.A);

	return true;
}

bool ShapeChunk::Import(const VirtualFile& file)
{
	BaseChunk::Import(file);
	mShapeCore.Import(file);
	::Import(mLocalRot, file);
	::Import(mLocalPos, file);

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		mLocalPos *= gZCB2_RenderScale;
	}

	mType = PintShape(file.ReadDword());
	mMaterialID = file.ReadDword();
	if(GetCore().mVersion>=2)
	{
		mRendererID = file.ReadDword();

		if(GetCore().IsSet(SHAPE_CHUNK_FLAG_COLOR))
		{
			mColor.R = file.ReadFloat();
			mColor.G = file.ReadFloat();
			mColor.B = file.ReadFloat();
			mColor.A = file.ReadFloat();
		}
	}
	else
		mRendererID = INVALID_ID;
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define SPHERE_SHAPE_VERSION	1
IMPLEMENT_PEEL_CHUNK(SphereShapeChunk, ShapeChunk)

void SphereShapeChunk::Init(float radius)
{
	mChunkType					= SphereShapeType;
	mSphereShapeCore.mVersion	= SPHERE_SHAPE_VERSION;
	//
	mType						= PINT_SHAPE_SPHERE;
	mRadius						= radius;
}

SphereShapeChunk::SphereShapeChunk(const PINT_SPHERE_CREATE& create) : ShapeChunk(create)
{
	Init(create.mRadius);
}

SphereShapeChunk::SphereShapeChunk(const SphereShapeData& data) : ShapeChunk(data)
{
	Init(data.mRadius);
}

bool SphereShapeChunk::Export(CustomArray& array)
{
	ShapeChunk::Export(array);
	mSphereShapeCore.Export(array);
	array.Store(mRadius);
	return true;
}

bool SphereShapeChunk::Import(const VirtualFile& file)
{
	ShapeChunk::Import(file);
	mSphereShapeCore.Import(file);
	mRadius = file.ReadFloat();
	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
		mRadius *= gZCB2_RenderScale;
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define CAPSULE_SHAPE_VERSION	1
IMPLEMENT_PEEL_CHUNK(CapsuleShapeChunk, ShapeChunk)

void CapsuleShapeChunk::Init(float radius, float half_height)
{
	mChunkType					= CapsuleShapeType;
	mCapsuleShapeCore.mVersion	= CAPSULE_SHAPE_VERSION;
	//
	mType						= PINT_SHAPE_CAPSULE;
	mRadius						= radius;
	mHalfHeight					= half_height;
}

CapsuleShapeChunk::CapsuleShapeChunk(const PINT_CAPSULE_CREATE& create) : ShapeChunk(create)
{
	Init(create.mRadius, create.mHalfHeight);
}

CapsuleShapeChunk::CapsuleShapeChunk(const CapsuleShapeData& data) : ShapeChunk(data)
{
	Init(data.mRadius, data.mHalfHeight);
}

bool CapsuleShapeChunk::Export(CustomArray& array)
{
	ShapeChunk::Export(array);
	mCapsuleShapeCore.Export(array);
	array.Store(mRadius).Store(mHalfHeight);
	return true;
}

bool CapsuleShapeChunk::Import(const VirtualFile& file)
{
	ShapeChunk::Import(file);
	mCapsuleShapeCore.Import(file);
	mRadius = file.ReadFloat();
	mHalfHeight = file.ReadFloat();

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		mRadius *= gZCB2_RenderScale;
		mHalfHeight *= gZCB2_RenderScale;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define CYLINDER_SHAPE_VERSION	1
IMPLEMENT_PEEL_CHUNK(CylinderShapeChunk, ShapeChunk)

void CylinderShapeChunk::Init(float radius, float half_height)
{
	mChunkType					= CylinderShapeType;
	mCylinderShapeCore.mVersion	= CYLINDER_SHAPE_VERSION;
	//
	mType						= PINT_SHAPE_CYLINDER;
	mRadius						= radius;
	mHalfHeight					= half_height;
}

CylinderShapeChunk::CylinderShapeChunk(const PINT_CYLINDER_CREATE& create) : ShapeChunk(create)
{
	Init(create.mRadius, create.mHalfHeight);
}

CylinderShapeChunk::CylinderShapeChunk(const CylinderShapeData& data) : ShapeChunk(data)
{
	Init(data.mRadius, data.mHalfHeight);
}

bool CylinderShapeChunk::Export(CustomArray& array)
{
	ShapeChunk::Export(array);
	mCylinderShapeCore.Export(array);
	array.Store(mRadius).Store(mHalfHeight);
	return true;
}

bool CylinderShapeChunk::Import(const VirtualFile& file)
{
	ShapeChunk::Import(file);
	mCylinderShapeCore.Import(file);
	mRadius = file.ReadFloat();
	mHalfHeight = file.ReadFloat();

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		mRadius *= gZCB2_RenderScale;
		mHalfHeight *= gZCB2_RenderScale;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define BOX_SHAPE_VERSION	1
IMPLEMENT_PEEL_CHUNK(BoxShapeChunk, ShapeChunk)

void BoxShapeChunk::Init(const Point& extents)
{
	mChunkType				= BoxShapeType;
	mBoxShapeCore.mVersion	= BOX_SHAPE_VERSION;
	//
	mType					= PINT_SHAPE_BOX;
	mExtents				= extents;
}

BoxShapeChunk::BoxShapeChunk(const PINT_BOX_CREATE& create) : ShapeChunk(create)
{
	Init(create.mExtents);
}

BoxShapeChunk::BoxShapeChunk(const BoxShapeData& data) : ShapeChunk(data)
{
	Init(data.mExtents);
}

bool BoxShapeChunk::Export(CustomArray& array)
{
	ShapeChunk::Export(array);
	mBoxShapeCore.Export(array);
	::Export(mExtents, array);
	return true;
}

bool BoxShapeChunk::Import(const VirtualFile& file)
{
	ShapeChunk::Import(file);
	mBoxShapeCore.Import(file);
	::Import(mExtents, file);

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		mExtents *= gZCB2_RenderScale;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define CONVEX_SHAPE_VERSION	1
IMPLEMENT_PEEL_CHUNK(ConvexShapeChunk, ShapeChunk)

void ConvexShapeChunk::Init(udword nb_verts, const Point* verts)
{
	mChunkType					= ConvexShapeType;
	mConvexShapeCore.mVersion	= CONVEX_SHAPE_VERSION;
	//
	mType						= PINT_SHAPE_CONVEX;

	mConvexData.SetDiscardLast(false);
	mConvexData.SetQuantized(false);
	mConvexData.SetPts(nb_verts, verts);
}

ConvexShapeChunk::ConvexShapeChunk(const PINT_CONVEX_CREATE& create) : ShapeChunk(create)
{
	Init(create.mNbVerts, create.mVerts);
}

ConvexShapeChunk::ConvexShapeChunk(const ConvexShapeData& data) : ShapeChunk(data)
{
	Init(data.mNbVerts, data.mVerts);
}

bool ConvexShapeChunk::Export(CustomArray& array)
{
	ShapeChunk::Export(array);
	mConvexShapeCore.Export(array);
	mConvexData.Export(array);
	return true;
}

bool ConvexShapeChunk::Import(const VirtualFile& file)
{
	ShapeChunk::Import(file);
	mConvexShapeCore.Import(file);
	mConvexData.Import(file);

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		const udword NbPts = mConvexData.GetNbPts();
		Point* Pts = const_cast<Point*>(mConvexData.GetPts());
		for(udword i=0;i<NbPts;i++)
			Pts[i] *= gZCB2_RenderScale;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define MESH_SHAPE_VERSION	1
IMPLEMENT_PEEL_CHUNK(MeshShapeChunk, ShapeChunk)

void MeshShapeChunk::Init(udword nb_verts, const Point* verts, udword nb_faces, const udword* dfaces, const uword* wfaces)
{
	mChunkType				= MeshShapeType;
	mMeshShapeCore.mVersion	= MESH_SHAPE_VERSION;
	//
	mType					= PINT_SHAPE_MESH;

	mTriangleData.GetCore().Enable(ZCB2_FACE_NO_DELTA);

	mVertexData.SetDiscardLast(false);
	mVertexData.SetQuantized(false);
	mVertexData.SetPts(nb_verts, verts);
	mTriangleData.SetFaces(nb_faces, dfaces, wfaces);
}

MeshShapeChunk::MeshShapeChunk(const PINT_MESH_CREATE& create) : ShapeChunk(create)
{
	const SurfaceInterface& SI = create.GetSurface();
	Init(SI.mNbVerts, SI.mVerts, SI.mNbFaces, SI.mDFaces, SI.mWFaces);
}

MeshShapeChunk::MeshShapeChunk(const MeshShapeData& data) : ShapeChunk(data)
{
	Init(data.mNbVerts, data.mVerts, data.mNbTris, data.mTris->mRef, null);
}

bool MeshShapeChunk::Export(CustomArray& array)
{
	ShapeChunk::Export(array);
	mMeshShapeCore.Export(array);
	mVertexData.Export(array);
	mTriangleData.Export(array);
	return true;
}

bool MeshShapeChunk::Import(const VirtualFile& file)
{
	ShapeChunk::Import(file);
	mMeshShapeCore.Import(file);
	mVertexData.Import(file);
	mTriangleData.Import(file);

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		const udword NbPts = mVertexData.GetNbPts();
		Point* Pts = const_cast<Point*>(mVertexData.GetPts());
		for(udword i=0;i<NbPts;i++)
			Pts[i] *= gZCB2_RenderScale;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define MESH_SHAPE2_VERSION	1
IMPLEMENT_PEEL_CHUNK(MeshShapeChunk2, ShapeChunk)

void MeshShapeChunk2::Init()
{
	mChunkType				= MeshShape2Type;
	mMeshShapeCore.mVersion	= MESH_SHAPE2_VERSION;
	//
	mType					= PINT_SHAPE_MESH2;
	mMeshID					= INVALID_ID;
}

MeshShapeChunk2::MeshShapeChunk2(const PINT_MESH_CREATE2& create) : ShapeChunk(create)
{
	Init();
}

MeshShapeChunk2::MeshShapeChunk2(const MeshShapeData2& data) : ShapeChunk(data)
{
	Init();

	if(data.mMeshData)
		mMeshID = data.mMeshData->mID;
}

bool MeshShapeChunk2::Export(CustomArray& array)
{
	ShapeChunk::Export(array);
	mMeshShapeCore.Export(array);
	array.Store(mMeshID);
	return true;
}

bool MeshShapeChunk2::Import(const VirtualFile& file)
{
	ShapeChunk::Import(file);
	mMeshShapeCore.Import(file);
	mMeshID = file.ReadDword();
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define CONVEX_VERSION	1
IMPLEMENT_PEEL_CHUNK(ConvexDataChunk, BaseChunk)

void ConvexDataChunk::Init(udword nb_verts, const Point* verts)
{
	mChunkType				= ConvexType;
	mConvexCore.mVersion	= CONVEX_VERSION;

	mConvexData.SetDiscardLast(false);
	mConvexData.SetQuantized(false);
	mConvexData.SetPts(nb_verts, verts);
}

ConvexDataChunk::ConvexDataChunk(const PINT_CONVEX_DATA_CREATE& create)
{
	Init(create.mNbVerts, create.mVerts);

//	if(create.mName)
//		SetName(create.mName);
}

ConvexDataChunk::ConvexDataChunk(const ConvexMeshData& data)
{
	Init(data.mNbVerts, data.mVerts);

	if(data.mName.Get())
		SetName(data.mName);

	SetID(data.mID);
}

bool ConvexDataChunk::Export(CustomArray& array)
{
	BaseChunk::Export(array);
	mConvexCore.Export(array);
	mConvexData.Export(array);
	return true;
}

bool ConvexDataChunk::Import(const VirtualFile& file)
{
	BaseChunk::Import(file);
	mConvexCore.Import(file);
	mConvexData.Import(file);

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		const udword NbPts = mConvexData.GetNbPts();
		Point* Pts = const_cast<Point*>(mConvexData.GetPts());
		for(udword i=0;i<NbPts;i++)
			Pts[i] *= gZCB2_RenderScale;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define MESH_VERSION	1
IMPLEMENT_PEEL_CHUNK(MeshDataChunk, BaseChunk)

void MeshDataChunk::Init(udword nb_verts, const Point* verts, udword nb_faces, const udword* dfaces, const uword* wfaces)
{
	mChunkType			= MeshType;
	mMeshCore.mVersion	= MESH_VERSION;

	mTriangleData.GetCore().Enable(ZCB2_FACE_NO_DELTA);

	mVertexData.SetDiscardLast(false);
	mVertexData.SetQuantized(false);
	mVertexData.SetPts(nb_verts, verts);
	mTriangleData.SetFaces(nb_faces, dfaces, wfaces);
}

MeshDataChunk::MeshDataChunk(const PINT_MESH_DATA_CREATE& create)
{
	const SurfaceInterface& SI = create.GetSurface();
	Init(SI.mNbVerts, SI.mVerts, SI.mNbFaces, SI.mDFaces, SI.mWFaces);

//	if(create.mName)
//		SetName(create.mName);
}

MeshDataChunk::MeshDataChunk(const MeshData& data)
{
	Init(data.mNbVerts, data.mVerts, data.mNbTris, data.mTris->mRef, null);

	if(data.mName.Get())
		SetName(data.mName);

	SetID(data.mID);
}

bool MeshDataChunk::Export(CustomArray& array)
{
	BaseChunk::Export(array);
	mMeshCore.Export(array);
	mVertexData.Export(array);
	mTriangleData.Export(array);
	return true;
}

bool MeshDataChunk::Import(const VirtualFile& file)
{
	BaseChunk::Import(file);
	mMeshCore.Import(file);
	mVertexData.Import(file);
	mTriangleData.Import(file);

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		const udword NbPts = mVertexData.GetNbPts();
		Point* Pts = const_cast<Point*>(mVertexData.GetPts());
		for(udword i=0;i<NbPts;i++)
			Pts[i] *= gZCB2_RenderScale;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define JOINT_VERSION	1
IMPLEMENT_PEEL_CHUNK(JointChunk, BaseChunk)

JointChunk::JointChunk(const PINT_JOINT_CREATE& create)
{
	mChunkType			= JointType;
	mJointCore.mVersion	= JOINT_VERSION;
	//
//	mType				= PINT_JOINT_UNDEFINED;
//	mObject0			= create.mObject0;
//	mObject1			= create.mObject1;
	ActorChunk* Actor0 = reinterpret_cast<ActorChunk*>(create.mObject0);
	ActorChunk* Actor1 = reinterpret_cast<ActorChunk*>(create.mObject1);
	mObject0 = Actor0 ? Actor0->GetID() : INVALID_ID;
	mObject1 = Actor1 ? Actor1->GetID() : INVALID_ID;

	if(create.mName)
		SetName(create.mName);
}

JointChunk::JointChunk(const JointData& data)
{
	mChunkType			= JointType;
	mJointCore.mVersion	= JOINT_VERSION;
	//
	ActorData* Actor0 = data.mObject0;
	ActorData* Actor1 = data.mObject1;
	mObject0 = Actor0 ? Actor0->mID : INVALID_ID;
	mObject1 = Actor1 ? Actor1->mID : INVALID_ID;

	if(data.mName.Get())
		SetName(data.mName);
}

bool JointChunk::Export(CustomArray& array)
{
	BaseChunk::Export(array);
	mJointCore.Export(array);
	array.Store(mObject0).Store(mObject1);
	return true;
}

bool JointChunk::Import(const VirtualFile& file)
{
	BaseChunk::Import(file);
	mJointCore.Import(file);
	mObject0 = file.ReadDword();
	mObject1 = file.ReadDword();
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define SPHERICAL_JOINT_VERSION	3
IMPLEMENT_PEEL_CHUNK(SphericalJointChunk, JointChunk)

SphericalJointChunk::SphericalJointChunk(const PINT_SPHERICAL_JOINT_CREATE& create) : JointChunk(create)
{
	mChunkType						= SphericalJointType;
	mSphericalJointCore.mVersion	= SPHERICAL_JOINT_VERSION;
	//
	mLocalPivot0					= create.mLocalPivot0;
	mLocalPivot1					= create.mLocalPivot1;
	mLimits							= create.mLimits;
}

SphericalJointChunk::SphericalJointChunk(const SphericalJointData& data) : JointChunk(data)
{
	mChunkType						= SphericalJointType;
	mSphericalJointCore.mVersion	= SPHERICAL_JOINT_VERSION;
	//
	mLocalPivot0					= data.mLocalPivot0;
	mLocalPivot1					= data.mLocalPivot1;
	mLimits							= data.mLimits;
}

bool SphericalJointChunk::Export(CustomArray& array)
{
	JointChunk::Export(array);
	mSphericalJointCore.Export(array);
	::Export(mLocalPivot0, array);
	::Export(mLocalPivot1, array);
	::Export(mLimits, array);
	return true;
}

bool SphericalJointChunk::Import(const VirtualFile& file)
{
	JointChunk::Import(file);
	mSphericalJointCore.Import(file);

	SetSphericalLimitDisabled(mLimits);

	if(mSphericalJointCore.mVersion==1)
	{
		mLocalPivot0.Identity();
		mLocalPivot1.Identity();
		::Import(mLocalPivot0.mPos, file);
		::Import(mLocalPivot1.mPos, file);
	}
	else
	{
		::Import(mLocalPivot0, file);
		::Import(mLocalPivot1, file);

		if(mSphericalJointCore.mVersion>=3)
			::Import(mLimits, file);
	}

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		mLocalPivot0.mPos *= gZCB2_RenderScale;
		mLocalPivot1.mPos *= gZCB2_RenderScale;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define FIXED_JOINT_VERSION	1
IMPLEMENT_PEEL_CHUNK(FixedJointChunk, JointChunk)

FixedJointChunk::FixedJointChunk(const PINT_FIXED_JOINT_CREATE& create) : JointChunk(create)
{
	mChunkType					= FixedJointType;
	mFixedJointCore.mVersion	= JOINT_VERSION;
	//
	mLocalPivot0				= create.mLocalPivot0;
	mLocalPivot1				= create.mLocalPivot1;
}

FixedJointChunk::FixedJointChunk(const FixedJointData& data) : JointChunk(data)
{
	mChunkType					= FixedJointType;
	mFixedJointCore.mVersion	= JOINT_VERSION;
	//
	mLocalPivot0				= data.mLocalPivot0;
	mLocalPivot1				= data.mLocalPivot1;
}

bool FixedJointChunk::Export(CustomArray& array)
{
	JointChunk::Export(array);
	mFixedJointCore.Export(array);
	::Export(mLocalPivot0, array);
	::Export(mLocalPivot1, array);
	return true;
}

bool FixedJointChunk::Import(const VirtualFile& file)
{
	JointChunk::Import(file);
	mFixedJointCore.Import(file);
	::Import(mLocalPivot0, file);
	::Import(mLocalPivot1, file);

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		mLocalPivot0 *= gZCB2_RenderScale;
		mLocalPivot1 *= gZCB2_RenderScale;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define DISTANCE_JOINT_VERSION	1
IMPLEMENT_PEEL_CHUNK(DistanceJointChunk, JointChunk)

DistanceJointChunk::DistanceJointChunk(const PINT_DISTANCE_JOINT_CREATE& create) : JointChunk(create)
{
	mChunkType					= DistanceJointType;
	mDistanceJointCore.mVersion	= DISTANCE_JOINT_VERSION;
	//
	mLocalPivot0				= create.mLocalPivot0;
	mLocalPivot1				= create.mLocalPivot1;
	mLimits						= create.mLimits;
}

DistanceJointChunk::DistanceJointChunk(const DistanceJointData& data) : JointChunk(data)
{
	mChunkType					= DistanceJointType;
	mDistanceJointCore.mVersion	= DISTANCE_JOINT_VERSION;
	//
	mLocalPivot0				= data.mLocalPivot0;
	mLocalPivot1				= data.mLocalPivot1;
	mLimits						= data.mLimits;
}

bool DistanceJointChunk::Export(CustomArray& array)
{
	JointChunk::Export(array);
	mDistanceJointCore.Export(array);
	::Export(mLocalPivot0, array);
	::Export(mLocalPivot1, array);
	::Export(mLimits, array);
	return true;
}

bool DistanceJointChunk::Import(const VirtualFile& file)
{
	JointChunk::Import(file);
	mDistanceJointCore.Import(file);
	::Import(mLocalPivot0, file);
	::Import(mLocalPivot1, file);
	::Import(mLimits, file);

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		mLocalPivot0 *= gZCB2_RenderScale;
		mLocalPivot1 *= gZCB2_RenderScale;
		if(IsMinDistanceLimitEnabled(mLimits))
			mLimits.mMinValue *= gZCB2_RenderScale;
		if(IsMaxDistanceLimitEnabled(mLimits))
			mLimits.mMaxValue *= gZCB2_RenderScale;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define PRISMATIC_JOINT_VERSION	2
IMPLEMENT_PEEL_CHUNK(PrismaticJointChunk, JointChunk)

PrismaticJointChunk::PrismaticJointChunk(const PINT_PRISMATIC_JOINT_CREATE& create) : JointChunk(create)
{
	mChunkType						= PrismaticJointType;
	mPrismaticJointCore.mVersion	= PRISMATIC_JOINT_VERSION;
	//
	mLocalPivot0					= create.mLocalPivot0;
	mLocalPivot1					= create.mLocalPivot1;
	mLocalAxis0						= create.mLocalAxis0;
	mLocalAxis1						= create.mLocalAxis1;
	mLimits							= create.mLimits;
	mSpring							= create.mSpring;
}

PrismaticJointChunk::PrismaticJointChunk(const PrismaticJointData& data) : JointChunk(data)
{
	mChunkType						= PrismaticJointType;
	mPrismaticJointCore.mVersion	= PRISMATIC_JOINT_VERSION;
	//
	mLocalPivot0					= data.mLocalPivot0;
	mLocalPivot1					= data.mLocalPivot1;
	mLocalAxis0						= data.mLocalAxis0;
	mLocalAxis1						= data.mLocalAxis1;
	mLimits							= data.mLimits;
	mSpring							= data.mSpring;
}

bool PrismaticJointChunk::Export(CustomArray& array)
{
	JointChunk::Export(array);
	mPrismaticJointCore.Export(array);
	::Export(mLocalPivot0, array);
	::Export(mLocalPivot1, array);
	::Export(mLocalAxis0, array);
	::Export(mLocalAxis1, array);
	::Export(mLimits, array);
	::Export(mSpring, array);
	return true;
}

bool PrismaticJointChunk::Import(const VirtualFile& file)
{
	JointChunk::Import(file);
	mPrismaticJointCore.Import(file);
	if(mPrismaticJointCore.mVersion>1)
	{
		::Import(mLocalPivot0, file);
		::Import(mLocalPivot1, file);
	}
	else
	{
		::Import(mLocalPivot0.mPos, file);
		::Import(mLocalPivot1.mPos, file);
		mLocalPivot0.mRot.Identity();
		mLocalPivot1.mRot.Identity();
	}
	::Import(mLocalAxis0, file);
	::Import(mLocalAxis1, file);
	::Import(mLimits, file);
	::Import(mSpring, file);

//	mMinLimit		= 1.0f;
//	mMaxLimit		= -1.0f;
//	mSpringStiffness= 0.0f;
//	mSpringDamping	= 0.0f;

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		mLocalPivot0.mPos *= gZCB2_RenderScale;
		mLocalPivot1.mPos *= gZCB2_RenderScale;
		if(IsPrismaticLimitEnabled(mLimits))
		{
			mLimits.mMinValue *= gZCB2_RenderScale;
			mLimits.mMaxValue *= gZCB2_RenderScale;
		}
	}

/*	static int count = 0;
	count++;
	if(count==5)
	{
		mSpringStiffness *= 1000.0f;
		mSpringDamping *= 1000.0f;
	}*/

	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define HINGE_JOINT_VERSION	1
IMPLEMENT_PEEL_CHUNK(HingeJointChunk, JointChunk)

HingeJointChunk::HingeJointChunk(const PINT_HINGE_JOINT_CREATE& create) : JointChunk(create)
{
	mChunkType					= HingeJointType;
	mHingeJointCore.mVersion	= HINGE_JOINT_VERSION;
	//
	mLocalPivot0				= create.mLocalPivot0;
	mLocalPivot1				= create.mLocalPivot1;
	mLocalAxis0					= create.mLocalAxis0;
	mLocalAxis1					= create.mLocalAxis1;
	mLimits						= create.mLimits;
	mGlobalAnchor				= create.mGlobalAnchor;
	mGlobalAxis					= create.mGlobalAxis;
	mUseMotor					= create.mUseMotor;
	mDriveVelocity				= create.mDriveVelocity;
}

HingeJointChunk::HingeJointChunk(const HingeJointData& data) : JointChunk(data)
{
	mChunkType					= HingeJointType;
	mHingeJointCore.mVersion	= HINGE_JOINT_VERSION;
	//
	mLocalPivot0				= data.mLocalPivot0;
	mLocalPivot1				= data.mLocalPivot1;
	mLocalAxis0					= data.mLocalAxis0;
	mLocalAxis1					= data.mLocalAxis1;
	mLimits						= data.mLimits;
	mGlobalAnchor				= data.mGlobalAnchor;
	mGlobalAxis					= data.mGlobalAxis;
	mUseMotor					= data.mUseMotor;
	mDriveVelocity				= data.mDriveVelocity;
}

bool HingeJointChunk::Export(CustomArray& array)
{
	JointChunk::Export(array);
	mHingeJointCore.Export(array);
	::Export(mLocalPivot0, array);
	::Export(mLocalPivot1, array);
	::Export(mLocalAxis0, array);
	::Export(mLocalAxis1, array);
	::Export(mGlobalAnchor, array);
	::Export(mGlobalAxis, array);
	::Export(mLimits, array);
	array.Store(mDriveVelocity).Store(mUseMotor?udword(1):udword(0));
	return true;
}

bool HingeJointChunk::Import(const VirtualFile& file)
{
	JointChunk::Import(file);
	mHingeJointCore.Import(file);
	::Import(mLocalPivot0, file);
	::Import(mLocalPivot1, file);
	::Import(mLocalAxis0, file);
	::Import(mLocalAxis1, file);
	::Import(mGlobalAnchor, file);
	::Import(mGlobalAxis, file);
	::Import(mLimits, file);
	mDriveVelocity = file.ReadFloat();
	mUseMotor = file.ReadDword() ? true : false;

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		mLocalPivot0 *= gZCB2_RenderScale;
		mLocalPivot1 *= gZCB2_RenderScale;

		if(!mGlobalAnchor.IsNotUsed())
			mGlobalAnchor *= gZCB2_RenderScale;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define HINGE2_JOINT_VERSION	1
IMPLEMENT_PEEL_CHUNK(Hinge2JointChunk, JointChunk)

Hinge2JointChunk::Hinge2JointChunk(const PINT_HINGE2_JOINT_CREATE& create) : JointChunk(create)
{
	mChunkType					= Hinge2JointType;
	mHinge2JointCore.mVersion	= HINGE2_JOINT_VERSION;
	//
	mLocalPivot0				= create.mLocalPivot0;
	mLocalPivot1				= create.mLocalPivot1;
	mLimits						= create.mLimits;
	mUseMotor					= create.mUseMotor;
	mDriveVelocity				= create.mDriveVelocity;
}

Hinge2JointChunk::Hinge2JointChunk(const Hinge2JointData& data) : JointChunk(data)
{
	mChunkType					= Hinge2JointType;
	mHinge2JointCore.mVersion	= HINGE2_JOINT_VERSION;
	//
	mLocalPivot0				= data.mLocalPivot0;
	mLocalPivot1				= data.mLocalPivot1;
	mLimits						= data.mLimits;
	mUseMotor					= data.mUseMotor;
	mDriveVelocity				= data.mDriveVelocity;
}

bool Hinge2JointChunk::Export(CustomArray& array)
{
	JointChunk::Export(array);
	mHinge2JointCore.Export(array);
	::Export(mLocalPivot0, array);
	::Export(mLocalPivot1, array);
	::Export(mLimits, array);
	array.Store(mDriveVelocity).Store(mUseMotor?udword(1):udword(0));
	return true;
}

bool Hinge2JointChunk::Import(const VirtualFile& file)
{
	JointChunk::Import(file);
	mHinge2JointCore.Import(file);
	::Import(mLocalPivot0, file);
	::Import(mLocalPivot1, file);
	::Import(mLimits, file);
	mDriveVelocity = file.ReadFloat();
	mUseMotor = file.ReadDword() ? true : false;

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		mLocalPivot0.mPos *= gZCB2_RenderScale;
		mLocalPivot1.mPos *= gZCB2_RenderScale;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define D6_JOINT_VERSION	1
IMPLEMENT_PEEL_CHUNK(D6JointChunk, JointChunk)

D6JointChunk::D6JointChunk(const PINT_D6_JOINT_CREATE& create) : JointChunk(create)
{
	mChunkType				= D6JointType;
	mD6JointCore.mVersion	= D6_JOINT_VERSION;
	//
	mLocalPivot0			= create.mLocalPivot0;
	mLocalPivot1			= create.mLocalPivot1;
	mLinearLimits			= create.mLinearLimits;
	mMinTwist				= create.mMinTwist;
	mMaxTwist				= create.mMaxTwist;
	mMaxSwingY				= create.mMaxSwingY;
	mMaxSwingZ				= create.mMaxSwingZ;
	mMotorFlags				= create.mMotorFlags;
	mMotorStiffness			= create.mMotorStiffness;
	mMotorDamping			= create.mMotorDamping;
}

D6JointChunk::D6JointChunk(const D6JointData& data) : JointChunk(data)
{
	mChunkType				= D6JointType;
	mD6JointCore.mVersion	= D6_JOINT_VERSION;
	//
	mLocalPivot0			= data.mLocalPivot0;
	mLocalPivot1			= data.mLocalPivot1;
	mLinearLimits			= data.mLinearLimits;
	mMinTwist				= data.mMinTwist;
	mMaxTwist				= data.mMaxTwist;
	mMaxSwingY				= data.mMaxSwingY;
	mMaxSwingZ				= data.mMaxSwingZ;
	mMotorFlags				= data.mMotorFlags;
	mMotorStiffness			= data.mMotorStiffness;
	mMotorDamping			= data.mMotorDamping;
}

bool D6JointChunk::Export(CustomArray& array)
{
	JointChunk::Export(array);
	mD6JointCore.Export(array);
	::Export(mLocalPivot0, array);
	::Export(mLocalPivot1, array);
	::Export(mLinearLimits, array);
	array
		.Store(mMinTwist)
		.Store(mMaxTwist)
		.Store(mMaxSwingY)
		.Store(mMaxSwingZ)
		.Store(mMotorFlags)
		.Store(mMotorStiffness)
		.Store(mMotorDamping);
	return true;
}

bool D6JointChunk::Import(const VirtualFile& file)
{
	JointChunk::Import(file);
	mD6JointCore.Import(file);
	::Import(mLocalPivot0, file);
	::Import(mLocalPivot1, file);
	::Import(mLinearLimits, file);
	mMinTwist = file.ReadFloat();
	mMaxTwist = file.ReadFloat();
	mMaxSwingY = file.ReadFloat();
	mMaxSwingZ = file.ReadFloat();
	mMotorFlags = file.ReadDword();
	mMotorStiffness = file.ReadFloat();
	mMotorDamping = file.ReadFloat();

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		mLocalPivot0.mPos *= gZCB2_RenderScale;
		mLocalPivot1.mPos *= gZCB2_RenderScale;
		mLinearLimits.mMin *= gZCB2_RenderScale;
		mLinearLimits.mMax *= gZCB2_RenderScale;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define GEAR_JOINT_VERSION	4
IMPLEMENT_PEEL_CHUNK(GearJointChunk, JointChunk)

GearJointChunk::GearJointChunk(const PINT_GEAR_JOINT_CREATE& create) : JointChunk(create)
{
	mChunkType				= GearJointType;
	mGearJointCore.mVersion	= GEAR_JOINT_VERSION;
	//
	JointChunk* J0 = reinterpret_cast<JointChunk*>(create.mHinge0);
	JointChunk* J1 = reinterpret_cast<JointChunk*>(create.mHinge1);
	mHinge0 = J0 ? J0->GetID() : INVALID_ID;
	mHinge1 = J1 ? J1->GetID() : INVALID_ID;
	//
	mLocalPivot0			= create.mLocalPivot0;
	mLocalPivot1			= create.mLocalPivot1;
	mGearRatio				= create.mGearRatio;
//	mRadius0				= create.mRadius0;
//	mRadius1				= create.mRadius1;
//	mErrorSign				= create.mErrorSign;
}

GearJointChunk::GearJointChunk(const GearJointData& data) : JointChunk(data)
{
	mChunkType				= GearJointType;
	mGearJointCore.mVersion	= GEAR_JOINT_VERSION;
	//
	JointData* J0 = data.mHinge0;
	JointData* J1 = data.mHinge1;
	mHinge0 = J0 ? J0->mID : INVALID_ID;
	mHinge1 = J1 ? J1->mID : INVALID_ID;
	//
	mLocalPivot0			= data.mLocalPivot0;
	mLocalPivot1			= data.mLocalPivot1;
	mGearRatio				= data.mGearRatio;
//	mRadius0				= data.mRadius0;
//	mRadius1				= data.mRadius1;
//	mErrorSign				= data.mErrorSign;
}

bool GearJointChunk::Export(CustomArray& array)
{
	JointChunk::Export(array);
	mGearJointCore.Export(array);
	array.Store(mHinge0).Store(mHinge1);
	::Export(mLocalPivot0, array);
	::Export(mLocalPivot1, array);
	array.Store(mGearRatio);//.Store(mRadius0).Store(mRadius1);//.Store(mErrorSign);
	return true;
}

bool GearJointChunk::Import(const VirtualFile& file)
{
	JointChunk::Import(file);
	mGearJointCore.Import(file);
	mHinge0 = file.ReadDword();
	mHinge1 = file.ReadDword();
	::Import(mLocalPivot0, file);
	::Import(mLocalPivot1, file);
	mGearRatio	= file.ReadFloat();
	if(mGearJointCore.mVersion<4)
	{
		const float mRadius0 = file.ReadFloat();
		const float mRadius1 = file.ReadFloat();
	}
	if(mGearJointCore.mVersion==2)
		file.ReadFloat();
/*	if(mGearJointCore.mVersion>1)
		mErrorSign = file.ReadFloat();
	else
		mErrorSign = 1.0f;*/

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		mLocalPivot0.mPos *= gZCB2_RenderScale;
		mLocalPivot1.mPos *= gZCB2_RenderScale;
	}

	/*static int hack = 0;
	if(!hack)
	{
		hack = 1;
		mGearRatio = -mGearRatio;
	}*/

	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define RACK_JOINT_VERSION	3
IMPLEMENT_PEEL_CHUNK(RackJointChunk, JointChunk)

RackJointChunk::RackJointChunk(const PINT_RACK_AND_PINION_JOINT_CREATE& create) : JointChunk(create)
{
	mChunkType				= RackJointType;
	mRackJointCore.mVersion	= RACK_JOINT_VERSION;
	//
	JointChunk* J0 = reinterpret_cast<JointChunk*>(create.mHinge);
	JointChunk* J1 = reinterpret_cast<JointChunk*>(create.mPrismatic);
	mHinge = J0 ? J0->GetID() : INVALID_ID;
	mPrismatic = J1 ? J1->GetID() : INVALID_ID;
	//
	mLocalPivot0			= create.mLocalPivot0;
	mLocalPivot1			= create.mLocalPivot1;
	mNbRackTeeth			= create.mNbRackTeeth;
	mNbPinionTeeth			= create.mNbPinionTeeth;
	mRackLength				= create.mRackLength;
//	mErrorSign				= create.mErrorSign;
}

RackJointChunk::RackJointChunk(const RackJointData& data) : JointChunk(data)
{
	mChunkType				= RackJointType;
	mRackJointCore.mVersion	= RACK_JOINT_VERSION;
	//
	JointData* J0 = data.mHinge;
	JointData* J1 = data.mPrismatic;
	mHinge = J0 ? J0->mID : INVALID_ID;
	mPrismatic = J1 ? J1->mID : INVALID_ID;
	//
	mLocalPivot0			= data.mLocalPivot0;
	mLocalPivot1			= data.mLocalPivot1;
	mNbRackTeeth			= data.mNbRackTeeth;
	mNbPinionTeeth			= data.mNbPinionTeeth;
	mRackLength				= data.mRackLength;
//	mErrorSign				= data.mErrorSign;
}

bool RackJointChunk::Export(CustomArray& array)
{
	JointChunk::Export(array);
	mRackJointCore.Export(array);
	array.Store(mHinge).Store(mPrismatic);
	::Export(mLocalPivot0, array);
	::Export(mLocalPivot1, array);
	array.Store(mNbRackTeeth).Store(mNbPinionTeeth).Store(mRackLength);//.Store(mErrorSign);
	return true;
}

bool RackJointChunk::Import(const VirtualFile& file)
{
	JointChunk::Import(file);
	mRackJointCore.Import(file);
	mHinge = file.ReadDword();
	mPrismatic = file.ReadDword();
	::Import(mLocalPivot0, file);
	::Import(mLocalPivot1, file);
	mNbRackTeeth	= file.ReadDword();
	mNbPinionTeeth	= file.ReadDword();
	mRackLength		= file.ReadFloat();
	if(mRackJointCore.mVersion==2)
		file.ReadFloat();
/*	if(mRackJointCore.mVersion>1)
		mErrorSign = file.ReadFloat();
	else
		mErrorSign = 1.0f;*/

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		mLocalPivot0.mPos *= gZCB2_RenderScale;
		mLocalPivot1.mPos *= gZCB2_RenderScale;
		mRackLength *= gZCB2_RenderScale;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define ACTOR_VERSION	2
IMPLEMENT_PEEL_CHUNK(ActorChunk, BaseChunk)

static void SetupActorFlags(ActorChunk& chunk, bool kinematic, bool add_to_world)
{
	chunk.SetKinematic(kinematic);
	chunk.SetAddToWorld(add_to_world);
}

ActorChunk::ActorChunk(const PINT_OBJECT_CREATE& create)
{
	mChunkType			= ActorType;
	mActorCore.mVersion	= ACTOR_VERSION;
	//
	mPosition			= create.mPosition;
	mRotation			= create.mRotation;
	mCOMLocalOffset		= create.mCOMLocalOffset;
	mLinearVelocity		= create.mLinearVelocity;
	mAngularVelocity	= create.mAngularVelocity;
	mMass				= create.mMass;
	mMassForInertia		= create.mMassForInertia;
	mCollisionGroup		= create.mCollisionGroup;

	SetupActorFlags(*this, create.mKinematic, create.mAddToWorld);

	if(create.mName)
		SetName(create.mName);

//	const udword NbShapes = create.GetNbShapes();
//	mShapes.SetSize(NbShapes);
}

ActorChunk::ActorChunk(const ActorData& data, bool hidden)
{
	mChunkType			= ActorType;
	mActorCore.mVersion	= ACTOR_VERSION;
	//
	mPosition			= data.mPosition;
	mRotation			= data.mRotation;
	mCOMLocalOffset		= data.mCOMLocalOffset;
	mLinearVelocity		= data.mLinearVelocity;
	mAngularVelocity	= data.mAngularVelocity;
	mMass				= data.mMass;
	mMassForInertia		= data.mMassForInertia;
	mCollisionGroup		= data.mCollisionGroup;

	SetupActorFlags(*this, data.mKinematic, data.mAddToWorld);

	SetHidden(hidden);

	if(data.mName.Get())
		SetName(data.mName);

	SetID(data.mID);

//	const udword NbShapes = create.GetNbShapes();
//	mShapes.SetSize(NbShapes);
}

bool ActorChunk::Export(CustomArray& array)
{
	BaseChunk::Export(array);
	mActorCore.Export(array);
	::Export(mPosition, array);
	::Export(mRotation, array);
	::Export(mCOMLocalOffset, array);
	::Export(mLinearVelocity, array);
	::Export(mAngularVelocity, array);
	array
		.Store(mMass)
		.Store(mMassForInertia)
		.Store(mCollisionGroup);

/*	const udword NbShapes = mShapes.GetNbEntries();
	array.Store(NbShapes);
	for(udword i=0;i<NbShapes;i++)
		array.Store(mShapes.GetEntry(i));*/
	::Export(mShapes, array);

	return true;
}

bool ActorChunk::Import(const VirtualFile& file)
{
	BaseChunk::Import(file);
	mActorCore.Import(file);
	::Import(mPosition, file);
	::Import(mRotation, file);
	::Import(mCOMLocalOffset, file);
	::Import(mLinearVelocity, file);
	::Import(mAngularVelocity, file);
	mMass = file.ReadFloat();
	mMassForInertia = file.ReadFloat();
	mCollisionGroup = file.ReadWord();

	if(mActorCore.mVersion==1)
	{
		const bool mKinematic = file.ReadBool();
		const bool mAddToWorld = file.ReadBool();
		SetupActorFlags(*this, mKinematic, mAddToWorld);
	}

/*	mShapes.Empty();
	const udword NbShapes = file.ReadDword();
	udword* Buffer = mShapes.Reserve(NbShapes);
	for(udword i=0;i<NbShapes;i++)
		Buffer[i] = file.ReadDword();*/
	::Import(mShapes, file);

	if(gUseRenderScale && gZCB2_RenderScale!=1.0f)
	{
		mPosition *= gZCB2_RenderScale;
		mCOMLocalOffset *= gZCB2_RenderScale;
		mLinearVelocity *= gZCB2_RenderScale;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define AGGREGATE_VERSION	1
IMPLEMENT_PEEL_CHUNK(AggregateChunk, BaseChunk)

AggregateChunk::AggregateChunk(const PINT_AGGREGATE_CREATE& create)
{
	mChunkType				= AggregateType;
	mAggregateCore.mVersion	= AGGREGATE_VERSION;
	//
	mMaxSize				= create.mMaxSize;

	SetSelfCollision(create.mSelfCollision);
}

AggregateChunk::AggregateChunk(const AggregateData& data)
{
	mChunkType				= AggregateType;
	mAggregateCore.mVersion	= AGGREGATE_VERSION;

	//
	mMaxSize				= data.mMaxSize;

	SetSelfCollision(data.mSelfCollision);
	SetAddedToScene(data.mAddedToScene);

	SetID(data.mID);
}

bool AggregateChunk::Export(CustomArray& array)
{
	BaseChunk::Export(array);
	mAggregateCore.Export(array);

	array.Store(mMaxSize);

	::Export(mActors, array);

	return true;
}

bool AggregateChunk::Import(const VirtualFile& file)
{
	BaseChunk::Import(file);
	mAggregateCore.Import(file);

	mMaxSize = file.ReadDword();

	::Import(mActors, file);

	return true;
}

///////////////////////////////////////////////////////////////////////////////

/*#define DESC_VERSION	1

DescChunk::DescChunk()
{
	mChunkType			= DescType;
	mDescCore.mVersion	= DESC_VERSION;
}

DescChunk::~DescChunk()
{
}

const char* DescChunk::Validate(){ return BaseChunk::Validate();}

bool DescChunk::Export(CustomArray& array)
{
	BaseChunk::Export(array);
	mDescCore.Export(array);
	return true;
}

bool DescChunk::Import(const VirtualFile& file)
{
	BaseChunk::Import(file);
	mDescCore.Import(file);
	return true;
}*/

///////////////////////////////////////////////////////////////////////////////

#define SCENE_VERSION	2
//#define SCENE_VERSION	1
IMPLEMENT_PEEL_CHUNK(SceneChunk, BaseChunk)

SceneChunk::SceneChunk(const PINT_WORLD_CREATE& create)
{
/*	mChunkType					= SceneType;
	mSceneCore.mVersion			= SCENE_VERSION;
	//
	SetName(create.GetTestName());
	mGlobalBounds				= create.mGlobalBounds;
	mGravity					= create.mGravity;
	mNbSimulateCallsPerFrame	= create.mNbSimulateCallsPerFrame;
	mTimestep					= create.mTimestep;
	CopyMemory(mCamera, create.mCamera, PINT_MAX_CAMERA_POSES*sizeof(PintCameraPose));

	SetCreateDefaultEnv(create.mCreateDefaultEnvironment);*/
	Init(create);
}

SceneChunk::SceneChunk(const SceneData& data)
{
	mChunkType					= SceneType;
	mSceneCore.mVersion			= SCENE_VERSION;
	//
	SetName(data.mName);
	mGlobalBounds				= data.mGlobalBounds;
	mGravity					= data.mGravity;
	mNbSimulateCallsPerFrame	= data.mNbSimulateCallsPerFrame;
	mTimestep					= data.mTimestep;
	CopyMemory(mCamera, data.mCamera, PINT_MAX_CAMERA_POSES*sizeof(PintCameraPose));

	SetID(data.mID);

	SetCreateDefaultEnv(data.mCreateDefaultEnvironment);

//	mDesc.SetName(data.mDesc);
	mDesc = data.mDesc;
}

void SceneChunk::Init(const PINT_WORLD_CREATE& create)
{
	mChunkType					= SceneType;
	mSceneCore.mVersion			= SCENE_VERSION;
	//
	SetName(create.GetTestName());
	mGlobalBounds				= create.mGlobalBounds;
	mGravity					= create.mGravity;
	mNbSimulateCallsPerFrame	= create.mNbSimulateCallsPerFrame;
	mTimestep					= create.mTimestep;
	CopyMemory(mCamera, create.mCamera, PINT_MAX_CAMERA_POSES*sizeof(PintCameraPose));

	SetCreateDefaultEnv(create.mCreateDefaultEnvironment);
}

bool SceneChunk::Export(CustomArray& array)
{
	BaseChunk::Export(array);
	mSceneCore.Export(array);
	::Export(mGlobalBounds, array);
	::Export(mGravity, array);
	array
		.Store(mNbSimulateCallsPerFrame)
		.Store(mTimestep)
		.Store(&mCamera, PINT_MAX_CAMERA_POSES*sizeof(PintCameraPose));
//	return mDesc.Export(array);
	::Export(mDesc, array);
	return true;
}

bool SceneChunk::Import(const VirtualFile& file)
{
	BaseChunk::Import(file);
	mSceneCore.Import(file);
	::Import(mGlobalBounds, file);
	::Import(mGravity, file);
	mNbSimulateCallsPerFrame = file.ReadDword();
	mTimestep = file.ReadFloat();
	file.ReadBuffer(&mCamera, PINT_MAX_CAMERA_POSES*sizeof(PintCameraPose));
	if(mSceneCore.mVersion>1)
//		return mDesc.Import(file);
		::Import(mDesc, file);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

