///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_H
#define PINT_H

#include "PintGUIHelper.h"
#include "PintDef.h"
#include "PintRenderPass.h"
#include "PintSurfaceInterface.h"

	class Pint;
	class PintSQ;
	class PintShapeRenderer;

	#define PINT_MAX_CAMERA_POSES	16

	enum PintShape
	{
		PINT_SHAPE_UNDEFINED,
		PINT_SHAPE_SPHERE,
		PINT_SHAPE_CAPSULE,
		PINT_SHAPE_CYLINDER,
		PINT_SHAPE_BOX,
		PINT_SHAPE_CONVEX,
		PINT_SHAPE_MESH,	// Legacy API
		PINT_SHAPE_MESH2,	// New API
		PINT_SHAPE_HEIGHTFIELD,

		PINT_SHAPE_FORCE_DWORD	= 0x7fffffff
	};

	const char* GetPintShapeName(PintShape type);

	enum PintJoint
	{
		PINT_JOINT_UNDEFINED,
		PINT_JOINT_SPHERICAL,		// Spherical joint a.k.a. point-to-point constraint a.k.a. ball-and-socket
		PINT_JOINT_HINGE,			// Hinge joints a.k.a. revolute joints
		PINT_JOINT_HINGE2,			// test
		PINT_JOINT_PRISMATIC,		// Prismatic joints, a.k.a. slider constraints
		PINT_JOINT_FIXED,			// Fixed joints
		PINT_JOINT_DISTANCE,		// Distance joints
		PINT_JOINT_D6,				// D6 joints
		PINT_JOINT_GEAR,			// Gear joints
		PINT_JOINT_RACK_AND_PINION,	// Rack-and-pinion joints

		PINT_JOINT_CHAIN,			// Chain joints
		PINT_JOINT_PORTAL,

		PINT_JOINT_NB,				// Number of available joint types

		PINT_JOINT_FORCE_DWORD	= 0x7fffffff
	};

	const char* GetPintJointName(PintJoint type);

	/*enum PintActionType
	{
		PINT_ACTION_FORCE,
		PINT_ACTION_IMPULSE,

		PINT_ACTION_FORCE_DWORD	= 0x7fffffff
	};*/

	struct PintCameraPose
	{
				PintCameraPose() :  mPos(50.0f, 50.0f, 50.0f), mDir(-0.6f, -0.2f, -0.7f)	{}
				PintCameraPose(const Point& pos, const Point& dir) : mPos(pos), mDir(dir)	{}

		bool	operator == (const PintCameraPose& other)	const
		{
			return mPos==other.mPos && mDir==other.mDir;
		}

		bool	operator != (const PintCameraPose& other)	const
		{
			return mPos!=other.mPos || mDir!=other.mDir;
		}

		Point	mPos;
		Point	mDir;
	};

	struct PintContactData
	{
		PintActorHandle	mObject0;
		PintActorHandle	mObject1;

		Point			mWorldPos;
		float			mSeparation;

		Point			mNormal;
		udword			mInternalFaceIndex0;

		Point			mImpulse;
		udword			mInternalFaceIndex1;
	};

