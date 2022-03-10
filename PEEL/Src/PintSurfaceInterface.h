///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_SURFACE_INTERFACE_H
#define PINT_SURFACE_INTERFACE_H

#include "CRC32.h"

	struct PintSurfaceInterface : SurfaceInterface
	{
		PintSurfaceInterface() :
			mCRC32_Verts	(0),
			mCRC32_Faces	(0)
		{
		}

		PintSurfaceInterface(const PintSurfaceInterface& that)
		{
			mNbVerts		= that.mNbVerts;
			mVerts			= that.mVerts;
			mNbFaces		= that.mNbFaces;
			mDFaces			= that.mDFaces;
			mWFaces			= that.mWFaces;
			mCRC32_Verts	= that.mCRC32_Verts;
			mCRC32_Faces	= that.mCRC32_Faces;
		}

		void	Init(const SurfaceInterface& surface)
		{
			static_cast<SurfaceInterface&>(*this) = surface;
			mCRC32_Verts = ComputeCRC32_Verts(*this);
			mCRC32_Faces = ComputeCRC32_Faces(*this);
		}

		explicit PintSurfaceInterface(const SurfaceInterface& that)
		{
			Init(that);
		}

		udword	mCRC32_Verts;
		udword	mCRC32_Faces;
	};

#endif
