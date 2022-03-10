///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for motion events.
 *	\file		IceMotionEvent.h
 *	\author		Pierre Terdiman
 *	\date		May, 09, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMOTIONEVENT_H
#define ICEMOTIONEVENT_H

	struct ICECHARACTER_API BasicMotionEvent : public Allocateable
	{
		inline_				BasicMotionEvent() :
							ID				(0),
							AllowedDelay	(0.0f)
#ifdef SUPPORT_TARGET_OVERRIDE
							, TargetOverride	(null)
#endif
							{
							Param.d	= 0;
							}
		udword				ID;					//!< Event's ID (user-defined, must be >0)
		ucell				Param;				//!< Event's parameter
		float				AllowedDelay;		//!< Once this delay is elapsed, the event is discarded
#ifdef SUPPORT_TARGET_OVERRIDE
		MotionCell*			TargetOverride;		//!< Possible target override
#endif
	};

#ifdef SUPPORT_COMPLEX_EVENTS
	struct ICECHARACTER_API MotionEvent : public Allocateable
	{
		udword				NbBasicEvents;					//!< A motion event can be made of multiple basic events (e.g. a combo in SoulCalibur)
		BasicMotionEvent	BasicEvent[MAX_BASIC_EVENTS];	//!< Basic events list ### FIX?
	};
#else
	typedef BasicMotionEvent MotionEvent;
#endif

	FUNCTION ICECHARACTER_API bool			RegisterMotionEvent(const char* name, udword id, udword param);
	FUNCTION ICECHARACTER_API udword		GetNbMotionEvents();
	FUNCTION ICECHARACTER_API const char*	EnumMotionEvent(udword& id, udword& param);
	FUNCTION ICECHARACTER_API const char*	GetMotionEvent(udword i, udword& id, udword& param);
	FUNCTION ICECHARACTER_API void			ReleaseMotionEvents();

#endif // ICEMOTIONEVENT_H