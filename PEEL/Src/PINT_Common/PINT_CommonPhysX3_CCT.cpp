///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX3 plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "PINT_CommonPhysX3_CCT.h"
#include "PINT_CommonPhysX3.h"

PhysX_CCT_API::PhysX_CCT_API(Pint& pint) : Pint_Character(pint), mControllerManager(null)
{
}

PhysX_CCT_API::~PhysX_CCT_API()
{
}

PxControllerManager* PhysX_CCT_API::GetControllerManager()
{
	SharedPhysX& physx = static_cast<SharedPhysX&>(mPint);

	PxScene* scene = physx.GetScene();
	ASSERT(scene);

	PxControllerManager* CM = PxCreateControllerManager(*scene, false);
	mControllerManager = CM;
	return CM;
}

void PhysX_CCT_API::ReleaseControllerManager()
{
	SAFE_RELEASE(mControllerManager);
}

PintCharacterHandle PhysX_CCT_API::CreateCharacter(const PINT_CHARACTER_CREATE& create)
{
	SharedPhysX& physx = static_cast<SharedPhysX&>(mPint);

	PxCapsuleControllerDesc capsuleCCTDesc;
	capsuleCCTDesc.height		= create.mCapsule.mHalfHeight*2.0f;
	capsuleCCTDesc.radius		= create.mCapsule.mRadius;
	capsuleCCTDesc.position		= PxExtendedVec3(create.mPosition.x, create.mPosition.y, create.mPosition.z);
	capsuleCCTDesc.scaleCoeff	= 0.8f;
	capsuleCCTDesc.scaleCoeff	= 1.0f;
	capsuleCCTDesc.stepOffset	= 0.1f;
	capsuleCCTDesc.slopeLimit	= 0.0f;
	capsuleCCTDesc.material		= physx.GetDefaultMaterial();

	PxControllerManager* CM = GetControllerManager();
	PxController* CCT = CM->createController(capsuleCCTDesc);
	if(CCT)
	{
		PxRigidDynamic* dyna = CCT->getActor();
		if(dyna)
		{
			PxShape* shape = null;
			dyna->getShapes(&shape, 1);
			if(shape)
				shape->userData = create.mCapsule.mRenderer;
			physx.AddActorToManager(dyna);
		}
	}
	return PintCharacterHandle(CCT);
}

PintActorHandle PhysX_CCT_API::GetCharacterActor(PintCharacterHandle h)
{
	PxController* CCT = reinterpret_cast<PxController*>(h);
	if(!CCT)
		return null;
	return PintActorHandle(CCT->getActor());
}

udword PhysX_CCT_API::MoveCharacter(PintCharacterHandle h, const Point& disp)
{
	PxController* CCT = reinterpret_cast<PxController*>(h);
	if(!CCT)
		return 0;

	const PxF32 minDist = 0.0f;
	const PxF32 elapsedTime = 0.0f;

	const PxControllerFilters filters;

	return CCT->move(ToPxVec3(disp), minDist, elapsedTime, filters, null);
}

