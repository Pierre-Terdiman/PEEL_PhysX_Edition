///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PEEL_THREADS_H
#define PEEL_THREADS_H

	void	PEEL_InitThreads();
	void	PEEL_ReleaseThreads();

	typedef int (*PEEL_ThreadCallback)(void*);

	void	PEEL_AddThreadWork(udword index, PEEL_ThreadCallback callback, void* user_data);
	void	PEEL_StartThreadWork(udword nb_threads);
	void	PEEL_EndThreadWork(udword nb_threads);

#endif
