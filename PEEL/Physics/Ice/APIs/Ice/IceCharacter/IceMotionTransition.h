///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for motion transitions.
 *	\file		IceMotionTransition.h
 *	\author		Pierre Terdiman
 *	\date		May, 08, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMOTIONTRANSITION_H
#define ICEMOTIONTRANSITION_H

	ICECHARACTER_API udword GetNbTransitions();

	// Forward declarations
	class MotionCell;
	class MotionTransition;

	class ICECHARACTER_API StateMachineLoadContext
	{
		public:
		virtual						~StateMachineLoadContext()			{}
		virtual	MotionCell*			GetMotionCell(uqword id)			= 0;
		virtual	MotionTransition*	GetSharedTransition(uqword id)		= 0;
		virtual	CharacterMotion*	GetMotionFile(const char* name)		= 0;
		virtual	udword				GetSoundHandle(const String& name)	= 0;
	};

	class ICECHARACTER_API StateMachineSaveContext
	{
		public:
		virtual						~StateMachineSaveContext()			{}
		virtual	bool				GetSoundName(String& name, udword id)	= 0;
	};

	#define	EVENT_NONE		0
	#define PREVIOUS_CELL	(MotionCell*)1
	#define	LAST_FRAME		MAX_UDWORD-1

	enum TimeBlendRelation
	{
		TBLEND_LINEAR		= 0,	//!< Time value is linearly mapped to the blending factor
		TBLEND_QUADRA		= 1,	//!< Blending factor = mBlendFactor = a*t^2 + (1-a)*t
		TBLEND_CUBIC		= 2,	//!< Blending factor = mBlendFactor = a*t^3 + b*t^2 + (1-a-b)*t

		TBLEND_FORCE_DWORD	= 0x7fffffff
	};

#ifdef OBSOLETE_ACTIVATION
	enum BlendActivate
	{
		BLEND_DIRECT		= 0,	//!< Motion blending begins/ends at current frame
		BLEND_DELAYED		= 1,	//!< Motion blending begins/ends N frames before starting motion ends/after ending motion begins

		BLEND_FORCE_DWORD	= 0x7fffffff
	};
#endif

#ifdef SUPPORT_TIME_BLEND_MAPPING
	struct ICECHARACTER_API TimeBlendMapping
	{
		inline_	void		Reset()			{ MappingType = TBLEND_LINEAR; a = b = 0.0f; }
		TimeBlendRelation	MappingType;
		float				a,b;			// Mapping parameters
	};
