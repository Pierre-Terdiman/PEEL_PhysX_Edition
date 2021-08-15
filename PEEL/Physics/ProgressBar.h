///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

	void CreateProgressBar(udword nb, const char* label);
	void SetProgress(udword i);
	void ReleaseProgressBar();

#endif
