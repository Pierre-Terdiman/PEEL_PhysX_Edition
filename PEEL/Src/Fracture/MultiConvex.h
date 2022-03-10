///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef MULTI_CONVEX
#define MULTI_CONVEX

#include "..\Common.h"
#include "..\PintDef.h"

#include "PsArray.h"
namespace ps = physx::shdfnd;

#ifdef PEEL_USE_ICE_FRACTURE
	#include <meshmerizer\IceFractureConvex.h>
	typedef FractureConvex Convex;
#endif

class FracturePattern;
class Pint;

	class MultiConvex : public Allocateable
	{
		public:
						MultiConvex();
						~MultiConvex();

		void			createFromConvex(Pint& pint, const FractureConvex* convex, const Point& offset);

//		bool			rayCast(Pint& pint, const Ray& ray, float& dist, int& convexNr);
		bool			randomFracture(Pint& pint, const int convexNr, const Ray& ray, int numCuts, float vel = 0.0f);
		bool			patternFracture(Pint& pint, const int convexNr, const Ray& ray, const FracturePattern& pattern, float scale = 1.0f, float vel = 0.0f);

		bool			DeleteConvex(Pint& pint, PintActorHandle actor);

		private:
		void			clear();
		struct ConvexData
		{
			FractureConvex*		mConvex;
			PintActorHandle		mActor;
			Pint*				mPint;
			Matrix4x4			mPose;
		};
		ps::Array<ConvexData>	mConvexData;

		void			AddConvex(FractureConvex* c, Pint& pint, PintActorHandle actor, const Matrix4x4& pose);
		void			DeleteConvex(Pint* pint, PintActorHandle actor, FractureConvex* convex);
		void			DeleteConvex(const ConvexData& cd)
						{
							DeleteConvex(cd.mPint, cd.mActor, cd.mConvex);
						}
		Matrix4x4		getActorPose(const ConvexData& cd)	const;
		bool			RayCast(const ConvexData& cd, const Ray& ray, float& dist)	const;
	};

#endif