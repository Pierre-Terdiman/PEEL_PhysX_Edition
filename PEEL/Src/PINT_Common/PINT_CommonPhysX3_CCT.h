///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_CCT_H
#define PINT_COMMON_PHYSX3_CCT_H

#include "..\Pint.h"

	class PhysX_CCT_API : public Pint_Character
	{
		public:
										PhysX_CCT_API(Pint& pint);
		virtual							~PhysX_CCT_API();

		virtual	PintCharacterHandle		CreateCharacter(const PINT_CHARACTER_CREATE& create)	override;
		virtual	PintActorHandle			GetCharacterActor(PintCharacterHandle h)	override;
		virtual	udword					MoveCharacter(PintCharacterHandle h, const Point& disp)	override;

				PxControllerManager*	GetControllerManager();
				void					ReleaseControllerManager();

				PxControllerManager*	mControllerManager;
	};

#endif