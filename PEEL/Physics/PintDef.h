///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_DEF_H
#define PINT_DEF_H

#include "Common.h"

	struct PintConvexHandleImpl;			typedef PintConvexHandleImpl*				PintConvexHandle;
	struct PintMeshHandleImpl;				typedef PintMeshHandleImpl*					PintMeshHandle;
	struct PintHeightfieldHandleImpl;		typedef PintHeightfieldHandleImpl*			PintHeightfieldHandle;
	struct PintActorHandleImpl;				typedef PintActorHandleImpl*				PintActorHandle;
	struct PintShapeHandleImpl;				typedef PintShapeHandleImpl*				PintShapeHandle;
//	struct PintObjectHandleImpl;			typedef PintObjectHandleImpl*				PintObjectHandle;
	struct PintAggregateHandleImpl;			typedef PintAggregateHandleImpl*			PintAggregateHandle;
	struct PintJointHandleImpl;				typedef PintJointHandleImpl*				PintJointHandle;
	struct PintArticHandleImpl;				typedef PintArticHandleImpl*				PintArticHandle;
	struct PintCharacterHandleImpl;			typedef PintCharacterHandleImpl*			PintCharacterHandle;
	struct PintVehicleHandleImpl;			typedef PintVehicleHandleImpl*				PintVehicleHandle;
	struct PintSQThreadContextImpl;			typedef PintSQThreadContextImpl*			PintSQThreadContext;
	struct PintContactModifyPairHandleImpl;	typedef PintContactModifyPairHandleImpl*	PintContactModifyPairHandle;

//	typedef void*	PintObjectHandle;
//	typedef PintActorHandle	PintObjectHandle;

	typedef	uword	PintCollisionGroup;	// Must be < 32
	typedef	udword	PintConvexIndex;
	typedef	udword	PintMeshIndex;
	typedef	udword	PintHeightfieldIndex;

#endif
