#pragma once

#include <stdint.h>

namespace Bepu
{
	/// <summary>
	/// Represents an index with an associated type packed into a single integer.
	/// </summary>
	struct TypedIndex
	{
		/// <summary>
		/// Bit packed representation of the typed index.
		/// </summary>
		uint32_t Packed;

		/// <summary>
		/// Gets the type index of the object.
		/// </summary>
		int32_t GetType() { return (int32_t)(Packed & 0x7F000000) >> 24; }

		/// <summary>
		/// Gets the index of the object.
		/// </summary>
		int32_t GetIndex() { return (int32_t)(Packed & 0x00FFFFFF); }

		/// <summary>
		/// Gets whether this index actually refers to anything. The Type and Index should only be used if this is true.
		/// </summary>
		bool Exists() { return (Packed & (1 << 31)) > 0; }
	};

	/// <summary>
	/// Points to an instance in an instance directory.
	/// </summary>
	struct InstanceHandle
	{
		int32_t RawValue;

		int32_t GetIndex() { return RawValue & 0x00FFFFFF; }
		int32_t GetVersion() { return (RawValue >> 24) & 0xF; }
		int32_t GetTypeIndex() { return (RawValue >> 28) & 0x7; }

		bool IsNull() { return RawValue == 0; }
	};

	struct BodyHandle
	{
		int32_t Value;
	};
	struct StaticHandle
	{
		int32_t Value;
	};
	struct ConstraintHandle
	{
		int32_t Value;
	};

	typedef InstanceHandle SimulationHandle;
	typedef InstanceHandle BufferPoolHandle;
	typedef InstanceHandle ThreadDispatcherHandle;
}