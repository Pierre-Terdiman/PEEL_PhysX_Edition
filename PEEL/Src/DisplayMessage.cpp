///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DisplayMessage.h"
#include "GLFontRenderer.h"

static	bool		gDisplayMessage = false;
static	float		gDisplayMessageDelay = 0.0f;
static	MessageType	gDisplayMessageType = SAVING_RESULTS;

void StartDisplayMessage(MessageType type)
{
	gDisplayMessage = true;
	gDisplayMessageType = type;
}

float DisplayMessage(GLFontRenderer& texter, float y, float textScale)
{
	if(gDisplayMessage)
	{
		gDisplayMessage = false;
		gDisplayMessageDelay = 1.0f;
	}
	if(gDisplayMessageDelay>0.0f)
	{
		y -= textScale * 2.0f;
		texter.setColor(1.0f, 0.0f, 0.0f, 1.0f);
		if(gDisplayMessageType==SAVING_RESULTS)
			texter.print(0.0f, y, textScale, "Saving results...");
		if(gDisplayMessageType==SAVING_SCREENSHOT)
			texter.print(0.0f, y, textScale, "Saving screenshot...");
		else if(gDisplayMessageType==PRIVATE_BUILD)
			texter.print(0.0f, y, textScale, "This private test is disabled in public builds.");
	}
	return y;
}

void UpdateDisplayMessage(float elapsedTime)
{
	if(gDisplayMessageDelay>0.0f)
	{
		gDisplayMessageDelay -= elapsedTime;
		if(gDisplayMessageDelay<0.0f)
			gDisplayMessageDelay = 0.0f;
	}
}
