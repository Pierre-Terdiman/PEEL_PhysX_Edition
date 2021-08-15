///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for motion cells.
 *	\file		IceMotionCell.h
 *	\author		Pierre Terdiman
 *	\date		May, 08, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Notes:
//
//	A CharacterMotion may have multiple definitions:
//	- sampled PRS for all frames
//	- sampled keyframes and interpolated ones
//	- procedural animation
//	- anything!
//
//	A MotionCell holds a single CharacterMotion, whatever that CharacterMotion can be.
//	MotionCells are children of the MotionGraph.
//	MotionCells' children are MotionTransitions. One MotionCell may have multiple MotionTransitions, but there can only be a single active transition at a particular time.
//	The active transition is of course one of the cell's children-transitions.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMOTIONCELL_H
#define ICEMOTIONCELL_H

	ICECHARACTER_API udword GetNbMotionCells();

	enum ImpactType
	{
		IMPACT_NONE				= 0,
		IMPACT_RIGHT_HAND		= (1<<0),
		IMPACT_LEFT_HAND		= (1<<1),
		IMPACT_HANDS			= IMPACT_RIGHT_HAND|IMPACT_LEFT_HAND,

		IMPACT_RIGHT_ELBOW		= (1<<2),
		IMPACT_LEFT_ELBOW		= (1<<3),
		IMPACT_ELBOWS			= IMPACT_RIGHT_ELBOW|IMPACT_LEFT_ELBOW,

		IMPACT_RIGHT_SHOULDER	= (1<<4),
		IMPACT_LEFT_SHOULDER	= (1<<5),
		IMPACT_SHOULDERS		= IMPACT_RIGHT_SHOULDER|IMPACT_LEFT_SHOULDER,

		IMPACT_RIGHT_ARM		= IMPACT_RIGHT_SHOULDER|IMPACT_RIGHT_ELBOW|IMPACT_RIGHT_HAND,
		IMPACT_LEFT_ARM			= IMPACT_LEFT_SHOULDER|IMPACT_LEFT_ELBOW|IMPACT_LEFT_HAND,

		IMPACT_RIGHT_FOOT		= (1<<6),
		IMPACT_LEFT_FOOT		= (1<<7),
		IMPACT_FEET				= IMPACT_RIGHT_FOOT|IMPACT_LEFT_FOOT,

		IMPACT_RIGHT_THIGH		= (1<<8),
		IMPACT_LEFT_THIGH		= (1<<9),
		IMPACT_THIGHS			= IMPACT_RIGHT_THIGH|IMPACT_LEFT_THIGH,

		IMPACT_RIGHT_CALF		= (1<<10),
		IMPACT_LEFT_CALF		= (1<<11),
		IMPACT_CALFS			= IMPACT_RIGHT_CALF|IMPACT_LEFT_CALF,

		IMPACT_RIGHT_LEG		= IMPACT_RIGHT_CALF|IMPACT_RIGHT_THIGH|IMPACT_RIGHT_FOOT,
		IMPACT_LEFT_LEG			= IMPACT_LEFT_CALF|IMPACT_LEFT_THIGH|IMPACT_LEFT_FOOT,

		IMPACT_ALL				= (1<<12),

		IMPACT_HIGH				= (1<<13),
		IMPACT_MEDIUM			= (1<<14),
		IMPACT_LOW				= (1<<15),

		IMPACT_GLASS			= (1<<16),	// Lame yet convenient design. Shouldn't be there.
		IMPACT_LOCK				= (1<<17),	// Lame yet convenient design. Shouldn't be there.
		IMPACT_FALLEN			= (1<<18),	// Lame yet convenient design. Shouldn't be there.
		IMPACT_KNOCKOUT			= (1<<19),	// Lame yet convenient design. Shouldn't be there.
		IMPACT_HIT_SWEEP		= (1<<20),	// Lame yet convenient design. Shouldn't be there.
		IMPACT_HIT_SMASH		= (1<<21),	// Lame yet convenient design. Shouldn't be there.
		IMPACT_HIT_BLOWNUP		= (1<<22),	// Lame yet convenient design. Shouldn't be there.
		IMPACT_HIT_ROTATE		= (1<<23),	// Lame yet convenient design. Shouldn't be there.
		IMPACT_HIT_JEWELS		= (1<<24),	// Lame yet convenient design. Shouldn't be there.
		IMPACT_HIT_FOOT_OUCH	= (1<<25),	// Lame yet convenient design. Shouldn't be there.

		IMPACT_BLOCK_RIGHT		= (1<<26),	// Lame yet convenient design. Shouldn't be there.
		IMPACT_BLOCK_LEFT		= (1<<27),	// Lame yet convenient design. Shouldn't be there.

		IMPACT_VERTICAL_IMPULSE	= (1<<28),	// Lame yet convenient design. Shouldn't be there.

		IMPACT_FORCE_DWORD		= 0x7fffffff
	};

	enum MotionType
	{
		MOTION_BLOCK			= (1<<0),
		MOTION_CROUCH			= (1<<1),
		MOTION_DOWN				= (1<<2),
		MOTION_ESCAPE			= (1<<3),
		MOTION_FALLEN			= (1<<4),
		MOTION_FLY				= (1<<5),
		MOTION_GETUP			= (1<<6),
		MOTION_HIT				= (1<<7),
		MOTION_IDLE				= (1<<8),
		MOTION_JUMP				= (1<<9),
		MOTION_KICK				= (1<<10),
		MOTION_HIT_LOCK			= (1<<11),
		MOTION_PUNCH			= (1<<12),
		MOTION_RUN				= (1<<13),
		MOTION_SNEAK			= (1<<14),
		MOTION_THROW			= (1<<15),
		MOTION_THROW_TARGET		= (1<<16),
		MOTION_WALK				= (1<<17),
		MOTION_STUN				= (1<<18),
		MOTION_INVINCIBLE		= (1<<19),
		MOTION_LEDGE			= (1<<20),
	};

	//! Creation structure
	struct ICECHARACTER_API MOTIONCELLCREATE : public Allocateable
	{
							MOTIONCELLCREATE();

		const char*			mName;
		CharacterMotion*	mUnderlyingMotion;
#ifdef SUPPORT_MOTION_INTERVAL
		TimeSegment			mMotionInterval;
#endif
		Point				mMotionSpeedVector;
		float				mReplaySpeed;
		float				mImpactForce;
		ImpactType			mImpactType;
		udword				mImpactStart;
		udword				mImpactEnd;
		udword				mFreeRotStart;
		udword				mFreeRotEnd;
		udword				mFlipFrame;
#ifndef SUPPORT_WEAPON_MODIFIERS2
		BOOL				mUseWeapon;
#endif
		udword				mThrowIndex;
//		String				mDataType;
		udword				mMotionType;
		udword				mModifierIndex;
		udword				mEffectIndex;
		float				mVerticalOffset;
		float				mCollisionFactor;
#ifdef SUPPORT_MOTION_MODIFIERS
		udword				mNbModifiers;
#endif
#ifdef SUPPORT_WEAPON_MODIFIERS
		udword				mNbWModifiers;
#endif
		ubyte				mMotionSpeedMode;
		bool				mLoop;
	};

	class MotionState;

	class ICECHARACTER_API MotionCell : public Allocateable
	{
		private:
									MotionCell();
									~MotionCell();
		// Initialize
				bool				Init(const MOTIONCELLCREATE& create);
		public:
		// Transition creation
		inline_	MotionTransition*	CreateTransition(const MOTIONTRANSITIONCREATE& create)	{ return mTransitions.CreateTransition(create);	}
		inline_	bool				DeleteTransition(MotionTransition* trans)				{ return mTransitions.DeleteTransition(trans);	}
#ifdef SUPPORT_CELL_ACTIVE_TRANS
		// Activate
		inline_	void				Activate()										{ mActiveTransition	= null;								}

		// Settings
		inline_	void				SetActiveTransition(MotionTransition* trans)	{ mActiveTransition	= trans;							}
#endif

//		inline_	void				SetLoop(bool flag=true)							{ if(mRelatedMotion)	mRelatedMotion->SetLoop(flag);	}

		// Motion loop
		inline_	void				SetLoop(bool flag=true)							{ mLoop = flag;											}
		inline_	bool				IsCycle()								const	{ return mLoop;											}

		inline_	void				SetImpactForce(float value)						{ mImpactForce = value;									}
		inline_	void				SetImpactType(ImpactType impact_type)			{ mImpactType = impact_type;							}
		inline_	void				SetImpactStart(udword value)					{ mImpactStart = value;									}
		inline_	void				SetImpactEnd(udword value)						{ mImpactEnd = value;									}
		inline_	void				SetFreeRotStart(udword value)					{ mFreeRotStart = value;								}
		inline_	void				SetFreeRotEnd(udword value)						{ mFreeRotEnd = value;									}
		inline_	void				SetFlipFrame(udword frame)						{ mFlipFrame = frame;									}
#ifndef SUPPORT_WEAPON_MODIFIERS2
		inline_	void				SetWeapon(BOOL flag=TRUE)						{ mUseWeapon = flag;									}
#endif
		inline_	void				SetThrowIndex(udword index)						{ mThrowIndex = index;									}
		inline_	void				SetCollisionFactor(float f)						{ mCollisionFactor = f;									}

		// Data access
#ifdef SUPPORT_WEAPON_MODIFIERS2
		inline_	CharacterMotion*	GetRelatedMotion(udword i)				const
									{
										if(mRelatedMotion[i])	return mRelatedMotion[i];
										return mRelatedMotion[0];
									}
#else
		inline_	CharacterMotion*	GetRelatedMotion()						const	{ return mRelatedMotion;								}
#endif
				bool				SetCharacterMotion(CharacterMotion* motion);

#ifdef SUPPORT_CELL_ACTIVE_TRANS
		inline_	MotionTransition*	GetActiveTransition()					const	{ return mActiveTransition;								}
#endif
//		inline_	udword				GetNbTransitions()						const	{ return mTransitions.GetNbEntries();					}
//		inline_	MotionTransition*	GetTransition(udword id)				const	{ return (MotionTransition*)mTransitions.GetEntry(id);	}
		inline_	const Transitions&	GetTransitions()						const	{ return mTransitions;									}
		inline_	Transitions&		GetTransitions()								{ return mTransitions;									}

#ifdef SUPPORT_NEW_DESIGN
				MotionCell&			DispatchEventToTransitions(MotionState& state, const BasicMotionEvent& event, float event_time, float current_time);
				MotionTransition*	GetValidTransition(MotionState& state)	const;
#else
				MotionCell&			DispatchEventToTransitions(const BasicMotionEvent& event, float event_time, float current_time);
				MotionTransition*	GetValidTransition()					const;
#endif
#ifdef SUPPORT_WEAPON_MODIFIERS2
		inline_	udword				GetLastFrame(udword i)					const	{ return GetRelatedMotion(i)->GetMotionDuration()-1;	}
#else
		inline_	udword				GetLastFrame()							const	{ return mRelatedMotion->GetMotionDuration()-1;			}
#endif
#ifdef SUPPORT_MOTION_INTERVAL
		inline_	const TimeSegment*	GetMotionInterval()						const	{ return &mMotionInterval;								}
#endif

		// Local time management
//		inline_	MotionCell&			ResetLocalTime(udword baseframe=0, float deltatm=0.0f)	{ mLocalTime = deltatm * float(baseframe); return *this; }
//				MotionCell&			UpdateLocalTime(float relativetime, float deltatm);

		// Data access
//		inline_	float				GetLocalTime()							const	{ return mLocalTime;							}
//		inline_	float				GetInterpolCoeff()						const	{ return mt;									}
//		inline_	udword				GetLocalFrame()							const	{ return mLocalFrame;							}

		// Local time management
#ifndef SUPPORT_NEW_DESIGN2
	#ifdef SUPPORT_WEAPON_MODIFIERS2
		inline_	void				ResetLocalTime(udword base_frame, udword i)
	#else
		inline_	void				ResetLocalTime(udword base_frame=0)
	#endif
									{
										mTime.ResetGlobalTime(base_frame);
	#ifdef SUPPORT_WEAPON_MODIFIERS2
	if(GetRelatedMotion(i)->GetOffsets())
	{
		udword RealFrame = ClampFrameIndex(base_frame, GetLastFrame(i), IsCycle());
		if(RealFrame==INVALID_ID)	RealFrame=0;
		mOffset = GetRelatedMotion(i)->GetOffset(RealFrame);
	}
	#else
	udword RealFrame = ClampFrameIndex(base_frame, GetLastFrame(), IsCycle());
	if(RealFrame==INVALID_ID)	RealFrame=0;
	mOffset = mRelatedMotion->GetOffset(RealFrame);
	#endif
									}
		inline_	void				UpdateLocalTime(float relative_time)			{ mTime.UpdateGlobalTime(relative_time);				}

		inline_	float				GetLocalTime()							const	{ return mTime.GetGlobalTime();							}
		inline_	float				GetInterpolCoeff()						const	{ return mTime.GetInterpolant();						}
		inline_	udword				GetLocalFrame()							const	{ return mTime.GetGlobalFrameCounter();					}
		inline_	void				SetLocalFrame(udword frame)						{ mTime.SetGlobalFrameCounter(frame);					}

		inline_	const Point&		GetOffset()								const	{ return mOffset;										}
		inline_	void				SetOffset(const Point& offset)					{ mOffset = offset;										}
#endif
		// Motion speed
		inline_	void				SetMotionSpeedVector(const Point& speed)		{ mMotionSpeedVector = speed;							}
		inline_	const Point&		GetMotionSpeedVector()					const	{ return mMotionSpeedVector;							}
		inline_	void				SetMotionSpeedMode(ubyte mode)					{ mMotionSpeedMode = mode;								}
		inline_	ubyte				GetMotionSpeedMode()					const	{ return mMotionSpeedMode;								}

		// Replay speed
		inline_	void				SetReplaySpeed(float speed)						{ mReplaySpeed = speed;									}
		inline_	float				GetReplaySpeed()						const	{ return mReplaySpeed;									}

		// Name
		inline_	void				SetName(const char* name)						{ mName = name;											}
		inline_	const String&		GetName()								const	{ return mName;											}

		// Data type
//		inline_	void				SetDataType(const String& type)					{ mDataType = type;										}
//		inline_	const String&		GetDataType()							const	{ return mDataType;										}
		inline_	void				SetMotionType(udword flags)						{ mMotionType = flags;									}
		inline_	udword				GetMotionType()							const	{ return mMotionType;									}

		// Modifier index
		inline_	void				SetModifierIndex(udword modifier_index)			{ mModifierIndex = modifier_index;						}
		inline_	udword				GetModifierIndex()						const	{ return mModifierIndex;								}

		// Effect index
		inline_	void				SetEffectIndex(udword effect_index)				{ mEffectIndex = effect_index;							}
		inline_	udword				GetEffectIndex()						const	{ return mEffectIndex;									}

		// Vertical offset
		inline_	void				SetVerticalOffset(float offset)					{ mVerticalOffset = offset;								}
		inline_	float				GetVerticalOffset()						const	{ return mVerticalOffset;								}

#ifdef SUPPORT_WEAPON_MODIFIERS2
		inline_	udword				LastFrameCheck(udword value, udword index)	const
									{
										if(value==LAST_FRAME)	return 1+GetLastFrame(index);
										return value;
									}
#else
		inline_	udword				LastFrameCheck(udword value)			const
									{
										if(value==LAST_FRAME)	return 1+GetLastFrame();
										return value;
									}
#endif

		inline_	float				GetImpactForce()						const	{ return mImpactForce;									}
		inline_	ImpactType			GetImpactType()							const	{ return mImpactType;									}
#ifdef SUPPORT_WEAPON_MODIFIERS2
		inline_	udword				GetImpactStart(udword index)			const	{ return LastFrameCheck(mImpactStart, index);			}
		inline_	udword				GetImpactEnd(udword index)				const	{ return LastFrameCheck(mImpactEnd, index);				}
		inline_	udword				GetFreeRotStart(udword index)			const	{ return LastFrameCheck(mFreeRotStart, index);			}
		inline_	udword				GetFreeRotEnd(udword index)				const	{ return LastFrameCheck(mFreeRotEnd, index);			}
#else
		inline_	udword				GetImpactStart()						const	{ return LastFrameCheck(mImpactStart);					}
		inline_	udword				GetImpactEnd()							const	{ return LastFrameCheck(mImpactEnd);					}
		inline_	udword				GetFreeRotStart()						const	{ return LastFrameCheck(mFreeRotStart);					}
		inline_	udword				GetFreeRotEnd()							const	{ return LastFrameCheck(mFreeRotEnd);					}
#endif
		inline_	udword				GetImpactStartUnchecked()				const	{ return mImpactStart;									}
		inline_	udword				GetImpactEndUnchecked()					const	{ return mImpactEnd;									}
		inline_	udword				GetFreeRotStartUnchecked()				const	{ return mFreeRotStart;									}
		inline_	udword				GetFreeRotEndUnchecked()				const	{ return mFreeRotEnd;									}

		inline_	udword				GetFlipFrame()							const	{ return mFlipFrame;									}
#ifndef SUPPORT_WEAPON_MODIFIERS2
		inline_	BOOL				UseWeapon()								const	{ return mUseWeapon;									}
#endif
		inline_	udword				GetThrowIndex()							const	{ return mThrowIndex;									}
		inline_	float				GetCollisionFactor()					const	{ return mCollisionFactor;								}

#ifdef SUPPORT_MOTION_MODIFIERS
		inline_	bool				SetNbModifiers(udword nb)						{ return mModifiers.SetSize(nb);						}
		inline_	udword				GetNbModifiers()						const	{ return mModifiers.GetNbEntries();						}
		inline_	MotionCell*			GetModifier(udword i)					const	{ return (MotionCell*)mModifiers.GetEntry(i);			}
		inline_	bool				SetModifier(udword i, const MotionCell* cell)
									{
										if(i>=GetNbModifiers())	return false;
										mModifiers[i] = (void*)(cell);
										return true;
									}
#endif
#ifdef SUPPORT_WEAPON_MODIFIERS
		inline_	bool				SetNbWeaponModifiers(udword nb)					{ return mWModifiers.SetSize(nb);						}
		inline_	udword				GetNbWeaponModifiers()					const	{ return mWModifiers.GetNbEntries();					}
		inline_	MotionCell*			GetWeaponModifier(udword i)				const	{ return (MotionCell*)mWModifiers.GetEntry(i);			}
		inline_	bool				SetWeaponModifier(udword i, const MotionCell* cell)
									{
										if(i>=GetNbWeaponModifiers())	return false;
										mWModifiers[i] = udword(cell);
										return true;
									}
#endif
#ifdef SUPPORT_MOTION_LINKS
		inline_	const MotionCell*	GetLink(udword i)						const	{ return mLinks[i];										}
		inline_	bool				SetLink(udword i, const MotionCell* cell)
									{
										if(i>=SUPPORT_MOTION_LINKS)	return false;
										mLinks[i] = cell;
										return true;
									}
#endif
#ifdef SUPPORT_WEAPON_MODIFIERS2
		inline_	void				SetWeaponMotion(udword i, CharacterMotion* motion)	{ mRelatedMotion[i] = motion;						}
		inline_	CharacterMotion*	GetWeaponMotion(udword i)							{ return mRelatedMotion[i];							}
#endif
				void				(*mCallBack)(class MotionCell* cell, void* user_data);

				udword				GetSourceBaseFrame(const MotionTransition& trans, udword i)		const;
				TimeSegment			GetValidityInterval(const MotionTransition& trans, udword i)	const;

				bool				Save(CustomArray& ca, StateMachineSaveContext& context)	const;
				bool				Load(const CustomArray& ca, StateMachineLoadContext& context);
				bool				PostLoad(StateMachineLoadContext& context);

									PREVENT_COPY(MotionCell)
		private:
				String				mName;
		// Link to underlying motion
#ifdef SUPPORT_WEAPON_MODIFIERS2
				CharacterMotion*	mRelatedMotion[SUPPORT_WEAPON_MODIFIERS2];
#else
				CharacterMotion*	mRelatedMotion;
#endif
		// Link to active transition (null if no one is played)
#ifdef SUPPORT_CELL_ACTIVE_TRANS
/* State */		MotionTransition*	mActiveTransition;
#endif
		// Motion interval
#ifdef SUPPORT_MOTION_INTERVAL
				TimeSegment			mMotionInterval;
#endif
				Point				mMotionSpeedVector;
				float				mReplaySpeed;
				float				mImpactForce;
				ImpactType			mImpactType;
				udword				mImpactStart;
				udword				mImpactEnd;
				udword				mFreeRotStart;
				udword				mFreeRotEnd;
				udword				mFlipFrame;
#ifndef SUPPORT_WEAPON_MODIFIERS2
				BOOL				mUseWeapon;
#endif
				udword				mThrowIndex;
				udword				mModifierIndex;
				udword				mEffectIndex;
				float				mVerticalOffset;
				float				mCollisionFactor;
				udword				mMotionType;
		// Motion transitions
//				Container			mTransitions;
				Transitions			mTransitions;
#ifndef SUPPORT_NEW_DESIGN2
		// Local time
/* State */		TimeInfo			mTime;
/* State */		Point				mOffset;
#endif
//				float				mLocalTime;			//!< Motion's local time
//				float				mt;					//!< Interpolation coeff between 0.0f and 1.0f
//				udword				mLocalFrame;		//!< Current local frame
#ifdef SUPPORT_MOTION_MODIFIERS
				PtrContainer		mModifiers;
#endif
#ifdef SUPPORT_WEAPON_MODIFIERS
				Container			mWModifiers;
#endif
#ifdef SUPPORT_MOTION_LINKS
				const MotionCell*	mLinks[SUPPORT_MOTION_LINKS];
#endif
				ubyte				mMotionSpeedMode;
				bool				mLoop;				//!< Does this motion loop?
		// Strings
//				String				mDataType;			//!< User-defined generic identifier

		friend	class				StateMachine;
	};

