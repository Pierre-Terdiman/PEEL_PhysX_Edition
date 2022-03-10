///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_GUI_HELPER_H
#define PINT_GUI_HELPER_H

#include "Common.h"

	class Widgets : public Allocateable
	{
		public:
							Widgets()
							{
							}
							~Widgets()
							{
								Release();
							}

		inline_	void		Register(IceGUIElement* widget)
							{
								mWidgets.AddPtr(widget);
							}

		inline_	void		Release()
							{
								DeleteOwnedObjects<IceGUIElement>(mWidgets);
							}
		private:
			PtrContainer	mWidgets;
	};

	// An instance of this class is passed to PINT plugins to help them create UI elements.
	class PintGUIHelper	: public Allocateable
	{
		public:
								PintGUIHelper()		{}
		virtual					~PintGUIHelper()	{}

		virtual	const char*		Convert(float value)	= 0;
//		virtual	const char*		GetRootDirectory()		= 0;

		virtual	IceWindow*		CreateMainWindow(Widgets*& gui, IceWidget* parent, udword id, const char* label)	= 0;
		virtual	IceLabel*		CreateLabel		(IceWidget* parent,				sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner)	= 0;
		virtual	IceCheckBox*	CreateCheckBox	(IceWidget* parent, udword id,	sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, bool state, CBCallback callback, const char* tooltip=null)	= 0;
		virtual	IceEditBox*		CreateEditBox	(IceWidget* parent, udword id,	sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, EditBoxFilter filter, EBCallback callback, const char* tooltip=null)	= 0;
		virtual	IceButton*		CreateButton	(IceWidget* parent, udword id,	sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, BCallback callback, void* user_data, const char* tooltip=null)	= 0;
	};

	template<class ComboBoxType>
	ComboBoxType* CreateComboBox(IceWidget* parent, udword id, sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, const char* tooltip)
	{
		ComboBoxDesc CBD;
		CBD.mID		= id;
		CBD.mParent	= parent;
		CBD.mX		= x;
		CBD.mY		= y;
		CBD.mWidth	= width;
		CBD.mHeight	= height;
		CBD.mLabel	= label;
		ComboBoxType* CB = ICE_NEW(ComboBoxType)(CBD);
		CB->SetVisible(true);
		if(owner)
			owner->Register(CB);

		if(tooltip)
			CB->AddToolTip(tooltip);
		return CB;
	}

	IceLabel*		GUI_CreateLabel(IceWidget* parent, sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner);
	IceCheckBox*	GUI_CreateCheckBox(IceWidget* parent, udword id, sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, bool state, CBCallback callback, const char* tooltip);
	IceEditBox*		GUI_CreateEditBox(IceWidget* parent, udword id, sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, EditBoxFilter filter, EBCallback callback, const char* tooltip);
	IceButton*		GUI_CreateButton(IceWidget* parent, udword id, sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, BCallback callback, void* user_data, const char* tooltip);

	template<class WidgetT>
	struct WidgetPtr
	{
		WidgetPtr() : mWidget(null)	{}

		inline_	WidgetPtr&		operator = (WidgetT* widget)	{ mWidget = widget;	return *this;	}
		inline_					operator WidgetT*()		const	{ return mWidget;	}
		inline_	const WidgetT*	operator->()			const	{ return mWidget;	}
		inline_	WidgetT*		operator->()					{ return mWidget;	}

//		inline_					operator void*()				{ return (void*)mWidget;	}
//		inline_	bool			operator !()					{ return mWidget==null;		}

				WidgetT*		mWidget;
	};

	class EditPosWindow;

	typedef WidgetPtr<IceEditBox>		EditBoxPtr;
	typedef WidgetPtr<IceComboBox>		ComboBoxPtr;
	typedef WidgetPtr<IceCheckBox>		CheckBoxPtr;
	typedef WidgetPtr<IceButton>		ButtonPtr;
	typedef WidgetPtr<IceLabel>			LabelPtr;
	typedef WidgetPtr<IceListBox>		ListBoxPtr;
	typedef WidgetPtr<IceSlider>		SliderPtr;
	typedef WidgetPtr<EditPosWindow>	EditPosWindowPtr;

#endif
