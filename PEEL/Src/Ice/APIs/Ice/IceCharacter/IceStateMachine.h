///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for the state machine.
 *	\file		IceStateMachine.h
 *	\author		Pierre Terdiman
 *	\date		May, 08, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESTATEMACHINE_H
#define ICESTATEMACHINE_H

	// Forward declarations
	class Skeleton;

	// Callbacks
//	typedef bool	(*TransitionCallback)	(MotionCell*, MotionTransition*, Skeleton*, void*);
//	typedef void	(*MotionCallback)		(MotionCell*, Skeleton*, void*);
	typedef CharacterMotion*	(*SMC_MOTION_CALLBACK)		(const String& filename, void* user_data);
	typedef udword				(*SMC_SOUND_CALLBACK)		(const String& filename, void* user_data);
	typedef bool				(*SMC_SOUND_CALLBACK_SAVE)	(String& name, udword handle, void* user_data);

	struct ICECHARACTER_API STATEMACHINECREATE : public Allocateable
	{
									STATEMACHINECREATE();

				const char*			mName;	//!< State machine's name, or null
	};

	class ICECHARACTER_API StateMachine : public Allocateable
	{
		private:
		// Construction/Destruction
									StateMachine();
									~StateMachine();
		public:
		// Initialize
				bool				Init(const STATEMACHINECREATE& create);
				void				Reset();

				bool				Update(Skeleton& skeleton, float current_time)	const;
		// 
				MotionCell*			CreateMotionCell(const MOTIONCELLCREATE& create);
				bool				DeleteMotionCell(MotionCell* mc);

				// SHAREDTRANS
				MotionTransition*	CreateSharedTransition(const MOTIONTRANSITIONCREATE& create);
				bool				DeleteSharedTransition(MotionTransition* trans);

		// Data access
		inline_	const String&		GetName()						const			{ return mName;									}

		inline_	udword				GetNbMotions()					const			{ return mMotionCells.GetNbEntries();			}
		inline_	MotionCell*			GetMotion(udword i)				const			{ return (MotionCell*)mMotionCells.GetEntry(i);	}
				MotionCell*			GetMotion(const char* name, bool cell_name=false)	const;

		// Callbacks
/*		inline_	void				SetTransitionCallback(TransitionCallback cb)	{ mTransitionCB		= cb;						}
		inline_	void				SetMotionStartCallback(MotionCallback cb)		{ mMotionStartCB	= cb;						}
		inline_	void				SetMotionCallback(MotionCallback cb)			{ mMotionCB			= cb;						}
		inline_	void				SetUserData(void* user_data)					{ mUserData			= user_data;				}

		inline_	TransitionCallback	GetTransitionCallback()			const			{ return mTransitionCB;							}
		inline_	MotionCallback		GetMotionCallback()				const			{ return mMotionCB;								}
		inline_	MotionCallback		GetMotionStartCallback()		const			{ return mMotionStartCB;						}
		inline_	void*				GetUserData()					const			{ return mUserData;								}
*/
		inline_	MotionCell*			GetDefaultCell()				const			{ return mDefaultCell;							}
		inline_	void				SetDefaultCell(MotionCell* cell)				{ mDefaultCell = cell;							}

		inline_	void				SetUserData(void* user_data)					{ mUserData	= user_data;						}
		inline_	void*				GetUserData()					const			{ return mUserData;								}

		// DEBUG
				void				(*mCallBack)(const char* event, void* user_data);

				bool				Save(CustomArray& ca, SMC_SOUND_CALLBACK_SAVE sound_cb, void* user_data)	const;
				bool				Load(const CustomArray& ca, SMC_MOTION_CALLBACK motion_cb, SMC_SOUND_CALLBACK sound_cb, void* user_data);

									PREVENT_COPY(StateMachine)
		private:
				String				mName;
				MotionCell*			mDefaultCell;
				PtrContainer		mMotionCells;
				// SHAREDTRANS
				PtrContainer		mSharedTransitions;
				void*				mUserData;
		//
		// Callbacks
/*				TransitionCallback	mTransitionCB;
				MotionCallback		mMotionStartCB;
				MotionCallback		mMotionCB;
				void*				mUserData;*/

		friend	class				StateMachineFactory;
	};

	class ICECHARACTER_API StateMachineFactory : public Allocateable
	{
		public:
									StateMachineFactory();
									~StateMachineFactory();
		// Data access
		inline_	udword				GetNbStateMachines()	const	{ return mMachines.GetNbEntries();					}
		inline_	StateMachine**		GetStateMachines()		const	{ return (StateMachine**)mMachines.GetEntries();	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		*	Creates a state machine
		*	\param		create	[in] state machine creation structure
		*	\return		new state machine, or null if failed
		*/
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				StateMachine*		CreateStateMachine(const STATEMACHINECREATE& create);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		*	Deletes all state machines
		*/
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				void				DeleteMachines();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		*	Finds a state machine by name
		*	\param		name	[in] name of requested state machine
		*	\return		requested state machine, or null if not found
		*/
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				StateMachine*		FindStateMachine(const char* name);
		private:
				PtrContainer		mMachines;
		// Internal methods
	};

#endif // ICESTATEMACHINE_H