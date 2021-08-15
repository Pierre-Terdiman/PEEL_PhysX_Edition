///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SOURCE_RAY_H
#define SOURCE_RAY_H

	struct __declspec(align(16)) VectorAligned { float x,y,z,w; };

	struct Source1_Ray_t
	{
		VectorAligned	m_Start;		// starting point, centered within the extents
		VectorAligned	m_Delta;		// direction + length of the ray
		VectorAligned	m_StartOffset;	// Add this to m_Start to get the actual ray start
		VectorAligned	m_Extents;		// Describes an axis aligned box extruded along a ray
		void*			m_pDummy;
		bool			m_IsRay;		// are the extents zero?
		bool			m_IsSwept;		// is delta != 0?
	};

#endif
