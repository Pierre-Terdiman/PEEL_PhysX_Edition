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
		virtual	IceWindow*	InitGUI(IceWidget* parent, PintGUIHelper& helper)	override;
		virtual	void		CloseGUI()											override;

		virtual	void		Init(const PINT_WORLD_CREATE& desc)					override;
		virtual	void		Close()												override;

		virtual	Pint*		GetPint()											override;

		// For per-test UI
		virtual	IceWindow*	InitTestGUI(const char* test_name, IceWidget* parent, PintGUIHelper& helper, Widgets& owner)	override;
		virtual	void		CloseTestGUI()																					override;
		virtual	const char*	GetTestGUIName()																				override;
		virtual	void		ApplyTestUIParams(const char* test_name)														override;
	};

#endif