///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	MOVE chunk for ZB2 format.
 *	\file		ChunkMOVE.h
 *	\author		Pierre Terdiman
 *	\date		September, 13, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CHUNKMOVE_H
#define CHUNKMOVE_H

	#define MOVE_VERSION	3

	enum ZCB2_MOVE_Flag
	{
		ZCB2_MOVE_HAS_NO_POS	= (1<<0),
		ZCB2_MOVE_HAS_NO_ROT	= (1<<1),
	};

	class ZCB2_API MOVEChunk : public BaseChunk
	{
		DECLARE_CHUNK(MOVEChunk, mMOVECore)

		DECLARE_STD_MEMBER(ReplayFrequency, udword)
//		DECLARE_STD_ARRAY(Heights, float)
		DECLARE_STD_ICE_ARRAY(Offsets, Point)

		// Data access
		inline_	udword			GetNbBones()	const	{ return mNbBones;		}
		inline_	MotionData*		GetData()		const	{ return mData;			}

		// Chunk definition

		// Iterative setup
				bool			SetNbBones(udword nb_bones);
				bool			SetBone(udword i, udword csid, udword nb_frames, const PR* track);
		private:
		// Chunk data
				udword			mNbBones;	//!< Number of bones
				MotionData*		mData;		//!< One data block / bone
	};

#endif // CHUNKMOVE_H
