///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SCRIPT_H
#define SCRIPT_H

	void ReleaseAutomatedTests();

	void RunScript(const char* filename);
	void UpdateAutomatedTests(udword frame_nb, bool menu_is_visible);

	void StartTest(const char* name);

#endif
