///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_TRANSFORM_H
#define TOOL_TRANSFORM_H

#include "PintDef.h"

	class Pint;

	class Transformer
	{
		public:
			Transformer();
			~Transformer();

			void	Reset();

			void	Start(Pint& pint, PintActorHandle h);
			void	Stop(Pint& pint, PintActorHandle h);
			void	SetPose(Pint& pint, PintActorHandle h, const PR& pose);

		protected:
			bool	mIsKine;
			bool	mActive;
	};

#endif
