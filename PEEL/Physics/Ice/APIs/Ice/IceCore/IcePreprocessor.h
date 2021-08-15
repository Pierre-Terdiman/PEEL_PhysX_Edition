///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains preprocessor stuff. This should be the first included header.
 *	\file		IcePreprocessor.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPREPROCESSOR_H
#define ICEPREPROCESSOR_H

	// July 31, 2013:
	// - removed pragma messages, spawns console when not using PCH
	// - removed old "for" define, see ICE_ANSI_FOR

	// Check platform
	#if defined( _WIN32 ) || defined( WIN32 )
//		#pragma message("----Compiling on Windows...")
		#define PLATFORM_WINDOWS
	#elif defined(_XBOX)
//		#pragma message("----Compiling on XBOX...")
	#else
//		#pragma message("----Compiling on unknown platform...")
	#endif

	// Check compiler
	#if defined(_MSC_VER)
//		#pragma message("----Compiling with VC++...")
		#define COMPILER_VISUAL_CPP

		#if _MSC_VER > 1300
//			#pragma message("----Compiling with VC7")
			#define COMPILER_VC7
		#else
//			#pragma message("----Compiling with VC6")
			#define COMPILER_VC6
		#endif

		#pragma warning( disable : 4100 ) // unreferenced formal parameter
		#pragma warning( disable : 4127 ) // conditional expression is constant

/*		#pragma warning( disable : 4062 ) // enumerate 'xxx' in switch of enum 'xxx' is not handled.
		#pragma warning( disable : 4201 ) // Nonstandard extension used : nameless struct/union.
		#pragma warning( disable : 4619 ) // #pragma warning : there is no warning number xxx.
		#pragma warning( disable : 4668 ) // xxx is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'.
		#pragma warning( disable : 4710 ) // function not inlined.
		#pragma warning( disable : 4820 ) // 'x' bytes padding added after member xxx.
*/
		#pragma warning( default : 4265 ) // 'class' : class has virtual functions, but destructor is not virtual.
		#pragma warning( default : 4242 ) // 'identifier' : conversion from 'type1' to 'type2', possible loss of data.
		#pragma warning( default : 4287 ) // 'operator' : unsigned/negative constant mismatch.
		#pragma warning( default : 4296 ) // 'operator' : expression is always false.
		#pragma warning( default : 4302 ) // 'conversion' : truncation from 'type 1' to 'type 2'.
		#pragma warning( default : 4529 ) // 'member_name' : forming a pointer-to-member requires explicit use of the address-of operator ('&') and a qualified name.
		#pragma warning( default : 4555 ) // expression has no effect; expected expression with side-effect.

		#pragma warning( disable : 4996 ) // BS deprecated warnings from VC8
		#pragma warning( disable : 4530 ) // disable warnings when including 3rd party code using exceptions

	#else
//		#pragma message("----Compiling with unknown compiler...")
	#endif

	// Check compiler options. If this file is included in user-apps, this
	// shouldn't be needed, so that they can use what they like best.
	#ifndef ICE_DONT_CHECK_COMPILER_OPTIONS
		#ifdef COMPILER_VISUAL_CPP
			#if defined(_CHAR_UNSIGNED)
			#endif

			#if defined(_CPPRTTI)
				#error Please disable RTTI...
			#endif

			#if defined(_CPPUNWIND)
				#error Please disable exceptions...
			#endif

			#if defined(_MT)
				// Multithreading
			#endif
		#endif
	#endif

	// Check debug mode
	#ifdef	DEBUG			// May be defined instead of _DEBUG. Let's fix it.
		#ifndef	_DEBUG
			#define _DEBUG
		#endif
	#endif

	#ifdef  _DEBUG
		// Here you may define items for debug builds
	#endif

	#ifndef THIS_FILE
		#define THIS_FILE			__FILE__
	#endif

	#ifndef ICE_NO_DLL
		#ifdef ICECORE_EXPORTS
			#define ICECORE_API		__declspec(dllexport)
		#else
			#define ICECORE_API		__declspec(dllimport)
		#endif
	#else
			#define ICECORE_API
	#endif

	#define FUNCTION				extern "C"

	// Cosmetic stuff [mainly useful with multiple inheritance]
	#define	override(base_class)	virtual

	// Our own inline keyword, so that:
	// - we can switch to __forceinline to check it's really better or not
	// - we can remove __forceinline if the compiler doesn't support it
//	#define inline_				__forceinline
//	#define inline_				inline

	// Contributed by Bruce Mitchener
	#if defined(COMPILER_VISUAL_CPP)
		#define inline_			__forceinline
//		#define inline_			inline
	#elif defined(__GNUC__) && __GNUC__ < 3
		#define inline_ inline
	#elif defined(__GNUC__)
			#define inline_ inline
// SJB - too aggressive for gcc-3.4.2
//			#define inline_ inline __attribute__ ((always_inline))
	#else
		#define inline_ inline
	#endif

	#define restrict_	__restrict

	// Down the hatch
	#pragma inline_depth( 255 )

	#ifdef COMPILER_VISUAL_CPP
		#pragma intrinsic(memcmp)
		#pragma intrinsic(memcpy)
		#pragma intrinsic(memset)
		#pragma intrinsic(strcat)
		#pragma intrinsic(strcmp)
		#pragma intrinsic(strcpy)
		#pragma intrinsic(strlen)
		#pragma intrinsic(abs)
		#pragma intrinsic(labs)
	#endif

	// ANSI compliance
	#ifdef ICE_ANSI_FOR
		#ifdef  _DEBUG
			// Remove painful warning in debug
			inline_ bool ReturnsFalse(){ return false; }
			#define for if(ReturnsFalse()){}	else for
		#else
			#define for if(0){}	else for
		#endif
	#endif

	// Don't override new/delete
	#define DEFAULT_NEWDELETE
	#define DONT_TRACK_MEMORY_LEAKS

	//! Macro used to give me a clue when it crashes in release and only the assembly is available
#ifndef _WIN64
	#define	INCLUDE_GUARDIANS
#endif
	#ifdef INCLUDE_GUARDIANS
		#define	GUARD(x)						\
			{									\
			static const char guard_text[] = x;	\
			_asm	push	eax					\
			_asm	nop							\
			_asm	nop							\
			_asm	nop							\
			_asm	nop							\
			_asm	lea		eax, guard_text		\
			_asm	nop							\
			_asm	nop							\
			_asm	nop							\
			_asm	nop							\
			_asm	pop		eax					\
			}
	#else
		#define	GUARD(x)
	#endif

	#define	ICE_WIN32_CALL(x)	IceCore::x

#endif // ICEPREPROCESSOR_H