/*	class PintFilterCallback
	{
		public:
		virtual	bool	PairFound(Pint& pint, PintActorHandle actor0, PintShapeHandle shape0, PintActorHandle actor1, PintShapeHandle shape1)	= 0;
	};*/

	class PintContactNotifyCallback
	{
		public:
		// True to buffer contacts and send them after simulation is completed,
		// false to send contacts as soon as they're available.
		virtual	bool	BufferContacts()		const												= 0;

		enum _
		{
			CONTACT_FOUND	= (1<<0),
			CONTACT_PERSIST	= (1<<1),
			CONTACT_LOST	= (1<<2),
			CONTACT_ALL		= CONTACT_FOUND|CONTACT_PERSIST|CONTACT_LOST
		};
		virtual	udword	GetContactFlags()		const												= 0;

		virtual	float	GetContactThreshold()	const												= 0;
		virtual	void	OnContact(Pint& pint, udword nb_contacts, const PintContactData* contacts)	= 0;
	};

	class PintContactModifyCallback
	{
		public:

		// Called for each contact pair. Returns true to enable further contact processing for the pair.
		virtual	bool	PrepContactModify(Pint& pint, udword nb_contacts, PintActorHandle h0, PintActorHandle h1, PintShapeHandle s0, PintShapeHandle s1)	= 0;

		enum ContactModif
		{
			CONTACT_AS_IS,
			CONTACT_MODIFY,
			CONTACT_IGNORE,
			CONTACT_DELAYED,
		};

		virtual	ContactModif	ModifyContact(Pint& pint, const PR& pose0, const PR& pose1, Point& p, Point& n, float& s, udword feature0, udword feature1)	= 0;
		virtual	ContactModif	ModifyDelayedContact(Pint& pint, const PR& pose0, const PR& pose1, Point& p, Point& n, float& s, udword feature0, udword feature1, udword index)	= 0;
	};

	class PintContactModifyProvider
	{
		public:

		struct ContactPairData
		{
			PintActorHandle	mActor0;
			PintActorHandle	mActor1;
			PintShapeHandle	mShape0;
			PintShapeHandle	mShape1;
			udword			mNbContacts;
		};

		virtual	PintContactModifyPairHandle	GetContactPairData(udword pair_index, ContactPairData& data, PR* pose0, PR* pose1)	= 0;

		struct ContactData
		{
			Point	mPos;
			float	mSeparation;
			Point	mNormal;
			udword	mInternalFaceIndex1;
		};
		virtual	void	GetContactData(PintContactModifyPairHandle handle, ContactData* data, udword nb_contacts)				= 0;

		virtual	void	IgnoreContact(PintContactModifyPairHandle handle, udword contact_index)									= 0;
		virtual	void	SetContactData(PintContactModifyPairHandle handle, udword contact_index, const ContactData& data)		= 0;
	};

	class PintContactModifyCallback2
	{
		public:

		virtual	void	ModifyContacts(Pint& pint, udword nb_pairs, PintContactModifyProvider& provider)	= 0;
	};

	//! Contains scene-related parameters. This is used to initialize each PINT engine, *before* the test itself is setup.
	class PINT_WORLD_CREATE : public Allocateable
	{
		protected:
		const char*					mTestName;	// Setup by the system
//		const char*					mTestDesc;	// Setup by the system
		public:
									PINT_WORLD_CREATE() :
										mTestName					(null),
//										mTestDesc					(null),
										mGravity					(0.0f, 0.0f, 0.0f),
										mNbSimulateCallsPerFrame	(1),
										mTimestep					(1.0f/60.0f),
										mCreateDefaultEnvironment	(false),
//										mFilterCallback				(null),
										mContactNotifyCallback		(null),
										mContactModifyCallback		(null),
										mContactModifyCallback2		(null)
									{
										mGlobalBounds.SetEmpty();
									}

		// If set by the test, this is used to setup the broadphase bounds in engines that need them. Otherwise some
		// default bounds are used. Fine-tuning the bounds may improve the broadphase performance.
				AABB				mGlobalBounds;

		// Camera poses for current test. You can define up to PINT_MAX_CAMERA_POSES poses per test. Just write them
		// sequentially in the array, PEEL will automatically find how many there are.
				PintCameraPose		mCamera[PINT_MAX_CAMERA_POSES];

		// Gravity vector for current test.
				Point				mGravity;

		// Number of simulate calls per render frame. It is usually 1 but it can be set to more than 1 to
		// artificially speed up the test scene.
				udword				mNbSimulateCallsPerFrame;

		// Timestep for one simulate call. It is usually 1/60 (for 60Hz).
				float				mTimestep;

		// True to create the "default environment" i.e. the ground plane.
				bool				mCreateDefaultEnvironment;

		// XP
		// Above are engine-agnostic parameters that can easily map to all engines.
		// Below are engine-specific parameters that might only make sense for some engines.
//		PintFilterCallback*			mFilterCallback;
		PintContactNotifyCallback*	mContactNotifyCallback;
		PintContactModifyCallback*	mContactModifyCallback;
		PintContactModifyCallback2*	mContactModifyCallback2;

		inline	const char*			GetTestName()	const	{ return mTestName;	}
//		inline	const char*			GetTestDesc()	const	{ return mTestDesc;	}
	};

	struct PINT_MATERIAL_CREATE	: public Allocateable
	{
					PINT_MATERIAL_CREATE(float static_friction=0.0f, float dynamic_friction=0.0f, float restitution=0.0f) :
						mName				(null),
						mStaticFriction		(static_friction),
						mDynamicFriction	(dynamic_friction),
						mRestitution		(restitution)
					{
					}

					PINT_MATERIAL_CREATE(const PINT_MATERIAL_CREATE& material) :
						mName				(material.mName),
						mStaticFriction		(material.mStaticFriction),
						mDynamicFriction	(material.mDynamicFriction),
						mRestitution		(material.mRestitution)
					{
					}

		const char*	mName;
		float		mStaticFriction;
		float		mDynamicFriction;
		float		mRestitution;
	};

	///////////////////////////////////////////////////////////////////////////

	enum ShapeSharing
	{
		SHAPE_SHARING_UNDEFINED,	// Let the per-plugin UI decide
		SHAPE_SHARING_YES,
		SHAPE_SHARING_NO
	};

	struct PINT_SHAPE_CREATE : public Allocateable
	{
											PINT_SHAPE_CREATE(PintShape type) :
												mName		(null),
												mType		(type),
												mMaterial	(null),
												mRenderer	(null),
												mSharing	(SHAPE_SHARING_UNDEFINED),
												mNext		(null)
											{
												mLocalPos.Zero();
												mLocalRot.Identity();
											}

											PINT_SHAPE_CREATE(const PINT_SHAPE_CREATE& desc) :
												mName		(desc.mName),
												mType		(desc.mType),
												mLocalPos	(desc.mLocalPos),
												mLocalRot	(desc.mLocalRot),
												mMaterial	(desc.mMaterial),
												mRenderer	(desc.mRenderer),
												mSharing	(desc.mSharing),
												mNext		(desc.mNext)
											{
											}

		inline_		bool					CanShare(bool defaultValue)	const
											{
												if(mSharing==SHAPE_SHARING_YES)
													return true;
												if(mSharing==SHAPE_SHARING_NO)
													return false;
												return defaultValue;
											}

		const char*							mName;
		const PintShape						mType;
		Point								mLocalPos;
		Quat								mLocalRot;
		const PINT_MATERIAL_CREATE*			mMaterial;
		mutable PintShapeRenderer*			mRenderer;
		ShapeSharing						mSharing;

		inline_	const PINT_SHAPE_CREATE*	_GetNext()						const	{ return mNext;	}
		inline_	void						SetNext(const PINT_SHAPE_CREATE* next)	{ mNext = next;	}

		private:
		const PINT_SHAPE_CREATE*			mNext;
	};

	struct PINT_SPHERE_CREATE : PINT_SHAPE_CREATE
	{
				PINT_SPHERE_CREATE(float radius=0.0f) :
					PINT_SHAPE_CREATE	(PINT_SHAPE_SPHERE),
					mRadius				(radius)
				{
				}

				PINT_SPHERE_CREATE(const PINT_SPHERE_CREATE& desc) :
					PINT_SHAPE_CREATE	(desc),
					mRadius				(desc.mRadius)
				{
				}

		float	mRadius;
	};

	struct PINT_CAPSULE_CREATE : PINT_SHAPE_CREATE
	{
				PINT_CAPSULE_CREATE(float radius=0.0f, float half_height=0.0f) :
					PINT_SHAPE_CREATE	(PINT_SHAPE_CAPSULE),
					mRadius				(radius),
					mHalfHeight			(half_height)
				{
				}

				PINT_CAPSULE_CREATE(const PINT_CAPSULE_CREATE& desc) :
					PINT_SHAPE_CREATE	(desc),
					mRadius				(desc.mRadius),
					mHalfHeight			(desc.mHalfHeight)
				{
				}

		float	mRadius;
		float	mHalfHeight;
	};

	struct PINT_CYLINDER_CREATE : PINT_SHAPE_CREATE
	{
				PINT_CYLINDER_CREATE(float radius=0.0f, float half_height=0.0f) :
					PINT_SHAPE_CREATE	(PINT_SHAPE_CYLINDER),
					mRadius				(radius),
					mHalfHeight			(half_height)
				{
				}

				PINT_CYLINDER_CREATE(const PINT_CYLINDER_CREATE& desc) :
					PINT_SHAPE_CREATE	(desc),
					mRadius				(desc.mRadius),
					mHalfHeight			(desc.mHalfHeight)
				{
				}

		float	mRadius;
		float	mHalfHeight;
	};

	struct PINT_BOX_CREATE : PINT_SHAPE_CREATE
	{
				PINT_BOX_CREATE(float x=0.0f, float y=0.0f, float z=0.0f) :
					PINT_SHAPE_CREATE	(PINT_SHAPE_BOX),
					mExtents			(x, y, z)
				{
				}

				PINT_BOX_CREATE(const Point& extents) :
					PINT_SHAPE_CREATE	(PINT_SHAPE_BOX),
					mExtents			(extents)
				{
				}

				PINT_BOX_CREATE(const PINT_BOX_CREATE& desc) :
					PINT_SHAPE_CREATE	(desc),
					mExtents			(desc.mExtents)
				{
				}

		Point	mExtents;
	};

	struct PINT_CONVEX_DATA_CREATE// : public Allocateable
	{
						PINT_CONVEX_DATA_CREATE(udword nb_verts=0, const Point* verts=null) :
							mNbVerts	(nb_verts),
							mVerts		(verts)
						{
						}

						PINT_CONVEX_DATA_CREATE(const PINT_CONVEX_DATA_CREATE& desc) :
							mNbVerts	(desc.mNbVerts),
							mVerts		(desc.mVerts)
						{
						}

		udword			mNbVerts;
		const Point*	mVerts;
	};

	struct PINT_CONVEX_CREATE : PINT_SHAPE_CREATE, PINT_CONVEX_DATA_CREATE
	{
		PINT_CONVEX_CREATE(udword nb_verts=0, const Point* verts=null) :
			PINT_SHAPE_CREATE		(PINT_SHAPE_CONVEX),
			PINT_CONVEX_DATA_CREATE	(nb_verts, verts)
		{
		}

		PINT_CONVEX_CREATE(const PINT_CONVEX_CREATE& desc) :
			PINT_SHAPE_CREATE		(desc),
			PINT_CONVEX_DATA_CREATE	(desc.mNbVerts, desc.mVerts)
		{
		}
	};

	struct PINT_MESH_DATA_CREATE// : public Allocateable
	{
									PINT_MESH_DATA_CREATE() : mDynamic(false), mDeformable(false)
									{
									}

									PINT_MESH_DATA_CREATE(const PINT_MESH_DATA_CREATE& desc) :
										mSurface	(desc.mSurface),
										mDynamic	(desc.mDynamic),
										mDeformable	(desc.mDeformable)
									{
									}

		inline_	const PintSurfaceInterface&	GetSurface()	const	{ return mSurface;	}

		inline_	void				SetSurfaceData(udword nb_verts, const Point* verts, udword nb_faces, const udword* dfaces, const uword* wfaces)
									{
										mSurface.mNbVerts	= nb_verts;
										mSurface.mVerts		= verts;
										mSurface.mNbFaces	= nb_faces;
										mSurface.mDFaces	= dfaces;
										mSurface.mWFaces	= wfaces;
										mSurface.mCRC32_Verts	= ComputeCRC32_Verts(mSurface);
										mSurface.mCRC32_Faces	= ComputeCRC32_Faces(mSurface);
									}

		inline_	void				SetSurfaceData(const SurfaceInterface& surface)
									{
										mSurface.Init(surface);
									}

		inline_	void				RecomputeCRC32_Verts()	{ mSurface.mCRC32_Verts = ComputeCRC32_Verts(mSurface);	}

		private:
		PintSurfaceInterface		mSurface;
		public:
		bool						mDynamic;
		bool						mDeformable;
	};

	struct PINT_MESH_CREATE : PINT_SHAPE_CREATE, PINT_MESH_DATA_CREATE
	{
		PINT_MESH_CREATE() : PINT_SHAPE_CREATE(PINT_SHAPE_MESH)
		{
		}

		PINT_MESH_CREATE(const PINT_MESH_CREATE& desc) :
			PINT_SHAPE_CREATE		(desc),
			PINT_MESH_DATA_CREATE	(desc)
		{
		}
	};

	struct PINT_MESH_CREATE2 : PINT_SHAPE_CREATE
	{
						PINT_MESH_CREATE2(PintMeshHandle h=null) : PINT_SHAPE_CREATE(PINT_SHAPE_MESH2), mTriangleMesh(h)
						{
						}

						PINT_MESH_CREATE2(const PINT_MESH_CREATE2& desc) :
							PINT_SHAPE_CREATE	(desc),
							mTriangleMesh		(desc.mTriangleMesh)
						{
						}

		PintMeshHandle	mTriangleMesh;
	};

	struct PintHeightfieldData
	{
		float	mHeightScale;
		float	mMaxHeight;
		float	mMinHeight;
	};

	struct PINT_HEIGHTFIELD_DATA_CREATE// : public Allocateable
	{
						PINT_HEIGHTFIELD_DATA_CREATE(udword nbu=0, udword nbv=0, const float* heights=null, float v=0.0f) :
							mNbU			(nbu),
							mNbV			(nbv),
							mHeights		(heights),
							mUniqueValue	(v)
						{
						}

		udword			mNbU;
		udword			mNbV;
		const float*	mHeights;
		float			mUniqueValue;
	};

	struct PINT_HEIGHTFIELD_CREATE : PINT_SHAPE_CREATE
	{
								PINT_HEIGHTFIELD_CREATE(PintHeightfieldHandle h=null, float scale_u=1.0f, float scale_v=1.0f) :
									PINT_SHAPE_CREATE	(PINT_SHAPE_HEIGHTFIELD),
									mHeightfield		(h),
									mScaleU				(scale_u),
									mScaleV				(scale_v)
								{
								}

								PINT_HEIGHTFIELD_CREATE(const PINT_HEIGHTFIELD_CREATE& desc) :
									PINT_SHAPE_CREATE	(desc),
									mHeightfield		(desc.mHeightfield),
									mScaleU				(desc.mScaleU),
									mScaleV				(desc.mScaleV)
								{
								}

		PintHeightfieldHandle	mHeightfield;
		float					mScaleU;
		float					mScaleV;
	};

	class PintShapeEnumerateCallback
	{
		public:
		virtual	void	ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data)	= 0;
	};

	struct PINT_OBJECT_CREATE : public Allocateable
	{
											PINT_OBJECT_CREATE(const PINT_SHAPE_CREATE* shape = null) :
												mName			(null),
												mExtraShapes	(null),
												mShape			(shape),
												mMass			(0.0f),
												mMassForInertia	(-1.0f),
												mCollisionGroup	(0),
												mKinematic		(false),
												mAddToWorld		(true),
												mAddToDatabase	(true)
											{
												mPosition.Zero();
												mRotation.Identity();
												mLinearVelocity.Zero();
												mAngularVelocity.Zero();
												mCOMLocalOffset.Zero();
											}

											PINT_OBJECT_CREATE(const PINT_OBJECT_CREATE& desc) :
												mName			(desc.mName),
												mExtraShapes	(desc.mExtraShapes),
												mShape			(desc.mShape),
												mPosition		(desc.mPosition),
												mRotation		(desc.mRotation),
												mCOMLocalOffset	(desc.mCOMLocalOffset),
												mLinearVelocity	(desc.mLinearVelocity),
												mAngularVelocity(desc.mAngularVelocity),
												mMass			(desc.mMass),
												mMassForInertia	(desc.mMassForInertia),
												mCollisionGroup	(desc.mCollisionGroup),
												mKinematic		(desc.mKinematic),
												mAddToWorld		(desc.mAddToWorld),
												mAddToDatabase	(desc.mAddToDatabase)
											{
											}

											~PINT_OBJECT_CREATE()
											{
												DELETESINGLE(mExtraShapes);
											}

				const char*					mName;
		private:
				PtrContainer*				mExtraShapes;	// New safer codepath, mainly for compounds
				const PINT_SHAPE_CREATE*	mShape;			// Legacy shape / quick single-shape setup
		public:
				Point						mPosition;
				Quat						mRotation;
				Point						mCOMLocalOffset;
				Point						mLinearVelocity;
				Point						mAngularVelocity;
				float						mMass;
				float						mMassForInertia;	// If negative, use the same as mMass.
				PintCollisionGroup			mCollisionGroup;	// 0-31
				bool						mKinematic;
				bool						mAddToWorld;		// Add to scene. Typically false for aggregated parts.
				bool						mAddToDatabase;		// For the editor. Typically false for "default environment" parts.

		// Helper for new shape setup
				void						AddShape(const PINT_SHAPE_CREATE* shape)
											{
												if(!mShape)
												{
													ASSERT(!mExtraShapes);
													mShape = shape;
												}
												else
												{
													if(!mExtraShapes)
														mExtraShapes = ICE_NEW(PtrContainer);
													mExtraShapes->AddPtr(shape);
												}
											}

		inline_	const PtrContainer*			GetExtraShapes()	const	{ return mExtraShapes;	}

		// Should work for both new & legacy shape setup
				udword						GetNbShapes(PintShapeEnumerateCallback* callback=null, void* user_data=null)	const
											{
												udword NbShapes = 0;
												const PINT_SHAPE_CREATE* CurrentShape = mShape;
												while(CurrentShape)
												{
													const PINT_SHAPE_CREATE* NextShape = CurrentShape->_GetNext();

													if(callback)
														callback->ReportShape(*CurrentShape, NbShapes, user_data);

													NbShapes++;
													CurrentShape = NextShape;
												}

												if(mExtraShapes)
												{
													ASSERT(NbShapes==1);

													const udword NbExtraShapes = mExtraShapes->GetNbEntries();

													if(callback)
													{
														for(udword i=0;i<NbExtraShapes;i++)
															callback->ReportShape(*reinterpret_cast<const PINT_SHAPE_CREATE*>(mExtraShapes->GetEntry(i)), NbShapes+i, user_data);
													}

													NbShapes += NbExtraShapes;
												}

												return NbShapes;
											}

		// Helpers for legacy shape setup
		inline_	void						SetShape(const PINT_SHAPE_CREATE* shape)
											{
												ASSERT(!mExtraShapes);
												mShape = shape;
											}

		inline_	PINT_SHAPE_CREATE*			AddShape(PINT_SHAPE_CREATE* shape, PINT_SHAPE_CREATE* previous_shape)
											{
												if(!mShape)
												{
													ASSERT(!previous_shape);
													SetShape(shape);
												}
												else
												{
													ASSERT(previous_shape);
													previous_shape->SetNext(shape);
												}
												return shape;
											}

		inline_	const PINT_SHAPE_CREATE*	GetFirstShape()	const
											{
												return mShape;
											};
	};

	///////////////////////////////////////////////////////////////////////////

	struct PINT_AGGREGATE_CREATE : public Allocateable
	{
				PINT_AGGREGATE_CREATE(udword max_size, bool self_collision) :
					mMaxSize		(max_size),
					mSelfCollision	(self_collision)
				{
				}

				PINT_AGGREGATE_CREATE(const PINT_AGGREGATE_CREATE& desc) :
					mMaxSize		(desc.mMaxSize),
					mSelfCollision	(desc.mSelfCollision)
				{
				}

		udword	mMaxSize;
		bool	mSelfCollision;
	};

	///////////////////////////////////////////////////////////////////////////

	struct PintLimits
	{
		inline_	PintLimits(float m=0.0f, float M=0.0f) : mMinValue(m), mMaxValue(M)								{}
		inline_	PintLimits(const PintLimits& limits) : mMinValue(limits.mMinValue), mMaxValue(limits.mMaxValue)	{}

		inline_	void	Set(float m, float M)
		{
			mMinValue = m;
			mMaxValue = M;
		}

		float	mMinValue;
		float	mMaxValue;
	};

	inline_	bool	IsSphericalLimitEnabled(const PintLimits& limits)	{ return limits.mMinValue>0.0f && limits.mMaxValue>0.0f;	}
	inline_	void	SetSphericalLimitDisabled(PintLimits& limits)		{ limits.Set(-1.0f, -1.0f);									}

	inline_	bool	IsHingeLimitEnabled(const PintLimits& limits)		{ return limits.mMinValue<=limits.mMaxValue;	}
	inline_	void	SetHingeLimitDisabled(PintLimits& limits)			{ limits.Set(1.0f, -1.0f);						}

	inline_	bool	IsPrismaticLimitEnabled(const PintLimits& limits)	{ return limits.mMinValue<=limits.mMaxValue;	}
	inline_	void	SetPrismaticLimitDisabled(PintLimits& limits)		{ limits.Set(1.0f, -1.0f);						}

	inline_	bool	IsMinDistanceLimitEnabled(const PintLimits& limits)	{ return limits.mMinValue>=0.0f;	}
	inline_	bool	IsMaxDistanceLimitEnabled(const PintLimits& limits)	{ return limits.mMaxValue>=0.0f;	}
	inline_	void	SetMinDistanceLimitDisabled(PintLimits& limits)		{ limits.mMinValue = -1.0f;			}
	inline_	void	SetMaxDistanceLimitDisabled(PintLimits& limits)		{ limits.mMaxValue = -1.0f;			}

	inline_	bool	IsD6LinearLimitEnabled(const PintLimits& limits)	{ return limits.mMinValue<=limits.mMaxValue;	}
	inline_	void	SetD6LinearLimitDisabled(PintLimits& limits)		{ limits.Set(1.0f, -1.0f);						}

	struct PintSpring
	{
		inline_	PintSpring(float s=0.0f, float d=0.0f) : mStiffness(s), mDamping(d)	{}

		float	mStiffness;
		float	mDamping;
	};

	struct PintHingeDynamicData
	{
		float	mTwistAngle;
	};

	struct PintD6DynamicData
	{
		float	mTwistAngle;
		float	mSwingYAngle;
		float	mSwingZAngle;
	};

	struct PINT_JOINT_CREATE : public Allocateable
	{
						PINT_JOINT_CREATE(PintJoint type) :
							mName		(null),
							mType		(type),
							mObject0	(null),
							mObject1	(null)
						{
						}

						PINT_JOINT_CREATE(const PINT_JOINT_CREATE& create) :
							mName		(create.mName),
							mType		(create.mType),
							mObject0	(create.mObject0),
							mObject1	(create.mObject1)
						{
						}

		const char*		mName;
		PintJoint		mType;
		PintActorHandle	mObject0;
		PintActorHandle	mObject1;
	};

	struct PINT_SPHERICAL_JOINT_CREATE : PINT_JOINT_CREATE
	{
				PINT_SPHERICAL_JOINT_CREATE() :
					PINT_JOINT_CREATE	(PINT_JOINT_SPHERICAL),
					mLocalPivot0		(Idt),
					mLocalPivot1		(Idt),
					mLimits				(-1.0f, -1.0f)
				{
				}

				PINT_SPHERICAL_JOINT_CREATE(PintActorHandle object0, PintActorHandle object1, const PR& p0, const PR& p1) :
					PINT_JOINT_CREATE	(PINT_JOINT_SPHERICAL),
					mLocalPivot0		(p0),
					mLocalPivot1		(p1),
					mLimits				(-1.0f, -1.0f)
				{
					mObject0 = object0;
					mObject1 = object1;
				}

				PINT_SPHERICAL_JOINT_CREATE(PintActorHandle object0, PintActorHandle object1, const Point& p0, const Point& p1) :
					PINT_JOINT_CREATE	(PINT_JOINT_SPHERICAL),
					mLimits				(-1.0f, -1.0f)
				{
					mLocalPivot0.mPos = p0;
					mLocalPivot0.mRot.Identity();
					mLocalPivot1.mPos = p1;
					mLocalPivot1.mRot.Identity();
					mObject0 = object0;
					mObject1 = object1;
				}

				PINT_SPHERICAL_JOINT_CREATE(const PINT_SPHERICAL_JOINT_CREATE& create) :
					PINT_JOINT_CREATE	(create),
					mLocalPivot0		(create.mLocalPivot0),
					mLocalPivot1		(create.mLocalPivot1),
					mLimits				(create.mLimits)
				{
				}

		PR			mLocalPivot0;
		PR			mLocalPivot1;
		PintLimits	mLimits;	// Cone limits
	};

	struct PINT_HINGE_JOINT_CREATE : PINT_JOINT_CREATE
	{
							PINT_HINGE_JOINT_CREATE() :
								PINT_JOINT_CREATE	(PINT_JOINT_HINGE),
								mLocalPivot0		(Point(0.0f, 0.0f, 0.0f)),
								mLocalPivot1		(Point(0.0f, 0.0f, 0.0f)),
								mLocalAxis0			(Point(0.0f, 0.0f, 0.0f)),
								mLocalAxis1			(Point(0.0f, 0.0f, 0.0f)),
								mLimits				(1.0f, -1.0f),
								mUseMotor			(false),
								mDriveVelocity		(0.0f)
							{
								//###temp
								mGlobalAnchor.SetNotUsed();
								mGlobalAxis.SetNotUsed();
							}

							PINT_HINGE_JOINT_CREATE(const PINT_HINGE_JOINT_CREATE& create) :
								PINT_JOINT_CREATE	(create),
								mLocalPivot0		(create.mLocalPivot0),
								mLocalPivot1		(create.mLocalPivot1),
								mLocalAxis0			(create.mLocalAxis0),
								mLocalAxis1			(create.mLocalAxis1),
								mLimits				(create.mLimits),
								mGlobalAnchor		(create.mGlobalAnchor),
								mGlobalAxis			(create.mGlobalAxis),
								mUseMotor			(create.mUseMotor),
								mDriveVelocity		(create.mDriveVelocity)
							{
							}

				Point		mLocalPivot0;
				Point		mLocalPivot1;
				Point		mLocalAxis0;
				Point		mLocalAxis1;
				PintLimits	mLimits;	// Angle limits
				//###temp
				Point		mGlobalAnchor;
				Point		mGlobalAxis;

				// Experimental motor/drive API, PhysX-based, just for tests so far
				bool		mUseMotor;
				float		mDriveVelocity;
	};

	// New version with a cleaned-up API
	struct PINT_HINGE2_JOINT_CREATE : PINT_JOINT_CREATE
	{
							PINT_HINGE2_JOINT_CREATE() :
								PINT_JOINT_CREATE	(PINT_JOINT_HINGE2),
								mLocalPivot0		(Idt),
								mLocalPivot1		(Idt),
								mLimits				(1.0f, -1.0f),
								mUseMotor			(false),
								mDriveVelocity		(0.0f)
							{
							}

							PINT_HINGE2_JOINT_CREATE(const PINT_HINGE2_JOINT_CREATE& create) :
								PINT_JOINT_CREATE	(create),
								mLocalPivot0		(create.mLocalPivot0),
								mLocalPivot1		(create.mLocalPivot1),
								mLimits				(create.mLimits),
								mUseMotor			(create.mUseMotor),
								mDriveVelocity		(create.mDriveVelocity)
							{
							}

				PR			mLocalPivot0;
				PR			mLocalPivot1;
				PintLimits	mLimits;	// Angle limits

				// Experimental motor/drive API, PhysX-based, just for tests so far
				bool		mUseMotor;
				float		mDriveVelocity;
	};

	struct PINT_PRISMATIC_JOINT_CREATE : PINT_JOINT_CREATE
	{
					PINT_PRISMATIC_JOINT_CREATE() :
						PINT_JOINT_CREATE(PINT_JOINT_PRISMATIC),
						mLocalPivot0	(Idt),	//###PRISMATIC2
						mLocalPivot1	(Idt),	//###PRISMATIC2
						mLocalAxis0		(Point(0.0f, 0.0f, 0.0f)),
						mLocalAxis1		(Point(0.0f, 0.0f, 0.0f)),
						mLimits			(1.0f, -1.0f)
					{
					}

					PINT_PRISMATIC_JOINT_CREATE(const PINT_PRISMATIC_JOINT_CREATE& create) :
						PINT_JOINT_CREATE	(create),
						mLocalPivot0		(create.mLocalPivot0),
						mLocalPivot1		(create.mLocalPivot1),
						mLocalAxis0			(create.mLocalAxis0),
						mLocalAxis1			(create.mLocalAxis1),
						mLimits				(create.mLimits),
						mSpring				(create.mSpring)
					{
					}

		PR			mLocalPivot0;	//###PRISMATIC2
		PR			mLocalPivot1;	//###PRISMATIC2
		Point		mLocalAxis0;	// Old API - set to zero if not used
		Point		mLocalAxis1;	// Old API - set to zero if not used
		PintLimits	mLimits;
		PintSpring	mSpring;
	};

	struct PINT_FIXED_JOINT_CREATE : PINT_JOINT_CREATE
	{
				PINT_FIXED_JOINT_CREATE() : PINT_JOINT_CREATE(PINT_JOINT_FIXED)
				{
					mLocalPivot0.Zero();
					mLocalPivot1.Zero();
				}

				PINT_FIXED_JOINT_CREATE(const PINT_FIXED_JOINT_CREATE& create) :
					PINT_JOINT_CREATE	(create),
					mLocalPivot0		(create.mLocalPivot0),
					mLocalPivot1		(create.mLocalPivot1)
				{
				}

		Point	mLocalPivot0;
		Point	mLocalPivot1;
	};

	struct PINT_DISTANCE_JOINT_CREATE : PINT_JOINT_CREATE
	{
					PINT_DISTANCE_JOINT_CREATE() :
						PINT_JOINT_CREATE	(PINT_JOINT_DISTANCE),
						mLocalPivot0		(Point(0.0f, 0.0f, 0.0f)),
						mLocalPivot1		(Point(0.0f, 0.0f, 0.0f)),
						mLimits				(-1.0f, -1.0f)
					{
					}

					PINT_DISTANCE_JOINT_CREATE(const PINT_DISTANCE_JOINT_CREATE& create) :
						PINT_JOINT_CREATE	(create),
						mLocalPivot0		(create.mLocalPivot0),
						mLocalPivot1		(create.mLocalPivot1),
						mLimits				(create.mLimits)
					{
					}

		Point		mLocalPivot0;
		Point		mLocalPivot1;
		PintLimits	mLimits;
	};

	struct PINT_GEAR_JOINT_CREATE : PINT_JOINT_CREATE
	{
						PINT_GEAR_JOINT_CREATE() :
							PINT_JOINT_CREATE	(PINT_JOINT_GEAR),
							mHinge0				(null),
							mHinge1				(null),
							mLocalPivot0		(Idt),
							mLocalPivot1		(Idt),
							mGearRatio			(0.0f)
//							mRadius0			(0.0f),
//							mRadius1			(0.0f)
						{
						}

		PintJointHandle	mHinge0;
		PintJointHandle	mHinge1;
		PR				mLocalPivot0;
		PR				mLocalPivot1;
		float			mGearRatio;
//		float			mRadius0;
//		float			mRadius1;
	};

	struct PINT_RACK_AND_PINION_JOINT_CREATE : PINT_JOINT_CREATE
	{
						PINT_RACK_AND_PINION_JOINT_CREATE() :
							PINT_JOINT_CREATE	(PINT_JOINT_RACK_AND_PINION),
							mHinge				(null),
							mPrismatic			(null),
							mLocalPivot0		(Idt),
							mLocalPivot1		(Idt),
							mNbRackTeeth		(0),
							mNbPinionTeeth		(0),
							mRackLength			(0.0f)
						{
						}

		PintJointHandle	mHinge;
		PintJointHandle	mPrismatic;
		PR				mLocalPivot0;
		PR				mLocalPivot1;
		udword			mNbRackTeeth;
		udword			mNbPinionTeeth;
		float			mRackLength;
	};

	struct PINT_CHAIN_JOINT_CREATE : PINT_JOINT_CREATE
	{
						PINT_CHAIN_JOINT_CREATE() :
							PINT_JOINT_CREATE	(PINT_JOINT_CHAIN),
							mHinge				(null),
							mPrismatic			(null),
							mLocalPivot0		(Idt),
							mLocalPivot1		(Idt),
							mNbRackTeeth		(0),
							mNbPinionTeeth		(0),
							mRackLength			(0.0f)
						{
						}

		PintJointHandle	mHinge;
		PintJointHandle	mPrismatic;
		PR				mLocalPivot0;
		PR				mLocalPivot1;
		udword			mNbRackTeeth;
		udword			mNbPinionTeeth;
		float			mRackLength;
	};

	struct PINT_PORTAL_JOINT_CREATE : PINT_JOINT_CREATE
	{
				PINT_PORTAL_JOINT_CREATE() : PINT_JOINT_CREATE(PINT_JOINT_PORTAL)
				{
					mRelPose.Identity();
					mLocalPivot0.Zero();
					mLocalPivot1.Zero();
				}

				PINT_PORTAL_JOINT_CREATE(const PINT_PORTAL_JOINT_CREATE& create) :
					PINT_JOINT_CREATE	(create),
					mRelPose			(create.mRelPose),
					mLocalPivot0		(create.mLocalPivot0),
					mLocalPivot1		(create.mLocalPivot1)
				{
				}

		PR		mRelPose;
		Point	mLocalPivot0;
		Point	mLocalPivot1;
	};

	enum
	{
		PINT_D6_MOTOR_DRIVE_X	= 1<<0,
		PINT_D6_MOTOR_DRIVE_Y	= 1<<1,
		PINT_D6_MOTOR_DRIVE_Z	= 1<<2,
	};

	struct PINT_D6_JOINT_CREATE : PINT_JOINT_CREATE
	{
				PINT_D6_JOINT_CREATE() : PINT_JOINT_CREATE(PINT_JOINT_D6)
				{
					mLocalPivot0.Identity();
					mLocalPivot1.Identity();
					// For all DOFs:
					// - if min==max limit then DOF is locked
					// - if min>max limit then DOF is free
					// - if min<max limit then DOF is limited
					const Point Zero(0.0f, 0.0f, 0.0f);
					mLinearLimits.mMin = Zero;
					mLinearLimits.mMax = Zero;
//					mAngularLimitsMin = Zero;
//					mAngularLimitsMax = Zero;
					mMinTwist = 0.0f;
					mMaxTwist = 0.0f;
					mMaxSwingY = 0.0f;
					mMaxSwingZ = 0.0f;

					mMotorFlags = 0;
					mMotorStiffness = 0.0f;
					mMotorDamping = 0.0f;
				}

				PINT_D6_JOINT_CREATE(const PINT_D6_JOINT_CREATE& create) :
					PINT_JOINT_CREATE	(create),
					mLocalPivot0		(create.mLocalPivot0),
					mLocalPivot1		(create.mLocalPivot1),
					mLinearLimits		(create.mLinearLimits),
					mMinTwist			(create.mMinTwist),
					mMaxTwist			(create.mMaxTwist),
					mMaxSwingY			(create.mMaxSwingY),
					mMaxSwingZ			(create.mMaxSwingZ),
					mMotorFlags			(create.mMotorFlags),
					mMotorStiffness		(create.mMotorStiffness),
					mMotorDamping		(create.mMotorDamping)
				{
				}

		PR		mLocalPivot0;
		PR		mLocalPivot1;
		AABB	mLinearLimits;
//		Point	mAngularLimitsMin;	// X/Y/Z = twist / swing1 / swing2
//		Point	mAngularLimitsMax;
		float	mMinTwist;
		float	mMaxTwist;
		float	mMaxSwingY;	// Negative value for a free swing
		float	mMaxSwingZ;	// Negative value for a free swing

		udword	mMotorFlags;
		float	mMotorStiffness;
		float	mMotorDamping;
	};

	///////////////////////////////////////////////////////////////////////////

	// Experimental support for PhysX articulations
	struct PINT_ARTICULATION_CREATE : public Allocateable
	{
		PINT_ARTICULATION_CREATE()
		{
		}
	};

	struct PINT_ARTICULATED_MOTOR_CREATE : public Allocateable
	{
				PINT_ARTICULATED_MOTOR_CREATE() :
					mExternalCompliance	(0.0f),
					mInternalCompliance	(0.0f),
					mStiffness			(0.0f),
					mDamping			(0.0f)
				{
					mTargetVelocity.SetNotUsed();
					mTargetOrientation.SetNotUsed();
				}

		Quat	mTargetOrientation;
		Point	mTargetVelocity;
		float	mExternalCompliance;
		float	mInternalCompliance;
		float	mStiffness;
		float	mDamping;
	};

	struct PINT_ARTICULATED_BODY_CREATE	: public Allocateable
	{
									PINT_ARTICULATED_BODY_CREATE() :
										mParent				(null),
										mLocalPivot0		(Point(0.0f, 0.0f, 0.0f)),
										mLocalPivot1		(Point(0.0f, 0.0f, 0.0f)),
										mX					(Point(1.0f, 0.0f, 0.0f)),
										mSwingYLimit		(0.0f),
										mSwingZLimit		(0.0f),
										mTwistLowerLimit	(0.0f),
										mTwistUpperLimit	(0.0f),
										mEnableTwistLimit	(false),
										mEnableSwingLimit	(false),
										//
										mUseMotor			(false)
										{
										}

		PintActorHandle					mParent;
		Point							mLocalPivot0;	// parent
		Point							mLocalPivot1;	// child
		Point							mX;
		float							mSwingYLimit;
		float							mSwingZLimit;
		float							mTwistLowerLimit;
		float							mTwistUpperLimit;
		bool							mEnableTwistLimit;
		bool							mEnableSwingLimit;
		//
		bool							mUseMotor;
		PINT_ARTICULATED_MOTOR_CREATE	mMotor;
	};

	///////////////////////////////////////////////////////////////////////////

	// Experimental support for PhysX RC articulations
	struct PINT_RC_ARTICULATION_CREATE : public Allocateable
	{
				PINT_RC_ARTICULATION_CREATE(bool fix_base=false) :
					mFixBase	(fix_base)
				{
				}

		bool	mFixBase;
	};

	struct PINT_RC_ARTICULATED_MOTOR_CREATE : public Allocateable
	{
				PINT_RC_ARTICULATED_MOTOR_CREATE() :
					mStiffness			(0.0f),
					mDamping			(0.0f),
					mMaxForce			(FLT_MAX),
					mAccelerationDrive	(false)
				{
				}

		float	mStiffness;
		float	mDamping;
		float	mMaxForce;
		bool	mAccelerationDrive;
	};

	struct PINT_RC_ARTICULATED_BODY_CREATE	: public Allocateable
	{
									PINT_RC_ARTICULATED_BODY_CREATE() :
											mParent				(null),
											mJointType			(PINT_JOINT_SPHERICAL),
											mAxisIndex			(AXIS_FORCE_DWORD),
											mMinLimit			(1.0f),
											mMaxLimit			(-1.0f),
											mMinTwistLimit		(1.0f),
											mMaxTwistLimit		(-1.0f),
											mMinSwing1Limit		(1.0f),
											mMaxSwing1Limit		(-1.0f),
											mMinSwing2Limit		(1.0f),
											mMaxSwing2Limit		(-1.0f),
											mFrictionCoeff		(0.5f),
											mMaxJointVelocity	(1000000.f),
											mUseMotor			(false),
											mTargetVel			(0.0f)
											{
												mLocalPivot0.Identity();
												mLocalPivot1.Identity();
											}

		PintActorHandle						mParent;
		PR									mLocalPivot0;		// See PxArticulationJointReducedCoordinate::setParentPose()
		PR									mLocalPivot1;		// See PxArticulationJointReducedCoordinate::setChildPose()
		PintJoint							mJointType;			// See PxArticulationJointReducedCoordinate::setJointType()
		// For prismatic & hinge
		AxisIndex							mAxisIndex;
		float								mMinLimit;
		float								mMaxLimit;
		// For spherical
		float								mMinTwistLimit;
		float								mMaxTwistLimit;
		float								mMinSwing1Limit;
		float								mMaxSwing1Limit;
		float								mMinSwing2Limit;
		float								mMaxSwing2Limit;
		//
		float								mFrictionCoeff;		// See PxArticulationJointReducedCoordinate::setFrictionCoefficient()
		float								mMaxJointVelocity;	// See PxArticulationJointReducedCoordinate::setMaxJointVelocity()
		//
		bool								mUseMotor;
		PINT_RC_ARTICULATED_MOTOR_CREATE	mMotor;
		float								mTargetVel;
	};

	///////////////////////////////////////////////////////////////////////////

	// See the PintCaps ctor comments for explanations about the caps.
	struct PintCaps : public Allocateable
	{
				PintCaps();

		bool	mSupportRigidBodySimulation;
		bool	mSupportCylinders;
		bool	mSupportConvexes;
		bool	mSupportMeshes;
		bool	mSupportDynamicMeshes;
		bool	mSupportDeformableMeshes;
		bool	mSupportHeightfields;
		bool	mSupportContactNotifications;
		bool	mSupportContactModifications;
		bool	mSupportMassForInertia;
		bool	mSupportKinematics;
		bool	mSupportCollisionGroups;
		bool	mSupportCompounds;
		bool	mSupportAggregates;
		//
		bool	mSupportSphericalJoints;
		bool	mSupportHingeJoints;
		bool	mSupportFixedJoints;
		bool	mSupportPrismaticJoints;
		bool	mSupportDistanceJoints;
		bool	mSupportD6Joints;
		bool	mSupportGearJoints;
		bool	mSupportRackJoints;
		bool	mSupportPortalJoints;
		bool	mSupportMCArticulations;
		bool	mSupportRCArticulations;
		//
		bool	mSupportRaycasts;
		//
		bool	mSupportBoxSweeps;
		bool	mSupportSphereSweeps;
		bool	mSupportCapsuleSweeps;
		bool	mSupportConvexSweeps;
		//
		bool	mSupportSphereOverlaps;
		bool	mSupportBoxOverlaps;
		bool	mSupportCapsuleOverlaps;
		bool	mSupportConvexOverlaps;
		bool	mSupportMeshMeshOverlaps;
		//
		bool	mSupportVehicles;
		bool	mSupportCharacters;
	};

	struct PintDisabledGroups : public Allocateable
	{
							PintDisabledGroups(PintCollisionGroup group0=0xffff, PintCollisionGroup group1=0xffff) : mGroup0(group0), mGroup1(group1)	{}

		PintCollisionGroup	mGroup0;
		PintCollisionGroup	mGroup1;
	};

	struct PintBooleanHit : public Allocateable
	{
		bool	mHit;

		inline_	void	SetNoHit()
		{
			mHit = false;
		}
	};

	struct PintOverlapHit : public Allocateable
	{
		PintActorHandle	mTouchedActor;
		PintShapeHandle	mTouchedShape;

		inline_	void	SetNoHit()
		{
			mTouchedActor = null;
			mTouchedShape = null;
		}
	};

	struct PintRaycastHit : PintOverlapHit
	{
		Point	mImpact;
		Point	mNormal;
		float	mDistance;
		udword	mTriangleIndex;
	};
	Point ComputeLocalPoint(const Point& hit, const PR& world_pose);

	struct PintMultipleHits : public Allocateable
	{
		udword	mNbHits;
		udword	mOffset;

		inline_	void	SetNoHit()
		{
			mNbHits = mOffset = 0;
		}
	};

	struct PintRaycastData : public Allocateable
	{
		Point	mOrigin;
		Point	mDir;
		float	mMaxDist;
	};

	struct PintSphereOverlapData : public Allocateable
	{
		Sphere	mSphere;
	};

	struct PintBoxOverlapData : public Allocateable
	{
		OBB		mBox;
	};

	struct PintCapsuleOverlapData : public Allocateable
	{
		LSS		mCapsule;
	};

	struct PintConvexOverlapData : public Allocateable
	{
		// Experimental convex sweep support
		// So there is a design issue here. For simpler shapes we don't need per-plugin data in the sweeps (they can all share the same data)
		// but for convexes we need to create per-plugin convex objects, and the convex sweep data becomes engine dependent.
		// PintConvexIndex tries to make this work with the existing infrastructure.
		PintShapeRenderer*	mRenderer;
		PintConvexIndex		mConvexObjectIndex;
		PR					mTransform;
	};

	struct PintSweepData
	{
		Point	mDir;
		float	mMaxDist;
	};

	struct PintBoxSweepData : public PintBoxOverlapData, public PintSweepData
	{
	};

	struct PintSphereSweepData : public PintSphereOverlapData, public PintSweepData
	{
	};

	struct PintCapsuleSweepData : public PintCapsuleOverlapData, public PintSweepData
	{
	};

	struct PintConvexSweepData : public PintConvexOverlapData, public PintSweepData
	{
	};

