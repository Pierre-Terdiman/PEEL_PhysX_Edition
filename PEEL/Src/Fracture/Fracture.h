///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef FRACTURE_H
#define FRACTURE_H

#include "..\Common.h"

#include "PintDef.h"

	class Pint;
#ifdef PEEL_USE_ICE_FRACTURE
	#include <meshmerizer\IceFractureConvex.h>
	typedef FractureConvex Convex;
#endif

	class FractureManager
	{
		public:
							FractureManager();
							~FractureManager();

			void			RegisterConvex(Pint& pint, FractureConvex* cn, float mass, const Point& pos, const Quat& rot, const Point& lin_vel, const Point& ang_vel, bool kine);
			void			Release();
			FractureConvex*	FindConvex(PintActorHandle handle, udword* index)	const;
			void			RemoveLink(udword index);

		private:
			struct Link
			{
				FractureConvex*	mConvex;
				PintActorHandle	mHandle;
			};
			Container		mLinks;	// TODO: make this a hashmap later
	};

#endif