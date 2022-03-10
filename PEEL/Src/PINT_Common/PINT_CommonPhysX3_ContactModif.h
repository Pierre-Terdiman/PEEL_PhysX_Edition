///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_CONTACT_MODIF_H
#define PINT_COMMON_PHYSX3_CONTACT_MODIF_H

class PEEL_ContactModifyCallback : public PxContactModifyCallback, public PintContactModifyProvider
{
	public:
										PEEL_ContactModifyCallback();

			void						Init(Pint& owner, PintContactModifyCallback* cb, PintContactModifyCallback2* cb2);

	// PintContactModifyProvider
	virtual	PintContactModifyPairHandle	GetContactPairData(udword pair_index, ContactPairData& data, PR* pose0, PR* pose1);
	virtual	void						GetContactData(PintContactModifyPairHandle handle, ContactData* data, udword nb_contacts);
	virtual	void						IgnoreContact(PintContactModifyPairHandle handle, udword contact_index);
	virtual	void						SetContactData(PintContactModifyPairHandle handle, udword contact_index, const ContactData& data);
	//~PintContactModifyProvider

	// PxContactModifyCallback
	virtual void						onContactModify(PxContactModifyPair* const pairs, PxU32 count);
	//~PxContactModifyCallback

	private:
			Pint*						mOwner;
			PintContactModifyCallback*	mContactModifyCallback;
			PintContactModifyCallback2*	mContactModifyCallback2;
			const PxContactModifyPair*	mPairs;
			PxU32						mNbPairs;
};

#endif