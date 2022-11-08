///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX4_IMM_H
#define PINT_COMMON_PHYSX4_IMM_H

#define SHAPE_CENTRIC

	class BlockBasedAllocator
	{
		struct AllocationPage
		{
			static const PxU32 PageSize = 32 * 1024;
			PxU8 mPage[PageSize];

			PxU32 currentIndex;

			AllocationPage() : currentIndex(0){}

			PxU8* allocate(const PxU32 size)
			{
				PxU32 alignedSize = (size + 15)&(~15);
				if ((currentIndex + alignedSize) < PageSize)
				{
					PxU8* ret = &mPage[currentIndex];
					currentIndex += alignedSize;
					return ret;
				}
				return NULL;
			}
		};

		AllocationPage* currentPage;

		_array<AllocationPage*> mAllocatedBlocks;
		PxU32 mCurrentIndex;

	public:
		BlockBasedAllocator() : currentPage(NULL), mCurrentIndex(0)
		{
		}

		virtual PxU8* allocate(const PxU32 byteSize)
		{
			if (currentPage)
			{
				PxU8* data = currentPage->allocate(byteSize);
				if (data)
					return data;
			}

			if (mCurrentIndex < mAllocatedBlocks.size())
			{
				currentPage = mAllocatedBlocks[mCurrentIndex++];
				currentPage->currentIndex = 0;
				return currentPage->allocate(byteSize);
			}
			currentPage = PX_PLACEMENT_NEW(PX_ALLOC(sizeof(AllocationPage), "AllocationPage"), AllocationPage)();
			mAllocatedBlocks.pushBack(currentPage);
			mCurrentIndex = mAllocatedBlocks.size();

			return currentPage->allocate(byteSize);
		}

		void release() { for (PxU32 a = 0; a < mAllocatedBlocks.size(); ++a) PX_FREE(mAllocatedBlocks[a]); mAllocatedBlocks.clear(); currentPage = NULL; mCurrentIndex = 0; }

		void reset() { currentPage = NULL; mCurrentIndex = 0; }

		virtual ~BlockBasedAllocator()
		{
			release();
		}
	};

	class TestCacheAllocator : public PxCacheAllocator
	{
		BlockBasedAllocator mAllocator[2];
		PxU32 currIdx;

	public:

		TestCacheAllocator() : currIdx(0)
		{
		}

		virtual PxU8* allocateCacheData(const PxU32 byteSize)
		{
			return mAllocator[currIdx].allocate(byteSize);
		}

		void release() { currIdx = 1 - currIdx; mAllocator[currIdx].release(); }

		void reset() { currIdx = 1 - currIdx; mAllocator[currIdx].reset(); }

		virtual ~TestCacheAllocator(){}
	};

	class TestConstraintAllocator : public PxConstraintAllocator
	{
		BlockBasedAllocator mConstraintAllocator;
		BlockBasedAllocator mFrictionAllocator[2];

		PxU32 currIdx;
	public:

		TestConstraintAllocator() : currIdx(0)
		{
		}
		virtual PxU8* reserveConstraintData(const PxU32 byteSize){ return mConstraintAllocator.allocate(byteSize); }
		virtual PxU8* reserveFrictionData(const PxU32 byteSize){ return mFrictionAllocator[currIdx].allocate(byteSize); }

		void release() { currIdx = 1 - currIdx; mConstraintAllocator.release(); mFrictionAllocator[currIdx].release(); }

		virtual ~TestConstraintAllocator() {}
	};

#ifdef PX_DEBUG
	struct ImmActorHandleImpl;	typedef ImmActorHandleImpl*	ImmActorHandle;
	struct ImmShapeHandleImpl;	typedef ImmShapeHandleImpl*	ImmShapeHandle;
#else
	typedef PxU32	ImmActorHandle;
	typedef PxU32	ImmShapeHandle;
