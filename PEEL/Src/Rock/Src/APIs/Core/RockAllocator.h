#ifndef ROCK_ALLOCATOR_H
#define ROCK_ALLOCATOR_H

#include <Core/RockCommon.h>
#include <malloc.h>

namespace Rock
{
	enum MemoryType
	{
		ROCK_MEMORY_PERSISTENT,
		ROCK_MEMORY_TEMP,
	};

	class ROCK_API Allocator
	{
		public:
						Allocator()		{}
		virtual			~Allocator()	{}

		virtual void*	malloc(size_t size, MemoryType type)																				= 0;
		virtual void*	mallocDebug(size_t size, const char* filename, u32 line, const char* class_name, MemoryType type, bool from_new)	= 0;
		//virtual void*	realloc(void* memory, size_t size)																					= 0;
		//virtual void*	shrink(void* memory, size_t size)																					= 0;
		virtual void	free(void* memory, bool from_new)																					= 0;
	};

	ROCK_FUNCTION ROCK_API	Allocator&	GetAllocator();

	class ROCK_API UserAllocated
	{
		public:
		inline_	void*	operator new		(size_t size, MemoryType type)															{ return GetAllocator().malloc(size, type);											}
		inline_	void*	operator new		(size_t size, const char * filename, int line, const char* class_name, MemoryType type)	{ return GetAllocator().mallocDebug(size, filename, line, class_name, type, true);	}
		inline_	void*	operator new[]		(size_t size, MemoryType type)															{ return GetAllocator().malloc(size, type);											}
		inline_	void*	operator new[]		(size_t size, const char * filename, int line, const char* class_name, MemoryType type)	{ return GetAllocator().mallocDebug(size, filename, line, class_name, type, true);	}
		inline_	void	operator delete		(void* p)																				{ GetAllocator().free(p, true);	}
		inline_	void	operator delete		(void* p, MemoryType)																	{ GetAllocator().free(p, true);	}
		inline_	void	operator delete		(void* p, const char*, int, const char*, MemoryType)									{ GetAllocator().free(p, true);	}
		inline_	void	operator delete[]	(void* p)																				{ GetAllocator().free(p, true);	}
		inline_	void	operator delete[]	(void* p, MemoryType)																	{ GetAllocator().free(p, true);	}
		inline_	void	operator delete[]	(void* p, const char*, int, const char*, MemoryType)									{ GetAllocator().free(p, true);	}
	};
}

#ifdef _DEBUG
	#define ROCK_ALLOC_TMP(x, y)	GetAllocator().mallocDebug(x, __FL__, #y, ROCK_MEMORY_TEMP, false)
	#define ROCK_ALLOC(x, y)		GetAllocator().mallocDebug(x, __FL__, #y, ROCK_MEMORY_PERSISTENT, false)
#else
	#define ROCK_ALLOC_TMP(x, y)	GetAllocator().malloc(x, ROCK_MEMORY_TEMP)
	#define ROCK_ALLOC(x, y)		GetAllocator().malloc(x, ROCK_MEMORY_PERSISTENT)
#endif
	#define	ROCK_FREE(x)			if(x)	{ GetAllocator().free(x, false); x = null;	}

	#define ROCK_ALLOCATE_TMP(type, count, name)	reinterpret_cast<type*>(ROCK_ALLOC_TMP(count*sizeof(type), name))
	#define ROCK_ALLOCATE(type, count, name)		reinterpret_cast<type*>(ROCK_ALLOC(count*sizeof(type), name))

#ifdef _DEBUG
	#define ROCK_NEW_TMP(x)	new(__FL__, #x, ROCK_MEMORY_TEMP) x
	#define ROCK_NEW(x)		new(__FL__, #x, ROCK_MEMORY_PERSISTENT) x
#else
	#define ROCK_NEW_TMP(x)	new(ROCK_MEMORY_TEMP) x
	#define ROCK_NEW(x)		new(ROCK_MEMORY_PERSISTENT) x
#endif

	#define ROCK_PLACEMENT_NEW(p, T)	new (p) T

namespace Rock
{
	template <typename T>
	class AllocaScopedPointer
	{
	  public:
		inline_	~AllocaScopedPointer()
		{
			if(mOwned)
				ROCK_FREE(mPointer);
		}

		inline_	operator T*() const
		{
			return mPointer;
		}

		T* mPointer;
		bool mOwned;
	};

	#define RockAlloca(x) _alloca(x)

	#define ROCK_ALLOCA(var, type, count)												\
		Rock::AllocaScopedPointer<type> var;											\
		{																				\
			const u32 size = sizeof(type) * (count);									\
			var.mOwned = size > 2048;													\
			if(var.mOwned)																\
				var.mPointer = reinterpret_cast<type*>(ROCK_ALLOC_TMP(size, "alloca"));	\
			else																		\
				var.mPointer = reinterpret_cast<type*>(RockAlloca(size));				\
		}
}

#endif
