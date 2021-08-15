///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains error & message codes for IceCore.
 *	\file		IceCoreErrors.h
 *	\author		Pierre Terdiman
 *	\date		July, 8, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECOREERRORS_H
#define ICECOREERRORS_H

	// Error codes for IceCore
	extern ICECORE_API const char* EC_OUT_OF_MEMORY;
	extern ICECORE_API const char* EC_FILE_NOT_FOUND;
	extern ICECORE_API const char* EC_INVALID_CALLBACK;
	extern ICECORE_API const char* EC_REPLICATION_FAILED;
	extern ICECORE_API const char* EC_TOO_MANY_MSGS;
	extern ICECORE_API const char* EC_CORRUPTED_KERNEL;
	extern ICECORE_API const char* EC_LOST_CELLS;
	extern ICECORE_API const char* EC_INCOMPATIBLE_FIELDS;
	extern ICECORE_API const char* EC_INVALID_PARAMETERS;
	extern ICECORE_API const char* EC_INVALID_STATE;

	// Script-related
	extern ICECORE_API const char* EC_INVALID_PARAM_NUMBER;
	extern ICECORE_API const char* EC_INVALID_PARAM_TYPE;
	extern ICECORE_API const char* EC_INVALID_IFDEF;
	extern ICECORE_API const char* EC_INVALID_ENDIF;

#endif // ICECOREERRORS_H
