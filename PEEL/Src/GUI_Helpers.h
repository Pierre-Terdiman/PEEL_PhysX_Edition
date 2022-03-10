///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GUI_HELPERS_H
#define GUI_HELPERS_H

	float			GetFloat(float default_value, const IceEditBox* edit_box);
	udword			GetInt(udword default_value, const IceEditBox* edit_box);

	IceComboBox*	CreateConvexObjectComboBox(IceWidget* parent, sdword x, sdword y, bool enabled);

	enum SupportedShapeMask
	{
		SSM_UNDEFINED	= (1<<0),
		SSM_SPHERE		= (1<<1),
		SSM_CAPSULE		= (1<<2),
		SSM_CYLINDER	= (1<<3),
		SSM_BOX			= (1<<4),
		SSM_CONVEX		= (1<<5),
		SSM_MESH		= (1<<6),
		SSM_ALL			= 0xffffffff
	};

	IceComboBox*	CreateShapeTypeComboBox(IceWidget* parent, sdword x, sdword y, bool enabled, udword mask=SSM_ALL);

	class PhysicsTest;
	class ResetButton : public IceButton
	{
		public:
								ResetButton(PhysicsTest& test, const ButtonDesc& desc);
		virtual					~ResetButton(){}

		virtual	void			OnClick();

				PhysicsTest&	mTest;
	};

#endif
