///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_SETUP_H
#define PINT_COMMON_PHYSX3_SETUP_H

	void	SetupSceneDesc(	PxSceneDesc& sceneDesc, const PINT_WORLD_CREATE& desc, const EditableParams& params,
							//PxDefaultCpuDispatcher* cpu_dispatcher, PxSimulationEventCallback* secb, PxContactModifyCallback* cmcb);
							PxCpuDispatcher* cpu_dispatcher, PxSimulationEventCallback* secb, PxContactModifyCallback* cmcb);
	void	SetupBroadphase(const PINT_WORLD_CREATE& desc, const EditableParams& mParams, PxScene* mScene);

#endif