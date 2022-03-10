///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains support code for convex hulls. Moved from testing ground.
 *	\file		IceConvexData.h
 *	\author		Pierre Terdiman
 *	\date		March, 27, 2009
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECONVEXDATA_H
#define ICECONVEXDATA_H

	class MESHMERIZER_API ConvexData
	{
		public:
								ConvexData();
								~ConvexData();

				void			Init(udword nb_verts, udword nb_polygons, udword nb_data);
				bool			Init(const ConvexHull& hull);

				void			ComputeCenter(Point& center)												const;
				void			Cut(ConvexData& cdp, ConvexData& cdn, const Plane& local_plane)				const;
				void			Cut2(ConvexData& cdp, ConvexData& cdn, const Plane& local_plane)			const;

		inline_	udword			GetNbVerts()		const	{ return mNbVerts;		}
		inline_	const Point*	GetVerts()			const	{ return mVerts;		}

		inline_	udword			GetNbPolygons()		const	{ return mNbPolygons;	}
		inline_	const udword*	GetPolygonData()	const	{ return mPolygonData;	}
		inline_	udword*			GetPolygonData()			{ return mPolygonData;	}

		protected:
				udword			mNbVerts;
				Point*			mVerts;
				udword			mNbPolygons;
				udword*			mPolygonData;
				bool*			mInternal;
	};

#endif // ICECONVEXDATA_H
