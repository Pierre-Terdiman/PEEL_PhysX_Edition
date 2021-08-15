///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SURFACE_MANAGER_H
#define SURFACE_MANAGER_H

#include "Pint.h"

	// Some physics engines don't copy the mesh data, so we need to keep it around for the lifetime of a test.
	class SurfaceManager
	{
		public:
									SurfaceManager();
		virtual						~SurfaceManager();

//				IndexedSurface*		CreateManagedSurface();
				void				ReleaseManagedSurfaces();

				bool				CreateMeshesFromRegisteredSurfaces(Pint& pint, const PintCaps& caps, const PINT_MATERIAL_CREATE* material=null, PtrContainer* created_objects=null, const char* name=null);

				// New API
				// TODO: consider storing the CRCs in SurfaceData
				struct SurfaceData
				{
					const IndexedSurface*		mSurface;
					PintShapeRenderer*			mRenderer;
					const PINT_MATERIAL_CREATE*	mMaterial;
					PR							mPose;
				};
				void				RegisterSurface(const IndexedSurface* surface, PintShapeRenderer* renderer=null, const PR* pose=null, const PINT_MATERIAL_CREATE* material=null);
				udword				GetNbRegisteredSurfaces()	const;
				const SurfaceData*	GetSurfaceData(udword i)	const;

/*		inline_	udword				GetNbSurfaces()				const
									{
										return mSurfaces.GetNbEntries();
									}*/

		inline_	void				SetGlobalBounds(const AABB& global_bounds)
									{
										mGlobalBounds = global_bounds;
									}

		inline_	void				GetGlobalBounds(Point& center, Point& extents)	const
									{
										mGlobalBounds.GetCenter(center);
										mGlobalBounds.GetExtents(extents);
									}
		private:
				AABB				mGlobalBounds;
//				PtrContainer		mSurfaces;
				Container			mSurfaceData;
	};

#endif
