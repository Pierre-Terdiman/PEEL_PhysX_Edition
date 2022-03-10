///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_ZCB2_H
#define PINT_ZCB2_H

#include "..\Pint.h"
#include "PhysicsData.h"

#define ZCB2_API

// Ignore unnecessary headers that don't compile on Win64
#define ICERENDERSTATEMANAGER_H
#define ICESTATEBLOCK_H
#define ICERENDERCORE_H
#define ICERENDERABLESURFACE_H
#define ICEVOIDRENDERER_H
#define ICEMOTIONCELL_H
#define ICEMOTIONSTATE_H
#define ICESTATEMACHINE_H
#define ICESKELETON_H
#define ICESTATEMACHINECOMPILER_H
#define ICESKIN_H

#include "ZCB2\ZCB2AFX.h"
//#include "ZCB2\ZCB2.h"
using namespace ZCB2;

	// We need FOURCC codes for each chunk. They don't have to be readable within the binary stream
	// so we just define a PEEL-related enumeration and roll with it. The only requirement is that
	// they shouldn't match the codes for ICE native chunks.
	enum FourCCType
	{
		MaterialType		= 'PL00',
		ShapeType			= 'PL01',
		SphereShapeType		= 'PL02',
		CapsuleShapeType	= 'PL03',
		CylinderShapeType	= 'PL04',
		BoxShapeType		= 'PL05',
		ConvexShapeType		= 'PL06',
		MeshShapeType		= 'PL07',
		ActorType			= 'PL08',
		SceneType			= 'PL09',
		CollisionGroupsType	= 'PL10',
		JointType			= 'PL11',
		FixedJointType		= 'PL12',
		SphericalJointType	= 'PL13',
		DistanceJointType	= 'PL14',
		PrismaticJointType	= 'PL15',
		HingeJointType		= 'PL16',
		D6JointType			= 'PL17',
		ConvexType			= 'PL18',
		MeshType			= 'PL19',
		MeshShape2Type		= 'PL20',
		AggregateType		= 'PL21',
		Hinge2JointType		= 'PL22',
		DescType			= 'PL23',
		GearJointType		= 'PL24',
		RackJointType		= 'PL25',
	};

	#define DECLARE_PEEL_CHUNK(current_class, pint_create, core)	\
		DECLARE_CHUNK(current_class, core)							\
		current_class(const pint_create& create);

	///////////////////////////////////////////////////////////////////////////

	class CollisionGroupsChunk : public BaseChunk
	{
		DECLARE_CHUNK(CollisionGroupsChunk, mCollisionGroupsCore)
		CollisionGroupsChunk(const CollisionGroupsData&);

		CollisionGroupsChunk(udword nb_groups, const PintDisabledGroups* groups);
		Container	mGroups;

		protected:
		void		Init(udword nb_groups, const PintDisabledGroups* groups);
	};

	///////////////////////////////////////////////////////////////////////////

	class MaterialChunk : public BaseChunk
	{
		DECLARE_PEEL_CHUNK(MaterialChunk, PINT_MATERIAL_CREATE, mMaterialCore)
		MaterialChunk(const MaterialData&);

		float	mStaticFriction;
		float	mDynamicFriction;
		float	mRestitution;

		inline_	bool	IsTheSameAs(const PINT_MATERIAL_CREATE& create)	const
		{
			if(mStaticFriction!=create.mStaticFriction)
				return false;
			if(mDynamicFriction!=create.mDynamicFriction)
				return false;
			if(mRestitution!=create.mRestitution)
				return false;
			return true;
		}

		protected:
		void	Init(const char* name, float static_friction, float dynamic_friction, float restitution);
	};

	///////////////////////////////////////////////////////////////////////////

	enum ShapeChunkFlags
	{
		SHAPE_CHUNK_FLAG_COLOR	= (1<<0),	// mColor field is used
	};

	class ShapeChunk : public BaseChunk
	{
		DECLARE_PEEL_CHUNK(ShapeChunk, PINT_SHAPE_CREATE, mShapeCore)
		ShapeChunk(const ShapeData&);

		Quat		mLocalRot;
		Point		mLocalPos;
		PintShape	mType;
		udword		mMaterialID;
		udword		mRendererID;
		RGBAColor	mColor;

/*		inline_	bool	IsTheSameAs(const PINT_SHAPE_CREATE& create)	const
		{
			if(mLocalRot!=create.mLocalRot)
				return false;
			if(mLocalPos!=create.mLocalPos)
				return false;
			if(mMaterialID!=create.mMaterialID)
				return false;
			return true;
		}*/
		protected:
		void		InitBase(const char* name, const Point& local_pos, const Quat& local_rot, PintShapeRenderer* renderer);
	};

	class SphereShapeChunk : public ShapeChunk
	{
		DECLARE_PEEL_CHUNK(SphereShapeChunk, PINT_SPHERE_CREATE, mSphereShapeCore)
		SphereShapeChunk(const SphereShapeData&);

		float	mRadius;

		protected:
		void	Init(float radius);
	};

	class CapsuleShapeChunk : public ShapeChunk
	{
		DECLARE_PEEL_CHUNK(CapsuleShapeChunk, PINT_CAPSULE_CREATE, mCapsuleShapeCore)
		CapsuleShapeChunk(const CapsuleShapeData&);

		float	mRadius;
		float	mHalfHeight;

		protected:
		void	Init(float radius, float half_height);
	};

	class CylinderShapeChunk : public ShapeChunk
	{
		DECLARE_PEEL_CHUNK(CylinderShapeChunk, PINT_CYLINDER_CREATE, mCylinderShapeCore)
		CylinderShapeChunk(const CylinderShapeData&);

		float	mRadius;
		float	mHalfHeight;

		protected:
		void	Init(float radius, float half_height);
	};

	class BoxShapeChunk : public ShapeChunk
	{
		DECLARE_PEEL_CHUNK(BoxShapeChunk, PINT_BOX_CREATE, mBoxShapeCore)
		BoxShapeChunk(const BoxShapeData&);

		Point	mExtents;

		protected:
		void	Init(const Point& extents);
	};

	class ConvexShapeChunk : public ShapeChunk
	{
		DECLARE_PEEL_CHUNK(ConvexShapeChunk, PINT_CONVEX_CREATE, mConvexShapeCore)
		ConvexShapeChunk(const ConvexShapeData&);

		PNTSChunk	mConvexData;

		protected:
		void		Init(udword nb_verts, const Point* verts);
	};

	class MeshShapeChunk : public ShapeChunk
	{
		DECLARE_PEEL_CHUNK(MeshShapeChunk, PINT_MESH_CREATE, mMeshShapeCore)
		MeshShapeChunk(const MeshShapeData&);

		PNTSChunk	mVertexData;
		FACEChunk	mTriangleData;

		protected:
		void		Init(udword nb_verts, const Point* verts, udword nb_faces, const udword* dfaces, const uword* wfaces);
	};

	class MeshShapeChunk2 : public ShapeChunk
	{
		DECLARE_PEEL_CHUNK(MeshShapeChunk2, PINT_MESH_CREATE2, mMeshShapeCore)
		MeshShapeChunk2(const MeshShapeData2&);

		udword	mMeshID;

		protected:
		void	Init();
	};

	class ConvexDataChunk : public BaseChunk
	{
		DECLARE_PEEL_CHUNK(ConvexDataChunk, PINT_CONVEX_DATA_CREATE, mConvexCore)
		ConvexDataChunk(const ConvexMeshData&);

		PNTSChunk	mConvexData;

		protected:
		void		Init(udword nb_verts, const Point* verts);
	};

	class MeshDataChunk : public BaseChunk
	{
		DECLARE_PEEL_CHUNK(MeshDataChunk, PINT_MESH_DATA_CREATE, mMeshCore)
		MeshDataChunk(const MeshData&);

		PNTSChunk	mVertexData;
		FACEChunk	mTriangleData;

		protected:
		void	Init(udword nb_verts, const Point* verts, udword nb_faces, const udword* dfaces, const uword* wfaces);
	};

	///////////////////////////////////////////////////////////////////////////

	class JointChunk : public BaseChunk
	{
		DECLARE_PEEL_CHUNK(JointChunk, PINT_JOINT_CREATE, mJointCore)
		JointChunk(const JointData&);

//		PintJoint		mType;	// Redundant with chunk type
//		PintActorHandle	mObject0;
//		PintActorHandle	mObject1;
		udword			mObject0;
		udword			mObject1;
	};

	class SphericalJointChunk : public JointChunk
	{
		DECLARE_PEEL_CHUNK(SphericalJointChunk, PINT_SPHERICAL_JOINT_CREATE, mSphericalJointCore)
		SphericalJointChunk(const SphericalJointData&);

		PR			mLocalPivot0;
		PR			mLocalPivot1;
		PintLimits	mLimits;
	};

	class FixedJointChunk : public JointChunk
	{
		DECLARE_PEEL_CHUNK(FixedJointChunk, PINT_FIXED_JOINT_CREATE, mFixedJointCore)
		FixedJointChunk(const FixedJointData&);

		Point	mLocalPivot0;
		Point	mLocalPivot1;
	};

	class DistanceJointChunk : public JointChunk
	{
		DECLARE_PEEL_CHUNK(DistanceJointChunk, PINT_DISTANCE_JOINT_CREATE, mDistanceJointCore)
		DistanceJointChunk(const DistanceJointData&);

		Point		mLocalPivot0;
		Point		mLocalPivot1;
		PintLimits	mLimits;
	};

	class PrismaticJointChunk : public JointChunk
	{
		DECLARE_PEEL_CHUNK(PrismaticJointChunk, PINT_PRISMATIC_JOINT_CREATE, mPrismaticJointCore)
		PrismaticJointChunk(const PrismaticJointData&);

		PR			mLocalPivot0;
		PR			mLocalPivot1;
		Point		mLocalAxis0;
		Point		mLocalAxis1;
		PintLimits	mLimits;
		PintSpring	mSpring;
	};

	class HingeJointChunk : public JointChunk
	{
		DECLARE_PEEL_CHUNK(HingeJointChunk, PINT_HINGE_JOINT_CREATE, mHingeJointCore)
		HingeJointChunk(const HingeJointData&);

		Point		mLocalPivot0;
		Point		mLocalPivot1;
		Point		mLocalAxis0;
		Point		mLocalAxis1;
		PintLimits	mLimits;
		Point		mGlobalAnchor;
		Point		mGlobalAxis;
		bool		mUseMotor;
		float		mDriveVelocity;
	};

	class Hinge2JointChunk : public JointChunk
	{
		DECLARE_PEEL_CHUNK(Hinge2JointChunk, PINT_HINGE2_JOINT_CREATE, mHinge2JointCore)
		Hinge2JointChunk(const Hinge2JointData&);

		PR			mLocalPivot0;
		PR			mLocalPivot1;
		PintLimits	mLimits;
		bool		mUseMotor;	// TODO: use DECLARE_STD_FLAG
		float		mDriveVelocity;
	};

	class D6JointChunk : public JointChunk
	{
		DECLARE_PEEL_CHUNK(D6JointChunk, PINT_D6_JOINT_CREATE, mD6JointCore)
		D6JointChunk(const D6JointData&);

		PR		mLocalPivot0;
		PR		mLocalPivot1;
		AABB	mLinearLimits;
		float	mMinTwist;
		float	mMaxTwist;
		float	mMaxSwingY;
		float	mMaxSwingZ;
		udword	mMotorFlags;
		float	mMotorStiffness;
		float	mMotorDamping;
	};

	class GearJointChunk : public JointChunk
	{
		DECLARE_PEEL_CHUNK(GearJointChunk, PINT_GEAR_JOINT_CREATE, mGearJointCore)
		GearJointChunk(const GearJointData&);

		udword	mHinge0;
		udword	mHinge1;
		PR		mLocalPivot0;
		PR		mLocalPivot1;
		float	mGearRatio;
//		float	mRadius0;
//		float	mRadius1;
//		float	mErrorSign;
	};

	class RackJointChunk : public JointChunk
	{
		DECLARE_PEEL_CHUNK(RackJointChunk, PINT_RACK_AND_PINION_JOINT_CREATE, mRackJointCore)
		RackJointChunk(const RackJointData&);

		udword	mHinge;
		udword	mPrismatic;
		PR		mLocalPivot0;
		PR		mLocalPivot1;
		udword	mNbRackTeeth;
		udword	mNbPinionTeeth;
		float	mRackLength;
//		float	mErrorSign;
	};

	///////////////////////////////////////////////////////////////////////////

	enum ActorChunkFlags
	{
		ACTOR_CHUNK_FLAG_KINEMATIC		= (1<<0),
		ACTOR_CHUNK_FLAG_ADD_TO_WORLD	= (1<<1),
		ACTOR_CHUNK_FLAG_HIDDEN			= (1<<2),
	};

	class ActorChunk : public BaseChunk
	{
		DECLARE_PEEL_CHUNK(ActorChunk, PINT_OBJECT_CREATE, mActorCore)
		DECLARE_STD_FLAG(Kinematic,		ACTOR_CHUNK_FLAG_KINEMATIC)
		DECLARE_STD_FLAG(AddToWorld,	ACTOR_CHUNK_FLAG_ADD_TO_WORLD)
		DECLARE_STD_FLAG(Hidden,		ACTOR_CHUNK_FLAG_HIDDEN)
		ActorChunk(const ActorData&, bool);

		Container			mShapes;
		Point				mPosition;
		Quat				mRotation;
		Point				mCOMLocalOffset;
		Point				mLinearVelocity;
		Point				mAngularVelocity;
		float				mMass;
		float				mMassForInertia;
		PintCollisionGroup	mCollisionGroup;
	};

	enum AggregateChunkFlags
	{
		AGGREGATE_CHUNK_FLAG_SELF_COLLISIONS	= (1<<0),
		AGGREGATE_CHUNK_FLAG_ADDED_TO_SCENE		= (1<<1),
	};

	class AggregateChunk : public BaseChunk
	{
		DECLARE_PEEL_CHUNK(AggregateChunk, PINT_AGGREGATE_CREATE, mAggregateCore)
		DECLARE_STD_FLAG(SelfCollision,		AGGREGATE_CHUNK_FLAG_SELF_COLLISIONS)
		DECLARE_STD_FLAG(AddedToScene,		AGGREGATE_CHUNK_FLAG_ADDED_TO_SCENE)
		AggregateChunk(const AggregateData&);

		Container			mActors;
		udword				mMaxSize;
	};

	enum SceneChunkFlags
	{
		SCENE_CHUNK_FLAG_CREATE_DEFAULT_ENV	= (1<<0),
	};

