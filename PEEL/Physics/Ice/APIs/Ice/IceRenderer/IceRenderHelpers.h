///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains rendering helpers
 *	\file		IceRenderHelpers.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERENDERHELPERS_H
#define ICERENDERHELPERS_H

	enum SegmentStyle
	{
		SEGMENT_NORMAL,
		SEGMENT_CORNERS,
		SEGMENT_DASHED,

		SEGMENT_FORCE_DWORD = 0x7fffffff
	};

	enum SegmentType
	{
		SEGMENT_STRIP	= (1<<0),
		SEGMENT_CLOSED	= (1<<1)
	};

	enum ClippedQuadFlag
	{
		CLIPPED_QUAD_FLIP_X	= (1<<0),
		CLIPPED_QUAD_FLIP_Y	= (1<<1),
	};

	// Forward references
	class RenderableSurface;
	struct ClipBox;

	//
	ICERENDERER_API bool RenderPlane				(Renderer* rd, const AABB& camera_box, const Plane& plane, udword color);
	ICERENDERER_API bool RenderCircle				(Renderer* rd, udword nb_segments, const PR& pr, udword color, float radius, Orientation orientation, bool wire=true);
	ICERENDERER_API bool RenderScreenSpaceCircle	(Renderer* rd, udword nb_segments, const Point& pos, udword color, float radius, bool wire=true);
	ICERENDERER_API bool RenderBSphere				(Renderer* rd, udword nb_segments, const PR& pr, udword color, float radius);
	ICERENDERER_API bool RenderCylinder				(Renderer* rd, const Point& p, const Point& dir, float length, float radius, udword color, udword nbseg);
	ICERENDERER_API bool RenderCone					(Renderer* rd, const Point& p, const Point& dir, float length, float radius, udword color, udword nbseg);
	ICERENDERER_API bool RenderFrame				(Renderer* rd, float size, bool render_arrows=false);
	ICERENDERER_API bool RenderFrames				(Renderer* rd, udword nb_frames, const Point* frames, float size, bool render_arrows=false);
	ICERENDERER_API bool RenderSegments				(Renderer* rd, const Matrix4x4* mat, udword color, udword nb_segments, const Point* segments, udword type=0, SegmentStyle style=SEGMENT_NORMAL);
	ICERENDERER_API bool RenderIndexedSegments		(Renderer* rd, const Matrix4x4* mat, udword color, udword nb_segments, const Point* segments, const uword* indices, udword nb_verts);
	ICERENDERER_API bool RenderDots					(Renderer* rd, const Matrix4x4* mat, udword color, udword nbdots, const Point* dots);
	ICERENDERER_API bool RenderPicture				(Renderer* rd, const Picture* pic, udword x, udword y, udword width, udword height, udword color);
	ICERENDERER_API bool RenderAABB					(Renderer* rd, const Matrix4x4& world, const Point& center, const Point& extents, udword color=ARGB_YELLOW);
	ICERENDERER_API bool RenderAABB2				(Renderer* rd, const AABB& aabb, udword color=ARGB_RED, const Matrix4x4* world=null, SegmentStyle style=SEGMENT_NORMAL);
	ICERENDERER_API bool RenderOBB					(Renderer* rd, const Matrix4x4* world, const AABB& aabb, udword color=ARGB_WHITE);
	ICERENDERER_API bool RenderOBB					(Renderer* rd, const OBB& box, udword color);
	ICERENDERER_API bool RenderLSS					(Renderer* rd, const LSS& lss, udword color);
	ICERENDERER_API bool RenderGround				(Renderer* rd, float y, float size, float tile);
	ICERENDERER_API bool RenderScreenRect			(Renderer* rd, const ScreenRect& rect, udword color);
	ICERENDERER_API bool RenderClippedQuad			(Renderer* rd, sdword x, sdword y, sdword width, sdword height, const ClipBox* clip_box=null, udword flags=0, udword color=0xffffffff);
	//
	ICERENDERER_API bool RenderConvexHull			(Renderer* rd, const ConvexHull* hull, const Matrix4x4* world=null, bool wire=true);
	ICERENDERER_API bool RenderHullPolygon			(Renderer* rd, const HullPolygon& polygon, const Point* verts, const Matrix4x4* world, udword poly_color, udword normal_color, float normal_scale);
	ICERENDERER_API bool RenderHullPolygons			(Renderer* rd, const ConvexHull& hull, const Matrix4x4* world, udword poly_color, udword normal_color, float normal_scale);
	//
	ICERENDERER_API bool RenderFaceNormals			(Renderer* rd, const IndexedSurface& surface, udword color=ARGB_RED, const Matrix4x4* world=null);
	ICERENDERER_API bool RenderVertexNormals		(Renderer* rd, const RenderableSurface& surface, udword color=ARGB_RED, const Matrix4x4* world=null);
	ICERENDERER_API bool RenderSurface				(Renderer* rd, const IndexedSurface& surface, const RGBAColor& color, const Matrix4x4* world=null);
	ICERENDERER_API bool RenderSurfaceAttributes	(Renderer* rd, const IndexedSurface& surface, const uword* wattrib=null, const udword* dattrib=null, const float* vals=null, const Matrix4x4* world=null);
	ICERENDERER_API	bool RenderStandardBackface		(Renderer* rd, const IndexedSurface& surface, const Point& view_pt, const Matrix4x4* world=null, bool wireframe=true, udword color=ARGB_WHITE);
	ICERENDERER_API	bool RenderNormalMapBackface	(Renderer* rd, const IndexedSurface& surface, const Point& view_pt, const Point& view_dir, float fov, NormalMask* normal_mask, const Matrix4x4* world=null);

#endif // ICERENDERHELPERS_H
