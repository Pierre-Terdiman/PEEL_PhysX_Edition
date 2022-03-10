///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TERRAIN_MANAGER_H
#define TERRAIN_MANAGER_H

#include "PintDef.h"
#include "PsHashMap.h"
namespace ps = physx::shdfnd;

	struct ActorShape
	{
		inline_	ActorShape(PintActorHandle actor, PintShapeHandle shape) : mActor(actor), mShape(shape)	{}

		PintActorHandle	mActor;
		PintShapeHandle	mShape;
	};

	inline_ bool operator==(const ActorShape& h0, const ActorShape& h1)
	{
		if(h0.mActor!=h1.mActor)
			return false;
		if(h0.mShape!=h1.mShape)
			return false;
		return true;
	}

	inline_ udword hash(const ActorShape& key)
	{
		// ### not great but ...
		const udword id0 = udword(size_t(key.mActor));
		const udword id1 = udword(size_t(key.mShape));
		const uint64_t mix = (uint64_t(id0)<<32)|uint64_t(id1);
		return ps::hash(mix);
	}

	class Pint;
	struct PintVehicleData;
	class TerrainRegionManager;
	class DeformManager : public Allocateable
	{
		public:
					DeformManager(Pint& owner);
					~DeformManager();

		bool		Update(const PintVehicleData& vehicle, TerrainRegionManager& region_manager, float wheel_radius, float dt);

		private:

		Pint&		mOwner;

		ps::HashMap<ActorShape, void*>	mObjects;

		public:
		Vertices	mTouchedTris;
		Container	mVertexIndices;
	};

#endif
