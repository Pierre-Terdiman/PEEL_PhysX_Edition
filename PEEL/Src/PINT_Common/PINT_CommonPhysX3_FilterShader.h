///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_FILTER_SHADER_H
#define PINT_COMMON_PHYSX3_FILTER_SHADER_H

	#define PEEL_GROUP_SIZE	32

	PxFilterFlags PhysX3_SimulationFilterShader(
		PxFilterObjectAttributes attributes0, PxFilterData filterData0, 
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize);

	void PhysX3_SetGroupCollisionFlag(const PxU16 group1, const PxU16 group2, const bool enable);
	void PhysX3_SetGroup(PxShape& shape, PxU16 collision_group);
	void PhysX3_SetFilterCallback(bool enabled);
	void PhysX3_InitFilterShader();

	extern PxU32 gFilterShaderExtraPairFlags;

#endif