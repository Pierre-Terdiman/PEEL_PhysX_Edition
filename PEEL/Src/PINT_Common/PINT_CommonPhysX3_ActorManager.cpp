///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PINT_CommonPhysX3_ActorManager.h"

CHECK_CONTAINER_ITEM(ActorData)
CHECK_CONTAINER_ITEM(ActorCachedData)

ActorManager::ActorManager() : mTimestamp(0), mCachedTimestamp(INVALID_ID)
{
}

ActorManager::~ActorManager()
{
}

void ActorManager::Reset()
{
	mStaticActors.Reset();
	mDynamicActors.Reset();
	mTimestamp = 0;
	mCachedTimestamp = INVALID_ID;
}

void ActorManager::Add(PxRigidActor* actor)
{
	ASSERT(!actor->userData);
	Container* Actors;
	if(actor->getConcreteType()==PxConcreteType::eRIGID_STATIC)
	{
		Actors = &mStaticActors;
	}
	else
	{
		ASSERT(actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC || actor->getConcreteType()==PxConcreteType::eARTICULATION_LINK);													
		Actors = &mDynamicActors;
	}

	const udword Index = Actors->GetNbEntries()/ACTOR_DATA_SIZE;
	ActorData* AD = reinterpret_cast<ActorData*>(Actors->Reserve(ACTOR_DATA_SIZE));
	AD->mActor = actor;
	AD->mSingleShape = null;

	if(actor->getNbShapes()==1)
	{
		PxShape* shape = null;
		if(1==actor->getShapes(&shape, 1))
		{
			const PxTransform LocalPose = shape->getLocalPose();
			if(LocalPose.p.x==0.0f && LocalPose.p.y==0.0f && LocalPose.p.z==0.0f
				&& LocalPose.q.x==0.0f && LocalPose.q.y==0.0f && LocalPose.q.z==0.0f && LocalPose.q.w==1.0f)
			{
				AD->mSingleShape = shape;
			}
		}
	}
	actor->userData = reinterpret_cast<void*>(size_t(Index));

	mCachedTimestamp = INVALID_ID;
}

bool ActorManager::Remove(PxRigidActor* actor)
{
	Container* Actors;
	const size_t Index = size_t(actor->userData);
	if(actor->getConcreteType()==PxConcreteType::eRIGID_STATIC)
	{
		Actors = &mStaticActors;
	}
	else
	{
		ASSERT(actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC || actor->getConcreteType()==PxConcreteType::eARTICULATION_LINK);													
		Actors = &mDynamicActors;
	}

	ActorData* AD = reinterpret_cast<ActorData*>(Actors->GetEntries());
	udword Size = Actors->GetNbEntries()/ACTOR_DATA_SIZE;
	ASSERT(AD[Index].mActor==actor);
	const udword LastIndex = --Size;
	AD[Index] = AD[LastIndex];
	PxRigidActor* MovedActor = AD[Index].mActor;
	if(MovedActor!=actor)
	{
		ASSERT(udword(size_t(MovedActor->userData))==LastIndex);
		MovedActor->userData = reinterpret_cast<void*>(Index);
	}
	Actors->ForceSize(LastIndex*ACTOR_DATA_SIZE);

	mCachedTimestamp = INVALID_ID;
	return true;
}

ActorData* ActorManager::GetActorData(PxRigidActor* actor) const
{
	const Container* Actors;
	const size_t Index = size_t(actor->userData);
	if(actor->getConcreteType()==PxConcreteType::eRIGID_STATIC)
	{
		Actors = &mStaticActors;
	}
	else
	{
		ASSERT(actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC || actor->getConcreteType()==PxConcreteType::eARTICULATION_LINK);													
		Actors = &mDynamicActors;
	}

	ActorData* AD = reinterpret_cast<ActorData*>(Actors->GetEntries());
	ASSERT(AD[Index].mActor==actor);
	return AD + Index;
}

#define	ACTOR_CACHED_DATA_SIZE	(sizeof(ActorCachedData)/sizeof(udword))

void ActorManager::UpdateCachedData()
{
	if(mTimestamp==mCachedTimestamp)
		return;
	mCachedTimestamp = mTimestamp;

	{
		udword NbStaticActors = GetNbStaticActors();
		const ActorData* StaticActors = GetStaticActors();
		mStaticActors_CachedData.Reset();
		ActorCachedData* CachedStatic = reinterpret_cast<ActorCachedData*>(mStaticActors_CachedData.Reserve(NbStaticActors*ACTOR_CACHED_DATA_SIZE));
		while(NbStaticActors--)
		{
			CachedStatic->mGlobalBounds = StaticActors->mActor->getWorldBounds();
			CachedStatic->mGlobalPose = StaticActors->mActor->getGlobalPose();
			CachedStatic++;
			StaticActors++;
		}
	}

	{
		udword NbDynamicActors = GetNbDynamicActors();
		const ActorData* DynamicActors = GetDynamicActors();
		mDynamicActors_CachedData.Reset();
		ActorCachedData* CachedDynamic = reinterpret_cast<ActorCachedData*>(mDynamicActors_CachedData.Reserve(NbDynamicActors*ACTOR_CACHED_DATA_SIZE));
		while(NbDynamicActors--)
		{
			CachedDynamic->mGlobalBounds = DynamicActors->mActor->getWorldBounds();
			CachedDynamic->mGlobalPose = DynamicActors->mActor->getGlobalPose();
			CachedDynamic++;
			DynamicActors++;
		}
	}
}

