///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TEST_SELECTOR_H
#define TEST_SELECTOR_H

	class GLFontRenderer;
	class PhysicsTest;

	int TestSelectionKeyboardCallback(int key, int current_test);

	PhysicsTest* RenderTestSelector(const GLFontRenderer& texter, float x, float text_scale, int start_index, float& y_last);

#endif