/*	class PintQueryFilterCallback
	{
		public:
		virtual	bool	PreFilter(PintActorHandle actor, PintShapeHandle shape)	= 0;
	};*/

	//class SelectionManager;
	class VisibilityManager;
	class PintRender : public Allocateable
	{
		public:
								PintRender()		{}
		virtual					~PintRender()		{}

		virtual	void			Print				(float x, float y, float fontSize, const char* string, const Point& color)	= 0;

		// These ones with an explicit color
		virtual	void			DrawLine			(const Point& p0, const Point& p1, const Point& color)						= 0;
		virtual	void			DrawLines			(udword nb_lines, const Point* pts, const Point& color)						= 0;
		virtual	void			DrawLines2D			(udword nb_verts, const float* vertices, const Point& color)				= 0;
		virtual	void			DrawLine2D			(float x0, float x1, float y0, float y1, const Point& color)				= 0;
		virtual	void			DrawRectangle2D		(float x0, float x1, float y0, float y1, const Point& color, float alpha)	= 0;
		virtual	void			DrawTriangle		(const Point& p0, const Point& p1, const Point& p2, const Point& color)		= 0;
		virtual	void			DrawWireframeAABB	(udword nb_boxes, const AABB* boxes, const Point& color)					= 0;
		virtual	void			DrawWireframeOBB	(const OBB& box, const Point& color)										= 0;
		virtual	void			DrawWireframeSphere	(const Sphere& sphere, const Point& color)									= 0;

		// These ones with current color
		virtual	void			DrawSphere			(float radius, const PR& pose)												= 0;
		virtual	void			DrawBox				(const Point& extents, const PR& pose)										= 0;
		virtual	void			DrawCapsule			(float radius, float height, const PR& pose)								= 0;
		virtual	void			DrawSurface			(const SurfaceInterface& surface, const PR& pose)							= 0;
		virtual	void			DrawPoints			(udword nb, const Point* pts, udword stride)								= 0;

		virtual	const Plane*	GetFrustumPlanes()	{ return null;	}

		// New batching API compatible with editor's hide/show feature
		//virtual	void			StartRender			(Pint* pint, SelectionManager* sel_manager, PintRenderPass render_pass)		{}
		//virtual	void			EndRender			()																			{}
		virtual	bool			SetCurrentActor		(PintActorHandle h)															{ return true;	}
		virtual	bool			SetCurrentShape		(PintShapeHandle h)															{ return true;	}
		virtual	void			DrawShape			(PintShapeRenderer* shape, const PR& pose)									{}
	};

	///////////////////////////////////////////////////////////////////////////

	enum PintFlag
	{
		PINT_IS_ACTIVE				= (1<<0),
		PINT_HAS_RAYTRACING_WINDOW	= (1<<1),
		PINT_DEFAULT				= PINT_IS_ACTIVE|PINT_HAS_RAYTRACING_WINDOW,
	};

	class Pint_Scene;
	class Pint_Actor;
	class Pint_Shape;
	class Pint_Joint;
