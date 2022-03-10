#ifndef ICECONVEXHULL2_H
#define ICECONVEXHULL2_H

	class MESHMERIZER_API ConvexHull2 : public Allocateable
	{
		public:
										ConvexHull2();
										~ConvexHull2();

		// Wrappers to make the ConvexHull-based code compile
		inline_	udword					GetNbVerts()			const	{ return mNbVerts;		}
		inline_	const Point*			GetVerts()				const	{ return mVerts;		}

		inline_	udword					GetNbPolygons()			const	{ return mNbPolygons;	}
		inline_	const HullPolygon&		GetPolygon(udword i)	const	{ return mPolygons[i];	}

				Point*					InitVertices(udword nb);
				udword					mNbVerts;
				Point*					mVerts;

				udword*					InitIndices(udword nb);
				udword					mNbIndices;
				udword*					mIndices;

				HullPolygon*			InitPolygons(udword nb);
				udword					mNbPolygons;
				HullPolygon*			mPolygons;
	};

	FUNCTION MESHMERIZER_API ConvexHull2*	CreateConvexHull2(udword nb_verts, const Point* verts);

#endif	// ICECONVEXHULL2_H
