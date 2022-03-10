///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PINT_Common.h"
#include "PINT_IceAllocatorSwitch.h"

static MyIceAllocator* gIceAllocator = null;

///////////////////////////////////////////////////////////////////////////////

void InitIceAllocator(const char* name)
{
	ASSERT(!gIceAllocator);
	gIceAllocator = new MyIceAllocator(name);
	ASSERT(gIceAllocator);
}

void ReleaseIceAllocator()
{
	DELETESINGLE(gIceAllocator);
}

udword GetIceAllocatorUsedMemory()
{
	return gIceAllocator->mUsedMemory;
}

///////////////////////////////////////////////////////////////////////////////

AllocSwitch::AllocSwitch()
{
	ASSERT(gIceAllocator);
	SetAllocator(*gIceAllocator);
}

AllocSwitch::~AllocSwitch()
{
	SetAllocator(*gIceAllocator->mPreviousAllocator);
}

///////////////////////////////////////////////////////////////////////////////
