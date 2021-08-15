///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for the motion state.
 *	\file		IceMotionState.h
 *	\author		Pierre Terdiman
 *	\date		May, 08, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMOTIONSTATE_H
#define ICEMOTIONSTATE_H

	#define STATEMACHINE_MAX_NB_EVENTS	128		// We keep track of STATEMACHINE_MAX_NB_EVENTS in an event stack ### Make it dynamic + resize methods
//	#define STATEMACHINE_MAX_NB_EVENTS	32		// We keep track of STATEMACHINE_MAX_NB_EVENTS in an event stack ### Make it dynamic + resize methods
//	#define STATEMACHINE_MAX_NB_EVENTS	3		// We keep track of STATEMACHINE_MAX_NB_EVENTS in an event stack ### Make it dynamic + resize methods

	// ### hmmm, how many do we need?
//	#define	MAX_NB_CELL_STATES			4
	#define	MAX_NB_CELL_STATES			16

	enum EventCode
	{
		EVENT_DISCARDED	=	(1<<0),
	};

	class Skeleton;

	// Callbacks
	typedef void	(*EventCallback)		(const BasicMotionEvent&, EventCode, void*);
	typedef bool	(*TransitionCallback)	(MotionCell*, MotionTransition*, Skeleton*, void*);
	typedef void	(*MotionCallback)		(MotionCell*, Skeleton*, void*);

	struct ICECHARACTER_API StateMachineEvent : public Allocateable
	{
		BasicMotionEvent	mEvent;
		float				mTime;
//		MotionCell*			mCell;
	};

	class ICECHARACTER_API EventStack
	{
		public:
										EventStack();
										~EventStack();

				bool					Init();
				void					Reset();
		// Events
		inline_	bool					IsEventStackFull()				const			{ return mNbEvents==STATEMACHINE_MAX_NB_EVENTS;	}
				bool					SetMotionEvent(const BasicMotionEvent& event, float relative_time);
		inline_	const BasicMotionEvent&	GetMotionEvent()				const			{ return mPendingEvents[0].mEvent;				}
		inline_	float					GetMotionEventTime()			const			{ return mPendingEvents[0].mTime;				}
				void					ShiftEventStack(udword nb=1);
		inline_	udword					GetNbEvents()					const			{ return mNbEvents;								}
				void					Save(udword& nb_events, StateMachineEvent* events)	const;
				void					Restore(udword nb_events, const StateMachineEvent* events);

		// Events
				udword					mNbEvents;			//!< Number of events in the event stack
//				StateMachineEvent*		mPendingEvents;		//!< Event stack
				StateMachineEvent		mPendingEvents[STATEMACHINE_MAX_NB_EVENTS];		//!< Event stack;
	};

	// ### design in progress
	class ICECHARACTER_API MotionState
	{
		public:
									MotionState();
									~MotionState();

				bool				Save(CustomArray& ca)	const;
				bool				Load(const CustomArray& ca);

				bool				Init();
				void				Reset();

				void				SetActiveCell(MotionCell* cell);
		inline_	MotionCell*			GetActiveCell()					const			{ return mActiveMotionCell;						}
				void				SetPreviousCell(MotionCell* cell)				{ mPreviousCell = cell;							}
		inline_	MotionCell*			GetPreviousCell()				const			{ return mPreviousCell;							}


				void				FlushEvents(udword nb);
		inline_	void				FlushAllEvents()								{ FlushEvents(mEvents.mNbEvents);				}

				MotionTransition*	GetNextTransition(float current_time);
				MotionTransition*	FindBetterTransition(udword motion_local_frame, udword motion_index);
				MotionTransition*	FindBreakTransition(udword motion_local_frame, udword motion_index);

		inline_	void				SetEventCallback(EventCallback cb)				{ mEventCB			= cb;						}
		inline_	void				SetTransitionCallback(TransitionCallback cb)	{ mTransitionCB		= cb;						}
		inline_	void				SetMotionStartCallback(MotionCallback cb)		{ mMotionStartCB	= cb;						}
		inline_	void				SetMotionCallback(MotionCallback cb)			{ mMotionCB			= cb;						}
		inline_	void				SetUserData(void* user_data)					{ mUserData			= user_data;				}

		inline_	EventCallback		GetEventCallback()				const			{ return mEventCB;								}
		inline_	TransitionCallback	GetTransitionCallback()			const			{ return mTransitionCB;							}
		inline_	MotionCallback		GetMotionCallback()				const			{ return mMotionCB;								}
		inline_	MotionCallback		GetMotionStartCallback()		const			{ return mMotionStartCB;						}
		inline_	void*				GetUserData()					const			{ return mUserData;								}

#ifdef SUPPORT_NEW_DESIGN
				TransitionState*	InitTransitions();
				void				ResetTransitions();
#endif

		static	EventCallback		mEventCB;
		static	TransitionCallback	mTransitionCB;
		static	MotionCallback		mMotionStartCB;
		static	MotionCallback		mMotionCB;
				void*				mUserData;
		private:
				MotionCell*			mActiveMotionCell;
				MotionCell*			mPreviousCell;		//!< Keep track of previous motion cell
		public:
//				MotionCell*			mStartMotion;		//!< Starting motion in the state machine
//				MotionCell*			mEndMotion;			//!< Ending motion in the state machine
				MotionTransition*	mTrans;				//!< Current transition
#ifndef SUPPORT_CELL_ACTIVE_TRANS
				MotionTransition*	mActiveTransition;
#endif
#if SUPPORT_MOTION_MODIFIERS||SUPPORT_WEAPON_MODIFIERS
				MotionCell*			mTargetCell;
#endif
#ifdef SUPPORT_NEW_DESIGN
				udword				mNbStates;
				TransitionState*	mTransStates;
#endif
#ifdef SUPPORT_NEW_DESIGN2
				MotionCellState&	FindState(const MotionCell* cell);
				MotionCellState		mCellStates[MAX_NB_CELL_STATES];
#endif
				EventStack			mEvents;
		private:
				void				SelectTransition(MotionTransition* mtr, udword event_id);

/*#ifdef SUPPORT_NEW_DESIGN

	#ifdef SUPPORT_WEAPON_MODIFIERS2
				udword				SrcLastFrameCheck(udword value, udword index) const;
				udword				DstLastFrameCheck(udword value, udword index) const;
	#else
				udword				SrcLastFrameCheck(udword value) const;
				udword				DstLastFrameCheck(udword value) const;
	#endif

	#ifdef SUPPORT_WEAPON_MODIFIERS2
				udword				GetTargetBaseFrame(udword i) const;
				udword				GetSourceBaseFrame(udword i) const;
				TimeSegment			GetValidityInterval(udword i) const;
	#endif
#endif*/
	};

#endif // ICEMOTIONSTATE_H