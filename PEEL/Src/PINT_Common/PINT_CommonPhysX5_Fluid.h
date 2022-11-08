///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX5_FLUID_H
#define PINT_COMMON_PHYSX5_FLUID_H

//#define TEST_FLUIDS
#ifdef TEST_FLUIDS
	struct EditableParams;
	class PintRender;

	PxPBDParticleSystem* initParticles(PxPhysics& physics, PxScene& scene, const EditableParams& params, const PxU32 numX, const PxU32 numY, const PxU32 numZ, const PxVec3& position = PxVec3(0, 0, 0), const PxReal particleSpacing = 0.2f, const PxU32 maxDiffuseParticles = 100000);

	void	allocParticleBuffers(PxPhysics& physics, PxScene& scene, PxPBDParticleSystem* particleSystem);
	void	clearupParticleBuffers();

	void	onBeforeRenderParticles(PxPBDParticleSystem* particleSystem);
	void	renderParticles(PintRender& renderer, PxPBDParticleSystem* particleSystem);
#endif

#endif
