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

#include "extensions\PxExtensionsAPI.h"
//#include "common/PxIO.h"
#include "common/PxRenderBuffer.h"

#include "PINT_CommonPhysX3.h"
#include "PINT_CommonPhysX3_ContactModif.h"

PEEL_ContactModifyCallback::PEEL_ContactModifyCallback() : mOwner(null), mPairs(null), mNbPairs(0), mContactModifyCallback(null), mContactModifyCallback2(null)
{
}

void PEEL_ContactModifyCallback::Init(Pint& owner, PintContactModifyCallback* cb, PintContactModifyCallback2* cb2)
{
	mOwner = &owner;
	mContactModifyCallback = cb;
	mContactModifyCallback2 = cb2;
}

PintContactModifyPairHandle	PEEL_ContactModifyCallback::GetContactPairData(udword pair_index, ContactPairData& data, PR* pose0, PR* pose1)
{
	ASSERT(mPairs);
	ASSERT(pair_index<mNbPairs);
	if(pair_index>=mNbPairs)
		return null;

	const PxContactModifyPair& cp = mPairs[pair_index];
	data.mActor0		= PintActorHandle(cp.actor[0]);
	data.mActor1		= PintActorHandle(cp.actor[1]);
	data.mShape0		= PintShapeHandle(cp.shape[0]);
	data.mShape1		= PintShapeHandle(cp.shape[1]);
	data.mNbContacts	= cp.contacts.size();
	if(pose0)
		*pose0 = ToPR(cp.transform[0]);
	if(pose1)
		*pose1 = ToPR(cp.transform[1]);
	return PintContactModifyPairHandle(&cp);
}

void PEEL_ContactModifyCallback::GetContactData(PintContactModifyPairHandle handle, ContactData* data, udword nb_contacts)
{
	ASSERT(handle);
	const PxContactModifyPair* cp = reinterpret_cast<const PxContactModifyPair*>(handle);

	const PxContactSet& set = cp->contacts;
	ASSERT(nb_contacts==set.size());

	for(PxU32 j=0;j<nb_contacts;j++)
	{
		data[j].mPos				= ToPoint(set.getPoint(j));
		data[j].mSeparation			= set.getSeparation(j);
		data[j].mNormal				= ToPoint(set.getNormal(j));
		data[j].mInternalFaceIndex1	= const_cast<PxContactSet&>(set).getInternalFaceIndex1(j);	// ### const missing in PhysX
	}
}

void PEEL_ContactModifyCallback::IgnoreContact(PintContactModifyPairHandle handle, udword contact_index)
{
	ASSERT(handle);
	PxContactModifyPair* cp = reinterpret_cast<PxContactModifyPair*>(handle);
	ASSERT(contact_index<cp->contacts.size());
	cp->contacts.ignore(contact_index);
}

void PEEL_ContactModifyCallback::SetContactData(PintContactModifyPairHandle handle, udword contact_index, const ContactData& data)
{
	ASSERT(handle);
	PxContactModifyPair* cp = reinterpret_cast<PxContactModifyPair*>(handle);
	ASSERT(contact_index<cp->contacts.size());

	if(1)
	{
		cp->contacts.setPoint(contact_index, ToPxVec3(data.mPos));
		cp->contacts.setNormal(contact_index, ToPxVec3(data.mNormal));
		cp->contacts.setSeparation(contact_index, data.mSeparation);
	}
	else
	{
		class MyPxContactSet : public PxContactSet
		{
			public:
			PX_FORCE_INLINE		void setNormal2(PxU32 i, const PxVec3& n)		
			{ 
				PxContactPatch* patch = getPatch();
				//patch->internalFlags |= PxContactPatch::eREGENERATE_PATCHES;
				mContacts[i].normal = n;
			}
		};

		cp->contacts.setPoint(contact_index, ToPxVec3(data.mPos));
		((MyPxContactSet&)cp->contacts).setNormal2(contact_index, ToPxVec3(data.mNormal));
		cp->contacts.setSeparation(contact_index, data.mSeparation);
	}
}