//	class Pint_Mesh;
	class Pint_Vehicle;
	class Pint_Character;

	class Pint : public Allocateable	// PINT = Physics INTerface
	{
		public:
									Pint() : mDefaultEnvHandle(null), mSQHelper(null), mVisHelper(null), mUserData(null)	{}
		virtual						~Pint()																					{}

		// This function is called when a test calls a non-implemented function.
		// This lets users know that the test might legitimately misbehave for a specific engine.
		inline_ bool				NotImplemented(const char* name)	const
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED|FOREGROUND_INTENSITY);
			printf("%s: non-implemented function %s has been called. Test may behave incorrectly.\n", GetUIName(), name);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
			return false;
		}

		virtual	const char*			GetName()				const								= 0;
		virtual	const char*			GetUIName()				const								{ return GetName();		}
		virtual	void				GetCaps(PintCaps& caps)	const								= 0;
		virtual	udword				GetFlags()				const								{ return PINT_DEFAULT;	}
		virtual	void				Init(const PINT_WORLD_CREATE& desc)							= 0;
//		virtual	PintSceneHandle		Init(const PINT_WORLD_CREATE& desc)							= 0;
		virtual	void				Close()														= 0;
		virtual	udword				Update(float dt)											= 0;
		virtual	void				UpdateNonProfiled(float dt)									{}
		virtual	Point				GetMainColor()												= 0;
		virtual	void				Render(PintRender& renderer, PintRenderPass render_pass)	= 0;
		virtual	void				RenderDebugData(PintRender& renderer)						= 0;

		// Scene-related functions
		virtual	void				SetGravity(const Point& gravity)														{ NotImplemented("SetGravity");							}
		virtual	void				SetDisabledGroups(udword nb_groups, const PintDisabledGroups* groups)					{ NotImplemented("SetDisabledGroups");					}

		virtual	PintConvexHandle	CreateConvexObject(const PINT_CONVEX_DATA_CREATE& desc, PintConvexIndex* index=null)	{ NotImplemented("CreateConvexObject");	return null;	}
		virtual	bool				DeleteConvexObject(PintConvexHandle handle=null, const PintConvexIndex* index=null)		{ NotImplemented("DeleteConvexObject");	return false;	}

		virtual	PintMeshHandle		CreateMeshObject(const PINT_MESH_DATA_CREATE& desc, PintMeshIndex* index=null)			{ NotImplemented("CreateMeshObject");	return null;	}
		virtual	bool				DeleteMeshObject(PintMeshHandle handle=null, const PintMeshIndex* index=null)			{ NotImplemented("DeleteMeshObject");	return false;	}

		virtual	PintHeightfieldHandle	CreateHeightfieldObject(const PINT_HEIGHTFIELD_DATA_CREATE& desc, PintHeightfieldData& data, PintHeightfieldIndex* index=null)	{ NotImplemented("CreateHeightfieldObject");	return null;	}
		virtual	bool					DeleteHeightfieldObject(PintHeightfieldHandle handle=null, const PintHeightfieldIndex* index=null)								{ NotImplemented("DeleteHeightfieldObject");	return false;	}

		private:
		virtual	PintActorHandle		CreateObject(const PINT_OBJECT_CREATE& desc)											= 0;
		virtual	bool				ReleaseObject(PintActorHandle handle)													= 0;
		public:
		inline_	PintActorHandle		_CreateObject(const PINT_OBJECT_CREATE& desc)	{ return CreateObject(desc);	}
		inline_	bool				_ReleaseObject(PintActorHandle handle)			{ return ReleaseObject(handle);	}

		virtual	PintJointHandle		CreateJoint(const PINT_JOINT_CREATE& desc)												= 0;
		virtual	bool				ReleaseJoint(PintJointHandle handle)													{ return NotImplemented("ReleaseJoint");				}

		// SQ filtering - disable scene queries for hidden/invisible objects (mainly used by the editor). This is a bit tricky:
		// - we cannot use the per-shape "filter data" because of shared shapes (they share the filter data as well).
		// - we cannot move temporarily objects away to infinity because this isn't allowed on RC articulation links,
		//   and it would break the simulation if it's still running anyway.
		// - we cannot use a filter callback because some engines don't support that.
		// - we cannot use a second SQ system within PEEL, because we (usually) don't keep copies of geom data / bounds there.
		//   Also it would defeat the purpose of doing SQ in native engines.
		// => so for now we use a generic API capturing the desired intent, and we let each engine deal with it the way they want.
		virtual	bool				SetSQFlag					(PintActorHandle actor, bool flag)							{ return NotImplemented("SetSQFlag");		}
		virtual	bool				SetSQFlag					(PintActorHandle actor, PintShapeHandle shape, bool flag)	{ return NotImplemented("SetSQFlag");		}
		virtual	bool				ResetSQFilters				()															{ return NotImplemented("ResetSQFilters");	}

		// Raycasts
		virtual	udword				BatchRaycasts				(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)							{ return NotImplemented("BatchRaycasts");	}
		virtual	udword				BatchRaycastAny				(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintRaycastData* raycasts)							{ return NotImplemented("BatchRaycastAny");	}
		virtual	udword				BatchRaycastAll				(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintRaycastData* raycasts)	{ return NotImplemented("BatchRaycastAll");	}
