///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX3 plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "PINT_CommonPhysX3_Scene.h"
#include "PINT_CommonPhysX3.h"

PhysX_SceneAPI::PhysX_SceneAPI(Pint& pint) : Pint_Scene(pint)
{
}

PhysX_SceneAPI::~PhysX_SceneAPI()
{
}

bool PhysX_SceneAPI::AddActors(udword nb_actors, const PintActorHandle* actors)
{
	SharedPhysX& physx = static_cast<SharedPhysX&>(mPint);

	PxScene* Scene = physx.GetScene();
	if(!Scene)
		return false;

	{
		PxActor*const* pxActors = reinterpret_cast<PxActor*const*>(actors);
		Scene->addActors(pxActors, nb_actors);
	}

	{
		PxRigidActor*const* pxRigidActors = reinterpret_cast<PxRigidActor*const*>(actors);
		for(udword i=0;i<nb_actors;i++)
		{
			physx.AddActorToManager(pxRigidActors[i]);

			if(pxRigidActors[i]->getConcreteType()!=PxConcreteType::eRIGID_STATIC)
			{
				static_cast<PxRigidDynamic*>(pxRigidActors[i])->wakeUp();
			}

		}
	}

	return true;
}


void PhysX_SceneAPI::GetActors(Reporter& reporter) const
{
	SharedPhysX& physx = static_cast<SharedPhysX&>(mPint);

	if(!physx.GetScene())
		return;

	ActorManager& AM = physx.GetActorManager();

	for(udword pass=0;pass<2;pass++)
	{
		udword NbActors = pass ? AM.GetNbStaticActors() : AM.GetNbDynamicActors();
		const ActorData* Actors = pass ? AM.GetStaticActors() : AM.GetDynamicActors();

		while(NbActors--)
		{
			const ActorData& CurrentActor = *Actors++;
			if(!reporter.ReportObject(PintActorHandle(CurrentActor.mActor)))
				return;
		}
	}
}

void PhysX_SceneAPI::Cull(udword nb_planes, const Plane* planes, Reporter& reporter)	const
{
	SharedPhysX& physx = static_cast<SharedPhysX&>(mPint);

	if(!physx.GetScene() || !nb_planes || !planes)
		return;

	ActorManager& AM = physx.GetActorManager();

	const udword Mask = (1<<nb_planes)-1;

	for(udword pass=0;pass<2;pass++)
	{
		udword NbActors = pass ? AM.GetNbStaticActors() : AM.GetNbDynamicActors();
		const ActorData* Actors = pass ? AM.GetStaticActors() : AM.GetDynamicActors();

		while(NbActors--)
		{
			const ActorData& CurrentActor = *Actors++;
			PxRigidActor* rigidActor = CurrentActor.mActor;

			{
				// TODO: use fixed bounding sphere, don't recompute bounds
				const PxBounds3 ActorBounds = rigidActor->getWorldBounds();
				udword OutClipMask;
				if(!PlanesAABBOverlap((const AABB&)ActorBounds, planes, OutClipMask, Mask))
					continue;
			}

			if(!reporter.ReportObject(PintActorHandle(rigidActor)))
				return;
		}
	}
}
