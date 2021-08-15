///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains all memory-related code.
 *	\file		IceMemoryManager.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMEMORYMANAGER_H
#define ICEMEMORYMANAGER_H

#ifndef DONT_TRACK_MEMORY_LEAKS
	#ifdef _DEBUG
	ICECORE_API void* Alloc(udword size, const char* file, sdword line);
	ICECORE_API bool Free(void* addr);
	#else
	ICECORE_API void* Alloc(udword size);
	ICECORE_API void Free(void* addr);
	#endif

	#ifdef _DEBUG
		#define MEMBLOCKSTART		64
		inline_ void* __cdecl operator new(unsigned int size, const char* file, sdword line)	{	return Alloc(size, file, line);	}	//!< new overload
		inline_ void operator delete(void* addr, const char* file, sdword line)					{	Free(addr);						}	//!< delete overload
		inline_ void operator delete(void* addr)												{	Free(addr);						}	//!< delete overload
		#undef NEW_D
		#undef DELETE_D
		#define NEW_D ::new(THIS_FILE, __LINE__)
		#define DELETE_D ::delete
		#define new NEW_D
		#define delete DELETE_D
	#else
		#ifndef DEFAULT_NEWDELETE
			inline_ void* __cdecl operator new(unsigned int size)								{	return Alloc(size);				}	//!< new overload
			inline_ void operator delete(void* addr)											{	Free(addr);						}	//!< delete overload
		#endif
	#endif
#endif

#endif // ICEMEMORYMANAGER_H
