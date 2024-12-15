///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef DISPLAY_MESSAGE_H
#define DISPLAY_MESSAGE_H

	class GLFontRenderer;

	enum MessageType
	{
		SAVING_RESULTS,
		SAVING_SCREENSHOT,
		PRIVATE_BUILD,
	};

	void	StartDisplayMessage(MessageType type);
	float	DisplayMessage(GLFontRenderer& texter, float y, float textScale);
	void	UpdateDisplayMessage(float elapsedTime);

#endif