void PEEL_ContactModifyCallback::onContactModify(PxContactModifyPair* const pairs, PxU32 count)
{
	if(0)
	{
		printf("Hardcoded fix %d\n", count);
		for(PxU32 p=0;p<count;p++)
		{
			const PxU32 nbContacts = pairs[p].contacts.size();
			printf("--nbContacts %d\n", nbContacts);
			for(PxU32 i=0;i<nbContacts;i++)
			{
				pairs[p].contacts.setDynamicFriction(i, 0.0f);
				pairs[p].contacts.setNormal(i, PxVec3(0.0f, 1.0f, 0.0f));
//					pairs[p].contacts.setSeparation(i, 0.0f);
			}
		}
		return;
	}

//	printf("onContactModify: %d pairs\n", count);

	if(!mContactModifyCallback && !mContactModifyCallback2)
		return;

	if(mContactModifyCallback2)
	{
		mPairs = pairs;
		mNbPairs = count;
		mContactModifyCallback2->ModifyContacts(*mOwner, count, *this);
	}

	if(mContactModifyCallback)
	{
		struct DelayedContact
		{
			PX_FORCE_INLINE	DelayedContact(PxU32 i, PxU32 j, PxU32 k) : mPairIndex(i), mContactIndex(j), mUserIndex(k){}
			PxU32	mPairIndex;
			PxU32	mContactIndex;
			PxU32	mUserIndex;
		};
		_array<DelayedContact> delayed;

		for(PxU32 i=0;i<count;i++)
		{
			PxContactModifyPair& cp = pairs[i];

			const PxRigidActor* actor0 = cp.actor[0];
			const PxRigidActor* actor1 = cp.actor[1];
			const PxShape* shape0 = cp.shape[0];
			const PxShape* shape1 = cp.shape[1];
			const PxU32 nbContacts = cp.contacts.size();

			if(mContactModifyCallback->PrepContactModify(*mOwner, nbContacts, PintActorHandle(actor0), PintActorHandle(actor1), PintShapeHandle(shape0), PintShapeHandle(shape1)))
			{
				const PR IcePose0 = ToPR(cp.transform[0]);
				const PR IcePose1 = ToPR(cp.transform[1]);

				PxContactSet& set = cp.contacts;
				for(PxU32 j=0;j<nbContacts;j++)
				{
					Point p = ToPoint(set.getPoint(j));
					Point n = ToPoint(set.getNormal(j));
					PxReal s = set.getSeparation(j);
					const PxU32 feature0 = set.getInternalFaceIndex0(j);
					const PxU32 feature1 = set.getInternalFaceIndex1(j);

					const PintContactModifyCallback::ContactModif Action = mContactModifyCallback->ModifyContact(*mOwner, IcePose0, IcePose1, p, n, s, feature0, feature1);
					if(Action==PintContactModifyCallback::CONTACT_MODIFY)
					{
						set.setPoint(j, ToPxVec3(p));
						set.setNormal(j, ToPxVec3(n));
						set.setSeparation(j, s);
					}
					else if(Action==PintContactModifyCallback::CONTACT_IGNORE)
						set.ignore(j);
					else if(Action>=PintContactModifyCallback::CONTACT_DELAYED)
					{
						delayed.pushBack(DelayedContact(i, j, Action));
					}
				}
			}
		}

		const PxU32 nbDelayed = delayed.size();
		for(PxU32 i=0;i<nbDelayed;i++)
		{
			const DelayedContact& dc = delayed[i];

			PxContactModifyPair& cp = pairs[dc.mPairIndex];

			const PxRigidActor* actor0 = cp.actor[0];
			const PxRigidActor* actor1 = cp.actor[1];
			const PxShape* shape0 = cp.shape[0];
			const PxShape* shape1 = cp.shape[1];
			const PxU32 nbContacts = cp.contacts.size();

			if(mContactModifyCallback->PrepContactModify(*mOwner, nbContacts, PintActorHandle(actor0), PintActorHandle(actor1), PintShapeHandle(shape0), PintShapeHandle(shape1)))
			{
				const PR IcePose0 = ToPR(cp.transform[0]);
				const PR IcePose1 = ToPR(cp.transform[1]);

				PxContactSet& set = cp.contacts;
				//for(PxU32 j=0;j<nbContacts;j++)
				const PxU32 j = dc.mContactIndex;
				{
					Point p = ToPoint(set.getPoint(j));
					Point n = ToPoint(set.getNormal(j));
					PxReal s = set.getSeparation(j);
					const PxU32 feature0 = set.getInternalFaceIndex0(j);
					const PxU32 feature1 = set.getInternalFaceIndex1(j);

					const PintContactModifyCallback::ContactModif Action = mContactModifyCallback->ModifyDelayedContact(*mOwner, IcePose0, IcePose1, p, n, s, feature0, feature1, dc.mUserIndex);
					if(Action==PintContactModifyCallback::CONTACT_MODIFY)
					{
						set.setPoint(j, ToPxVec3(p));
						set.setNormal(j, ToPxVec3(n));
						set.setSeparation(j, s);
					}
					else if(Action==PintContactModifyCallback::CONTACT_IGNORE)
						set.ignore(j);
					//else if(Action==PintContactModifyCallback::CONTACT_DELAYED)
					//	delayed.AddPair(i, j);
				}
			}
		}
	}
}