/*	class DescChunk : public BaseChunk
	{
		DECLARE_CHUNK(DescChunk,	mDescCore)
	};*/

	class SceneChunk : public BaseChunk
	{
		DECLARE_PEEL_CHUNK(SceneChunk, PINT_WORLD_CREATE, mSceneCore)
		DECLARE_STD_FLAG(CreateDefaultEnv,	SCENE_CHUNK_FLAG_CREATE_DEFAULT_ENV)
		SceneChunk(const SceneData&);

		void			Init(const PINT_WORLD_CREATE& create);

//		DescChunk		mDesc;
		String			mDesc;

		AABB			mGlobalBounds;
		PintCameraPose	mCamera[PINT_MAX_CAMERA_POSES];
		Point			mGravity;
		udword			mNbSimulateCallsPerFrame;
		float			mTimestep;
	};

	///////////////////////////////////////////////////////////////////////////

	class ZB2CustomizeCallback
	{
		public:
						ZB2CustomizeCallback()	{}
		virtual			~ZB2CustomizeCallback()	{}

		virtual	void	CustomizeMaterial(PINT_MATERIAL_CREATE&)	= 0;
		virtual	void	CustomizeMesh(PINT_MESH_DATA_CREATE&)		= 0;
		virtual	void	CustomizeShape(PINT_SHAPE_CREATE&)			= 0;
		virtual	void	CustomizeActor(PINT_OBJECT_CREATE&)			= 0;
		virtual	void	CustomizeJoint(PINT_JOINT_CREATE&)			= 0;
	};

	class ZCB2Factory : public ZCB2Importer
	{
		public:
											ZCB2Factory();
		virtual								~ZCB2Factory();

				void						Reset();

//		virtual	bool						NewChunk(const BaseChunk* chunk);
		virtual	void						NewUnknownChunk(udword type, const char* name, const VirtualFile& file);
		virtual	PintShapeRenderer*			GetShapeRenderer(udword id)	{ return null;	}

		virtual	void						ReleasePintIndependentData();

				struct ActorCreate
				{
					inline_	ActorCreate() : mCreate(null), mFlags(0)	{}

					inline_	void	Init(const PINT_OBJECT_CREATE* create, udword flags)
					{
						mCreate	= create;
						mFlags	= flags;
					}

					inline_	void	Release()
					{
						DELETESINGLE(mCreate);
					}

					const PINT_OBJECT_CREATE*	mCreate;
					udword						mFlags;
				};

		inline_	udword						GetNbActors()		const	{ return mActorsCreate.GetNbEntries()/(sizeof(ActorCreate)/sizeof(udword));	}
		inline_	const ActorCreate*			GetActors()			const	{ return (const ActorCreate*)mActorsCreate.GetEntries();					}
		inline_	ActorCreate*				AddActor(const PINT_OBJECT_CREATE* create, udword flags)
											{
												ActorCreate* NewActor = ICE_RESERVE(ActorCreate, mActorsCreate);
												NewActor->Init(create, flags);		
												return NewActor;
											}

		inline_	udword						GetNbShapes()		const	{ return mShapeCreate.GetNbEntries();								}
		inline_	const PINT_SHAPE_CREATE**	GetShapes()			const	{ return (const PINT_SHAPE_CREATE**)mShapeCreate.GetEntries();		}

		inline_	udword						GetNbAggregates()	const	{ return mAggregateChunks.GetNbEntries();							}
		inline_	const AggregateChunk**		GetAggregates()		const	{ return (const AggregateChunk**)mAggregateChunks.GetEntries();		}

		public:
				ZB2CustomizeCallback*		mCustomize;
				PtrContainer				mMaterials;
				Container					mShapeColors;
		protected:	// Allow access for ZCB2FactoryEx
				PtrContainer				mShapeCreate;	// PINT_SHAPE_CREATE pointers
				Container					mActorsCreate;	// ActorCreate structures
//				PtrContainer				mAggregates;
				PtrContainer				mAggregateChunks;				
		public:
				PtrContainer				mJoints;
//				Container					mJointsActorIndices;
				PtrContainer				mConvexChunks;
				PtrContainer				mMeshChunks;
				PtrContainer				mMeshObjects;				
				PtrContainer				mMeshDataChunks;				
				udword						mNbDisabledGroups;
				PintDisabledGroups*			mDisabledGroups;
				//PINT_WORLD_CREATE			mScene;
				SceneChunk					mScene;
				Strings						mNames;

				struct ZCB2MeshRecord
				{
					PINT_MESH_CREATE2*		mShapeCreate_;
					udword					mMeshID;
				};
				Container					mZCB2MeshRecords;

				void						SetupShape(PINT_SHAPE_CREATE* create, const ShapeChunk& chunk, const char* name);
	};

#endif
