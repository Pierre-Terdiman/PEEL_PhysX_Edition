///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef VORONOI2D_H
#define VORONOI2D_H

	class PintRender;

	void TestVoronoi2D(PintRender& renderer);

	///

	struct Point2D
	{
		inline_	Point2D()									{}
		inline_	Point2D(float xx, float yy) : x(xx), y(yy)	{}

		float x;
		float y;
	};

	class VoronoiCallback
	{
		public:
		virtual	void	OnGeneratedCell(const Point2D& site, udword nb_verts, const Point2D* verts)	= 0;
	};

	void GenerateVoronoiCells(udword nb_cells, float scale_x, float scale_z, VoronoiCallback& callback);

#endif
