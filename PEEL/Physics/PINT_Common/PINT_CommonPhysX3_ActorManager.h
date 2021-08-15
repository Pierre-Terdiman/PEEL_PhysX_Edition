///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_ACTOR_MANAGER_H
#define PINT_COMMON_PHYSX3_ACTOR_MANAGER_H

	struct ActorData
	{
		PxRigidActor*	mActor;
		PxShape*		mSingleShape;	// Valid if non-compound, and local-pose == identity
	};
	#define	ACTOR_DATA_SIZE	(sizeof(ActorData)/sizeof(udword))

	struct ActorCachedData
	{
		PxBounds3	mGlobalBounds;
		PxTransform	mGlobalPose;
	};

	class ActorManager
	{
		public:
										ActorManager();
										~ActorManager();

				void					Reset();

				void					Add(PxRigidActor* actor);
				bool					Remove(PxRigidActor* actor);

		inline_	ActorData*				GetActorData(PxRigidActor* actor)	const;

		inline_	udword					GetNbStaticActors()				const	{ return mStaticActors.GetNbEntries()/ACTOR_DATA_SIZE;						}
		inline_	udword					GetNbDynamicActors()			const	{ return mDynamicActors.GetNbEntries()/ACTOR_DATA_SIZE;						}
		inline_	const ActorData*		GetStaticActors()				const	{ return reinterpret_cast<const ActorData*>(mStaticActors.GetEntries());	}
		inline_	const ActorData*		GetDynamicActors()				const	{ return reinterpret_cast<const ActorData*>(mDynamicActors.GetEntries());	}

		inline_	void					UpdateTimestamp()						{ mTimestamp++;	}
		inline_	const ActorCachedData*	GetStaticActorsCachedData()		const
										{
											const_cast<ActorManager*>(this)->UpdateCachedData();
											return reinterpret_cast<const ActorCachedData*>(mStaticActors_CachedData.GetEntries());
										}
		inline_	const ActorCachedData*	GetDynamicActorsCachedData()	const
										{
											const_cast<ActorManager*>(this)->UpdateCachedData();
											return reinterpret_cast<const ActorCachedData*>(mDynamicActors_CachedData.GetEntries());
										}
		private:
				Container				mStaticActors;
				Container				mDynamicActors;
				udword					mTimestamp;
				udword					mCachedTimestamp;
				Container				mStaticActors_CachedData;
				Container				mDynamicActors_CachedData;
				void					UpdateCachedData();
	};

#endif