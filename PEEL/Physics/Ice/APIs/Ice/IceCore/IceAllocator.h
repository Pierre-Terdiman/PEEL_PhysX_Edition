///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an allocator base class.
 *	\file		IceAllocator.h
 *	\author		Pierre Terdiman
 *	\date		December, 19, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEALLOCATOR_H
#define ICEALLOCATOR_H

	enum MemoryType
	{
		MEMORY_PERSISTENT,
		MEMORY_TEMP,
	};

	class ICECORE_API Allocator
	{
		public:
						Allocator()																											{}
		virtual			~Allocator()																										{}

		virtual void*	malloc(size_t size, MemoryType type)																				= 0;
		virtual void*	mallocDebug(size_t size, const char* filename, udword line, const char* class_name, MemoryType type, bool from_new)	= 0;
		virtual void*	realloc(void* memory, size_t size)																					= 0;
		virtual void*	shrink(void* memory, size_t size)																					= 0;
		virtual void	free(void* memory, bool from_new)																					= 0;

		virtual	void	FrameAnalysis()																										{}
	};

	FUNCTION ICECORE_API Allocator*	CreateDefaultAllocator();
	FUNCTION ICECORE_API Allocator* GetAllocator();
	FUNCTION ICECORE_API bool		SetAllocator(Allocator& allocator);
	FUNCTION ICECORE_API void		DumpMemory();

	class ICECORE_API Allocateable
	{
		public:
#ifdef DONT_TRACK_MEMORY_LEAKS
		inline_	void*	operator new		(size_t size, MemoryType type)															{ return GetAllocator()->malloc(size, type);										}
		inline_	void*	operator new		(size_t size, const char * filename, int line, const char* class_name, MemoryType type)	{ return GetAllocator()->mallocDebug(size, filename, line, class_name, type, true);	}
		inline_	void*	operator new[]		(size_t size, MemoryType type)															{ return GetAllocator()->malloc(size, type);										}
		inline_	void*	operator new[]		(size_t size, const char * filename, int line, const char* class_name, MemoryType type)	{ return GetAllocator()->mallocDebug(size, filename, line, class_name, type, true);	}
		inline_	void	operator delete		(void* p)																				{ GetAllocator()->free(p, true);	}
		inline_	void	operator delete		(void* p, MemoryType)																	{ GetAllocator()->free(p, true);	}
		inline_	void	operator delete		(void* p, const char*, int, const char*, MemoryType)									{ GetAllocator()->free(p, true);	}
		inline_	void	operator delete[]	(void* p)																				{ GetAllocator()->free(p, true);	}
		inline_	void	operator delete[]	(void* p, MemoryType)																	{ GetAllocator()->free(p, true);	}
		inline_	void	operator delete[]	(void* p, const char*, int, const char*, MemoryType)									{ GetAllocator()->free(p, true);	}
#endif
	};

#endif // ICEALLOCATOR_H
