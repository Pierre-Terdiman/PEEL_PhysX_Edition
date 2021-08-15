///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CRC32.h"

udword ComputeCRC32_Verts(const SurfaceInterface& surface)
{
	if(!surface.mVerts || !surface.mNbVerts)
		return 0;
	return Crc32(surface.mVerts, surface.mNbVerts*sizeof(Point));
}

udword ComputeCRC32_Faces(const SurfaceInterface& surface)
{
	if(!surface.mNbFaces)
		return 0;
	if(surface.mDFaces)
		return Crc32(surface.mDFaces, surface.mNbFaces*sizeof(udword));
	else if(surface.mWFaces)
		return Crc32(surface.mWFaces, surface.mNbFaces*sizeof(uword));
	else
		return 0;
}
