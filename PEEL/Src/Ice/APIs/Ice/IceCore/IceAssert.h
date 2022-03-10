///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains custom assertion code.
 *	\file		IceAssert.h
 *	\author		Pierre Terdiman
 *	\date		January, 14, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEASSERT_H
#define ICEASSERT_H

#ifdef _WIN64
	#define	ICE_DEBUG_BREAK	__debugbreak();
#else
	#define	ICE_DEBUG_BREAK	_asm { int 3 }
#endif

	FUNCTION ICECORE_API bool CustomAssertFunction(size_t, char*, int, char*, bool&);

// Leave the {} so that you can write this kind of things safely in release mode:
//	if(condition)	ASSERT()

#ifndef ASSERT
	#if defined( _DEBUG )
		//! Custom ASSERT function. Various usages:
		//! ASSERT(condition)
		//! ASSERT(!"Not implemented")
		//! ASSERT(condition && "error text")
		#define ASSERT(exp)																		\
		{																						\
			static bool IgnoreAlways = false;													\
			if(!IgnoreAlways)																	\
			{																					\
				if(CustomAssertFunction((size_t)(exp), #exp, __LINE__, __FILE__, IgnoreAlways))	\
				{																				\
					ICE_DEBUG_BREAK																\
				}																				\
			}																					\
		}
	#else
		#define ASSERT(exp)	{}
	#endif
#endif

#ifndef assert
	#define assert	ASSERT
#endif

	#define ICE_COMPILE_TIME_ASSERT(exp)	extern char ICE_Dummy[ (exp) ? 1 : -1 ]
	#define CHECK_CONTAINER_ITEM(x)			ICE_COMPILE_TIME_ASSERT((sizeof(x)&3)==0);

#endif // ICEASSERT_H
