///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_CONTACT_NOTIF_H
#define PINT_COMMON_PHYSX3_CONTACT_NOTIF_H

	class PEEL_SimulationEventCallback : public PxSimulationEventCallback
	{
		public:
											PEEL_SimulationEventCallback() : mContactCallback(null), mOwner(null)	{}

				void						Init(Pint& owner, PintContactNotifyCallback* cb);
				void						Release();

		// PxSimulationEventCallback
		virtual void						onConstraintBreak(PxConstraintInfo* constraints, PxU32 count);
		virtual void						onWake(PxActor** actors, PxU32 count);
		virtual void						onSleep(PxActor** actors, PxU32 count);
		virtual void						onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs);
		virtual void						onTrigger(PxTriggerPair* pairs, PxU32 count);
		virtual void						onAdvance(const PxRigidBody*const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count);
		//~PxSimulationEventCallback

				void						sendContacts();
		private:
				PintContactNotifyCallback*	mContactCallback;
				Pint*						mOwner;
				Container					mContactData;	// Buffered contact data, sent to users after simulation
	};

#endif