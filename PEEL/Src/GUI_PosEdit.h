///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GUI_POS_EDIT_H
#define GUI_POS_EDIT_H

#include "GUI_EditBox.h"

	class EditPosWindow : public IceWindow, public EditBoxInterface
	{
		public:
									EditPosWindow(const WindowDesc& desc, EditBoxInterface* owner=null);
		virtual						~EditPosWindow();

		// EditBoxInterface
		virtual	void				ChangeNotification()	override;
		//~EditBoxInterface

				void				SetPos(const Point& pos);
				Point				GetPos()	const;

				void				SetEulerAngles(const Quat& rot);
				void				GetEulerAngles(Quat& rot)	const;

				void				Reset();
				void				SetEnabled(bool b);

				bool				SomethingChanged()	const;
				void				ResetChangedState();

				EditBoxInterface*	mOwner;
				PEEL_EditBox*		mEditBoxX;
				PEEL_EditBox*		mEditBoxY;
				PEEL_EditBox*		mEditBoxZ;
		mutable	Point				mEdited;
				bool				mHasEdit;
				bool				mEnabled;
	};

#endif