#endif

	struct ActorPair
	{
		PX_FORCE_INLINE	ActorPair()																	{}
		PX_FORCE_INLINE	ActorPair(ImmActorHandle id0, ImmActorHandle id1) : mID0(id0), mID1(id1)	{}
		PX_FORCE_INLINE	ActorPair(const ActorPair& that) : mID0(that.mID0), mID1(that.mID1)			{}

		ImmActorHandle	mID0;
		ImmActorHandle	mID1;

		PX_FORCE_INLINE	ActorPair& operator=(const ActorPair& that)
		{
			mID0 = that.mID0;
			mID1 = that.mID1;
			return *this;
		}

		PX_FORCE_INLINE	bool operator == (const ActorPair& other) const
		{
			return mID0 == other.mID0 && mID1 == other.mID1;
		}
	};

	PX_FORCE_INLINE uint32_t _computeHash(const ActorPair& p)
	{
		return ps::_computeHash(uint64_t(p.mID0)|(uint64_t(p.mID1)<<32));
	}

	struct MassProps
	{
		PxVec3	mInvInertia;
		float	mInvMass;
	};

	void imm_computeMassProps(MassProps& props, const PxGeometry& geometry, float mass);

	// WITH_PERSISTENCY
	struct PersistentContactPair
	{
		PersistentContactPair()
		{
			reset();
		}

		PxCache	cache;
		PxU8*	frictions;
		PxU32	nbFrictions;

		PX_FORCE_INLINE	void	reset()
		{
			cache = PxCache();
			frictions = NULL;
			nbFrictions = 0;
		}
	};

	// TEST_IMMEDIATE_JOINTS
	struct MyJointData : Ext::JointData
	{
		ImmActorHandle	mActors[2];
		PxTransform		mLocalFrames[2];

		void			initInvMassScale()
		{
			invMassScale.linear0	= 1.0f;
			invMassScale.angular0	= 1.0f;
			invMassScale.linear1	= 1.0f;
			invMassScale.angular1	= 1.0f;
		}
	};

	class PintShapeRenderer;

	class ImmediateShape
	{
		public:
			ImmediateShape(const PxGeometry& geom, const PxTransform& localPose, void* userData) :
				mGeom		(geom),
				mLocalPose	(localPose),
				mUserData	(userData)
			{
			}
			ImmediateShape()	{}
			~ImmediateShape()	{}

			PxGeometryHolder	mGeom;
			PxTransform			mLocalPose;
			void*				mUserData;
	};

	class ImmediateActor
	{
		public:
			enum Type
			{
				eSTATIC,
				eDYNAMIC,
				eLINK,
				eFORCE_DWORD	= 0x7fffffff,
			};

			ImmediateActor(Type type, PxU32 group) : mType(type), mCollisionGroup(group), mNbShapes(0)	{}
			ImmediateActor()	{}
			~ImmediateActor()	{}

			Type						mType;
			PxU32						mCollisionGroup;
			MassProps					mMassProps;
			PxU32						mNbShapes;
			ImmShapeHandle				mShapeHandle[16];	//### temp
			PxVec3						mLinearVelocity;
			PxVec3						mAngularVelocity;
			PxVec3						mExternalForce;		//### TODO: move this to separate array when it works
			Dy::ArticulationLinkHandle	mLink;
	};

	struct BPEntry
	{
		PX_FORCE_INLINE	BPEntry()																			{}
		PX_FORCE_INLINE	BPEntry(ImmActorHandle actor, ImmShapeHandle shape) : mActor(actor), mShape(shape)	{}
		PX_FORCE_INLINE	BPEntry(const BPEntry& that) : mActor(that.mActor), mShape(that.mShape)				{}

		ImmActorHandle	mActor;
		ImmShapeHandle	mShape;

		PX_FORCE_INLINE	BPEntry& operator=(const BPEntry& that)
		{
			mActor = that.mActor;
			mShape = that.mShape;
			return *this;
		}

		PX_FORCE_INLINE	bool operator == (const BPEntry& other) const
		{
			return mActor == other.mActor && mShape == other.mShape;
		}
	};

	struct BPEntryPair
	{
		PxU32	mBPEntry0;
		PxU32	mBPEntry1;
	};

	PX_FORCE_INLINE uint32_t hash(const BPEntryPair& p)
	{
		return ps::_computeHash(uint64_t(p.mBPEntry0)|(uint64_t(p.mBPEntry1)<<32));
	}

	struct ShapePair
	{
		PX_FORCE_INLINE	ShapePair()																	{}
		PX_FORCE_INLINE	ShapePair(ImmShapeHandle id0, ImmShapeHandle id1) : mID0(id0), mID1(id1)	{}
		PX_FORCE_INLINE	ShapePair(const ShapePair& that) : mID0(that.mID0), mID1(that.mID1)			{}

		ImmShapeHandle	mID0;
		ImmShapeHandle	mID1;

		PX_FORCE_INLINE	ShapePair& operator=(const ShapePair& that)
		{
			mID0 = that.mID0;
			mID1 = that.mID1;
			return *this;
		}

		PX_FORCE_INLINE	bool operator == (const ShapePair& other) const
		{
			return mID0 == other.mID0 && mID1 == other.mID1;
		}
	};

	PX_FORCE_INLINE uint32_t _computeHash(const ShapePair& p)
	{
		return ps::_computeHash(uint64_t(p.mID0)|(uint64_t(p.mID1)<<32));
	}

	struct ImmRaycastHit
	{
		ImmActorHandle	mTouchedActor;
		ImmShapeHandle	mTouchedShape;
		PxVec3			mPos;
		PxVec3			mNormal;
		float			mDistance;
		PxU32			mFaceIndex;
	};

	class ImmediateScene
	{
															PX_NOCOPY(ImmediateScene)
		public:
															ImmediateScene(bool useTGS, bool usePersistency, bool batchContacts);
															~ImmediateScene();

						void								reset();

						ImmShapeHandle						createShape(const PxGeometry& geometry, const PxTransform& localPose, void* userData);
#ifdef SHAPE_CENTRIC
						ImmActorHandle						createActor(PxU32 nbShapes, const ImmShapeHandle* shapes, const PxTransform& pose, PxU32 group, const MassProps* massProps, Dy::ArticulationLinkHandle link);
#else
						ImmActorHandle						createActor(ImmShapeHandle shape, const PxTransform& pose, PxU32 group, const MassProps* massProps, Dy::ArticulationLinkHandle link);
#endif
//						bool								addShape(ImmActorHandle actor, ImmShapeHandle shape);

						ImmActorHandle						createGroundPlane()
															{
																const ImmShapeHandle planeShape = createShape(PxPlaneGeometry(), PxTransformFromPlaneEquation(PxPlane(0.0f, 1.0f, 0.0f, 0.0f)), null);
																return createActor(1, &planeShape, PxTransform(PxIdentity), 0, null, null);
															}

						Dy::ArticulationV*					createArticulation(bool fixBase);
						void								addArticulationToScene(Dy::ArticulationV*);

						// TEST_IMMEDIATE_JOINTS
						void								createSphericalJoint(ImmActorHandle id0, ImmActorHandle id1, const PxTransform& localFrame0, const PxTransform& localFrame1, const PxTransform* pose0=NULL, const PxTransform* pose1=NULL);

						void								updateArticulations(float dt, const PxVec3& gravity, PxU32 nbIterPos);
						void								updateBounds(float boundsInflation);
						void								broadPhase();
						void								narrowPhase(float contactDistance, float meshContactMargin, float toleranceLength, float staticFriction, float dynamicFriction, float restitution);
						void								buildSolverBodyData(float dt, const PxVec3& gravity, float maxDepenetrationVelocity, float maxContactImpulse, float linearDamping, float angularDamping, float maxLinearVelocity, float maxAngularVelocity);
						void								buildSolverConstraintDesc();
						void								createContactConstraints(float dt, float invDt, float bounceThreshold, float frictionOffsetThreshold, float correlationDistance, float lengthScale, const PxU32 nbPosIterations);
						void								solveAndIntegrate(float dt, PxU32 nbIterPos, PxU32 nbIterVel);

		PX_FORCE_INLINE	PxU32								raycastShape(PxU32 i, PxU32 maxNbHits, PxGeomRaycastHit* hits, const PxVec3& origin, const PxVec3& dir, float dist, PxHitFlags flags)
															{
																const PxBounds3& bounds = mShapeBounds[i];
																const BPEntry& entry = mBPEntries[i];

																const ImmediateActor& actor = getActor(entry.mActor);
																const ImmediateShape& shape = getShape(entry.mShape);
																const PxTransform& actorPose = getActorGlobalPose(entry.mActor);
																const PxGeometry& geom = shape.mGeom.any();
																const PxTransform& shapeLocalPose = shape.mLocalPose;
																const PxTransform shapePose = actorPose * shapeLocalPose;
#ifdef PHYSX_NEW_PUBLIC_API
																return PxGeometryQuery::raycast(origin, dir, geom, shapePose, dist, flags, maxNbHits, hits, sizeof(PxGeomRaycastHit), PxGeometryQueryFlag::Enum(0));
//																return PxGeometryQuery::raycastXP(origin, dir, geom, shapePose, dist, flags, maxNbHits, hits, sizeof(PxGeomRaycastHit));
#else
																return PxGeometryQuery::raycast(origin, dir, geom, shapePose, dist, flags, maxNbHits, hits);
#endif
															}

		PX_FORCE_INLINE	bool								overlapShape(PxU32 i, const PxGeometry& queryVolume, const PxTransform& queryPose)
															{
																const PxBounds3& bounds = mShapeBounds[i];
																const BPEntry& entry = mBPEntries[i];

																const ImmediateActor& actor = getActor(entry.mActor);
																const ImmediateShape& shape = getShape(entry.mShape);
																const PxTransform& actorPose = getActorGlobalPose(entry.mActor);
																const PxGeometry& geom = shape.mGeom.any();
																const PxTransform& shapeLocalPose = shape.mLocalPose;
																const PxTransform shapePose = actorPose * shapeLocalPose;
#ifdef PHYSX_NEW_PUBLIC_API
																return PxGeometryQuery::overlap(queryVolume, queryPose, geom, shapePose, PxGeometryQueryFlag::Enum(0));
//																return PxGeometryQuery::overlapXP(queryVolume, queryPose, geom, shapePose);
#else
																return PxGeometryQuery::overlap(queryVolume, queryPose, geom, shapePose);
#endif
															}

		PX_FORCE_INLINE	bool								sweepShape(PxU32 i, PxGeomSweepHit& hit, const PxGeometry& queryVolume, const PxTransform& queryPose, const PxVec3& dir, float dist, PxHitFlags flags)
															{
																const PxBounds3& bounds = mShapeBounds[i];
																const BPEntry& entry = mBPEntries[i];

																const ImmediateActor& actor = getActor(entry.mActor);
																const ImmediateShape& shape = getShape(entry.mShape);
																const PxTransform& actorPose = getActorGlobalPose(entry.mActor);
																const PxGeometry& geom = shape.mGeom.any();
																const PxTransform& shapeLocalPose = shape.mLocalPose;
																const PxTransform shapePose = actorPose * shapeLocalPose;
#ifdef PHYSX_NEW_PUBLIC_API
																return PxGeometryQuery::sweep(dir, dist, queryVolume, queryPose, geom, shapePose, hit, flags, 0.0f, PxGeometryQueryFlag::Enum(0));
#else
																return PxGeometryQuery::sweep(dir, dist, queryVolume, queryPose, geom, shapePose, hit, flags, 0.0f);
#endif
															}

						bool								raycastClosest(PxGeomRaycastHit& pxHit, float& minDist, PxU32 i, const PxVec3& origin, const PxVec3& dir, float dist, ImmRaycastHit& hit);
						bool								raycastClosest(const PxVec3& origin, const PxVec3& dir, float dist, ImmRaycastHit& hit);

#ifdef SHAPE_CENTRIC
		PX_FORCE_INLINE	const BPEntry&						GetEntry(PxU32 i)	const	{ return mBPEntries[i];	}
#endif

		private:
						TestCacheAllocator*					mCacheAllocator;
						TestConstraintAllocator*			mConstraintAllocator;
						_array<PxTransform>					mActorGlobalPoses;
#ifdef SHAPE_CENTRIC
						_array<PxBounds3>					mShapeBounds;
						_array<BPEntry>						mBPEntries;
#endif
//						_array<PxBounds3>					mBounds;
						_array<ImmediateActor>				mActors;
//						_array<ImmediateActor>				mStaticActors;
//						_array<ImmediateActor>				mDynamicActors;
						_array<ImmediateShape>				mShapes;
		public:

		PX_FORCE_INLINE	ImmediateShape&						getShape(ImmShapeHandle handle)
															{
																const PxU32 shapeIndex = PxU32(size_t(handle));
																PX_ASSERT(shapeIndex<mShapes.size());
																return mShapes[shapeIndex];
															}

		PX_FORCE_INLINE	const PxBounds3*					getBounds(PxU32& nbBounds)	const
															{
#ifdef SHAPE_CENTRIC
																nbBounds = mShapeBounds.size();
																return mShapeBounds.begin();
#else
																nbBounds = mBounds.size();
																return mBounds.begin();
#endif
															}

		PX_FORCE_INLINE	const ImmediateActor*				getActors(PxU32& nbActors)	const
															{
																nbActors = mActors.size();
																return mActors.begin();
															}

		PX_FORCE_INLINE	ImmediateActor&						getActor(ImmActorHandle handle)
															{
																const PxU32 actorIndex = PxU32(size_t(handle));
																PX_ASSERT(actorIndex<mActors.size());
																return mActors[actorIndex];
															}

		PX_FORCE_INLINE	const PxTransform*					getActorGlobalPoses(PxU32& nbPoses)	const
															{
																nbPoses = mActorGlobalPoses.size();
																return mActorGlobalPoses.begin();
															}

		PX_FORCE_INLINE	const PxTransform&					getActorGlobalPose(ImmActorHandle handle)	const
															{
																const PxU32 actorIndex = PxU32(size_t(handle));
																PX_ASSERT(actorIndex<mActorGlobalPoses.size());
																return mActorGlobalPoses[actorIndex];
															}

		PX_FORCE_INLINE	void								setActorGlobalPose(ImmActorHandle handle, const PxTransform& pose)
															{
																const PxU32 actorIndex = PxU32(size_t(handle));
																PX_ASSERT(actorIndex<mActorGlobalPoses.size());
																mActorGlobalPoses[actorIndex] = pose;
															}

		PX_FORCE_INLINE	void								disableCollision(ImmActorHandle i, ImmActorHandle j)
															{
																if(i>j)
																	TSwap(i, j);
																mFilteredPairs.insert(ActorPair(i, j));
															}

		PX_FORCE_INLINE	bool								isCollisionDisabled(ImmActorHandle i, ImmActorHandle j)	const
															{
																if(i>j)
																	TSwap(i, j);
																return mFilteredPairs.contains(ActorPair(i, j));
															}
		private:
						_array<PxTGSSolverBodyData>			mTGSSolverBodyData;			// For TGS
						_array<PxTGSSolverBodyVel>			mTGSSolverBodies;			// For TGS
						_array<PxTGSSolverBodyTxInertia>	mTGSSolverBodyTxInertias;	// For TGS
						_array<PxSolverBodyData>			mSolverBodyData;			// For PGS
						_array<PxSolverBody>				mSolverBodies;				// For PGS
						_array<Dy::ArticulationV*>			mArticulations;
						_array<MyJointData>					mJointData;					// TEST_IMMEDIATE_JOINTS
#ifdef SHAPE_CENTRIC
						_array<BPEntryPair>					mBroadphasePairs;
#else
						_array<ActorPair>					mBroadphasePairs;
#endif
						_hashset<ActorPair>					mFilteredPairs;

#ifdef SHAPE_CENTRIC
						struct ContactPair
						{
							PX_FORCE_INLINE	ContactPair()	{}

							PX_FORCE_INLINE	ContactPair(const ActorPair& actorPair, const ShapePair& shapePair, PxU32 nbContacts, PxU32 startIndex) :
										mActorPair(actorPair), mShapePair(shapePair), mNbContacts(nbContacts), mStartContactIndex(startIndex)	{}

							PX_FORCE_INLINE	ContactPair(const ContactPair& that) :
										mActorPair(that.mActorPair),
										mShapePair(that.mShapePair),
										mNbContacts(that.mNbContacts), mStartContactIndex(that.mStartContactIndex)	{}

							PX_FORCE_INLINE	ContactPair& operator=(const ContactPair& that)
							{
								mActorPair = that.mActorPair;
								mShapePair = that.mShapePair;
								mNbContacts = that.mNbContacts;
								mStartContactIndex = that.mStartContactIndex;
								return *this;
							}

							ActorPair		mActorPair;
							ShapePair		mShapePair;
							PxU32			mNbContacts;
							PxU32			mStartContactIndex;
						};
#else
						struct ContactPair : ActorPair
						{
							PX_FORCE_INLINE	ContactPair()	{}

							PX_FORCE_INLINE	ContactPair(const ActorPair& pair, PxU32 nbContacts, PxU32 startIndex) :
										ActorPair(pair), mNbContacts(nbContacts), mStartContactIndex(startIndex)	{}

							PX_FORCE_INLINE	ContactPair(const ContactPair& that) : ActorPair(that), mNbContacts(that.mNbContacts), mStartContactIndex(that.mStartContactIndex)	{}

							PX_FORCE_INLINE	ContactPair& operator=(const ContactPair& that)
							{
								mID0 = that.mID0;
								mID1 = that.mID1;
								mNbContacts = that.mNbContacts;
								mStartContactIndex = that.mStartContactIndex;
								return *this;
							}

							PxU32			mNbContacts;
							PxU32			mStartContactIndex;
						};
#endif
						// PT: we use separate arrays here because the immediate mode API expects an array of Gu::ContactPoint
						_array<ContactPair>					mContactPairs;
						_array<Gu::ContactPoint>				mContactPoints;
#ifdef SHAPE_CENTRIC
				_hashmap<ShapePair, PersistentContactPair>	mPersistentPairs;				// WITH_PERSISTENCY
#else
				_hashmap<ActorPair, PersistentContactPair>	mPersistentPairs;				// WITH_PERSISTENCY
#endif
						_array<PxSolverConstraintDesc>		mSolverConstraintDesc;
						_array<PxSolverConstraintDesc>		mOrderedSolverConstraintDesc;	// BATCH_CONTACTS
						_array<PxConstraintBatchHeader>		mHeaders;
						_array<PxReal>						mContactForces;
						_array<PxVec3>						mMotionLinearVelocity;	// Persistent to avoid runtime allocations but could be managed on the stack
						_array<PxVec3>						mMotionAngularVelocity;	// Persistent to avoid runtime allocations but could be managed on the stack

						PxU32								mNbStaticActors;
						PxU32								mNbArticulationLinks;

						const bool							mUseTGS;

						// Enables whether we want persistent state caching (contact cache, friction caching) or not.
						// Avoiding persistency results in one-shot collision detection and zero friction 
						// correlation but simplifies code by not longer needing to cache persistent pairs.
						const bool							mUsePersistency;

						//Toggles whether we batch constraints or not. Constraint batching is an optional process which
						// can improve performance by grouping together independent constraints. These independent constraints
						//can be solved in parallel by using multiple lanes of SIMD registers.
						const bool							mBatchContacts;

		template<class T>
						void								processContactHeader(PxConstraintBatchHeader& header, PxSolverConstraintDesc* orderedDescs, float invDt, float stepInvDt, float bounceThreshold, float frictionOffsetThreshold, float correlationDistance);
		template<class T>
						void								processJointHeader(PxConstraintBatchHeader& header, PxSolverConstraintDesc* orderedDescs, float dt, float invDt, float stepDt, float stepInvDt, float lengthScale);
	};

#endif
