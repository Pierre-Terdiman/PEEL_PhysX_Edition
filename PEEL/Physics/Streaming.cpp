///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Streaming.h"
#include "Pint.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////

Streamer::Streamer(float world_extent, udword nb_cells_per_side) : mTimestamp(0), mWorldExtent(world_extent), mNbCellsPerSide(nb_cells_per_side), mDebugDrawStreamingBounds(false)
{
	mCurrentBounds.SetEmpty();
	mStreamingBounds.SetEmpty();
}

Streamer::~Streamer()
{
}

void StreamInterface::RenderDebugRegion(PintRender& render, const Streamer& owner, const StreamRegion& region)
{
	if(owner.mDebugDrawStreamingBounds)
		render.DrawWireframeAABB(1, &region.mCellBounds, Point(0.0f, 1.0f, 0.0f));
}

void Streamer::RenderDebug(PintRender& render, StreamInterface& stream_interface)
{
	if(mDebugDrawStreamingBounds)
	{
		render.DrawWireframeAABB(1, &mCurrentBounds, Point(1.0f, 0.0f, 0.0f));
		render.DrawWireframeAABB(1, &mStreamingBounds, Point(0.0f, 0.0f, 1.0f));
	}

	udword Nb = 0;

	for(StreamingCache::Iterator iter = mStreamingCache.getIterator(); !iter.done(); ++iter)
	{
		stream_interface.RenderDebugRegion(render, *this, iter->second);
		Nb++;
	}

//	printf("Rendered %d regions\n", Nb);
}

void Streamer::Iterate(RegionIterator& iterator)
{
	for(StreamingCache::Iterator iter = mStreamingCache.getIterator(); !iter.done(); ++iter)
		iterator.ProcessRegion(iter->second);
}

void Streamer::Update(const AABB& bounds, StreamInterface& stream_interface)
{
	SPY_ZONE("Streamer::Update")

	mCurrentBounds = bounds;

	Point Center;
	bounds.GetCenter(Center);

	const float WorldExtent = mWorldExtent;
	mStreamingBounds.SetCenterExtents(Center, Point(WorldExtent, 1.0f, WorldExtent));

	stream_interface.PreUpdate(mStreamingBounds);

	const float WorldSize = WorldExtent*2.0f;
	const udword NbCellsPerSide = mNbCellsPerSide;
	const float CellSize = WorldSize/float(NbCellsPerSide);

	const float CellSizeX = CellSize;
	const float CellSizeZ = CellSize;

	const sdword x0 = sdword(floorf(mStreamingBounds.mMin.x/CellSizeX));
	const sdword z0 = sdword(floorf(mStreamingBounds.mMin.z/CellSizeZ));
	const sdword x1 = sdword(ceilf(mStreamingBounds.mMax.x/CellSizeX));
	const sdword z1 = sdword(ceilf(mStreamingBounds.mMax.z/CellSizeZ));

	udword Nb=0;

	// Generally speaking when streaming objects in and out of the game world, we want to first remove
	// old objects then add new objects (in this order) to give the system a chance to recycle removed
	// entries and use less resources overall. That's why we split the loop to add objects in two parts.
	// The first part below finds currently touched regions and updates their timestamp.
	for(sdword j=z0;j<z1;j++)
	{
		for(sdword i=x0;i<x1;i++)
		{
			const PxU64 Key = (PxU64(i)<<32)|PxU64(PxU32(j));

			StreamRegion& Region = mStreamingCache[Key];
			if(Region.mTimestamp!=INVALID_ID)
			{
				//ASSERT(Region.mX==i);
				//ASSERT(Region.mZ==j);
				ASSERT(Region.mKey==Key);
				Region.mTimestamp = mTimestamp;
				stream_interface.UpdateRegion(Region);
			}

			Nb++;
		}
	}

/*	for(sdword j=z0;j<z1;j++)
	{
		for(sdword i=x0;i<x1;i++)
		{
			const PxU64 Key = (PxU64(i)<<32)|PxU64(PxU32(j));

			StreamRegion& Region = mStreamingCache[Key];
			if(Region.mTimestamp==INVALID_ID)
			{
				// New entry
				//Region.mX = i;
				//Region.mZ = j;
				Region.mKey = Key;
				Region.mTimestamp = mTimestamp;
				Region.mCellBounds.SetMinMax(Point(float(i)*CellSizeX, 0.0f, float(j)*CellSizeZ), Point(float(i+1)*CellSizeX, 0.0f, float(j+1)*CellSizeZ));

				Region.mUserData = stream_interface.AddRegion(Region);
			}
			else
			{
				//ASSERT(Region.mX==i);
				//ASSERT(Region.mZ==j);
				ASSERT(Region.mKey==Key);
				Region.mTimestamp = mTimestamp;
			}

			Nb++;
		}
	}*/

//	printf("Updated %d regions\n", Nb);

	// ### super clumsy
	// This loop checks all regions in the system and removes the ones that are neither new
	// (mTimestamp==INVALID_ID) nor persistent (mTimestamp==current timestamp).
	{
		ps::Array<PxU64> ToRemove;	// Delayed removal to avoid touching the hashmap while we're iterating it
		for(StreamingCache::Iterator iter = mStreamingCache.getIterator(); !iter.done(); ++iter)
		{
			if(iter->second.mTimestamp!=mTimestamp && iter->second.mTimestamp!=INVALID_ID)
			//if(iter->second.mTimestamp!=mTimestamp)
			{
				stream_interface.RemoveRegion(iter->second);

				ToRemove.pushBack(iter->second.mKey);
			}
		}

		const PxU32 nbToGo = ToRemove.size();
		for(PxU32 i=0;i<nbToGo;i++)
		{
			bool b = mStreamingCache.erase(ToRemove[i]);
			ASSERT(b);
		}
	}

	// Finally we do our initial loop again looking for new regions (mTimestamp==INVALID_ID) and actually add them.
	for(sdword j=z0;j<z1;j++)
	{
		for(sdword i=x0;i<x1;i++)
		{
			const PxU64 Key = (PxU64(i)<<32)|PxU64(PxU32(j));

			StreamRegion& Region = mStreamingCache[Key];
			if(Region.mTimestamp==INVALID_ID)
			{
				// New entry
				//Region.mX = i;
				//Region.mZ = j;
				Region.mKey = Key;
				Region.mTimestamp = mTimestamp;
				Region.mCellBounds.SetMinMax(Point(float(i)*CellSizeX, 0.0f, float(j)*CellSizeZ), Point(float(i+1)*CellSizeX, 0.0f, float(j+1)*CellSizeZ));

				stream_interface.AddRegion(Region);
			}

			Nb++;
		}
	}

	stream_interface.PostUpdate(mTimestamp);
	mTimestamp++;
}

///////////////////////////////////////////////////////////////////////////////


