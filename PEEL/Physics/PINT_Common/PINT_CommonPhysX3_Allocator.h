///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_ALLOCATOR_H
#define PINT_COMMON_PHYSX3_ALLOCATOR_H

	class PEEL_PhysX3_AllocatorCallback : public PxAllocatorCallback
	{
	public:
		struct Header
		{
			udword		mMagic;
			udword		mSize;
			const char*	mType;
			const char*	mFilename;
			udword		mLine;
		};

						PEEL_PhysX3_AllocatorCallback();
		virtual			~PEEL_PhysX3_AllocatorCallback();

		virtual	void*	allocate(size_t size, const char* typeName, const char* filename, int line);
		virtual	void	deallocate(void* ptr);

				udword	mTotalNbAllocs;
				udword	mNbAllocs;
				udword	mCurrentMemory;
				bool	mLog;
	};

#endif

