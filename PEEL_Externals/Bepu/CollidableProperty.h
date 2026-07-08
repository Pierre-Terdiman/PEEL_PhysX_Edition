#pragma once

#include "BepuPhysics.h"

namespace Bepu
{
	/// <summary>
	/// Convenience collection that stores extra properties about bodies and statics, indexed by the body or static handle.
	/// </summary>
	/// <typeparam name="T">Type of the data to store.</typeparam>
	/// <remarks>This is built for use cases relying on random access like the narrow phase. For maximum performance with sequential access, an index-aligned structure would be better.</remarks>
	template<typename T>
	struct CollidableProperty
	{
		//Bodies and statics each have 'handle spaces', like namespaces. A body and static can have the same integer valued handle.
		//So, we need to have two different buffers for data.
		SimulationHandle Simulation;
		BufferPoolHandle Pool;
		Buffer<T> bodyData;
		Buffer<T> staticData;

		CollidableProperty()
		{
			Simulation = SimulationHandle();
			Pool = BufferPoolHandle();
			bodyData = Buffer<T>();
			staticData = Buffer<T>();
		}

		/// <summary>
		/// Constructs a new collection to store handle-aligned body and static properties.
		/// </summary>
		/// <param name="simulation">Simulation to track.</param>
		/// <param name="pool">Pool from which to pull internal resources.</param>
		CollidableProperty(SimulationHandle simulation, BufferPoolHandle pool)
		{
			Simulation = simulation;
			Pool = pool;
			Buffer<BodyMemoryLocation> bodyHandleToLocationMapping;
			Bepu::GetBodyHandleToLocationMapping(simulation, &bodyHandleToLocationMapping);
			bodyData = Bepu::AllocateAtLeast(Pool, bodyHandleToLocationMapping.Length * sizeof(T));
			Buffer<int32_t> staticHandleToIndexMapping;
			Bepu::GetStaticHandleToLocationMapping(simulation, &staticHandleToIndexMapping);
			staticData = Bepu::AllocateAtLeast(Pool, staticHandleToIndexMapping.Length * sizeof(T));
		}

		T& operator[](BodyHandle bodyHandle)
		{
			assert(bodyHandle.Value >= 0 && bodyHandle.Value < bodyData.Length);
			return bodyData[bodyHandle];
		}

		T& operator[](StaticHandle staticHandle)
		{
			assert(staticHandle.Value >= 0 && staticHandle.Value < staticData.Length);
			return staticData[staticHandle];
		}

		T& operator[](CollidableReference collidable)
		{
			if (collidable.GetMobility() == CollidableMobility::Static)
				return this[collidable.GetStaticHandle()];
			return this[collidable.GetBodyHandle()];
		}

		/// <summary>
		/// Ensures there is space for a given body handle and returns a reference to the used memory.
		/// </summary>
		/// <param name="bodyHandle">Body handle to allocate for.</param>
		/// <returns>Reference to the data for the given body.</returns>
		T& Allocate(BodyHandle bodyHandle)
		{
			if (bodyHandle.Value >= bodyData.Length)
			{
				auto copyCountInBytes = sizeof(T) * bodyData.Length;
				auto targetCapacityInBytes = sizeof(T) * (bodyHandle.Value + 1);
				ByteBuffer byteBuffer = bodyData;
				Bepu::ResizeToAtLeast(Pool, &byteBuffer, targetCapacityInBytes, copyCountInBytes);
				bodyData = byteBuffer;
			}
			return bodyData[bodyHandle.Value];
		}

		/// <summary>
		/// Ensures there is space for a given static handle and returns a reference to the used memory.
		/// </summary>
		/// <param name="handle">Static handle to allocate for.</param>
		/// <returns>Reference to the data for the given static.</returns>
		T& Allocate(StaticHandle staticHandle)
		{
			if (staticHandle.Value >= staticData.Length)
			{
				auto copyCountInBytes = sizeof(T) * staticData.Length;
				auto targetCapacityInBytes = sizeof(T) * (staticHandle.Value + 1);
				ByteBuffer byteBuffer = staticData;
				Bepu::ResizeToAtLeast(Pool, &byteBuffer, targetCapacityInBytes, copyCountInBytes);
				staticData = byteBuffer;
			}
			return staticData[staticHandle.Value];
		}


		/// <summary>
		/// Ensures there is space for a given collidable reference and returns a reference to the used memory.
		/// </summary>
		/// <param name="handle">Collidable reference to allocate for.</param>
		/// <returns>Reference to the data for the given collidable.</returns>
		T& Allocate(CollidableReference collidableReference)
		{
			if (collidableReference.GetMobility() == CollidableMobility::Static)
			{
				return Allocate(collidableReference.GetStaticHandle());
			}
			return Allocate(collidableReference.GetBodyHandle());
		}

		/// <summary>
		/// Ensures that the internal structures have at least the given capacity for bodies.
		/// </summary>
		/// <param name="capacity">Capacity to ensure.</param>
		void EnsureBodyCapacity(int32_t capacity)
		{
			if (capacity > bodyData.Length)
			{
				ByteBuffer byteBuffer = bodyData;
				Bepu::ResizeToAtLeast(Pool, bodyData, capacity * sizeof(T), byteBuffer.Length);
				bodyData = byteBuffer;
			}
		}
		/// <summary>
		/// Ensures that the internal structures have at least the given capacity for statics.
		/// </summary>
		/// <param name="capacity">Capacity to ensure.</param>
		void EnsureStaticCapacity(int32_t capacity)
		{
			if (capacity > staticData.Length)
			{
				ByteBuffer byteBuffer = staticData;
				Bepu::ResizeToAtLeast(Pool, staticData, capacity * sizeof(T), byteBuffer.Length);
				staticData = byteBuffer;
			}
		}

		//TODO: We just left out the compaction bits, but they're not too hard!

		/// <summary>
		/// Returns all held resources.
		/// </summary>
		void Dispose()
		{
			Bepu::DeallocateById(Pool, bodyData.Id);
			Bepu::DeallocateById(Pool, staticData.Id);
		}
	};
}
