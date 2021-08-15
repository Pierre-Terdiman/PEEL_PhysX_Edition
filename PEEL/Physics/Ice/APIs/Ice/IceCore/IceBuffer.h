///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains misc buffer related code.
 *	\file		IceBuffer.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEBUFFER_H
#define ICEBUFFER_H

	// Forward declarations
	class String;
//	class Container;

	enum CompressionCode
	{
		PACK_NONE,
		PACK_ZLIB,
		PACK_BZIP2,

		PACK_UNKNOWN	= 0x7fffffff
	};

	FUNCTION ICECORE_API bool			Delta(void* buffer, udword nb_items, udword item_size);
	FUNCTION ICECORE_API bool			UnDelta(void* buffer, udword nb_items, udword item_size);

	FUNCTION ICECORE_API bool			DisruptBuffer(void* buffer, udword nb_entries, udword item_size, udword stride);
	FUNCTION ICECORE_API bool			SaveAsSource(const char* filename, const char* label, const void* buffer, udword length, udword original_size, CompressionCode code);

	FUNCTION ICECORE_API udword			FindRank(udword* sorted_list, udword list_size, udword value);
	FUNCTION ICECORE_API udword			SortAndReduce(udword& nb, udword* list, udword* dest=null, Container* cnt=null);

	FUNCTION ICECORE_API bool			FindLineStrip(Container& line_strip, const Container& line_segments);
	FUNCTION ICECORE_API bool			InverseBuffer(udword nb_entries, udword* entries);

	FUNCTION ICECORE_API const void*	GetChunk(const char* chunk, const char* buffer, udword buffer_size);

	class ICECORE_API BufferReadStream : public ReadStream
	{
		public:
												BufferReadStream(const void* buffer, udword size);
		virtual									~BufferReadStream();

		override(ReadStream)	bool			Seek(udword offset)						const;

		// Loading API
		override(ReadStream)	ubyte			ReadByte()								const;
		override(ReadStream)	uword			ReadWord()								const;
		override(ReadStream)	udword			ReadDword()								const;
		override(ReadStream)	float			ReadFloat()								const;
		override(ReadStream)	double			ReadDouble()							const;
		override(ReadStream)	bool			ReadBuffer(void* buffer, udword size)	const;

								const ubyte*	ReadString()							const;
								const void*		ReadBuffer(udword size)					const;
								const void*		GetChunk(const char* chunk)				const;
		inline_					const void*		GetBaseAddress()						const	{ return (const void*)mBuffer;			}
		inline_					const void*		GetAddress()							const	{ return (const void*)mCurrentAddress;	}
		inline_					udword			GetSize()								const	{ return mSize;							}
		inline_					bool			Reset(udword offset)					const	{ return Seek(offset);					}
		private:
								const ubyte*	mBuffer;
		mutable					const ubyte*	mCurrentAddress;
								udword			mSize;
	};

#endif // ICEBUFFER_H
