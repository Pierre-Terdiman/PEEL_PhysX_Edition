///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef STREAMING_H
#define STREAMING_H

#include "PintDef.h"
#include "PsHashMap.h"

	namespace px = physx;
	namespace ps = physx::shdfnd;

	struct StreamRegion
	{
//		inline_	StreamRegion() : mX(INVALID_ID), mZ(INVALID_ID), mTimestamp(INVALID_ID)	{}
		inline_	StreamRegion() : mKey(0), mTimestamp(INVALID_ID), mUserData(null)	{}

		//sdword	mX, mZ;
		px::PxU64	mKey;
		udword		mTimestamp;
		AABB		mCellBounds;
		void*		mUserData;
	};

	typedef ps::HashMap<px::PxU64, StreamRegion>	StreamingCache;

	class PintRender;
	class Streamer;

	class RegionIterator
	{
		public:
						RegionIterator()	{}
		virtual			~RegionIterator()	{}

		virtual	void	ProcessRegion(const StreamRegion& region)	{}
	};

	class StreamInterface : public Allocateable
	{
		public:
						StreamInterface()	{}
		virtual			~StreamInterface()	{}

		virtual	void	PreUpdate(const AABB& streaming_bounds)	{}
		virtual	void	PostUpdate(udword timestamp)			{}

		virtual	void	AddRegion(StreamRegion& region)				= 0;
		virtual	void	UpdateRegion(const StreamRegion& region)	= 0;
		virtual	void	RemoveRegion(const StreamRegion& region)	= 0;

		virtual	void	RenderDebugRegion(PintRender& render, const Streamer& owner, const StreamRegion& region);
	};

	class Streamer : public Allocateable
	{
		public:
						Streamer(float world_extent=5.0f, udword nb_cells_per_side=4);
						~Streamer();

		void			Update(const AABB& bounds, StreamInterface& stream_interface);
		void			RenderDebug(PintRender& render, StreamInterface& stream_interface);

		void			Iterate(RegionIterator&);

		AABB			mCurrentBounds;
		AABB			mStreamingBounds;

		StreamingCache	mStreamingCache;
		udword			mTimestamp;

		const float		mWorldExtent;
		const udword	mNbCellsPerSide;

		bool			mDebugDrawStreamingBounds;
	};

#endif
