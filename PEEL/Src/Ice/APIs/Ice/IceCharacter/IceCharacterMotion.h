///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for character motions.
 *	\file		IceCharacterMotion.h
 *	\author		Pierre Terdiman
 *	\date		May, 08, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECHARACTERMOTION_H
#define ICECHARACTERMOTION_H

	ICECHARACTER_API udword GetNbCharacterMotions();

	class ICECHARACTER_API MotionData : public Allocateable
	{
		public:
#ifndef SUPPORT_NEW_MOTIONDATA_DESIGN
		inline_					MotionData() : mCSID(CSID_INVALID), mNbFrames(0), mPRData(null)	{}
#else
		inline_					MotionData() : mCSID(CSID_INVALID), mNbFrames(0), mData(null), mFlags(0)	{}
#endif
		inline_					~MotionData()													{ Release();	}

#ifndef SUPPORT_NEW_MOTIONDATA_DESIGN
				bool			Init(udword nb_frames);
#else
				bool			Init(udword nb_frames, udword flags);
#endif
				void			Release();
				void			Inverse();

				CSID			mCSID;
				udword			mNbFrames;
#ifndef SUPPORT_NEW_MOTIONDATA_DESIGN
		inline_	const PR*		GetData()							const	{ return mPRData;	}
		inline_	void			SetData(const PR& pr, udword frame)			{ ASSERT(frame<mNbFrames);	mPRData[frame] = pr;	}
		private:
				PR*				mPRData;
#else
		inline_	void			GetData(PR& pr, udword frame)	const
				{
					if(mFlags&1)
					{
						const Quat* Data = (const Quat*)mData;
						pr.mRot = Data[frame];
						pr.mPos.Zero();
					}
					else
					{
						const PR* Data = (const PR*)mData;
						pr = Data[frame];
					}
				}
		inline_	void			SetData(const PR& pr, udword frame)
				{
					ASSERT(frame<mNbFrames);
					if(mFlags&1)
					{
						Quat* Data = (Quat*)mData;
						Data[frame] = pr.mRot;
						ASSERT(pr.mPos.IsZero());
					}
					else
					{
						PR* Data = (PR*)mData;
						Data[frame] = pr;
					}
				}
		private:
				udword			mFlags;
				void*			mData;
#endif
	};

	//! Creation structure
	struct ICECHARACTER_API CHARACTERMOTIONCREATE : public Allocateable
	{
								CHARACTERMOTIONCREATE();

		const	char*			mName;			//!< Possible motion's name (or null)

				udword			mNbBones;		//!< Number of bones involved in the motion. [real + virtual]
				udword			mNbVBones;		//!< Number of virtual bones
				//
		const	udword*			mDirectData;	//!< Sequential/flat motion data, as exported in ZCB files for example. [see below]
		const	MotionData*		mIndirectData;	//!< Already organized motion data, as exported in ZCB2 files for example.
				//
				udword			mHz;			//!< Replay frequency
				//
#ifdef SUPPORT_HEIGHT_TRACK
//				udword			mNbHeights;
//		const	float*			mHeights;
				udword			mNbOffsets;
		const	Point*			mOffsets;
#endif
				//
//				bool			mLoop;			//!< Cycle yes/no
				bool			mGlobal;		//!< true => global/absolute values, false => local/relative values
				bool			mD3DCompliant;	//!< true => matches D3D frame, false => MAX frame
	};

