///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for contacts.
 *	\file		CTC_Contact.h
 *	\author		Pierre Terdiman
 *	\date		June, 26, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCCONTACT_H
#define CTCCONTACT_H

	class CONTACT_API Contact
	{
		public:
		inline_				Contact()	{}
		inline_				~Contact()	{}

		inline_	void		Zero()
							{
								mPos.Zero();
								mNormal.Zero();
								mDepth = 0.0f;
							}

				Point		mPos;		//!< Contact position
				Point		mNormal;	//!< Normal vector
				float		mDepth;		//!< Penetration depth
	};

	class CONTACT_API Contact2 : public Contact
	{
		public:
		inline_				Contact2()	{}
		inline_				~Contact2()	{}

				uword		mID0;		//!< ID of first object
				uword		mID1;		//!< ID of second object
	};

	// Convention:
	// if body 0 is moved along the normal vector by a distance depth
	// (or equivalently if body 1 is moved the same distance in the
	// opposite direction) then the contact depth will be reduced to
	// zero. This means that the normal vector points "in" to body 0.

#endif // CTCCONTACT_H