//		virtual	bool				Picking						(PintSQThreadContext context, PintRaycastHit* dest, const PintRaycastData& raycast, PintQueryFilterCallback* cb)		{ return NotImplemented("Picking");			}
		// Sweeps
		virtual	udword				BatchBoxSweeps				(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintBoxSweepData* sweeps)					{ NotImplemented("BatchBoxSweeps");		return 0;	}
		virtual	udword				BatchSphereSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps)				{ NotImplemented("BatchSphereSweeps");	return 0;	}
		virtual	udword				BatchCapsuleSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintCapsuleSweepData* sweeps)				{ NotImplemented("BatchCapsuleSweeps");	return 0;	}
		virtual	udword				BatchConvexSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintConvexSweepData* sweeps)				{ NotImplemented("BatchConvexSweeps");	return 0;	}
		// Overlaps
		virtual	udword				BatchSphereOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintSphereOverlapData* overlaps)			{ NotImplemented("BatchSphereOverlapAny");		return 0;	}
		virtual	udword				BatchBoxOverlapAny			(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintBoxOverlapData* overlaps)				{ NotImplemented("BatchBoxOverlapAny");			return 0;	}
		virtual	udword				BatchCapsuleOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintCapsuleOverlapData* overlaps)			{ NotImplemented("BatchCapsuleOverlapAny");		return 0;	}
		virtual	udword				BatchConvexOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintConvexOverlapData* overlaps)			{ NotImplemented("BatchConvexOverlapAny");		return 0;	}
		virtual	udword				BatchSphereOverlapObjects	(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintSphereOverlapData* overlaps)	{ NotImplemented("BatchSphereOverlapObjects");	return 0;	}
		virtual	udword				BatchBoxOverlapObjects		(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintBoxOverlapData* overlaps)		{ NotImplemented("BatchBoxOverlapObjects");		return 0;	}
		virtual	udword				BatchCapsuleOverlapObjects	(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintCapsuleOverlapData* overlaps)	{ NotImplemented("BatchCapsuleOverlapObjects"); return 0;	}
		virtual	udword				BatchConvexOverlapObjects	(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintConvexOverlapData* overlaps)	{ NotImplemented("BatchConvexOverlapObjects");	return 0;	}
		// Midphase
		virtual	udword				FindTriangles_MeshSphereOverlap	(PintSQThreadContext context, PintActorHandle handle, udword nb, PintMultipleHits* dest, Container& stream, const PintSphereOverlapData* overlaps)	{ NotImplemented("FindTriangles_MeshSphereOverlap");	return 0;	}
		virtual	udword				FindTriangles_MeshBoxOverlap	(PintSQThreadContext context, PintActorHandle handle, udword nb, PintMultipleHits* dest, Container& stream, const PintBoxOverlapData* overlaps)		{ NotImplemented("FindTriangles_MeshBoxOverlap");		return 0;	}
		virtual	udword				FindTriangles_MeshCapsuleOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, PintMultipleHits* dest, Container& stream, const PintCapsuleOverlapData* overlaps)	{ NotImplemented("FindTriangles_MeshCapsuleOverlap");	return 0;	}
		virtual	udword				FindTriangles_MeshMeshOverlap	(PintSQThreadContext context, PintActorHandle handle0, PintActorHandle handle1, Container& results)	{ NotImplemented("FindTriangles_MeshMeshOverlap");	return 0;	}

		// TODO: move to Pint_Actor API
		virtual	PR					GetWorldTransform(PintActorHandle handle)					{ NotImplemented("GetWorldTransform");	PR Idt; Idt.Identity();	return Idt;	}
		virtual	void				SetWorldTransform(PintActorHandle handle, const PR& pose)	{ NotImplemented("SetWorldTransform");	}

