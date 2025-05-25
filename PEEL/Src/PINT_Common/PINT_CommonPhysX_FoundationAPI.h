///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX_FOUNDATION_API_H
#define PINT_COMMON_PHYSX_FOUNDATION_API_H

#ifdef PHYSX_NEW_PUBLIC_API
	namespace ps = physx;
	#define	_getFoundation	PxGetFoundation
#else
	#include "PsFoundation.h"
	namespace ps = physx::shdfnd;
	#define	_getFoundation	ps::getFoundation
#endif

#endif