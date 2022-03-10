///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_TRANSLATE_H
#define TOOL_TRANSLATE_H

#include "ToolRayBased.h"
#include "ToolTransform.h"

#define SUPPORT_TRANSLATE_TOOL_UI

	class ToolTranslate : public ToolRayBased
	{
		public:
								ToolTranslate();
		virtual					~ToolTranslate();

#ifdef SUPPORT_TRANSLATE_TOOL_UI
		virtual	void			CreateUI			(PintGUIHelper& helper, IceWidget* parent, Widgets& owner);
#endif
		virtual	void			Select				();
		virtual	void			Deselect			();
		virtual	void			Reset				(udword pint_index);

		virtual	void			OnObjectReleased	(Pint& pint, PintActorHandle removed_object);

		virtual	void			KeyboardCallback	(Pint& pint, udword pint_index, unsigned char key, bool down);

		virtual	void			RightDownCallback	(Pint& pint, udword pint_index);
		virtual	void			RightDragCallback	(Pint& pint, udword pint_index);
		virtual	void			RightUpCallback		(Pint& pint, udword pint_index);

		// GUI_RenderInterface
		virtual	void			RenderCallback		(PintRender& render, Pint& pint, udword pint_index)	override;
		virtual	void			PostRenderCallback	()													override;
		//~GUI_RenderInterface

#ifdef SUPPORT_TRANSLATE_TOOL_UI
				EditPosWindowPtr	mEditPos;
#endif
				Point			mCachedOrigin;
				Point			mCachedDir;

				struct TranslateData
				{
					TranslateData()
					{
						Reset();
					}

					inline	void	Reset()
					{
						mObject = null;
						mDelta.Zero();
						mDistance = 0.0f;
						mCaptured = false;
						mTransformer.Reset();
					}

					PintActorHandle	mObject;
					Point			mDelta;
					float			mDistance;
					bool			mCaptured;
					Transformer		mTransformer;
				};

				TranslateData	mData[MAX_NB_ENGINES];
				bool			mDrag;
	};

#endif