#ifdef DEPRECATED
		virtual	bool				GetBounds(PintActorHandle handle, AABB& bounds)																			{ NotImplemented("GetBounds");	return false;	}

		virtual	const char*			GetName(PintActorHandle handle)																							{ NotImplemented("GetName");	return null;	}
		virtual	bool				SetName(PintActorHandle handle, const char* name)																		{ NotImplemented("SetName");	return false;	}
#endif

		// Deprecated
		// TODO: move to Pint_Actor API
//		virtual	void				ApplyActionAtPoint(PintActorHandle handle, PintActionType action_type, const Point& action, const Point& pos)	{ NotImplemented("ApplyActionAtPoint");	}
		virtual	void				AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos)			{ NotImplemented("AddWorldImpulseAtWorldPos");	}
		virtual	void				AddLocalTorque(PintActorHandle handle, const Point& local_torque)												{ NotImplemented("AddLocalTorque");	}

		// TODO: move to Pint_Actor API
		virtual	Point				GetLinearVelocity(PintActorHandle handle)									{ NotImplemented("GetLinearVelocity");	return Point(0.0f, 0.0f, 0.0f);	}
		virtual	void				SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity)		{ NotImplemented("SetLinearVelocity");	}

		// TODO: move to Pint_Actor API
		virtual	Point				GetAngularVelocity(PintActorHandle handle)									{ NotImplemented("GetAngularVelocity");	return Point(0.0f, 0.0f, 0.0f);	}
		virtual	void				SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity)	{ NotImplemented("SetAngularVelocity");	}

#ifdef DEPRECATED
		virtual	Point				GetWorldLinearVelocity(PintActorHandle handle)																			{ NotImplemented("GetWorldLinearVelocity");	return Point(0.0f, 0.0f, 0.0f);	}
		virtual	void				SetWorldLinearVelocity(PintActorHandle handle, const Point& linear_velocity)											{ NotImplemented("SetWorldLinearVelocity");	}

		virtual	Point				GetWorldAngularVelocity(PintActorHandle handle)																			{ NotImplemented("GetWorldAngularVelocity");	return Point(0.0f, 0.0f, 0.0f);	}
		virtual	void				SetWorldAngularVelocity(PintActorHandle handle, const Point& angular_velocity)											{ NotImplemented("SetWorldAngularVelocity");	}

		virtual	float				GetMass(PintActorHandle handle)																							{ NotImplemented("GetMass");			return 0.0f;	}
		virtual	void				SetMass(PintActorHandle handle, float mass)																				{ NotImplemented("SetMass");							}

		virtual	Point				GetLocalInertia(PintActorHandle handle)																					{ NotImplemented("GetLocalInertia");	return Point(0.0f, 0.0f, 0.0f);	}
		virtual	void				SetLocalInertia(PintActorHandle handle, const Point& inertia)															{ NotImplemented("SetLocalInertia");									}

		virtual	udword				GetShapes(PintActorHandle* shapes, PintActorHandle handle)																{ NotImplemented("GetShapes");			return 0; }
		virtual	void				SetLocalRot(PintActorHandle handle, const Quat& q)																		{ NotImplemented("SetLocalRot");	}

		virtual	bool				EnableGravity(PintActorHandle handle, bool flag)																		{ NotImplemented("EnableGravity");	return false;	}

		virtual	float				GetLinearDamping(PintActorHandle handle)																				{ NotImplemented("GetLinearDamping");	return 0.0f;	}
		virtual	float				GetAngularDamping(PintActorHandle handle)																				{ NotImplemented("GetAngularDamping");	return 0.0f;	}
		virtual	void				SetLinearDamping(PintActorHandle handle, float damping)																	{ NotImplemented("SetLinearDamping");	}
		virtual	void				SetAngularDamping(PintActorHandle handle, float damping)																{ NotImplemented("SetAngularDamping");	}
#endif

		// TODO: move to Pint_Actor API
		virtual	bool				SetKinematicPose(PintActorHandle handle, const Point& pos)	{ return NotImplemented("SetKinematicPose");	}
		virtual	bool				SetKinematicPose(PintActorHandle handle, const PR& pr)		{ return NotImplemented("SetKinematicPose");	}
		virtual	bool				IsKinematic(PintActorHandle handle)							{ return NotImplemented("IsKinematic");			}
		virtual	bool				EnableKinematic(PintActorHandle handle, bool flag)			{ return NotImplemented("EnableKinematic");		}

		// Creates/releases an optional per-thread structure (e.g. caches) for scene queries.
		virtual	PintSQThreadContext	CreateSQThreadContext()						{ return null;	}
		virtual	void				ReleaseSQThreadContext(PintSQThreadContext)	{}

		// Aggregates - currently based on the PhysX API, may change later to support the equivalent in other libs.
		virtual	PintAggregateHandle	CreateAggregate(udword max_size, bool enable_self_collision)			{ NotImplemented("CreateAggregate");		return null;		}
		inline_	PintAggregateHandle	CreateAggregate(const PINT_AGGREGATE_CREATE& desc)						{ return CreateAggregate(desc.mMaxSize, desc.mSelfCollision);	}
		virtual	bool				AddToAggregate(PintActorHandle object, PintAggregateHandle aggregate)	{ return NotImplemented("AddToAggregate");						}
		virtual	bool				AddAggregateToScene(PintAggregateHandle aggregate)						{ return NotImplemented("AddAggregateToScene");					}

		// Articulations - currently based on the PhysX API, may change later to support the equivalent in other libs.
		virtual	PintArticHandle		CreateArticulation(const PINT_ARTICULATION_CREATE&)																		{ NotImplemented("CreateArticulation");			return null;	}
		virtual	PintActorHandle		CreateArticulatedObject(const PINT_OBJECT_CREATE&, const PINT_ARTICULATED_BODY_CREATE&, PintArticHandle articulation)	{ NotImplemented("CreateArticulatedObject");	return null;	}
		virtual	bool				AddArticulationToScene(PintArticHandle articulation)																	{ return NotImplemented("AddArticulationToScene");				}
		virtual	void				SetArticulatedMotor(PintActorHandle object, const PINT_ARTICULATED_MOTOR_CREATE& motor)									{ NotImplemented("SetArticulatedMotor");						}

		// RC articulations - currently based on the PhysX API, may change later to support the equivalent in other libs.
		virtual	PintArticHandle		CreateRCArticulation(const PINT_RC_ARTICULATION_CREATE&)																	{ NotImplemented("CreateRCArticulation");		return null;	}
		virtual	PintActorHandle		CreateRCArticulatedObject(const PINT_OBJECT_CREATE&, const PINT_RC_ARTICULATED_BODY_CREATE&, PintArticHandle articulation)	{ NotImplemented("CreateRCArticulatedObject");	return null;	}
		virtual	bool				AddRCArticulationToScene(PintArticHandle articulation)																		{ return NotImplemented("AddRCArticulationToScene");			}
//		virtual	void				SetRCArticulatedMotor(PintActorHandle object, const PINT_ARTICULATED_MOTOR_CREATE& motor)									{ NotImplemented("SetRCArticulatedMotor");						}
		virtual	bool				AddRCArticulationToAggregate(PintArticHandle articulation, PintAggregateHandle aggregate)									{ return NotImplemented("AddRCArticulationToAggregate");		}
		virtual	bool				SetRCADriveEnabled(PintActorHandle handle, bool flag)																		{ return NotImplemented("SetRCADriveEnabled");					}
		virtual	bool				SetRCADriveVelocity(PintActorHandle handle, float velocity)																	{ return NotImplemented("SetRCADriveVelocity");					}

		// Joints - WIP
		virtual	bool				SetDriveEnabled(PintJointHandle handle, bool flag)									{ return NotImplemented("SetDriveEnabled");				}
		virtual	bool				SetDrivePosition(PintJointHandle handle, const PR& pose)							{ return NotImplemented("SetDrivePosition");			}
		virtual	bool				SetDriveVelocity(PintJointHandle handle, const Point& linear, const Point& angular)	{ return NotImplemented("SetDriveVelocity");			}
		virtual	PR					GetDrivePosition(PintJointHandle handle)											{ NotImplemented("GetDrivePosition");	return PR(Idt);	}
//		virtual	void				SetGearJointError(PintJointHandle handle, float error)								{ NotImplemented("SetGearJointError");					}
		virtual	void				SetPortalJointRelativePose(PintJointHandle handle, const PR& pose)					{ NotImplemented("SetPortalJointRelativePose");			}

		// New APIs - WIP - eventually everything should move to these domain-specific APIs
		virtual	Pint_Scene*			GetSceneAPI()		{ NotImplemented("GetSceneAPI");		return null;	}
		virtual	Pint_Actor*			GetActorAPI()		{ NotImplemented("GetActorAPI");		return null;	}
		virtual	Pint_Shape*			GetShapeAPI()		{ NotImplemented("GetShapeAPI");		return null;	}
		virtual	Pint_Joint*			GetJointAPI()		{ NotImplemented("GetJointAPI");		return null;	}
