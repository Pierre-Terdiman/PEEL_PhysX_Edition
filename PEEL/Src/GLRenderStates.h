///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GL_RENDER_STATES_H
#define GL_RENDER_STATES_H

	namespace GLRenderStates
	{
//		void	EnableFrontfaceCulling();
//		void	EnableBackfaceCulling();
//		void	DisableBackfaceCulling();
		void	SetDefaultCullMode(CULLMODE cull_mode=CULL_CW);
		void	SetDefaultRenderStates(bool flip_culling=false);
	}

#endif
