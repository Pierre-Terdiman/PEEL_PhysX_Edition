///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef MINIMAL_REGION_MANAGER_H
#define MINIMAL_REGION_MANAGER_H

#include "Streaming.h"

	class Pint;
	class MinimalRegionManager : public StreamInterface
	{
		Pint&	mPint;

		public:
						MinimalRegionManager(Pint& pint);

		virtual	void	AddRegion(StreamRegion& region)				override;
		virtual	void	UpdateRegion(const StreamRegion& region)	override;
		virtual	void	RemoveRegion(const StreamRegion& region)	override;
	};

#endif
