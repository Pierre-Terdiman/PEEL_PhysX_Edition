///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_TEXTURE_H
#define TOOL_TEXTURE_H

#include "ToolRayBased.h"

	class TextureComboBox;

	class ToolTexture : public ToolRayBased
	{
		public:
									ToolTexture();
		virtual						~ToolTexture();

		virtual	void				CreateUI			(PintGUIHelper& helper, IceWidget* parent, Widgets& owner);

		virtual	void				Select				();
		virtual	void				Deselect			();
		virtual	void				Reset				(udword pint_index);

		virtual	void				KeyboardCallback	(Pint& pint, udword pint_index, unsigned char key, bool down);

		virtual	void				RightDownCallback	(Pint& pint, udword pint_index);
		virtual	void				RightDragCallback	(Pint& pint, udword pint_index);
		virtual	void				RightUpCallback		(Pint& pint, udword pint_index);

		// GUI_RenderInterface
		virtual	void				FinalRenderCallback	()	override;
		//~GUI_RenderInterface

		private:
				TextureComboBox*	mTextureComboBox;
				TextureComboBox*	mTextureComboBox2;
				EditBoxPtr			mEditBox_Width;
				EditBoxPtr			mEditBox_Height;
				EditBoxPtr			mEditBox_Scale;
				udword				mTimestamp;
				udword				mTimestamp2;
				sdword				mMinX, mMinY;
				sdword				mMaxX, mMaxY;
				sdword				mOffsetX, mOffsetY;
				bool				mCapture;
				bool				mST;

				void				UpdateUI();
		friend	class	TextureComboBox;
	};

#endif
