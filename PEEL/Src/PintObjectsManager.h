///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_OBJECTS_MANAGER_H
#define PINT_OBJECTS_MANAGER_H

#include "Pint.h"

	void			DeletePintObjectShapes(PINT_OBJECT_CREATE& desc);

	PintActorHandle CreatePintObject(Pint& pint, const PINT_OBJECT_CREATE& desc);
	void			ReleasePintObject(Pint& pint, PintActorHandle removed_object, bool release_from_selection);

	bool			ReleasePintJoints(Pint& pint, PintActorHandle actor);

	bool			IsDefaultEnv(Pint& pint, PintActorHandle h);

#endif