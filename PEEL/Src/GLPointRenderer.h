///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GL_POINT_RENDERER_H
#define GL_POINT_RENDERER_H

	namespace GLPointRenderer
	{
		void	Init();
		void	Close();
		void	SetScreenResolution(int width, int height);
		void	Draw(const Point& color, udword nb_pts, const Point* pts, udword stride);
//		void	Draw(int n, int offset, float radius, float screenWidth, float screenAspect, float fov);
	};

#endif
