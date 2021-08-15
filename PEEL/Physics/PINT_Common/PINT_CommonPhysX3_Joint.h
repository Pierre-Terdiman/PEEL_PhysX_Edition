///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_JOINT_H
#define PINT_COMMON_PHYSX3_JOINT_H

#include "..\Pint.h"

	class PhysX_JointAPI : public Pint_Joint
	{
		public:
							PhysX_JointAPI(Pint& pint);
		virtual				~PhysX_JointAPI();

		virtual	const char*	GetName(PintJointHandle handle)														const;
		virtual	bool		SetName(PintJointHandle handle, const char* name);
		virtual	PintJoint	GetType(PintJointHandle handle)														const;
		virtual	bool		GetActors(PintJointHandle handle, PintActorHandle& actor0, PintActorHandle& actor1)	const;
		virtual	bool		GetFrames(PintJointHandle handle, PR& frame0, PR& frame1)							const;
	};

#endif