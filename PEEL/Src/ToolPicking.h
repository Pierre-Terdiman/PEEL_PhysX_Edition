///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_PICKING_H
#define TOOL_PICKING_H

#include "ToolRayBased.h"

	class ToolPicking : public ToolRayBased
	{
		public:
						ToolPicking();
		virtual			~ToolPicking();

		virtual	void	CreateUI			(PintGUIHelper& helper, IceWidget* parent, Widgets& owner);

		virtual	void	Reset				(udword pint_index);
		virtual	void	Deselect			()						{ Reset(INVALID_ID);	}

		virtual	void	OnObjectReleased	(Pint& pint, PintActorHandle removed_object);

		virtual	void	KeyboardCallback	(Pint& pint, udword pint_index, unsigned char key, bool down);

		virtual	void	RightDownCallback	(Pint& pint, udword pint_index);
		virtual	void	RightDragCallback	(Pint& pint, udword pint_index);
		virtual	void	RightUpCallback		(Pint& pint, udword pint_index);
		virtual	void	RightDblClkCallback	(Pint& pint, udword pint_index);

		virtual	bool	IsControllingCamera	()	const;

		// GUI_RenderInterface
		virtual	void	PreRenderCallback	()													override;
		virtual	void	RenderCallback		(PintRender& render, Pint& pint, udword pint_index)	override;
		//~GUI_RenderInterface

		struct TrackingData
		{
			TrackingData()
			{
				Reset();
			}

			inline	void	Reset()
			{
				mTrackedObject = null;
			}

			PintActorHandle	mTrackedObject;
		};

		TrackingData	mTrackedData[MAX_NB_ENGINES];

		struct PickingData
		{
			PickingData()
			{
				Reset();
			}

			inline	void	Reset()
			{
				mPickedObject = null;
				mImpact = mDragPoint = mLocalPoint = Point(0.0f, 0.0f, 0.0f);
				mDistance = mLinearDamping = mAngularDamping = 0.0f;
			}

			PintActorHandle	mPickedObject;
			Point			mImpact;
			Point			mDragPoint;
			Point			mLocalPoint;
			float			mDistance;
			float			mLinearDamping;
			float			mAngularDamping;
			Triangle		mTouchedTriangle;
		};

		PickingData	mPickData[MAX_NB_ENGINES];

		bool	mIsControllingCamera;

		// Debug mode
		PintMultipleHits	mHits;
		Container			mStream;
		Sphere				mSphere;
		Vertices			mDebugMeshVertices;
		PR					mDebugMeshPose;
		Pint*				mDebugPint;
		void				CreateDebugMesh();
	};

#endif
