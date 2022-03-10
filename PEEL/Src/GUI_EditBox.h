///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GUI_EDIT_BOX_H
#define GUI_EDIT_BOX_H

#include "PintGUIHelper.h"

	// We use a custom edit box to enable/disable other items when the edited field changes

	class EditBoxInterface
	{
		public:
			virtual	void	ChangeNotification()	= 0;
	};

	class PEEL_EditBox : public IceEditBox
	{
		public:
									PEEL_EditBox(const EditBoxDesc& desc, EditBoxInterface& owner, udword label_width=0, const char* label=null);
		virtual						~PEEL_EditBox();

		// IceWidget
		virtual	void				SetVisible(bool b)	override;
		//~IceWidget

		// IceEditBox
		virtual	bool				FilterKey(udword key)	const	override;
		//~IceEditBox

				EditBoxInterface&	mOwner;
				LabelPtr			mLabel;
				mutable	bool		mSomethingChanged;
	};

	PEEL_EditBox* CreateTextEditBox(EditBoxInterface& owner, IceWidget* parent, sdword x, sdword y, const char* label, udword label_width, udword edit_box_width, udword edit_box_height=20, udword id=0, const char* default_name=null);
	PEEL_EditBox* CreateFloatEditBox(EditBoxInterface& owner, IceWidget* parent, sdword x, sdword y, const char* label, udword label_width, udword edit_box_width, udword edit_box_height=20, udword id=0, float default_value=0.0f);

#endif
