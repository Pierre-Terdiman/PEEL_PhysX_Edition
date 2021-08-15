///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GLVSync.h"

// Based on the old ICE GL renderer. Should be refactored properly at some point.

static bool ExtensionSupported(const char* buf, const char* ext)
{
	if(!buf)
	{
		SetIceError("Extensions string is null", null);
		return false;
	}
	const int Length = int(strlen(buf));

	char* CurrentExt = new char[Length];
	CHECKALLOC(CurrentExt);

	int j=0;
	while(buf[j]!='\0')
	{
		int i=0;
		while(buf[j]!=' ' && buf[j]!='\0')
		{
			CurrentExt[i++] = buf[j++];
			if(i>=Length)
			{
				DELETEARRAY(CurrentExt);
				return SetIceError("Extensions string is too long!?", null);
			}
		}
		CurrentExt[i]='\0';
		j++;
		if(strcmp(ext, CurrentExt)==0)
		{
			DELETEARRAY(CurrentExt);
			return true;
		}
	}
	DELETEARRAY(CurrentExt);
	return false;
}

typedef void (APIENTRY * PFNWGLSWAPINTERVALPROC) (GLenum interval);
typedef GLenum (APIENTRY * PFNWGLGETSWAPINTERVALPROC) (void);

static PFNWGLSWAPINTERVALPROC		wglSwapIntervalEXT		= null;
static PFNWGLGETSWAPINTERVALPROC	wglGetSwapIntervalEXT	= null;

bool EnableGLExtensions()
{
	const char* Extensions = (const char*)glGetString(GL_EXTENSIONS);

	if(ExtensionSupported(Extensions, "WGL_EXT_swap_control"))
	{
		wglSwapIntervalEXT		= (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");
		wglGetSwapIntervalEXT	= (PFNWGLGETSWAPINTERVALPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
	}
	return true;
}

bool GL_SelectVSYNC(bool flag)
{
	// To select between VSYNC or not VSYNC, we need the appropriate extension.
	if(!wglSwapIntervalEXT)
		return false;

	// 0 => no vsync, 1 => vsync ==> use the bool
	wglSwapIntervalEXT(flag);
	return true;	// Tells the user we actually made the call
}
