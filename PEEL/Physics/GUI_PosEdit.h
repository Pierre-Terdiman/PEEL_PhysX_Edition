///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GUI_POS_EDIT_H
#define GUI_POS_EDIT_H

	class EditPosWindow : public IceWindow
	{
		public:
							EditPosWindow(const WindowDesc& desc);
		virtual				~EditPosWindow();

				void		SetPos(const Point& pos);
				Point		GetPos()	const;

				void		SetEulerAngles(const Quat& rot);
				void		GetEulerAngles(Quat& rot)	const;

				void		Reset();
				void		SetEnabled(bool b);

				IceLabel*	mLabelX;
				IceLabel*	mLabelY;
				IceLabel*	mLabelZ;
				IceEditBox*	mEditBoxX;
				IceEditBox*	mEditBoxY;
				IceEditBox*	mEditBoxZ;
		mutable	Point		mEdited;
				bool		mHasEdit;
				bool		mEnabled;
	};

#endif
