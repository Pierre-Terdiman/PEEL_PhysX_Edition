///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains silhouette computation using dual space.
 *	\file		IceDualSilhouette.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEDUALSILHOUETTE_H
#define ICEDUALSILHOUETTE_H

	struct MESHMERIZER_API DUALSILHOUETTECREATE
	{
		IndexedSurface*	mSurface;
	};

	// warning : words here
	struct MESHMERIZER_API SilEdgeRef
	{
		uword Ref0;
		uword Ref1;
	};

	//! A silhouette class.
	class MESHMERIZER_API DualSilhouette
	{
		public:
									DualSilhouette();
		virtual						~DualSilhouette();

				bool				Init(const DUALSILHOUETTECREATE& create);

				bool				ComputeSilhouette(const Point& view_point);

		// Data access
				udword				GetNbSilhouetteEdges()	const	{ return mSilhouetteEdges.GetNbEntries();/* not >>1 since words*/	}
				const SilEdgeRef*	GetSilhouetteEdges()	const	{ return (const SilEdgeRef*)mSilhouetteEdges.GetEntries();			}

		private:
				Container			mSilhouetteEdges;
				IndexedSurface*		mSurface;
				EdgeList*			mEdgeList;
				Point*				mDualPts;
				int*				mIndices;
	};

#endif // ICEDUALSILHOUETTE_H
