///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GUI_JOINT_EDIT_H
#define GUI_JOINT_EDIT_H

	class EditJointWindow : public IceWindow
	{
		public:
								EditJointWindow(const WindowDesc& desc);
		virtual					~EditJointWindow();
	};

	EditJointWindow*	CreateJointEditGUI(IceWidget* parent, sdword x, sdword y);
	void				CloseJointEditGUI();

	void				HideEditJointWindow();
	void				ShowEditJointWindow();

#endif
