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

	struct PINT_OBJECT_CREATE;
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

		struct Actor
		{
			PintActorHandle				mHandle;
			const PINT_OBJECT_CREATE*	mDesc;
		};

		PintMeshHandle*			mMeshes;
		Actor*					mActors;
		PintJointHandle*		mJoints;
		udword					mNbMeshObjects;
		udword					mNbActors;
		udword					mNbJoints;
	};

	class ZB2CreationCallback
	{
		public:
						ZB2CreationCallback()	{}
		virtual			~ZB2CreationCallback()	{}

		// Custom actor creation. Should return true to let the system know it's been taken care of.
		// Write new handle out in "handle". Cannot use null handle to replace the returned bool, since a null
		// handle already means actor creation failed.
		virtual	bool	CreateActor(PintActorHandle& handle, Pint& pint, const PINT_OBJECT_CREATE& desc)	{ return false;	}

		// Post object creation user notification
		virtual	void	NotifyCreatedObjects(Pint& pint, const ZB2CreatedObjects&, ZCB2Factory*)			{}
	};

	bool ImportZB2File(PINT_WORLD_CREATE& desc, const char* filename, ZCB2Factory* factory, ZB2CustomizeCallback* callback=null);
	bool CreateZB2Scene(Pint& pint, const PintCaps& caps, ZCB2Factory* factory, ZB2CreationCallback* callback=null);

#endif
