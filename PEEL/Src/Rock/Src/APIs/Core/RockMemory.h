#ifndef ROCK_MEMORY_H
#define ROCK_MEMORY_H

#include <Core/RockCommon.h>

#undef ZeroMemory
#undef CopyMemory
#undef MoveMemory
#undef FillMemory

namespace Rock
{
	inline_	void ZeroMemory_(void* dest, size_t size)					{ memset(dest, 0, size);	}
	inline_	void FillMemory_(void* dest, size_t size, u8 val)			{ memset(dest, val, size);	}
	inline_ void CopyMemory_(void* dest, const void* src, size_t size)	{ memcpy(dest, src, size);	}
	inline_ void MoveMemory_(void* dest, const void* src, size_t size)	{ memmove(dest, src, size);	}
	inline_ void StoreDwords_(u32* dest, u32 nb, u32 value)
	{
		while(nb--)
			*dest++ = value;
	}
}

	#define ROCK_DELETE(x)			if(x) { delete x;		x = null; }
	#define ROCK_DELETE_ARRAY(x)	if(x) { delete []x;		x = null; }
	#define ROCK_RELEASE(x)			if(x) { x->Release();	x = null; }

	#define ROCK_CHECKALLOC(x)		if(!x) return false;

#endif