#ifdef SUPPORT_NEW_DESIGN2
	class ICECHARACTER_API MotionCellState : public Allocateable
	{
		public:
									MotionCellState();
									~MotionCellState();

#ifdef SUPPORT_WEAPON_MODIFIERS2
		inline_	void				ResetLocalTime(udword base_frame, udword i)
#else
		inline_	void				ResetLocalTime(udword base_frame=0)
#endif
									{
										mTime.ResetGlobalTime(base_frame);
#ifdef SUPPORT_WEAPON_MODIFIERS2
	if(mMotion->GetRelatedMotion(i)->GetOffsets())
	{
		udword RealFrame = ClampFrameIndex(base_frame, mMotion->GetLastFrame(i), mMotion->IsCycle());
		if(RealFrame==INVALID_ID)	RealFrame=0;
		mOffset = mMotion->GetRelatedMotion(i)->GetOffset(RealFrame);
	}
#else
	udword RealFrame = ClampFrameIndex(base_frame, mMotion->GetLastFrame(), mMotion->IsCycle());
	if(RealFrame==INVALID_ID)	RealFrame=0;
	mOffset = mMotion->mRelatedMotion->GetOffset(RealFrame);
#endif
									}
		inline_	void				UpdateLocalTime(float relative_time)			{ mTime.UpdateGlobalTime(relative_time);				}

		inline_	float				GetLocalTime()							const	{ return mTime.GetGlobalTime();							}
		inline_	float				GetInterpolCoeff()						const	{ return mTime.GetInterpolant();						}
		inline_	udword				GetLocalFrame()							const	{ return mTime.GetGlobalFrameCounter();					}
		inline_	void				SetLocalFrame(udword frame)						{ mTime.SetGlobalFrameCounter(frame);					}

		inline_	const Point&		GetOffset()								const	{ return mOffset;										}
		inline_	void				SetOffset(const Point& offset)					{ mOffset = offset;										}

				const MotionCell*	mMotion;
				TimeInfo			mTime;
				Point				mOffset;
	};
#endif

#endif // ICEMOTIONCELL_H