///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a container for visual debug data.
 *	\file		IceVisualDebugData.h
 *	\author		Pierre Terdiman
 *	\date		February, 24, 2008
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEVISUALDEBUGDATA_H
#define ICEVISUALDEBUGDATA_H

	enum VisualDebugColor
	{
		ARGB_BLACK		= 0x7f000000,
		ARGB_RED		= 0x7fff0000,
		ARGB_GREEN		= 0x7f00ff00,
		ARGB_BLUE		= 0x7f0000ff,
		ARGB_YELLOW		= 0x7fffff00,
		ARGB_MAGENTA	= 0x7fff00ff,
		ARGB_CYAN		= 0x7f00ffff,
		ARGB_WHITE		= 0x7fffffff,
		ARGB_HC_BLUE	= 0x7f030327,
		ARGB_CUSTOM_1	= 0x7f102050,
		ARGB_CUSTOM_2	= 0x7fa02050,
	};

	struct ICEMATHS_API VisualDebugPoint
	{
		Point	p;
		udword	color;
	};

	struct ICEMATHS_API VisualDebugLine
	{
		Point	p0;
		Point	p1;
		udword	color;
	};

	struct ICEMATHS_API VisualDebugTriangle
	{
		Point	p0;
		Point	p1;
		Point	p2;
		udword	color;
	};

	class ICEMATHS_API VisualDebugData
	{
		public:
											VisualDebugData();
											~VisualDebugData();

				void						Reset();

				udword						GetNbPoints()		const;
				const VisualDebugPoint*		GetPoints()			const;

				udword						GetNbLines()		const;
				const VisualDebugLine*		GetLines()			const;

				udword						GetNbTriangles()	const;
				const VisualDebugTriangle*	GetTriangles()		const;

				void						AddPoint	(const Point& p, udword color);
				void						AddLine		(const Point& p0, const Point& p1, udword color);
				void						AddTriangle	(const Point& p0, const Point& p1, const Point& p2, udword color);
				void						AddOBB		(const OBB& box, udword color, bool render_frame);
				void						AddAABB		(const AABB& bounds, udword color, bool render_frame);
		private:
				Container					mPoints;
				Container					mLines;
				Container					mTriangles;
	};

	CHECK_CONTAINER_ITEM(VisualDebugPoint)
	CHECK_CONTAINER_ITEM(VisualDebugLine)
	CHECK_CONTAINER_ITEM(VisualDebugTriangle)

#endif // ICEVISUALDEBUGDATA_H
