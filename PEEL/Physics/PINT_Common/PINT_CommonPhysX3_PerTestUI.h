///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_PER_TEST_UI_H
#define PINT_COMMON_PHYSX3_PER_TEST_UI_H

#include "..\Pint.h"

	class PhysXPlugIn : public PintPlugin
	{
		public:
		virtual	IceWindow*	InitGUI(IceWidget* parent, PintGUIHelper& helper);
		virtual	void		CloseGUI();

		virtual	void		Init(const PINT_WORLD_CREATE& desc);
		virtual	void		Close();

		virtual	Pint*		GetPint();

		virtual	IceWindow*	InitTestGUI(const char* test_name, IceWidget* parent, PintGUIHelper& helper, Widgets& owner);
		virtual	void		CloseTestGUI();
		virtual	const char*	GetTestGUIName();
		virtual	void		ApplyTestUIParams(const char* test_name);
	};

#endif