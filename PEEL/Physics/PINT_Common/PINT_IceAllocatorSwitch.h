///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_ICE_ALLOCATOR_SWITCH_H
#define PINT_ICE_ALLOCATOR_SWITCH_H

	class AllocSwitch
	{
	public:
		AllocSwitch();
		~AllocSwitch();
	};

	void	InitIceAllocator(const char* name);
	void	ReleaseIceAllocator();
	udword	GetIceAllocatorUsedMemory();

#endif