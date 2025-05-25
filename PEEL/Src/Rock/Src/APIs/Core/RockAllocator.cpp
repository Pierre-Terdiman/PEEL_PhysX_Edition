#include <Core/RockAllocator.h>

using namespace Rock;

namespace
{
	class BasicAllocator : public Allocator
	{
		public:
						BasicAllocator()	{}
		virtual			~BasicAllocator()	{}

		virtual void*	malloc(size_t size, MemoryType /*type*/)	override
		{
			return _aligned_malloc(size, 16);
		}

		virtual void*	mallocDebug(size_t size, const char* /*filename*/, u32 /*line*/, const char* /*class_name*/, MemoryType /*type*/, bool /*from_new*/)	override
		{
			return _aligned_malloc(size, 16);
		}

		virtual void	free(void* memory, bool /*from_new*/)	override
		{
			_aligned_free(memory);
		}
	};
}

static BasicAllocator gBasicAllocator;
static Allocator* gCurrentAllocator = null;

void SetupAllocator(Allocator* allocator)
{
	gCurrentAllocator = allocator ? allocator : &gBasicAllocator;
}

Allocator& Rock::GetAllocator()
{
	ROCK_ASSERT(gCurrentAllocator);
	return *gCurrentAllocator;
}

