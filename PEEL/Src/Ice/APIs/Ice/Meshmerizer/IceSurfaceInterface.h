///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a surface interface.
 *	\file		IceSurfaceInterface.h
 *	\author		Pierre Terdiman
 *	\date		January, 18, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESURFACEINTERFACE_H
#define ICESURFACEINTERFACE_H

	//! This minimal interface is used to link one data structure to another in a unified way.
	struct MESHMERIZER_API SurfaceInterface
	{
		inline_ SurfaceInterface()
			: mNbVerts	(0),
			mVerts		(null),
			mNbFaces	(0),
			mDFaces		(null),
			mWFaces		(null)
		{}

		inline_ SurfaceInterface(
			udword			nb_verts,
			const Point*	verts,
			udword			nb_faces,
			const udword*	dfaces,
			const uword*	wfaces
			)
			: mNbVerts	(nb_verts),
			mVerts		(verts),
			mNbFaces	(nb_faces),
			mDFaces		(dfaces),
			mWFaces		(wfaces)
		{}

		udword			mNbVerts;	//!< Number of vertices
		const Point*	mVerts;		//!< List of vertices
		udword			mNbFaces;	//!< Number of faces
		const udword*	mDFaces;	//!< List of faces (dword indices)
		const uword*	mWFaces;	//!< List of faces (word indices)
	};

#endif // ICESURFACEINTERFACE_H
