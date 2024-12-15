///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GL_MSAA_H
#define GL_MSAA_H

	void	StartFrame_MSAA();
	void	EndFrame_MSAA();
	void	Release_MSAA();
	void	Resize_MSAA();
	void	Select_MSAA(udword index);

#endif