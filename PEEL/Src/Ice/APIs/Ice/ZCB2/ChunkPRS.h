///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	PRS chunk for ZB2 format.
 *	\file		ChunkPRS.h
 *	\author		Pierre Terdiman
 *	\date		August, 29, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CHUNKPRS_H
#define CHUNKPRS_H

	#define PRS_VERSION	2

	enum ZCB2_PRS_Flag
	{
		ZCB2_PRS_LOCAL			= (1<<0),
		ZCB2_PRS_GROUP			= (1<<1),
		ZCB2_PRS_INSTANCE		= (1<<2),
		ZCB2_PRS_HIDDEN			= (1<<3),
		ZCB2_PRS_D3D_COMPLIANT	= (1<<4),
	};

	class ZCB2_API PRSChunk : public ROBJChunk
	{
		DECLARE_CHUNK(PRSChunk,			mPRSCore)

		DECLARE_STD_MEMBER(PRS,			PRS)
		DECLARE_STD_MEMBER(WireColor,	udword)
		DECLARE_STD_MEMBER(ParentID,	udword)
//		DECLARE_STD_MEMBER(LinkID,		udword)
		DECLARE_STD_MEMBER(MasterID,	udword)
		DECLARE_STD_MEMBER(TargetID,	udword)

		DECLARE_STD_FLAG(LocalPRS,		ZCB2_PRS_LOCAL)
		DECLARE_STD_FLAG(Group,			ZCB2_PRS_GROUP)
		DECLARE_STD_FLAG(Instance,		ZCB2_PRS_INSTANCE)
		DECLARE_STD_FLAG(Hidden,		ZCB2_PRS_HIDDEN)
		DECLARE_STD_FLAG(D3DCompliant,	ZCB2_PRS_D3D_COMPLIANT)
	};

/*
				Pivot				mPivot;					//!< Pivot information [Flexporter 1.15]
				const ubyte*		mUserProps;				//!< User-defined properties
				JointDescriptor*	mIKData;				//!< IK data [Flexporter 1.15]
		// User-defined physics properties
				float				mDensity;				//!< Object's density [Flexporter 1.16]
				float				mMass;					//!< Object's mass [Flexporter 1.16]
				sdword				mSamplingDensity;		//!< Sampling rate [Flexporter 1.16]
				bool				mResetPivot;			//!< [Flexporter 1.16]
				bool				mIsCollidable;			//!< [Flexporter 1.16]
				bool				mLockPivot;				//!< Lock pivot point or not [Flexporter 1.16]
*/

#endif // CHUNKPRS_H