#endif

	// A time segment deals with frame numbers (integers) because it does not depends on mDeltaTM (i.e. arbitrary delay between frames)
	struct ICECHARACTER_API TimeSegment : public Allocateable
	{
		inline_				TimeSegment()													{}
		inline_				TimeSegment(udword s, udword e) : StartFrame(s), EndFrame(e)	{}

		inline_	void		Reset()			{ StartFrame = EndFrame = 0;	}
		udword				StartFrame;			//!< Time segment's first frame
		udword				EndFrame;			//!< Time segment's last frame
	};

	//! Creation structure
	class ICECHARACTER_API MOTIONTRANSITIONCREATE : public Allocateable
	{
		public:
							MOTIONTRANSITIONCREATE();

		const char*			mName;
		MotionEvent			mEvent;
		TimeSegment			mValidityInterval;
#ifdef SUPPORT_MOTION_INTERVAL
		TimeSegment			mMotionInterval;
#endif
		udword				mTargetBaseFrame;
		udword				mSourceBaseFrame;
//		MotionCell*			mSourceMotionCell;
		MotionCell*			mTargetMotionCell;
		float				mBlendDuration;
#ifdef SUPPORT_TIME_BLEND_MAPPING
		TimeBlendMapping	mTimeBlendData;
#endif
#ifdef OBSOLETE_ACTIVATION
		BlendActivate		mActivation;
#endif
		Point				mImpulse;
		//
		udword				mSoundHandle;
		float				mSoundVolume;
		float				mSoundRange;
		//
		void				(*mCallBack)(class MotionTransition* trans, void* user_data);
		ubyte				mPriority;
//		ubyte				mFlushEvents;
	};

	class ICECHARACTER_API MotionTransition : public Allocateable
	{
		private:
									MotionTransition();
									~MotionTransition();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Initializes the motion transition.
		 *	\param		create		[in] Creation structure
		 *	\return		true if success.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				Init(const MOTIONTRANSITIONCREATE& create);
		public:

		// Data access
		inline_	void				SetName(const char* name)			{ mName = name;									}
		inline_	const String&		GetName()					const	{ return mName;									}
#ifdef SUPPORT_TIME_BLEND_MAPPING
		inline_	TimeBlendMapping	GetTimeBlendMapping()		const	{ return mTimeBlendData;						}
#endif
//		inline_	MotionCell*			GetSourceCell()				const	{ return mSourceMotionCell;						}
#ifdef SUPPORT_TARGET_OVERRIDE
		inline_	MotionCell*			GetTargetCell()				const	{ return mTargetOverride ? mTargetOverride : mTargetMotionCell;	}
#else
		inline_	MotionCell*			GetTargetCell()				const	{ return mTargetMotionCell;	}
#endif
//		inline_	void				SetSourceCell(MotionCell* cell)		{ mSourceMotionCell = cell;						}
		inline_	void				SetTargetCell(MotionCell* cell)		{ mTargetMotionCell = cell;						}

		inline_	const MotionEvent&	GetEvent()					const	{ return mEvent;								}
		inline_	void				SetEvent(const MotionEvent& event)	{ mEvent = event;								}
#ifdef SUPPORT_MOTION_INTERVAL
		inline_	const TimeSegment&	GetMotionInterval()			const	{ return mMotionInterval;						}
#endif

//#ifndef SUPPORT_NEW_DESIGN
	#ifdef SUPPORT_WEAPON_MODIFIERS2
//				udword				SrcLastFrameCheck(udword value, udword index)	const;
				udword				DstLastFrameCheck(udword value, udword index)	const;
	#else
//				udword				SrcLastFrameCheck(udword value)			const;
				udword				DstLastFrameCheck(udword value)			const;
	#endif

//		inline_	udword				GetTargetBaseFrame(udword i)	const	{ return mTargetBaseFrame;	}
//		inline_	udword				GetSourceBaseFrame(udword i)	const	{ return mSourceBaseFrame;	}
//		inline_	TimeSegment			GetValidityInterval(udword i)	const	{ return mValidityInterval;	}
		inline_	udword				GetSourceBaseFrame()			const	{ return mSourceBaseFrame;	}
		inline_	TimeSegment			GetValidityInterval()			const	{ return mValidityInterval;	}
	#ifdef SUPPORT_WEAPON_MODIFIERS2
				udword				GetTargetBaseFrame(udword i)const;
//				udword				GetSourceBaseFrame(udword i)const;
//				TimeSegment			GetValidityInterval(udword i)const;
	#else
				udword				GetTargetBaseFrame()		const;
				udword				GetSourceBaseFrame()		const;
				TimeSegment			GetValidityInterval()		const;
	#endif
		inline_	void				SetTargetBaseFrame(udword frame)	{ mTargetBaseFrame = frame;						}
		inline_	void				SetSourceBaseFrame(udword frame)	{ mSourceBaseFrame = frame;						}
		inline_	void				SetValidityInterval(const TimeSegment& st)	{ mValidityInterval = st;				}
//#endif
		inline_	void				SetBlendDuration(float bd)			{ mBlendDuration = bd;							}
		inline_	float				GetBlendDuration()			const	{ return mBlendDuration;						}
		inline_	float				GetBlendInc()				const	{ return 1.0f / mBlendDuration;					}
#ifdef OBSOLETE_ACTIVATION
		inline_	BlendActivate		GetActivation()				const	{ return mBlendActivate;						}
#endif
		inline_	const Point&		GetImpulse()				const	{ return mImpulse;								}
		inline_	void				SetImpulse(const Point& impulse)	{ mImpulse = impulse;							}

		inline_	ubyte				GetPriority()				const	{ return mPriority;								}
		inline_	void				SetPriority(ubyte priority)			{ mPriority = priority;							}

		inline_	udword				GetSoundHandle()			const	{ return mSoundHandle;							}
		inline_	void				SetSoundHandle(udword handle)		{ mSoundHandle = handle;						}
		inline_	float				GetSoundVolume()			const	{ return mSoundVolume;							}
		inline_	void				SetSoundVolume(float volume)		{ mSoundVolume = volume;						}
		inline_	float				GetSoundRange()				const	{ return mSoundRange;							}
		inline_	void				SetSoundRange(float range)			{ mSoundRange = range;							}
//		inline_	ubyte				FlushEvents()				const	{ return mFlushEvents;							}

#ifdef SUPPORT_TARGET_OVERRIDE
		inline_	void				SetTargetOverride(MotionCell* cell)	{ mTargetOverride = cell;						}
#endif
		// Possible user callback
				void				(*mCallBack)(class MotionTransition* trans, void* user_data);
#ifndef SUPPORT_NEW_DESIGN
		// Management
		inline_	void				Reset()								{ mTrustLevel = 0;								}
#endif
#ifdef SUPPORT_COMPLEX_EVENTS
		inline_	bool				IsNullTransition()			const	{ return mEvent.BasicEvent[0].ID==EVENT_NONE;	}
#else
		inline_	bool				IsNullTransition()			const	{ return mEvent.ID==EVENT_NONE;					}
#endif

				bool				Save(CustomArray& ca, StateMachineSaveContext& context)	const;
				bool				Load(const CustomArray& ca, StateMachineLoadContext& context);
				bool				PostLoad(StateMachineLoadContext& context);

									PREVENT_COPY(MotionTransition)
		private:
				String				mName;
				MotionEvent			mEvent;					//!< Triggering event
				TimeSegment			mValidityInterval;		//!< Event is only recorded inside the validity interval
#ifdef SUPPORT_MOTION_INTERVAL
				TimeSegment			mMotionInterval;
#endif
		// Expected target motion cell
//				MotionCell*			mSourceMotionCell;		//!< Source cell
				MotionCell*			mTargetMotionCell;		//!< Destination cell
#ifdef SUPPORT_TARGET_OVERRIDE
/* State */		MotionCell*			mTargetOverride;		//!< Can possibly override normal target cell
#endif
				udword				mTargetBaseFrame;		//!< Blending ends at this target cell's frame
				udword				mSourceBaseFrame;		//!< Blending begins from this source cell's frame [not used in BLEND_DIRECT mode]
		// Transition parameters
//				MotionBlendType		mMBlendType;
#ifdef OBSOLETE_ACTIVATION
				BlendActivate		mBlendActivate;			//!< Direct or delayed blending
#endif
#ifdef SUPPORT_TIME_BLEND_MAPPING
				TimeBlendMapping	mTimeBlendData;			//!< Relation between time and blending coeff
#endif
				float				mBlendDuration;			//!< Transition time
				Point				mImpulse;				//!< Impulsion

				udword				mSoundHandle;			//!< Sound handle
				float				mSoundVolume;			//!< Volume multiplier
				float				mSoundRange;			//!< Sound range

#ifndef SUPPORT_NEW_DESIGN
/* State */		udword				mTrustLevel;			//!< From 0 to mEvent.NbBasicEvents. Transition is valid when reaching mEvent.NbBasicEvents.
#endif
				ubyte				mPriority;
//				ubyte				mFlushEvents;
				bool				mShared;				//!< SHAREDTRANS

		friend	class				Transitions;
		friend	class				StateMachine;			// SHAREDTRANS
	};

	//! A special container for motion transitions (lazy-sorted by priority)
	class ICECHARACTER_API Transitions : public Allocateable
	{
		public:
									Transitions();
									~Transitions();

				MotionTransition*	CreateTransition(const MOTIONTRANSITIONCREATE& create);
				bool				DeleteTransition(MotionTransition* trans);
				void				AddSharedTransition(MotionTransition* trans);	// SHAREDTRANS

		inline_	udword				GetNbTransitions()			const	{ return mTransitions.GetNbEntries();					}
		inline_	MotionTransition*	GetTransition(udword id)	const
									{
										if(!mSorted)	const_cast<Transitions* const>(this)->Sort();	// "mutable method"
										return (MotionTransition*)mTransitions.GetEntry(id);
									}
		inline_	MotionTransition**	GetTransitions()			const
									{
										if(!mSorted)	const_cast<Transitions* const>(this)->Sort();	// "mutable method"
										return (MotionTransition**)mTransitions.GetEntries();
									}
		inline_	MotionTransition*	GetNullTransition()			const
									{
										if(!mSorted)	const_cast<Transitions* const>(this)->Sort();	// "mutable method"
										return mNullTransition;
									}

				MotionTransition*	FindTransition(const BasicMotionEvent& event)	const;

				bool				Save(CustomArray& ca, StateMachineSaveContext& context)	const;
				bool				Load(const CustomArray& ca, StateMachineLoadContext& context);
				bool				PostLoad(StateMachineLoadContext& context);
		private:
				PtrContainer		mTransitions;
				MotionTransition*	mNullTransition;
				BOOL				mSorted;
		// Internal methods
				void				Sort();
	};

#ifdef SUPPORT_NEW_DESIGN
	class ICECHARACTER_API TransitionState : public Allocateable
	{
		public:
									TransitionState();
									~TransitionState();

				udword				mTrustLevel;			//!< From 0 to mEvent.NbBasicEvents. Transition is valid when reaching mEvent.NbBasicEvents.
	};
#endif

#endif // ICEMOTIONTRANSITION_H
