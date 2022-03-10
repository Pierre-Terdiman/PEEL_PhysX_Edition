///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains error-related code.
 *	\file		IceError.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEERROR_H
#define ICEERROR_H

	// Error structure
	class ICECORE_API IceError : public Allocateable
	{
		public:
								IceError();
								IceError(const IceError& error);
								~IceError();

		//! Operator for "IceError A = IceError B"
				void			operator = (const IceError& error);

				String			mErrorText;			//!< A string describing the last error
				const char*		mErrorCode;			//!< A possible error code
				udword			mErrorTimeStamp;	//!< A timestamp for the last error
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Sets an error message.
	 *	\relates	IceError
	 *	\fn			SetIceError_(const char* error, const char* error_code, bool update_log)
	 *	\param		error		[in] the error message string
	 *	\param		error_code	[in] a possible user-defined error code.
	 *	\param		update_log	[in] error message is reported on the log file as well, if update_log is true
	 *	\param		file		[in] source file's name (in which the error occured)
	 *	\param		line		[in] source file's line (in which the error occured)
	 *	\return		false, so that you can do "return SetIceError()" in one line.
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION ICECORE_API bool		SetIceError_(const char* error, const char* error_code=null, bool update_log=true, const char* file=null, udword line=0);

	//! This macro automatically completes error message with file & line information
	#define SetIceError(error, error_code)	SetIceError_(error, error_code, true, THIS_FILE, __LINE__)

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Gets the last registered error.
	 *	That error may contain an error string as well as a timestamp.
	 *	\relates	IceError
	 *	\fn			GetLastIceError()
	 *	\return		address of an IceError structure
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION ICECORE_API const IceError*	GetLastIceError();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Gets the last error in the error queue
	 *	\relates	IceError
	 *	\fn			GetError(IceError& error)
	 *	\param		error	[out] the last error
	 *	\return		true if success, else false if there was no error available
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION ICECORE_API	bool	GetError(IceError& error);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Flushes all errors.
	 *	\relates	IceError
	 *	\fn			FlushErrors(CustomArray* log)
	 *	\param		log		[out] the flushed errors (or null)
	 *	\return		the number of flushed errors
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION ICECORE_API	udword	FlushErrors(CustomArray* log=null);

	FUNCTION ICECORE_API	void	GetLastWindowsErrorString(String& error_string);

	#define	ICE_LAST_ERROR_TEXT		GetLastIceError()->mErrorText		//!< Laziness...
	#define	ICE_LAST_ERROR_CODE		GetLastIceError()->mErrorCode		//!< Laziness...

	// Following macros mimic the exception mechanism, without needing exception support (useful on Dreamcast).
	// Standard usage:
	//
	//	ICE_TRY		Subroutine
	//	ICE_CATCH	MessageBox(null, LASTERRORTEXT, "Oops", MB_OK);
	//
	// That's what I call "ICEptions" ...!
	//
	#define	ICE_TRY				udword __RDTSEC = GetLastIceError() ? GetLastIceError()->mErrorTimeStamp : 0;	//!< Read time stamp error counter
	#define ICE_CATCH			if(GetLastIceError() && GetLastIceError()->mErrorTimeStamp!=__RDTSEC)			//!< Compare to previous counter

	// Following macro allows layered ICE_TRY/ICE_CATCH blocks.
	// For example:
	//
	//		ICE_TRY
	//		{
	//			...
	//			ICE_TRY
	//				// Call some code that produces an error
	//			ICE_CATCH
	//			{
	//				// Here we catch the error but we can't fix it.
	//			}
	//			...
	//		}
	//		ICE_CATCH	// Here the error gets reported again, so the higher level gets a chance to handle the error.
	//
	//	The other behaviour is the following one:
	//
	//		ICE_TRY
	//		{
	//			...
	//			ICE_TRY
	//				// Call some code that produces an error
	//			ICE_CATCH
	//			{
	//				// Here we catch the error and handle it. So the error has been resolved
	//				// and shouldn't be reported below. So we tell the system to discard it.
	//				ICE_RESOLVED
	//			}
	//			...
	//		}
	//		ICE_CATCH	// Here no error is found
	//
	#define ICE_RESOLVED		if(GetLastIceError())	((IceError*)GetLastIceError())->mErrorTimeStamp=__RDTSEC;	//!< 

	// Catch all errors and display a message box
	#define ICE_CATCH_AND_DUMP																		\
		ICE_CATCH																					\
		{																							\
			CustomArray Errors;																		\
			FlushErrors(&Errors);																	\
			IceCore::MessageBox(null, (const char*)Errors.Collapse(), "Reported errors", MB_OK);	\
		}

#endif // ICEERROR_H
