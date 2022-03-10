///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains frustum-related code.
 *	\file		IceFrustum.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEFRUSTUM_H
#define ICEFRUSTUM_H

	enum FrustumPlaneIndex
	{
		FRUSTUM_PLANE_LEFT			= 0,	//!< Left clipping plane
		FRUSTUM_PLANE_RIGHT			= 1,	//!< Right clipping plane
		FRUSTUM_PLANE_TOP			= 2,	//!< Top clipping plane
		FRUSTUM_PLANE_BOTTOM		= 3,	//!< Bottom clipping plane
		FRUSTUM_PLANE_NEAR			= 4,	//!< Near clipping plane
		FRUSTUM_PLANE_FAR			= 5,	//!< Far clipping plane (must be last for infinite far clip)

		FRUSTUM_PLANE_FORCE_DWORD	= 0x7fffffff
	};

	class MESHMERIZER_API Frustum
	{
		public:
								Frustum()	{}
		virtual					~Frustum()	{}
		// Planes extraction
				Frustum&		ExtractPlanes(const Matrix4x4& combo, bool normalize=false, bool z_symmetry=false);
		// Data access
		inline_	const Plane*	GetPlanes()							const	{ return mPlanes;			}
		inline_	const Plane&	GetPlane(FrustumPlaneIndex index)	const	{ return mPlanes[index];	}

		private:
				Plane			mPlanes[6];			//!< Frustum planes
	};

#endif	// ICEFRUSTUM_H
