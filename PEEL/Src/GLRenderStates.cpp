///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GLRenderStates.h"
#include <IceRenderer/IceRenderStates.h>

/*void GLRenderStates::EnableFrontfaceCulling()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
}

void GLRenderStates::EnableBackfaceCulling()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void GLRenderStates::DisableBackfaceCulling()
{
	glDisable(GL_CULL_FACE);
}*/

static CULLMODE gDefaultCullMode = CULL_CW;

void GLRenderStates::SetDefaultCullMode(CULLMODE cull_mode)
{
	gDefaultCullMode = cull_mode;
}

void GLRenderStates::SetDefaultRenderStates(bool flip_culling)
{
	switch(gDefaultCullMode)
	{
		case CULL_NONE:	glDisable(GL_CULL_FACE);												break;
		case CULL_CW:	glEnable(GL_CULL_FACE);	glCullFace(flip_culling ? GL_FRONT : GL_BACK);	break;
		case CULL_CCW:	glEnable(GL_CULL_FACE);	glCullFace(flip_culling ? GL_BACK : GL_FRONT);	break;
	}
}
