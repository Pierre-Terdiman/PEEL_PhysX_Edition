///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef LOADER_REPX_H
#define LOADER_REPX_H

	class SurfaceManager;
	void LoadRepXFile_Obsolete(SurfaceManager& test, const char* filename, float scale, bool z_is_up);

	class Pint;
	void*	CreateRepXContext(const char* filename, float scale, bool z_is_up);
	void	ReleaseRepXContext(void* repx_context);
	bool	AddToPint(Pint& pint, void* repx_context);

#endif
