///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef ZB2_IMPORT_H
#define ZB2_IMPORT_H

#include "PintDef.h"

	class PINT_WORLD_CREATE;
	class ZCB2Factory;
	class ZB2CustomizeCallback;
	class Pint;
	struct PintCaps;

	struct ZB2CreatedObjects
	{
		ZB2CreatedObjects() : mMeshes(null), mActors(null), mJoints(null), mNbMeshObjects(0), mNbActors(0), mNbJoints(0)
		{
		}

		PintMeshHandle*		mMeshes;
		PintActorHandle*	mActors;
		PintJointHandle*	mJoints;
		udword				mNbMeshObjects;
		udword				mNbActors;
		udword				mNbJoints;
	};

	class ZB2CreationCallback
	{
		public:
						ZB2CreationCallback()	{}
		virtual			~ZB2CreationCallback()	{}

		virtual	void	NotifyCreatedObjects(Pint& pint, const ZB2CreatedObjects&, ZCB2Factory*)	= 0;
	};

	bool ImportZB2File(PINT_WORLD_CREATE& desc, const char* filename, ZCB2Factory* factory, ZB2CustomizeCallback* callback=null);
	bool CreateZB2Scene(Pint& pint, const PintCaps& caps, ZCB2Factory* factory, ZB2CreationCallback* callback=null);

#endif
