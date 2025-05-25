#ifndef ROCK_TYPES_H
#define ROCK_TYPES_H

#include <Core/RockAssert.h>

	// stdint uses "long long" for 64bit ints while we used "__int64" in the past. Not sure what's better.
	#define RockType64	__int64
	//#define RockType64	long long

	// We use these types to make the code shorter.

	typedef signed char			i8;		// int8_t
	typedef unsigned char		u8;		// uint8_t

	typedef signed short		i16;	// int16_t
	typedef unsigned short		u16;	// uint16_t

	typedef signed int			i32;	// int32_t
	typedef unsigned int		u32;	// uint32_t

	typedef signed RockType64	i64;	// int64_t
	typedef unsigned RockType64	u64;	// uint64_t

	typedef float				f32;
	typedef double				f64;

	typedef unsigned int		b32;	// BOOL / int-bool

	ROCK_COMPILE_TIME_ASSERT(sizeof(i8)==1);
	ROCK_COMPILE_TIME_ASSERT(sizeof(u8)==1);
	ROCK_COMPILE_TIME_ASSERT(sizeof(i16)==2);
	ROCK_COMPILE_TIME_ASSERT(sizeof(u16)==2);
	ROCK_COMPILE_TIME_ASSERT(sizeof(i32)==4);
	ROCK_COMPILE_TIME_ASSERT(sizeof(u32)==4);
	ROCK_COMPILE_TIME_ASSERT(sizeof(i64)==8);
	ROCK_COMPILE_TIME_ASSERT(sizeof(u64)==8);
	ROCK_COMPILE_TIME_ASSERT(sizeof(f32)==4);
	ROCK_COMPILE_TIME_ASSERT(sizeof(f64)==8);
	ROCK_COMPILE_TIME_ASSERT(sizeof(b32)==4);

namespace Rock
{
	struct _Zero	{ _Zero(){}	};
	static _Zero	InitZero;
}

#endif
