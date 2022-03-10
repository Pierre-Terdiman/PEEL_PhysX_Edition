///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GL_RENDER_HELPERS_H
#define GL_RENDER_HELPERS_H

	namespace GLRenderHelpers
	{
		void	DrawLine(const Point& p0, const Point& p1, const Point& color);
		void	DrawLines(udword nb_lines, const Point* pts, const Point& color);
		void	DrawLine2D(float x0, float x1, float y0, float y1, const Point& color);
		void	DrawLines2D(const float* vertices, udword nb_verts, const Point& color);
		void	DrawCircle(udword nb_segments, const Matrix4x4& matrix, const Point& color, float radius, bool semi_circle=false);
		void	DrawTriangle(const Point& p0, const Point& p1, const Point& p2, const Point& color);

		void	SetupGLMatrix(const PR& pose);

		void	DrawSphere(float radius, const PR& pose);
		void	DrawSphereWireframe(float radius, const PR& pose, const Point& color);
		void	DrawBox(const Point& extents, const PR& pose);
		void	DrawCapsule(float r, float h);
		void	DrawCapsule(float r, float h, const PR& pose);
		void	DrawCapsuleWireframe(float r, float h, const PR& pose, const Point& color);
		void	DrawCylinder(float r, float h);
		void	DrawCylinder(float r, float h, const PR& pose);

		void	DrawFrame(const Point& pt, float scale);
		void	DrawFrame(const PR& pose, float scale, bool symmetricFrames);
		void	DrawFatFrame(const Point& pos, float scale);
		void	DrawFatFrame(const PR& pos, float scale, bool symmetricFrames);

		enum ScreenQuadTextureFlags
		{
			SQT_DISABLED	= 0,
			SQT_TEXTURING	= (1<<0),
			SQT_FLIP_U		= (1<<1),
			SQT_FLIP_V		= (1<<2)
		};

		void	DrawRectangle(float x_start, float x_end, float y_start, float y_end, const Point& color_top, const Point& color_bottom, float alpha, udword screen_width, udword screen_height, bool draw_outline, udword texture_flags);

		void	DrawBoxCorners(const AABB& bounds, const Point& color, float size);
	}

#endif
