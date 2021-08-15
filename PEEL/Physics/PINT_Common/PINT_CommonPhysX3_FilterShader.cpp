///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX3 plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "PINT_CommonPhysX3_FilterShader.h"

using namespace physx;

static bool gFilterCallbackEnabled = false;
static bool gCollisionTable[PEEL_GROUP_SIZE][PEEL_GROUP_SIZE];

PxU32 gFilterShaderExtraPairFlags = 0;

void PhysX3_SetFilterCallback(bool enabled)
{
	gFilterCallbackEnabled = enabled;
}

PxFilterFlags PhysX3_SimulationFilterShader(
	PxFilterObjectAttributes attributes0, PxFilterData filterData0, 
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	PX_UNUSED(constantBlock);
	PX_UNUSED(constantBlockSize);

	PxFilterFlags defaultFlags;
	if(gFilterCallbackEnabled)
		defaultFlags |= PxFilterFlag::eCALLBACK;

	// let triggers through
	if(PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return defaultFlags;
	}

	if(!gCollisionTable[filterData0.word0][filterData1.word0])
		return PxFilterFlag::eKILL;

	pairFlags = PxPairFlag::eCONTACT_DEFAULT|PxPairFlag::Enum(gFilterShaderExtraPairFlags);

	return defaultFlags;
}

void PhysX3_SetGroupCollisionFlag(const PxU16 group1, const PxU16 group2, const bool enable)
{
	ASSERT(group1 < PEEL_GROUP_SIZE);
	ASSERT(group2 < PEEL_GROUP_SIZE);
	gCollisionTable[group1][group2] = enable;
	gCollisionTable[group2][group1] = enable;
}

void PhysX3_SetGroup(PxShape& shape, PxU16 collision_group)
{
	// retrieve current group mask
	PxFilterData fd = shape.getSimulationFilterData();
	fd.word0 = collision_group;
	// set new filter data
	shape.setSimulationFilterData(fd);
}

void PhysX3_InitFilterShader()
{
	gFilterCallbackEnabled = false;
	for(PxU16 j=0;j<PEEL_GROUP_SIZE;j++)
		for(PxU16 i=0;i<PEEL_GROUP_SIZE;i++)
			PhysX3_SetGroupCollisionFlag(i, j, true);
}
