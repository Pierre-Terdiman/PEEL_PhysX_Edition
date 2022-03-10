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

	///////////////////////////////////////////////////////////////////////////////

/*	class EditJointTypeCallback
	{
		public:
			virtual	void	JointTypeHasChanged()	= 0;
	};

	class JointTypeComboBox : public IceComboBox
	{
		EditJointTypeCallback&	mCallback;

		public:
						JointTypeComboBox(const ComboBoxDesc& desc, EditJointTypeCallback& cb) : IceComboBox(desc), mCallback(cb)	{}

		virtual	void	OnComboBoxEvent(ComboBoxEvent event)
		{
			if(event==CBE_SELECTION_CHANGED)
				mCallback.JointTypeHasChanged();
		}
	};

	JointTypeComboBox* CreateJointComboBox(IceWidget* parent, sdword x, sdword y, void* user_data, EditJointTypeCallback& callback);
*/
#endif
