///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a state machine compiler.
 *	\file		IceStateMachineCompiler.h
 *	\author		Pierre Terdiman
 *	\date		May, 08, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESTATEMACHINECOMPILER_H
#define ICESTATEMACHINECOMPILER_H

	FUNCTION ICECHARACTER_API	udword	DecodeImpactString(const String& str);
	FUNCTION ICECHARACTER_API	void	CreateImpactString(String& str, udword flags);

	FUNCTION ICECHARACTER_API	udword	DecodeMotionTypeString(const String& str);
	FUNCTION ICECHARACTER_API	void	CreateMotionTypeString(String& str, udword flags);

	//! Creation structure
	struct ICECHARACTER_API SMCOMPILERCREATE
	{
						SMCOMPILERCREATE() : mFilenames(null), mBuffer(null), mStateMachine(null)
#ifdef SUPPORT_MOTION_MODIFIERS
						, mNbModifiers(0)
#endif
#ifdef SUPPORT_WEAPON_MODIFIERS
						, mNbWModifiers(0)
#endif
						{}

		// Source
		const char*		mFilenames;		//!< Possible source files (separated by |)
		const char*		mBuffer;		//!< Possible source buffer
		// Destination
		StateMachine*	mStateMachine;	//!< Destination state machine
#ifdef SUPPORT_MOTION_MODIFIERS
		udword			mNbModifiers;	//!< Number of modifiers in each cell
#endif
#ifdef SUPPORT_WEAPON_MODIFIERS
		udword			mNbWModifiers;	//!< Number of weapon modifiers in each cell
#endif
	};

	class ICECHARACTER_API StateMachineCompiler : public Allocateable
	{
		public:
										StateMachineCompiler();
										~StateMachineCompiler();

				bool					CreateStateMachine(const SMCOMPILERCREATE& create);
				bool					AddMotion(const MotionCell* motion_cell);
				MotionCell*				FindMotion(const String& motion_name);
				void					FlushMotions();

				bool					RegisterTransition();

		// Callback control
		inline_	void					SetUserData(void* data)							{ mUserData	= data;				}
		inline_	void					SetMotionCallback(SMC_MOTION_CALLBACK callback)	{ mMotionCallback = callback;	}
		inline_	void					SetSoundCallback(SMC_SOUND_CALLBACK callback)	{ mSoundCallback = callback;	}

		inline_	void*					GetUserData()							const	{ return mUserData;				}
		inline_	SMC_MOTION_CALLBACK		GetMotionCallback()						const	{ return mMotionCallback;		}
		inline_	SMC_SOUND_CALLBACK		GetSoundCallback()						const	{ return mSoundCallback;		}

		inline_	StateMachine*			GetStateMachine()						const	{ return mSM;					}
//		inline_	Container&				GetMotions()									{ return mMotions;				}

		// 
				udword					ComputeSourceFrameIndex(const char* text);
				udword					ComputeDestFrameIndex(const char* text);
#ifdef OBSOLETE_ACTIVATION
				bool					SetupActivation(const String& param, const char* symbol_name);
#endif
				bool					SetupTimeBlendMapping(const String& param, const char* symbol_name);
		private:
//				Container				mMotions;
				ConstantManager			mMotions_;
				StateMachine*			mSM;
		public:
#ifdef SUPPORT_MOTION_MODIFIERS
				udword					mNbModifiers;	//!< Number of motion modifiers in each cell
#endif
#ifdef SUPPORT_WEAPON_MODIFIERS
				udword					mNbWModifiers;	//!< Number of weapon modifiers in each cell
#endif
				String					mTransName;
				MOTIONTRANSITIONCREATE*	mCurTrans;			//!< Current transition beeing assembled
				MotionCell*				mStartMotion;		//!< Current transition's start motion
				MotionCell*				mEndMotion;			//!< Current transition's end motion
				MotionTransition*		mLastSharedTrans;
		private:
		// User callback
				void*					mUserData;			//!< User-defined data sent to callbacks
				SMC_MOTION_CALLBACK		mMotionCallback;
				SMC_SOUND_CALLBACK		mSoundCallback;
	};

#endif // ICESTATEMACHINECOMPILER_H