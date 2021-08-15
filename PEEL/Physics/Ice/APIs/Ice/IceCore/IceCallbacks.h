///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for standard callbacks.
 *	\file		IceCallbacks.h
 *	\author		Pierre Terdiman
 *	\date		July, 8, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECALLBACKS_H
#define ICECALLBACKS_H

	//! Base structure for callback parameters
	struct ICECORE_API IceCallbackParams : public Allocateable
	{
	};

	//! Currently unused
	struct ICECORE_API IceCallbackBundle : public Allocateable
	{
		void*	Callback;
		void*	UserData;
	};

	//! Callback codes. The framework calls the user back for all those events.
	enum IceCoreCallbackCode
	{
		ICCB_REQUEST,				//!< Called to request a app-dependent implementation
		ICCB_ERROR,					//!< Called when an error happens.
//		ICCB_LOG,					//!< Obsolete.
		ICCB_MESSAGE,				//!< Called when a message is sent.
//		ICCB_NEWPLUGIN,				//!< Obsolete.
//		ICCB_DISCARDPLUGIN,			//!< Obsolete.
		ICCB_OPEN_INTERFACE,		//!< Called when a new interface is registered (after opening interface).
		ICCB_CLOSE_INTERFACE,		//!< Called when a new interface is unregistered (before closing interface).
		ICCB_REGISTRY_ACCESS,		//!< Called on each registry access.
		ICCB_DUPLICATE_NAME_FOUND,	//!< Called to solve naming conflicts (only if KF_CELL_UNIQUE_NAME is set)
		ICCB_EVENT_PROC,			//!< Called on each system event (event procedure)
		ICCB_OBJECT_LIFETIME,		//!< Called on each lifetime-related event

		ICCB_SIZE,

		ICCB_FORCE_DWORD			= 0x7fffffff
	};

	//! Callback definition
	typedef bool (*IceCallback)	(IceCoreCallbackCode callback_code, IceCallbackParams* params, void* user_data);

	#include "IceCallbacksDef.h"
	#include "IceCallbacksTemplateDef.h"

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Returns the callback manager.
	 *	\relates	IceCoreCallbacksManager
	 *	\fn			GetCallbacksManager()
	 *	\return		the global callback manager for this dll
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION ICECORE_API IceCoreCallbacksManager* GetCallbacksManager();

	//

	//! Callback parameters for ICCB_REQUEST
	struct ICECORE_API RequestParams : public IceCallbackParams
	{
		const char*	mRequestCode;
	};

	//! Callback parameters for ICCB_ERROR
	struct ICECORE_API ErrorParams : public IceCallbackParams
	{
		const char*	mError;
		const char*	mErrrorCode;
		bool		mUpdateLog;
	};

	//! Callback parameters for ICCB_MESSAGE
	struct ICECORE_API MessageParams : public IceCallbackParams
	{
		const IceMsg*	mMsg;
		float			mDelay;
	};

	//! Callback parameters for ICCB_OPEN_INTERFACE and ICCB_CLOSE_INTERFACE
	struct ICECORE_API InterfaceParams : public IceCallbackParams
	{
		IceInterface*	mInterface;
	};

	//! Callback parameters for ICCB_DUPLICATE_NAME_FOUND
	struct ICECORE_API DuplicateParams : public IceCallbackParams
	{
		Cell*	mCurrent;		//!< Object for which conflict has been detected
		Cell*	mDuplicate;		//!< Already existing object with same name
	};

	//! Callback parameters for ICCB_EVENT_PROC
	struct ICECORE_API EventProcParams : public IceCallbackParams
	{
		HWND	mHWnd;
		HWND	mRenderHWnd;
		UINT	mMsg;
		WPARAM	mWParam;
		LPARAM	mLParam;
	};

	enum LifetimeEvent
	{
		LIFETIME_CREATION,
		LIFETIME_DESTRUCTION,

		LIFETIME_FORCE_DWORD	= 0x7fffffff
	};

	//! Callback parameters for ICCB_OBJECT_LIFETIME
	struct ICECORE_API LifetimeEventParams : public IceCallbackParams
	{
		LifetimeEvent	mEventCode;
		Cell*			mObject;
	};

#endif // ICECALLBACKS_H
