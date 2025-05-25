#ifndef ROCK_STRING_H
#define ROCK_STRING_H

#include <Core/RockCommon.h>
#include <Core/RockAllocator.h>

namespace Rock
{
	class ROCK_API String : public UserAllocated
	{
		public:
					String(const char* string=null);
					String(const String& string);
					~String();

		void		Reset();
		bool		Set(const char* string);

		//! Cast operator for const char* = String
		inline_		operator const char*()	const	{ return (const char*)(mText);	}

		private:
		char*		mText;
	};
}

#endif