/*
	Here's what is expected in the "DirectData" field of a CHARACTERMOTIONCREATE structure:

	for each bone:
	- udword = CSID
	- udword = #frames
	- then for each frame
		- a PR structure
*/
	class ICECHARACTER_API CharacterMotion : public Allocateable
	{
		private:
								CharacterMotion();
								~CharacterMotion();
		// Initialize
				bool			Init(const CHARACTERMOTIONCREATE& create);
		public:

		// Motion loop
//		inline_	void			SetLoop(bool flag=true)					{ mLoop = flag;						}
//		inline_	bool			IsCycle()					const		{ return mLoop;						}

		// Data access
		inline_	bool			IsGlobal()					const		{ return mGlobal;					}
		inline_	udword			GetMotionDuration()			const		{ return mDuration;					}	// returns MAX(GetDuration(i))
		inline_	udword			GetNbBones()				const		{ return mNbBones;					}
		inline_	udword			GetNbVBones()				const		{ return mNbVirtualBones;			}

		// CSID accessors
#ifndef SUPPORT_NEW_MOTIONDATA_DESIGN
//		inline_	const PR*		GetPR(CSID id)				const		{ return mLinks[id]->mPRData;		}
		inline_	const PR*		GetPR(CSID id)				const		{ return mLinks[id]->GetData();		}
#else
		inline_	void			GetPR(PR& pr, CSID id, udword frame)	const	{ mLinks[id]->GetData(pr, frame);		}
#endif
		inline_	udword			GetDuration(CSID id)		const		{ return mLinks[id]->mNbFrames;		}

		// Naming
				void			SetName(const char* name)				{ mName = name;						}
		inline_	const String&	GetName()					const		{ return mName;						}

		// Replay frequency
		inline_	void			SetReplayFrequency(udword hz)			{ mHz = hz;							}
		inline_	udword			GetReplayFrequency()		const		{ return mHz;						}

#ifdef SUPPORT_HEIGHT_TRACK
//		inline_	udword			GetNbHeights()				const		{ return mNbHeights;				}
//		inline_	float			GetHeight(udword i)			const		{ return mHeights[i];				}

		inline_	udword			GetNbHeights()				const		{ return mNbOffsets;				}
		inline_	float			GetHeight(udword i)			const		{ return mOffsets[i].y;				}
		inline_	udword			GetNbOffsets()				const		{ return mNbOffsets;				}
		inline_	const Point&	GetOffset(udword i)			const		{ return mOffsets[i];				}
		inline_	const Point*	GetOffsets()				const		{ return mOffsets;					}
#endif

		// Helpers
				bool			BuildCompatibleSkeleton(SkeletonDescriptor& sd)	const;
				void			TweakTrack(const Point& offset_pos, const Matrix3x3& offset_rot, bool reverse_motion);

								PREVENT_COPY(CharacterMotion)
		private:
				String			mName;				//!< Motion name
				udword			mRefCount;			//!< Reference counter
				udword			mDuration;			//!< Motion duration = MAX(NbFrames(Bone(i)))
				udword			mNbBones;			//!< Number of bones the motion is related to [real+virtual]
				udword			mNbVirtualBones;	//!< Number of extra bones involved in this motion
				udword			mHz;				//!< Frequency
				MotionData**	mLinks;				//!< Link CSIDs to motion data
				MotionData*		mData;				//!< PR information (once/mNbBones) - accessed through mLinks
#ifdef SUPPORT_HEIGHT_TRACK
//				udword			mNbHeights;
//				float*			mHeights;
				udword			mNbOffsets;
				Point*			mOffsets;
#endif
//				bool			mLoop;				//!< Does this motion loop?
				bool			mGlobal;			//!< Are the values global or local ?
				bool			mD3DCompliant;		//!< D3D or MAX frame
		// Internal methods
				bool			InitLinkers(udword max_nb_nodes);

		friend	class			MotionFactory;
		friend	class			MotionCell;
	};

	class ICECHARACTER_API MotionFactory : public Allocateable
	{
		public:
									MotionFactory();
									~MotionFactory();
		// Data access
		inline_	udword				GetNbMotions()	const	{ return mMotions.GetNbEntries();					}
		inline_	CharacterMotion**	GetMotions()	const	{ return (CharacterMotion**)mMotions.GetEntries();	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Creates a new motion.
		 *	\param		create	[in] creation structure
		 *	\return		new motion, or null if failed
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				CharacterMotion*	CreateMotion(const CHARACTERMOTIONCREATE& create);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Finds a motion by name.
		 *	This is O(n).
		 *	\param		name	[in] motion name
		 *	\param		index	[out] motion index
		 *	\return		motion, or null if not found
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				CharacterMotion*	FindMotion(const char* name, udword* index=null)	const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Deletes a motion.
		 *	\param		name	[in] motion name
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				DeleteMotion(const char* name);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Purges unused motions.
		 *	\return		number of removed motions
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				udword				Purge();

		private:
				PtrContainer		mMotions;
		// Internal methods
				void				DeleteMotions();
	};

#endif // ICECHARACTERMOTION_H



