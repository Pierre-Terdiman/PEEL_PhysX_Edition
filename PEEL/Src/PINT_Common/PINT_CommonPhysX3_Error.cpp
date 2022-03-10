///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PINT_CommonPhysX3_Error.h"

///////////////////////////////////////////////////////////////////////////////

udword gNbPhysXErrors = 0;
void PEEL_PhysX3_ErrorCallback::reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	gNbPhysXErrors++;
	printf("%s\n", message);
	SetIceError(message, null);
}
