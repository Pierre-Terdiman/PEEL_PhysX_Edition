#pragma once

#include <stdint.h>

namespace Bepu
{
	struct ByteBuffer
	{
		/// <summary>
		/// Pointer to the beginning of the memory backing this buffer.
		/// </summary>
		uint8_t* Memory; //going to just assume 64 bit here.

		/// <summary>
		/// Length of the buffer in bytes.
		/// </summary>
		int32_t Length;

		/// <summary>
		/// Implementation specific identifier of the raw buffer set by its source. If taken from a BufferPool, Id includes the index in the power pool from which it was taken.
		/// </summary>
		int32_t Id;
	};

	/// <summary>
	/// Span over an unmanaged memory region.
	/// </summary>
	/// <typeparam name="T">Type of the memory exposed by the span.</typeparam>
	template<typename T>
	struct Buffer
	{
		/// <summary>
		/// Pointer to the beginning of the memory backing this buffer.
		/// </summary>
		T* Memory; //going to just assume 64 bit here.

		/// <summary>
		/// Length of the buffer in typed elements.
		/// </summary>
		int32_t Length;

		/// <summary>
		/// Implementation specific identifier of the raw buffer set by its source. If taken from a BufferPool, Id includes the index in the power pool from which it was taken.
		/// </summary>
		int32_t Id;

		T& operator[](int32_t index)
		{
			assert(index >= 0 && index < Length);
			return Memory[index];
		}

		operator ByteBuffer()
		{
			ByteBuffer buffer;
			buffer.Memory = (uint8_t*)Memory;
			buffer.Length = Length * sizeof(T);
			buffer.Id = Id;
			return buffer;
		}
		Buffer(ByteBuffer buffer)
		{
			Memory = (T*)buffer.Memory;
			Length = buffer.Length / sizeof(T);
			Id = buffer.Id;
		}
		Buffer(T* memory, int32_t length, int32_t id = 0)
		{
			Memory = memory;
			Length = length;
			Id = id;
		}
		Buffer()
		{
			Memory = nullptr;
			Length = 0;
			Id = 0;
		}
	};


	template<typename T>
	struct QuickList
	{
		/// <summary>
		/// Backing memory containing the elements of the list.
		/// Indices from 0 to Count-1 hold actual data. All other data is undefined.
		/// </summary>
		Buffer<T> Span;

		/// <summary>
		/// Number of elements in the list.
		/// </summary>
		int32_t Count;

		T& operator[](int32_t index)
		{
			assert(index >= 0 && index < Count);
			return Span[index];
		}
	};
}