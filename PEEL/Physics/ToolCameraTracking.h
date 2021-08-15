///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_CAMERA_TRACKING_H
#define TOOL_CAMERA_TRACKING_H

#ifdef REMOVED
#include "ToolRayBased.h"

	class ToolCameraTracking : public ToolRayBased
	{
		public:
						ToolCameraTracking();
		virtual			~ToolCameraTracking();

		virtual	void	Reset(udword pint_index);
		virtual	void	Deselect()	{ Reset(INVALID_ID);	}

		virtual	void	RightDownCallback	(Pint& pint, udword pint_index);
		virtual	void	RenderCallback		(Pint& pint, udword pint_index);

		struct TrackingData
		{
			TrackingData()
			{
				Reset();
			}

			inline	void	Reset()
			{
				mObject = null;
			}

			PintActorHandle	mObject;
		};

		TrackingData	mTrackedData[MAX_NB_ENGINES];
	};
#endif

#endif