//		virtual	Pint_Mesh*			GetMeshAPI()		{ NotImplemented("GetMeshAPI");			return null;	}
		virtual	Pint_Vehicle*		GetVehicleAPI()		{ NotImplemented("GetVehicleAPI");		return null;	}
		virtual	Pint_Character*		GetCharacterAPI()	{ NotImplemented("GetCharacterAPI");	return null;	}

		virtual	void				TestNewFeature()	{}

				PintActorHandle		mDefaultEnvHandle;
				PintSQ*				mSQHelper;
				VisibilityManager*	mVisHelper;
				void*				mUserData;
	};

	///////////////////////////////////////////////////////////////////////////

	class Reporter
	{
		public:
		virtual	bool	ReportObject(PintActorHandle)	= 0;
	};

	// New scene API
	class Pint_Scene : public Allocateable
	{
						PREVENT_COPY(Pint_Scene)
		public:
						Pint_Scene(Pint& pint) : mPint(pint)	{}
		virtual			~Pint_Scene()							{}

//		virtual	bool	SetGravity(/*PintSceneHandle handle,*/ const Point& gravity)	{ return mPint.NotImplemented("Pint_Scene::SetGravity");							}
//		virtual	Point	GetGravity(/*PintSceneHandle handle*/)	const					{ mPint.NotImplemented("Pint_Scene::GetGravity");	return Point(0.0f, 0.0f, 0.0f);	}

		virtual	bool	AddActors(udword nb_actors, const PintActorHandle* actors)				{ return mPint.NotImplemented("Pint_Scene::AddActors");						}

		virtual	void	GetActors(Reporter& reporter)									const	{ mPint.NotImplemented("Pint_Scene::GetActors");							}
		virtual	void	Cull(udword nb_planes, const Plane* planes, Reporter& reporter)	const	{ mPint.NotImplemented("Pint_Scene::Cull");									}

		protected:
				Pint&	mPint;
	};

	///////////////////////////////////////////////////////////////////////////

	// New actor API for PintActorHandle-based shapes. Old API remains compatible for old actors.
	class Pint_Actor : public Allocateable
	{
								PREVENT_COPY(Pint_Actor)
		public:
								Pint_Actor(Pint& pint) : mPint(pint)							{}
		virtual					~Pint_Actor()													{}

		virtual	const char*		GetName(PintActorHandle handle)							const	{ mPint.NotImplemented("Pint_Actor::GetName");				return null;		}
		virtual	bool			SetName(PintActorHandle handle, const char* name)				{ return mPint.NotImplemented("Pint_Actor::SetName");							}

		virtual	udword			GetNbShapes(PintActorHandle handle)						const	{ mPint.NotImplemented("Pint_Actor::GetNbShapes");			return 0;			}
		virtual	PintShapeHandle	GetShape(PintActorHandle handle, udword index)			const	{ mPint.NotImplemented("Pint_Actor::GetShape");				return null;		}

		virtual	udword			GetNbJoints(PintActorHandle handle)						const	{ mPint.NotImplemented("Pint_Actor::GetNbJoints");			return 0;			}
		virtual	PintJointHandle	GetJoint(PintActorHandle handle, udword index)			const	{ mPint.NotImplemented("Pint_Actor::GetJoint");				return null;		}

		virtual	bool			GetWorldBounds(PintActorHandle handle, AABB& bounds)	const	{ return mPint.NotImplemented("Pint_Actor::GetBounds");							}

		virtual	void			WakeUp(PintActorHandle handle)									{ mPint.NotImplemented("Pint_Actor::WakeUp");									}

		virtual	bool			SetGravityFlag(PintActorHandle handle, bool flag)				{ return mPint.NotImplemented("Pint_Actor::SetGravityFlag");					}
		virtual	bool			SetDebugVizFlag(PintActorHandle handle, bool flag)				{ return mPint.NotImplemented("Pint_Actor::SetDebugVizFlag");					}
		virtual	bool			SetSimulationFlag(PintActorHandle handle, bool flag)			{ return mPint.NotImplemented("Pint_Actor::SetSimulationFlag");					}

		virtual	float			GetLinearDamping(PintActorHandle handle)				const	{ mPint.NotImplemented("Pint_Actor::GetLinearDamping");		return 0.0f;		}
		virtual	bool			SetLinearDamping(PintActorHandle handle, float damping)			{ return mPint.NotImplemented("Pint_Actor::SetLinearDamping");					}

		virtual	float			GetAngularDamping(PintActorHandle handle)				const	{ mPint.NotImplemented("Pint_Actor::GetAngularDamping");	return 0.0f;		}
		virtual	bool			SetAngularDamping(PintActorHandle handle, float damping)		{ return mPint.NotImplemented("Pint_Actor::SetAngularDamping");					}

		virtual	bool			GetLinearVelocity(PintActorHandle handle, Point& linear_velocity, bool world_space)	const	{ return mPint.NotImplemented("GetLinearVelocity");	}
		virtual	bool			SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity, bool world_space)	{ return mPint.NotImplemented("SetLinearVelocity");	}

		virtual	bool			GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space)const	{ return mPint.NotImplemented("GetAngularVelocity");}
		virtual	bool			SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space)	{ return mPint.NotImplemented("SetAngularVelocity");}

		virtual	float			GetMass(PintActorHandle handle)							const	{ mPint.NotImplemented("Pint_Actor::GetMass");				return 0.0f;		}
		virtual	bool			SetMass(PintActorHandle handle, float mass)						{ return mPint.NotImplemented("Pint_Actor::SetMass");							}

		virtual	bool			GetLocalInertia(PintActorHandle handle, Point& inertia)	const	{ return mPint.NotImplemented("Pint_Actor::GetLocalInertia");					}
		virtual	bool			SetLocalInertia(PintActorHandle handle, const Point& inertia)	{ return mPint.NotImplemented("Pint_Actor::SetLocalInertia");					}

		virtual	bool			GetCMassLocalPose(PintActorHandle handle, PR& pose)		const	{ return mPint.NotImplemented("Pint_Actor::GetCMassLocalPose");					}
		virtual	bool			SetCMassLocalPose(PintActorHandle handle, const PR& pose)		{ return mPint.NotImplemented("Pint_Actor::SetCMassLocalPose");					}

		protected:
				Pint&			mPint;
	};

	///////////////////////////////////////////////////////////////////////////

	// New shape API for PintShapeHandle-based shapes. Old API remains compatible for old shapes.
	class Pint_Shape : public Allocateable
	{
									PREVENT_COPY(Pint_Shape)
		public:
									Pint_Shape(Pint& pint) : mPint(pint)	{}
		virtual						~Pint_Shape()							{}

		virtual	const char*			GetName(PintShapeHandle handle)				const	{ mPint.NotImplemented("Pint_Shape::GetName");	return null;	}
		virtual	bool				SetName(PintShapeHandle handle, const char* name)	{ return mPint.NotImplemented("Pint_Shape::SetName");			}

		virtual	PintShape			GetType(PintShapeHandle handle)				const	{ mPint.NotImplemented("Pint_Shape::GetType");	return PINT_SHAPE_UNDEFINED;	}

		virtual	PR					GetWorldTransform(PintActorHandle actor, PintShapeHandle shape)				const	{ mPint.NotImplemented("Pint_Shape::GetWorldTransform");	return PR(Idt);	}

		virtual	bool				GetWorldBounds(PintActorHandle actor, PintShapeHandle shape, AABB& bounds)	const	{ return mPint.NotImplemented("Pint_Shape::GetWorldBounds");				}

		// Mesh-related, temp design/location
		virtual	bool				GetTriangleMeshData(SurfaceInterface& surface, PintShapeHandle handle, bool)	const	{ return mPint.NotImplemented("Pint_Shape::GetTriangleMeshData");	}
		virtual	bool				GetTriangle(Triangle& tri, PintShapeHandle handle, udword index)				const	{ return mPint.NotImplemented("Pint_Shape::GetTriangle");	}
		virtual	bool				GetIndexedTriangle(IndexedTriangle& tri, PintShapeHandle handle, udword index)	const	{ return mPint.NotImplemented("Pint_Shape::GetIndexedTriangle");	}
		virtual	bool				FindTouchedTriangles(Container& indices, PintSQThreadContext context, PintShapeHandle handle, const PR& pose, const PintSphereOverlapData& sphere)	const	{ return mPint.NotImplemented("Pint_Shape::FindTouchedTriangles");	}
		virtual	bool				Refit(PintShapeHandle shape, PintActorHandle actor)	{ return mPint.NotImplemented("Pint_Shape::Refit");	}

		// Experimental / questionable
		virtual	PintShapeRenderer*	GetShapeRenderer(PintShapeHandle handle)	const	{ return null;	}

		protected:
				Pint&				mPint;
	};

	///////////////////////////////////////////////////////////////////////////

	class Pint_Joint : public Allocateable
	{
							PREVENT_COPY(Pint_Joint)
		public:
							Pint_Joint(Pint& pint) : mPint(pint)	{}
		virtual				~Pint_Joint()							{}

		virtual	const char*	GetName(PintJointHandle handle)				const	{ mPint.NotImplemented("Pint_Joint::GetName");	return null;	}
		virtual	bool		SetName(PintJointHandle handle, const char* name)	{ return mPint.NotImplemented("Pint_Joint::SetName");			}

		virtual	PintJoint	GetType(PintJointHandle handle)														const	{ mPint.NotImplemented("Pint_Joint::GetType");	return PINT_JOINT_UNDEFINED;	}

		virtual	bool		GetActors(PintJointHandle handle, PintActorHandle& actor0, PintActorHandle& actor1)	const	{ return mPint.NotImplemented("Pint_Joint::GetActors");	}
		virtual	bool		SetActors(PintJointHandle handle, PintActorHandle actor0, PintActorHandle actor1)			{ return mPint.NotImplemented("Pint_Joint::SetActors");	}

		virtual	bool		GetFrames(PintJointHandle handle, PR* frame0=null, PR* frame1=null)					const	{ return mPint.NotImplemented("Pint_Joint::GetFrames");	}
		virtual	bool		SetFrames(PintJointHandle handle, const PR* frame0=null, const PR* frame1=null)				{ return mPint.NotImplemented("Pint_Joint::SetFrames");	}

		// ### purely experimental design
		virtual	bool		GetLimits(PintJointHandle handle, PintLimits& limits, udword index)					const	{ return mPint.NotImplemented("Pint_Joint::GetLimits");	}
		virtual	bool		SetLimits(PintJointHandle handle, const PintLimits& limits, udword index)					{ return mPint.NotImplemented("Pint_Joint::SetLimits");	}

		virtual	bool		GetSpring(PintJointHandle handle, PintSpring& spring)								const	{ return mPint.NotImplemented("Pint_Joint::GetSpring");	}
		virtual	bool		SetSpring(PintJointHandle handle, const PintSpring& spring)									{ return mPint.NotImplemented("Pint_Joint::SetSpring");	}

		virtual	bool		GetGearRatio(PintJointHandle handle, float& ratio)									const	{ return mPint.NotImplemented("Pint_Joint::GetGearRatio");	}
		virtual	bool		SetGearRatio(PintJointHandle handle, float ratio)											{ return mPint.NotImplemented("Pint_Joint::SetGearRatio");	}

		// For hinge & D6 joints
		virtual	bool		GetHingeDynamicData(PintJointHandle handle, PintHingeDynamicData& data)				const	{ return mPint.NotImplemented("GetHingeDynamicData");		}
		// For D6
		virtual	bool		GetD6DynamicData(PintJointHandle handle, PintD6DynamicData& data)					const	{ return mPint.NotImplemented("GetD6DynamicData");			}

		protected:
				Pint&		mPint;
	};

	///////////////////////////////////////////////////////////////////////////

	// New mesh API
