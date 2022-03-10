///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains silhouette-related code.
 *	\file		IceSilhouette.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESILHOUETTE_H
#define ICESILHOUETTE_H

	class NormalMask;

	enum SilhouetteMethod
	{
		SIL_NONE		= 0,
		SIL_METHOD1		= 1,
		SIL_METHOD2		= 2,
		SIL_METHOD2B	= 3,
		SIL_METHOD3		= 4,
		SIL_METHOD4		= 5,
		SIL_METHOD5		= 6,

		SIL_FORCE_DWORD	= 0x7fffffff
	};

	//! A silhouette class.
	class MESHMERIZER_API Silhouette
	{
		public:
									Silhouette();
		virtual						~Silhouette();

				bool				ComputeSilhouetteEdges(const Point& sourcepoint, const SurfaceInterface& surface, SilhouetteMethod method, bool screenspace=false, NormalMask* nm=null);

		// Data access
				udword				GetNbSilhouetteEdges()	const	{ return mSilhouetteEdges.GetNbEntries();/* not >>1 since words*/	}
				const SilEdgeRef*	GetSilhouetteEdges()	const	{ return (const SilEdgeRef*)mSilhouetteEdges.GetEntries();			}
		private:
				Container			mSilhouetteEdges;
				RadixSort*			mSorter;			//! Embedded for temporal coherence
				EdgeList*			mEdgeList;
				Point*				mDualPts;

		// Internal methods
				bool				ComputeSilhouetteEdgesMethod1(const Point& sourcepoint, const SurfaceInterface& surface, bool screenspace=false, NormalMask* nm=null);
				bool				ComputeSilhouetteEdgesMethod2(const Point& sourcepoint, const SurfaceInterface& surface, bool screenspace=false, NormalMask* nm=null);
				bool				ComputeSilhouetteEdgesMethod2b(const Point& sourcepoint, const SurfaceInterface& surface, bool screenspace=false, NormalMask* nm=null);
				bool				ComputeSilhouetteEdgesMethod3(const Point& sourcepoint, const SurfaceInterface& surface, bool screenspace=false, NormalMask* nm=null);
				bool				ComputeSilhouetteEdgesMethod4(const Point& sourcepoint, const SurfaceInterface& surface, bool screenspace=false, NormalMask* nm=null);
				bool				ComputeSilhouetteEdgesMethod5(const Point& sourcepoint, const SurfaceInterface& surfacee, bool screenspace=false, NormalMask* nm=null);
	};

#endif // ICESILHOUETTE_H
