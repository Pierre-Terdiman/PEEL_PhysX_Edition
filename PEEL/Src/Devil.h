///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef DEVIL_H
#define DEVIL_H

	bool	InitDevil();
	bool	CloseDevil();

	bool	LoadWithDevil(const char* filename, Picture& pic);
	bool	SaveWithDevil(const char* filename, const Picture& pic);

#endif
