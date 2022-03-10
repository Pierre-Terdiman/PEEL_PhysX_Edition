///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains stabbing code.
 *	\file		IceStabbing.h
 *	\author		Pierre Terdiman
 *	\date		May, 29, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESTABBING_H
#define ICESTABBING_H

	enum StabbingCode
	{
		STAB_ALL			= (1<<0),			//!< Report all stabbed faces
		STAB_FIRST			= (1<<1),			//!< Report first stabbed face only
		STAB_CLOSEST		= (1<<2),			//!< Report closest stabbed face only
		STAB_GEOM_CULLING	= (1<<3),			//!< Perform geometric backface culling (CW or CCW ordering)
		STAB_RENDER_CULLING	= (1<<4),			//!< Perform render backface culling (single or double sided)

		STAB_FORCE_DWORD	= 0x7fffffff
	};

	struct MESHMERIZER_API STABCREATE
	{
								STABCREATE() : mMinDist(0.0f), mMaxDist(MAX_FLOAT), mStabCodes(STAB_ALL)	{}

				Ray				mWorldRay;		//!< Stabbing ray in world space
				float			mMinDist;		//!< Minimum stabbing distance
				float			mMaxDist;		//!< Maximum stabbing distance
				udword			mStabCodes;		//!< Combination of StabbingCodes
	};

	//! This structure describes a hit point
	class MESHMERIZER_API Hit : public Allocateable
	{
		public:
		inline_					Hit()					{ SetInvalid(); }
		inline_					~Hit()					{}

		inline_	void			SetInvalid()			{ mHitPoint.SetNotUsed();		}
		inline_	BOOL			IsInvalid()		const	{ return mHitPoint.IsNotUsed();	}

				Point			mHitPoint;		//!< Exact hit point
				udword			mNearbyVertex;	//!< Index of nearest vertex
				udword			mFaceID;		//!< Index of touched face
				float			mDistance;		//!< Distance from camera to hitpoint
	};

//	class Hitable;

	class MESHMERIZER_API StabbedFace : public Allocateable
	{
		public:
		inline_					StabbedFace() : mObjectID(INVALID_ID), mFaceID(INVALID_ID), mDistance(0.0f), mU(0.0f), mV(0.0f)	{}
		inline_					~StabbedFace()																					{}

		// pt = (1.0f - mU - mV)*p0 + mU*p1 + mV*p2;
		//
		// Pt0: mU = mV = 0
		// Pt1: mU = 1, mV = 0
		// Pt2: mU = 0, mV = 1

		inline_	void			SetBarycentricFirstPoint()		{ mU = 0.0f;	mV = 0.0f;	}
		inline_	void			SetBarycentricSecondPoint()		{ mU = 1.0f;	mV = 0.0f;	}
		inline_	void			SetBarycentricThirdPoint()		{ mU = 0.0f;	mV = 1.0f;	}

				udword			mObjectID;		//!< User-defined tag
//				Hitable*		mOwner;			//!< Owner object
				udword			mFaceID;		//!< Index of touched face
				float			mDistance;		//!< Distance from camera to hitpoint
				float			mU,mV;			//!< Impact barycentric coordinates
	};

	class MESHMERIZER_API StabbedFaces : public Container
	{
		public:
								StabbedFaces()				{}
								~StabbedFaces()				{}

		inline_	udword			GetNbStabbedFaces()	const	{ return GetNbEntries()/(sizeof(StabbedFace)/sizeof(udword));	}
		inline_	StabbedFace*	GetStabbedFaces()	const	{ return (StabbedFace*)GetEntries();							}

		inline_	void			AddStabbedFace(const StabbedFace& face)		{ Add((const udword*)&face, sizeof(StabbedFace)/sizeof(udword)); }

				bool			SortByDistance();
	};
#ifdef ICE_USE_STL_VECTOR
	#define LocalStabbedFaces(x, n)	StabbedFaces x;
#else
	#define LocalStabbedFaces(x, n)	StabbedFaces x;	x.InitSharedBuffers(n, (udword*)StackAlloc(sizeof(udword)*n), true);
#endif

	class MESHMERIZER_API Hitable
	{
		public:
		//! Constructor
								Hitable()	{}
		//! Destructor
		virtual					~Hitable()	{}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Stabbing method. Used to know if a ray stabs an object or not.
		 *	\param		stab_info	[in] stabbing data
		 *	\param		stabbed		[out] stabbed face(s)
		 *	\return		true if some faces have been stabbed
		 *	\see		ComputeImpactPoint
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	bool			Stab(const STABCREATE& stab_info, StabbedFaces& stabbed)						{ return false; }

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	After a stabbing query, the nearest face is sent back to determine the exact impact point.
		 *	\param		sf			[in] nearest stabbed face
		 *	\param		impact		[out] exact impact point in local space
		 *	\param		near_vtx	[out] index of the nearest vertex
		 *	\return		true if the returned impact point is valid
		 *	\see		Stab
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	bool			ComputeImpactPoint(const StabbedFace& sf, Point& impact, udword* near_vtx=null)	{ return false;	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	After nearest impact point has been computed, a hit structure is filled and returned to the stabbed object.
		 *	\param		hit		[in] structure filled with impact information
		 *	\return		true if the method has been overridden
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	bool			SetHit(const Hit& hit)															{ return false;	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Returns hit point in local space.
		 *	\param		hit		[out] structure filled with impact information
		 *	\return		true if the method has been overridden
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	bool			GetLocalHit(Hit& hit)															{ return false;	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Returns hit point in world space.
		 *	\param		hit		[out] structure filled with impact information
		 *	\return		true if the method has been overridden
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	bool			GetWorldHit(Hit& hit)															{ return false;	}
	};

#endif // ICESTABBING_H
