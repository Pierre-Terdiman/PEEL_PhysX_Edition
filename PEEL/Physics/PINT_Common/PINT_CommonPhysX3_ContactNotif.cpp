///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\Pint.h"

#include "..\PINT_Common\PINT_Common.h"

#include "PINT_CommonPhysX3.h"
#include "PINT_CommonPhysX3_ContactNotif.h"

static inline_	PintActorHandle CreateHandle(PxActor* actor)
{
	return CreateHandle(static_cast<PxRigidActor*>(actor));
}

static void	FillContactData(PintContactData& dst, const PxContactPairPoint& src, const PintActorHandle handle0, const PintActorHandle handle1)
{
	dst.mObject0			= handle0;
	dst.mObject1			= handle1;
	dst.mWorldPos			= ToPoint(src.position);
	dst.mSeparation			= src.separation;
	dst.mNormal				= ToPoint(src.normal);
	dst.mInternalFaceIndex0	= src.internalFaceIndex0;
	dst.mImpulse			= ToPoint(src.impulse);
	dst.mInternalFaceIndex1	= src.internalFaceIndex1;
}

void PEEL_SimulationEventCallback::Init(Pint& owner, PintContactNotifyCallback* cb)
{
	mOwner = &owner;
	mContactCallback = cb;
}

void PEEL_SimulationEventCallback::Release()
{
	mContactData.Empty();
}

void PEEL_SimulationEventCallback::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)
{
}

void PEEL_SimulationEventCallback::onWake(PxActor** actors, PxU32 count)
{
}

void PEEL_SimulationEventCallback::onSleep(PxActor** actors, PxU32 count)
{
}

void PEEL_SimulationEventCallback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	//printf("CHECKPOINT\n");

	if(!mContactCallback)
		return;

	//#######
/*		if(pairHeader.actors[0]->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC
		&& pairHeader.actors[1]->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC
		)
		return;*/


	const bool bufferContacts = mContactCallback->BufferContacts();

	const PintActorHandle handle0 = CreateHandle(pairHeader.actors[0]);
	const PintActorHandle handle1 = CreateHandle(pairHeader.actors[1]);

	const PxU32 bufferSize = 1024;	//###
	PxContactPairPoint contacts[bufferSize];
	for(PxU32 i=0; i<nbPairs; i++)
	{
		const PxContactPair& cp = pairs[i];
//		if (!(cp.events & (PxPairFlag::eNOTIFY_TOUCH_LOST | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_LOST)))
		{
			const PxU32 nbContacts = pairs[i].extractContacts(contacts, bufferSize);
			if(nbContacts)
			{
				for(PxU32 j=0; j<nbContacts; j++)
				{
					// ### hardcoded experiments for voronoi tests, to revisit
#ifdef USE_FORCE_THRESHOLD
					PintContactData* C = (PintContactData*)mContactData.Reserve(sizeof(PintContactData)/sizeof(udword));
					FillContactData(*C, contacts[j], handle0, handle1);
#else
					const float Force = contacts[j].impulse.magnitude()*60.0f;
//						printf("Force: %f\n", Force);
					if(!bufferContacts || Force>1000.0f)	// ### disabled for contact notify test
					{
						PintContactData* C = (PintContactData*)mContactData.Reserve(sizeof(PintContactData)/sizeof(udword));
						FillContactData(*C, contacts[j], handle0, handle1);
					}
#endif
				}
			}
		}
	}

	if(!bufferContacts)
		sendContacts();
}

void PEEL_SimulationEventCallback::sendContacts()
{
	if(mContactData.GetNbEntries())
	{
		if(mContactCallback)
		{
			const udword NbContacts = mContactData.GetNbEntries() / (sizeof(PintContactData) / sizeof(udword));
			ASSERT(mOwner);
			mContactCallback->OnContact(*mOwner, NbContacts, (const PintContactData*)mContactData.GetEntries());
		}
		mContactData.Reset();
	}
}

void PEEL_SimulationEventCallback::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
}

void PEEL_SimulationEventCallback::onAdvance(const PxRigidBody*const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count)
{
}


