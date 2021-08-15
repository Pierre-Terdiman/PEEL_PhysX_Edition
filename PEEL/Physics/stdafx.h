///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#pragma once

/*#pragma warning( disable : 4239 )	// nonstandard extension used
#pragma warning( disable : 4389 )	// signed/unsigned mismatch
#pragma warning( disable : 4505 )	// unreferenced local function has been removed
#pragma warning( disable : 4510 )	// default constructor could not be generated
#pragma warning( disable : 4512 )	// assignment operator could not be generated
#pragma warning( disable : 4189 )	// local variable is initialized but not referenced
#pragma warning( disable : 4701 )	// potentially uninitialized local variable 'hitDist' used
#pragma warning( disable : 4702 )	// unreachable code
#pragma warning( disable : 4706 )	// assignment within conditional expression
#pragma warning( disable : 4822 )	// local class member function does not have a body
#pragma warning( disable : 4610 )	// union ... can never be instantiated - user defined constructor required
#pragma warning( disable : 4201 )	// nonstandard extension used : nameless struct/union
#pragma warning( disable : 4245 )	// conversion from 'int' to 'udword', signed/unsigned mismatch
#pragma warning( disable : 4296 )	// expression is always true
//#pragma warning( disable : 4242 )	// conversion from 'physx::PxU32' to 'physx::PxU8', possible loss of data
*/

#include ".\PINT_Common\PINT_Ice.h"

#include "GL/glew.h"
#include "GlutX/Include/GlutX.h"
#include <vector>

#define STRINGIFY(A) #A

#ifndef _WIN64
	//#define USE_SPY
#endif
#ifdef USE_SPY
	#include "Spy\SpyClient.h"
	#define SPY_ZONE(Label)	Spy::Zone __SpyZone(Label);
#else
	#define SPY_ZONE(Label)
#endif
