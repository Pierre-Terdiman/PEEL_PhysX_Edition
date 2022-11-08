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
	#include "foundation/PxArray.h"
	#include "foundation/PxHashSet.h"
	#include "foundation/PxHashMap.h"
	namespace ps = physx;
	#define _hashmap		physx::PxHashMap
	#define _hashset		physx::PxHashSet
	#define _array			physx::PxArray
	#define _computeHash	PxComputeHash
#else
	#include "PsArray.h"
	#include "PsHashSet.h"
	#include "PsHashMap.h"
	namespace ps = physx::shdfnd;
	#define _hashmap		ps::HashMap
	#define _hashset		ps::HashSet
	#define _array			ps::Array
	#define _computeHash	hash
#endif

#endif