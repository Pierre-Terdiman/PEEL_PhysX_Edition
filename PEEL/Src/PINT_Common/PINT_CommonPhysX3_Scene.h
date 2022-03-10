///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_SCENE_H
#define PINT_COMMON_PHYSX3_SCENE_H

#include "..\Pint.h"

	class PhysX_SceneAPI : public Pint_Scene
	{
		public:
						PhysX_SceneAPI(Pint& pint);
		virtual			~PhysX_SceneAPI();

		virtual	bool	AddActors(udword nb_actors, const PintActorHandle* actors)	override;

		virtual	void	GetActors(Reporter& reporter)									const	override;
		virtual	void	Cull(udword nb_planes, const Plane* planes, Reporter& reporter)	const	override;
	};

#endif