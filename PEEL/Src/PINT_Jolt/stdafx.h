///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#define ICE_NO_DLL

//#define __SSE2__
//#define TASKING_INTERNAL
#pragma warning( disable : 4530 )

#define ICEINTERVAL_H
#define ICEINCLUSION_H
#define CTCCONTINUOUS_H
#include "..\PINT_Common\PINT_Ice.h"
#undef Interval

//#undef null

//#define USE_JOLT_0	// Jolt code from initial PEEL integration, looks like changes aren't backward compatible :(
#define USE_JOLT_1	// Jolt code from 12/03/2022