/*	class Pint_Mesh : public Allocateable
	{
		public:
						Pint_Mesh(Pint& pint) : mPint(pint)	{}
		virtual			~Pint_Mesh()						{}

		virtual	bool	FindTouchedTriangles(Container& indices, PintSQThreadContext context, PintMeshHandle handle, const PintSphereOverlapData& sphere)	const	{ return mPint.NotImplemented("Pint_Shape::FindTouchedTriangles");	}

		protected:
				Pint&	mPint;
	};*/

	///////////////////////////////////////////////////////////////////////////

	struct PINT_VEHICLE_INPUT : public Allocateable
	{
				PINT_VEHICLE_INPUT()
				{
					Reset();
				}

		inline_	void	Reset()
				{
					mGamepad_Accel			= 0.0f;
					mGamepad_Brake			= 0.0f;
					mGamepad_Steer			= 0.0f;
					mGamepad_Handbrake		= false;

					mKeyboard_Accelerate	= false;
					mKeyboard_Brake			= false;
					mKeyboard_HandBrake		= false;
					mKeyboard_Left			= false;
					mKeyboard_Right			= false;
				}

		// Gamepad
		float	mGamepad_Accel;
		float	mGamepad_Brake;
		float	mGamepad_Steer;
		bool	mGamepad_Handbrake;

		// Keyboard
		bool	mKeyboard_Accelerate;
		bool	mKeyboard_Brake;
		bool	mKeyboard_HandBrake;
		bool	mKeyboard_Left;
		bool	mKeyboard_Right;
	};

	enum PintVehicleDifferential
	{
		DIFFERENTIAL_LS_4WD,		// limited slip differential for car with 4 driven wheels
		DIFFERENTIAL_LS_FRONTWD,	// limited slip differential for car with front-wheel drive
		DIFFERENTIAL_LS_REARWD,		// limited slip differential for car with rear-wheel drive
		DIFFERENTIAL_OPEN_4WD,		// open differential for car with 4 driven wheels 
		DIFFERENTIAL_OPEN_FRONTWD,	// open differential for car with front-wheel drive
		DIFFERENTIAL_OPEN_REARWD,	// open differential for car with rear-wheel drive
		DIFFERENTIAL_UNDEFINED
	};

	// This struct is currently based on the PhysX API.
	struct PINT_VEHICLE_CREATE	: public Allocateable
	{
									PINT_VEHICLE_CREATE() :
										mChassis						(null),
										mChassisMass					(0.0f),
										mChassisMOICoeffY				(0.0f),
										mChassisCMOffsetY				(0.0f),
										mChassisCMOffsetZ				(0.0f),
										mForceApplicationCMOffsetY		(0.0f),
										mWheelMass						(0.0f),
										mWheelMaxBrakeTorqueFront		(0.0f),
										mWheelMaxBrakeTorqueRear		(0.0f),
										mWheelMaxHandBrakeTorqueFront	(0.0f),
										mWheelMaxHandBrakeTorqueRear	(0.0f),
										mWheelMaxSteerFront				(0.0f),
										mWheelMaxSteerRear				(0.0f),
										mWheelSubsteps					(1),
										mFrontTireFrictionMultiplier	(0.0f),
										mRearTireFrictionMultiplier		(0.0f),
										mAntirollBarStiffness			(0.0f),
										mEngineMOI						(1.0f),
										mEnginePeakTorque				(0.0f),
										mEngineMaxOmega					(0.0f),
										mGearsSwitchTime				(0.0f),
										mClutchStrength					(0.0f),
										mDifferential					(DIFFERENTIAL_UNDEFINED),
										mFrontRearSplit					(0.0f),
										mFrontLeftRightSplit			(0.0f),
										mRearLeftRightSplit				(0.0f),
										mCentreBias						(0.0f),
										mFrontBias						(0.0f),
										mRearBias						(0.0f),
										mSuspMaxCompression				(0.0f),
										mSuspMaxDroop					(0.0f),
										mSuspSpringStrength				(0.0f),
										mSuspSpringDamperRate			(0.0f),
										mSuspCamberAngleAtRest			(0.0f),
										mSuspCamberAngleAtMaxCompr		(0.0f),
										mSuspCamberAngleAtMaxDroop		(0.0f),
										mAckermannAccuracy				(0.0f),
										mThresholdForwardSpeed			(0.1f),
										mImmediateAutoReverse			(false)
									{
										mStartPose.Identity();
										mChassisLocalPose.Identity();
										for(udword i=0;i<4;i++)
										{
											mWheels[i] = null;
											mWheelOffset[i].Zero();
										}

										const float DefaultSteerVsForwardSpeedData[8]=
										{
											0.0f*3.6f,		1.0f,
											5.0f*3.6f,		0.75f,
											30.0f*3.6f,		0.125f,
											120.0f*3.6f,	0.1f,
										};
										CopyMemory(mSteerVsForwardSpeedData, DefaultSteerVsForwardSpeedData, sizeof(float)*8);

										const float DefaultSmoothingData[] = {	//				Rise		Fall
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
										CopyMemory(mSmoothingData, DefaultSmoothingData, sizeof(float)*16);
									}

		PR							mStartPose;

		// Chassis
		const PINT_SHAPE_CREATE*	mChassis;						// Must be convex shape for now.
		PR							mChassisLocalPose;
		float						mChassisMass;					// See PxVehicleChassisData::mMass for details
		// "A bit of tweaking here.  The car will have more responsive turning if we reduce the y-component of the chassis moment of inertia."
		float						mChassisMOICoeffY;				// See PxVehicleChassisData::mMOI for details
		float						mChassisCMOffsetY;				// See PxVehicleChassisData::mCMOffset for details
		float						mChassisCMOffsetZ;				// See PxVehicleChassisData::mCMOffset for details
		float						mForceApplicationCMOffsetY;
		// Wheels
		const PINT_SHAPE_CREATE*	mWheels[4];						// Unique for shared shapes, or define 4 different shapes. Must be convex shapes for now.
		Point						mWheelOffset[4];				// Only supports 4-wheeled vehicles for now
		float						mWheelMass;						// See PxVehicleWheelData::mMass for details
		float						mWheelMaxBrakeTorqueFront;		// See PxVehicleWheelData::mMaxBrakeTorque for details
		float						mWheelMaxBrakeTorqueRear;		// See PxVehicleWheelData::mMaxBrakeTorque for details
		float						mWheelMaxHandBrakeTorqueFront;	// See PxVehicleWheelData::mMaxHandBrakeTorque for details
		float						mWheelMaxHandBrakeTorqueRear;	// See PxVehicleWheelData::mMaxHandBrakeTorque for details
		float						mWheelMaxSteerFront;			// See PxVehicleWheelData::mMaxSteer for details
		float						mWheelMaxSteerRear;				// See PxVehicleWheelData::mMaxSteer for details
		udword						mWheelSubsteps;					// See PxVehicleWheelsSimData::setSubStepCount for details
		float						mFrontTireFrictionMultiplier;
		float						mRearTireFrictionMultiplier;
		float						mAntirollBarStiffness;
		// Engine
		float						mEngineMOI;						// See PxVehicleEngineData::mMOI for details
		float						mEnginePeakTorque;				// See PxVehicleEngineData::mPeakTorque for details
		float						mEngineMaxOmega;				// See PxVehicleEngineData::mMaxOmega for details
		// Gears
		float						mGearsSwitchTime;				// See PxVehicleGearsData::mSwitchTime for details
		// Clutch
		float						mClutchStrength;				// See PxVehicleClutchData::mStrength for details
		// Differential
		PintVehicleDifferential		mDifferential;					// See PxVehicleDifferential4WData for details
		float						mFrontRearSplit;				// See PxVehicleDifferential4WData for details
		float						mFrontLeftRightSplit;			// See PxVehicleDifferential4WData for details
		float						mRearLeftRightSplit;			// See PxVehicleDifferential4WData for details
		float						mCentreBias;					// See PxVehicleDifferential4WData for details
		float						mFrontBias;						// See PxVehicleDifferential4WData for details
		float						mRearBias;						// See PxVehicleDifferential4WData for details
		// Suspension
		float						mSuspMaxCompression;			// See PxVehicleSuspensionData::mMaxCompression for details
		float						mSuspMaxDroop;					// See PxVehicleSuspensionData::mMaxDroop for details
		float						mSuspSpringStrength;			// See PxVehicleSuspensionData::mSpringStrength for details
		float						mSuspSpringDamperRate;			// See PxVehicleSuspensionData::mSpringDamperRate for details
		float						mSuspCamberAngleAtRest;			// See PxVehicleSuspensionData::mCamberAtRest for details
		float						mSuspCamberAngleAtMaxCompr;		// See PxVehicleSuspensionData::mCamberAtMaxCompression for details
		float						mSuspCamberAngleAtMaxDroop;		// See PxVehicleSuspensionData::mCamberAtMaxDroop for details
		// Steering
		float						mSteerVsForwardSpeedData[8];
		float						mAckermannAccuracy;				// See PxVehicleAckermannGeometryData::mAccuracy for details
		// Controls
		float						mSmoothingData[16];
		float						mThresholdForwardSpeed;
		bool						mImmediateAutoReverse;
	};

	struct PintVehicleData
	{
		inline_ PintVehicleData() : mChassisActor(null), mChassisShape(null)
		{
			for(udword i=0;i<4;i++)
				mWheelShapes[i] = null;
		}

		PintActorHandle	mChassisActor;
		PintShapeHandle	mChassisShape;
		PintShapeHandle	mWheelShapes[4];
	};

	struct PintVehicleInfo
	{
		float	mForwardSpeed;
		float	mSidewaysSpeed;
		udword	mCurrentGear;
		float	mRevs;
	};

	class Pint_Vehicle : public Allocateable
	{
//									PREVENT_COPY(Pint_Vehicle)
		public:
									Pint_Vehicle(Pint& pint) : mPint(pint)	{}
		virtual						~Pint_Vehicle()							{}

		virtual	PintVehicleHandle	CreateVehicle(PintVehicleData& data, const PINT_VEHICLE_CREATE& vehicle)	{ mPint.NotImplemented("Pint_Vehicle::CreateVehicle");		return null;	}
		virtual	bool				SetVehicleInput(PintVehicleHandle vehicle, const PINT_VEHICLE_INPUT& input)	{ return mPint.NotImplemented("Pint_Vehicle::SetVehicleInput");				}
		virtual	PintActorHandle		GetVehicleActor(PintVehicleHandle vehicle)							const	{ mPint.NotImplemented("Pint_Vehicle::GetVehicleActor");	return null;	}
		virtual	bool				GetVehicleInfo(PintVehicleHandle vehicle, PintVehicleInfo& info)	const	{ return mPint.NotImplemented("Pint_Vehicle::GetVehicleInfo");				}
		virtual	bool				ResetVehicleData(PintVehicleHandle vehicle)									{ return mPint.NotImplemented("Pint_Vehicle::ResetVehicleData");			}
		virtual	bool				AddActor(PintVehicleHandle vehicle, PintActorHandle actor)					{ return mPint.NotImplemented("Pint_Vehicle::AddActor");					}
		virtual	bool				AddShape(PintVehicleHandle vehicle, const PINT_SHAPE_CREATE& create)		{ return mPint.NotImplemented("Pint_Vehicle::AddShape");					}

		protected:
				Pint&				mPint;
	};

	///////////////////////////////////////////////////////////////////////////

	enum PintCharacterCollisionFlag
	{
		PINT_CHARACTER_COLLISION_SIDES	= (1<<0),
		PINT_CHARACTER_COLLISION_UP		= (1<<1),
		PINT_CHARACTER_COLLISION_DOWN	= (1<<2)
	};

	struct PINT_CHARACTER_CREATE : public Allocateable
	{
		PINT_CHARACTER_CREATE()
		{
			mPosition = Point(0.0f, 0.0f, 0.0f);
		}

		Point				mPosition;
		PINT_CAPSULE_CREATE	mCapsule;
	};

	class Pint_Character : public Allocateable
	{
									PREVENT_COPY(Pint_Character)
		public:
									Pint_Character(Pint& pint) : mPint(pint)	{}
		virtual						~Pint_Character()							{}

		virtual	PintCharacterHandle	CreateCharacter(const PINT_CHARACTER_CREATE& create)	{ mPint.NotImplemented("CreateCharacter");		return null;	}
		virtual	PintActorHandle		GetCharacterActor(PintCharacterHandle h)				{ mPint.NotImplemented("GetCharacterActor");	return null;	}
		virtual	udword				MoveCharacter(PintCharacterHandle h, const Point& disp)	{ mPint.NotImplemented("MoveCharacter");		return 0;		}

		protected:
				Pint&				mPint;
	};

	///////////////////////////////////////////////////////////////////////////

	class PintPlugin : public Allocateable
	{
		public:
							PintPlugin()																	{}
		virtual				~PintPlugin()																	{}

		virtual	IceWindow*	InitGUI(IceWidget* parent, PintGUIHelper& helper)								= 0;
		virtual	void		CloseGUI()																		= 0;

		virtual	void		Init(const PINT_WORLD_CREATE& desc)												= 0;
		virtual	void		Close()																			= 0;

		virtual	Pint*		GetPint()																		= 0;

		virtual	IceWindow*	InitTestGUI(const char* test_name, IceWidget* parent, PintGUIHelper& helper, Widgets& owner)	{ return null;	}
		virtual	void		CloseTestGUI()																					{}
		virtual	const char*	GetTestGUIName()																				{ return null;	}
		virtual	void		ApplyTestUIParams(const char* test_name)														{}
	};

#endif
