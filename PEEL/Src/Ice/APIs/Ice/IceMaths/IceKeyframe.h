///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	This file contains code for some simple keyframes.
 *	\file		IceKeyframe.h
 *	\author		Pierre Terdiman
 *	\date		July, 21, 2007
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEKEYFRAME_H
#define ICEKEYFRAME_H

	//! Generic keyframe structure. Usually X = time and Y = some edited property.
	struct ICEMATHS_API Keyframe : public Allocateable
	{
		float	mX;
		float	mY;
	};

	//! Extended keyframe, with precomputed deltas
	struct ICEMATHS_API ExtKeyframe : public Keyframe
	{
		float	mDx;
		float	mDy;
	};

	class ICEMATHS_API KeyframedTrack : public Allocateable
	{
		public:
									KeyframedTrack();
									~KeyframedTrack();

				bool				Init(udword nb_keys, const Keyframe* keys=null);
				bool				SetKey(udword index, float x, float y);
		inline_	bool				SetKey(udword index, const Keyframe& key)	{ return SetKey(index, key.mX, key.mY);		}
				void				Finish();
				void				Reset();

		inline_	udword				GetNbKeys()				const	{ return mNbKeys;		}
		inline_	const ExtKeyframe&	GetKey(udword index)	const	{ return mKeys[index];	}

				float				ComputeValue(float x, udword* last_key=null)	const;
		private:
				udword				mNbKeys;	//!< Number of keyframes
				ExtKeyframe*		mKeys;		//!< Array of keyframes
	};

#endif	// ICEKEYFRAME_H
