///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolTransform.h"
#include "Pint.h"
#include "PEEL.h"

// TODO: the switch between paused/not paused can create issues
static bool CanUseKinematicMoves()
{
	// Kinematic moves require sim updates to be taken into account, i.e. we cannot move kinematic objects when paused.
	return !IsPaused();
}

Transformer::Transformer()
{
	Reset();
}

Transformer::~Transformer()
{
}

void Transformer::Reset()
{
	mIsKine	= false;
	mActive = false;
}

void Transformer::Start(Pint& pint, PintActorHandle h)
{
	const bool kinematic_moves = CanUseKinematicMoves();

	const bool IsKinematic = pint.IsKinematic(h);
	mIsKine = IsKinematic;

	if(kinematic_moves && !IsKinematic)
	{
		//printf("EnableKinematic true\n");
		pint.EnableKinematic(h, true);
	}

	mActive = true;
}

void Transformer::Stop(Pint& pint, PintActorHandle h)
{
	if(!mIsKine)
	{
//		printf("EnableKinematic false\n");
		pint.EnableKinematic(h, false);

		Pint_Actor* API = pint.GetActorAPI();
		if(API)
			API->WakeUp(h);
	}

	mActive = false;
}

void Transformer::SetPose(Pint& pint, PintActorHandle h, const PR& pose)
{
	const bool kinematic_moves = CanUseKinematicMoves();

	if(!mActive || !kinematic_moves || !pint.SetKinematicPose(h, pose))
	{
		pint.SetWorldTransform(h, pose);

		// Kill linear velocity to avoid gravity accumulating and making a mess when we release the object
		pint.SetLinearVelocity(h, Point(0.0f, 0.0f, 0.0f));
	}
}
