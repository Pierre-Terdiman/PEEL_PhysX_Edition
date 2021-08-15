///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_VISIBILITY_MANAGER_H
#define PINT_VISIBILITY_MANAGER_H

#include "PintDef.h"
#include "PsHashSet.h"
namespace ps = physx::shdfnd;

	class Pint;
	class VisibilityManager : public Allocateable
	{
		public:
												VisibilityManager();
												~VisibilityManager();

				void							Init(Pint* engine);
				void							Reset();

		inline_	Pint*							GetOwner()	{ return mEngine;	}

				void							SetRenderable(PintActorHandle handle, bool visible);
				// warning: this one for native handles. Called from render loop so we don't want to
				// do handle translations all the time.
				bool							IsRenderable(PintActorHandle handle)	const;
				// warning: this one for regular handles, translates handle inside, slower
				bool							IsRenderable2(PintActorHandle handle)	const;

		protected:
				Pint*							mEngine;
				ps::HashSet<PintActorHandle>	mInvisibles;	// Native handles in here
				bool							mNeedsTranslation;
	};

#endif