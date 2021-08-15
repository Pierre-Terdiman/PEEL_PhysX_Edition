///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for the skeleton.
 *	\file		IceSkeleton.h
 *	\author		Pierre Terdiman
 *	\date		May, 08, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESKELETON_H
#define ICESKELETON_H

	//! Skeleton flags
	enum SkelFlag
	{
		SKELETON_IS_BLENDING		= (1<<0),	//!< True if two motions are currently blended, else false
		SKELETON_TRANS_PENDING		= (1<<1),	//!< True => there's a motion transition pending
		SKELETON_IS_MOVING			= (1<<2),	//!< True => root PRS must be/is updated
		SKELETON_INVALID_PR			= (1<<3),	//!< True => absolute PR are invalid
		SKELETON_INVALID_MATRICES	= (1<<4),	//!< True => absolute matrices are invalid
		SKELETON_CONSTANT_TRANSLAT	= (1<<5),	//!< True => constant translations mode [Oni]
		SKELETON_INTERNAL_USE_ONLY	= (1<<6),	//!< Reserved for flip-rotation test
		SKELETON_INACTIVE			= (1<<7),	//!< True => skeleton is inactive and update is skipped

		SKELETON_INVALID_GLOBAL		= SKELETON_INVALID_PR|SKELETON_INVALID_MATRICES,
		SKELETON_READONLY			= SKELETON_CONSTANT_TRANSLAT|SKELETON_IS_BLENDING|SKELETON_TRANS_PENDING|SKELETON_IS_MOVING|SKELETON_INVALID_GLOBAL,

		SKELETON_FORCE_DWORD		= 0x7fffffff
	};

	// TEST!
	class Skeleton;
	typedef void	(*UpdateCallback)		(Skeleton&, void*);

	//! Creation structure
	struct ICECHARACTER_API SKELETONCREATE : public Allocateable
	{
		inline_	SKELETONCREATE() : mStateMachine(null), mSkelInfo(null), mScale(1.0f), mUserData(null)	{}

				SkeletonDescriptor*	mSkelInfo;
				StateMachine*		mStateMachine;
				float				mScale;
				void*				mUserData;
	};

	class ICECHARACTER_API Skeleton// : public Allocateable
	{
		public:
									Skeleton();
		virtual						~Skeleton();

									DECLARE_FLAGS(SkelFlag, mSkelFlags, SKELETON_READONLY)

				bool				Save(CustomArray& ca)	const;
				bool				Load(const CustomArray& ca);

//#ifdef TEST_STUFF
		virtual	void				ThisIsATest()	{}
//#endif
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Initializes the skeleton.
		 *	\param		create		[in] the creation structure
		 *	\return		true if success.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				Init(const SKELETONCREATE& create);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Builds a descriptor for this skeleton.
		 *	\param		desc	[out] skeleton's descriptor
		 *	\return		true if success.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				BuildDescriptor(SkeletonDescriptor& desc);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Updates the skeleton.
		 *	\param		time	[in] current time information
		 *	\return		true if success.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				Update(const TimeInfo& time);

				bool				ComputeBoneLengths();

		// Time settings
				void				SetDelay(float value)						{ mTime.SetDeltaTM(value);				}
		inline_	const TimeInfo&		GetTimeInfo()						const	{ return mTime;							}
		inline_	void				SetTimeInfo(const TimeInfo& time_info)		{ mTime = time_info;					}

		// Data access
		inline_	udword				GetNbBones()						const	{ return mNbBones;						}
		inline_	const Bone*			GetBone(udword i)					const	{ return &mBones[i];					}
//		inline_	const Bone*			GetRootBone()						const	{ return mBoneRoot;						}
		inline_	const udword*		GetLinkers()						const	{ return mCSIDToLocal;					}
		inline_	const Bone*			GetBoneByCSID(CSID csid)			const
									{
										// Check the CSID. Since it's a udword, it also handles INVALID_ID.
										if(csid>=BIPED_MAX_NB_NODES)	return null;
										// Get & check local ID
										udword LocalID = mCSIDToLocal[csid];
										if(LocalID>=mNbBones)			return null;
										// Return correct bone
										return &mBones[LocalID];
									}
		// State machine
				void				SetStateMachine(StateMachine* sm)			{ mStateMachine = sm;					}
		inline_	StateMachine*		GetStateMachine()					const	{ return mStateMachine;					}
		inline_	MotionState&		GetMotionState()							{ return mState;						}

		// Global replay speed
				void				SetGlobalReplaySpeed(float speed)			{ mGlobalReplaySpeed = speed;			}
		inline_	float				GetGlobalReplaySpeed()				const	{ return mGlobalReplaySpeed;			}

		// Access to motions
		inline_	MotionCell*			GetStartMotion()					const	{ return mStartMotion;					}
		inline_	MotionCell*			GetEndMotion()						const	{ return mEndMotion;					}

		// Access to internal values
		inline_	float				GetBlendGlobalTime()				const	{ return mBlendGlobalTime;				}
		inline_	float				GetLinearBlend()					const	{ return mLinearBlend;					}
		inline_	float				GetBlendFactor()					const	{ return mBlendFactor;					}
		inline_	const Point&		GetSpeedVector()					const	{ return mBlendedSpeedVector;			}
		inline_	const Point*		GetTranslations()					const	{ return mTranslations;					}
		inline_	float				GetVerticalOffset()					const	{ return mVerticalOffset;				}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets access to one matrix.
		 *	\param		i	[in] bone index
		 *	\return		cached global matrix
		 *	\warning	when not cached, the matrix needs computing
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	const Matrix4x4*	GetGlobalMatrix(udword i)
									{
										// Lazy-evaluation
										if(mSkelFlags&SKELETON_INVALID_MATRICES)	ComputeBoneGlobalMatrices();
										return &mGlobalMat[i];
									}
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets access to all matrices.
		 *	\return		cached global matrices
		 *	\warning	when not cached, the matrices need computing
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	const Matrix4x4*	GetGlobalMatrices()
									{
										// Lazy-evaluation
										if(mSkelFlags&SKELETON_INVALID_MATRICES)	ComputeBoneGlobalMatrices();
										return mGlobalMat;
									}
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets access to one PR.
		 *	\param		i	[in] bone index
		 *	\return		cached global PR
		 *	\warning	when not cached, the PR needs computing
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	const PR*			GetGlobalPR(udword i)
									{
										// Lazy-evaluation
										if(mSkelFlags&SKELETON_INVALID_PR)	ComputeBoneGlobalPR();
										return &mGlobalPR[i];
									}
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets access to all PRs.
		 *	\return		cached global PRs
		 *	\warning	when not cached, the PRs need computing
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	const PR*			GetGlobalPR()
									{
										// Lazy-evaluation
										if(mSkelFlags&SKELETON_INVALID_PR)	ComputeBoneGlobalPR();
										return mGlobalPR;
									}
		// Flags
		inline_	BOOL				IsBlending()						const	{ return mSkelFlags&SKELETON_IS_BLENDING;		}
		inline_	BOOL				IsTransPending()					const	{ return mSkelFlags&SKELETON_TRANS_PENDING;		}
		inline_	BOOL				IsMoving()							const	{ return mSkelFlags&SKELETON_IS_MOVING;			}
		inline_	BOOL				HasConstantTranslations()			const	{ return mSkelFlags&SKELETON_CONSTANT_TRANSLAT;	}

		// User angles
		inline_	void				SetHeadPitch(float pitch)					{ mHeadPitch = pitch;							}
		inline_	float				GetHeadPitch()						const	{ return mHeadPitch;							}
		inline_	void				SetHeadYaw(float yaw)						{ mHeadYaw = yaw;								}
		inline_	float				GetHeadYaw()						const	{ return mHeadYaw;								}

		inline_	void				SetRightArmPitch(float pitch)				{ mRightArmPitch = pitch;						}
		inline_	float				GetRightArmPitch()					const	{ return mRightArmPitch;						}
		inline_	void				SetRightArmYaw(float yaw)					{ mRightArmYaw = yaw;							}
		inline_	float				GetRightArmYaw()					const	{ return mRightArmYaw;							}

		inline_	void				SetLeftArmPitch(float pitch)				{ mLeftArmPitch = pitch;						}
		inline_	float				GetLeftArmPitch()					const	{ return mLeftArmPitch;							}
		inline_	void				SetLeftArmYaw(float yaw)					{ mLeftArmYaw = yaw;							}
		inline_	float				GetLeftArmYaw()						const	{ return mLeftArmYaw;							}

#ifdef SUPPORT_HEIGHT_TRACK
		inline_	float				GetHeightOffset()					const	{ return mHeightOffset;							}
		inline_	void				SetHeightOffset(float offset)				{ mHeightOffset = offset;						}
		inline_ const Point&		GetBlendedOffset()					const	{ return mBlendedOffset;						}

//		inline_ const Point&		GetAccumOffset()					const	{ return mAccumOffset;							}
//		inline_ void				ResetAccumOffset()							{ mAccumOffset.Zero();							}
#endif
				void				ResetInternals(MotionCell* current_cell=null);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Scales a skeleton by a given factor.
		 *	\param		scale	[in] scale factor
		 *	\return		true if success.
		 *	\warning	only supported for Oni mode so far
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	bool				SetScale(float scale);
		inline_	float				GetScale()							const	{ return mScale;								}

		inline_	void				SetUserData(void* user_data)				{ mUserData = user_data;						}
		inline_	void*				GetUserData()						const	{ return mUserData;								}

#ifdef SUPPORT_WEAPON_MODIFIERS2
		inline_	udword				GetMotionIndex()					const	{ return mMotionIndex;							}
				void				SetMotionIndex(udword index, float blend_time);
		inline_	bool				IsMotionIndexBlending()				const	{ return mMotionIndexStamp>0.0f;				}
#endif
		static	UpdateCallback		mUpdateCB;

									PREVENT_COPY(Skeleton)
		private:
				void*				mUserData;							//!< User-defined data
		// Bones
				udword				mNbBones;							//!< Number of bones in the skeleton
				Bone*				mBones;								//!< Array of bones (interpolated blended motion in local space)
				Point*				mTranslations;						//!< Possible constant translations [Oni mode]
				// ### could be bytes...
				udword				mCSIDToLocal[BIPED_MAX_NB_NODES];	//!< Maps a CSID to an index in mBones
		// Cache system
				PR*					mGlobalPR;							//!< Current motion expressed as global PR for each bone (###could be removed)
				Matrix4x4*			mGlobalMat;							//!< Current motion expressed as global matrices for each bone
//				Bone*				mBoneRoot;							//!< bonne route! ... hum, the root bone, shortcut in mBones
		// Time management
				float				mTimeStamp;							//!< Internal timestamp
				TimeInfo			mTime;								//!< Local time
				float				mMappedCoeff;						//!< Remapped interpolation coeff [currently unused]
				float				mGlobalReplaySpeed;					//!< Gloabal replay speed
		// Help information
				MotionCell*			mStartMotion;						//!< Starting motion in the state machine
				MotionCell*			mEndMotion;							//!< Ending motion in the state machine
		// Blending information
				float				mBlendGlobalTime;
				float				mLinearBlend;						//!< Linear blending coeff between two motions (0.0 => 1.0)
				float				mBlendFactor;						//!< Remapped blending coeff between two motions (0.0 => 1.0)
				float				mBlendedReplaySpeed;
				Point				mBlendedSpeedVector;				//!< Current speed vector (blended from motions' speeds)
				float				mVerticalOffset;					//!< Current vertical offset (blended from motions' offsets)
		// State machine
				StateMachine*		mStateMachine;						//!< Shortcut to the state machine
				MotionState			mState;								//!< Motion state
		// User angles
				float				mHeadPitch;							//!<
				float				mHeadYaw;							//!<
				float				mRightArmPitch;						//!<
				float				mRightArmYaw;						//!<
				float				mLeftArmPitch;						//!<
				float				mLeftArmYaw;						//!<
		// Global scale
				float				mScale;								//!< Global scale used to adjust some values
#ifdef SUPPORT_HEIGHT_TRACK
//				float				mBlendedHeight;
				Point				mBlendedOffset;
				float				mHeightOffset;
public:
				// ### useless now?
				MotionCell*			mPreviousStartMotion;
				MotionCell*			mPreviousEndMotion;
				udword				mPreviousStartFrame;
				udword				mPreviousEndFrame;
//				Point				mAccumOffset;

				Point				mBlendedOffset0;
				Point				mBlendedOffset1;
//				Point				mAccumOffset0;
//				Point				mAccumOffset1;
private:
#endif

#ifdef SUPPORT_MOTION_MODIFIERS
public:
				bool				mModifiers[SUPPORT_MOTION_MODIFIERS];	// ### hardcoded! bad!
private:
#endif
#ifdef SUPPORT_WEAPON_MODIFIERS
public:
				bool				mWModifiers[SUPPORT_WEAPON_MODIFIERS];	// ### hardcoded! bad!
private:
#endif

#ifdef SUPPORT_WEAPON_MODIFIERS2
				udword				mMotionIndex;
				udword				mNextMotionIndex;
				float				mMotionIndexStamp;
				float				mMotionIndexInvDuration;
#endif

		// Internal methods
			// Remapping
				Skeleton&			RemapTimeCoeff();
				Skeleton&			RemapBlendCoeff(const MotionTransition* trans);
				void				UpdateBlendedParameters();
			// Skinning
				bool				ComputeInterpolatedRelativePR();
				void				_LocalToGlobalPR(const Bone* curbone, const PR* fatherpr);
				void				ComputeBoneGlobalMatrices();
				void				ComputeBoneGlobalPR();
			// Experimental
				Skeleton&			LockVertices();

		friend class StateMachine;
	};

#endif // ICESKELETON_H
