///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for modified Butterfly (subdivision surfaces)
 *	\file		IceButterfly.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEBUTTERFLY_H
#define ICEBUTTERFLY_H

	class MESHMERIZER_API ButterflyScheme : public SubdivScheme
	{
		public:
												ButterflyScheme();
		virtual									~ButterflyScheme();

		protected:

								const Point*	mVerts;
#ifndef MSH_ADJACENCIES_INCLUDE_TOPOLOGY
						const IndexedTriangle*	mFaces;
#endif
								Valencies*		mValencies;
								Adjacencies*	mAdj;
		// Internal methods
								bool			ComputeRegularVertex		(udword face_id, udword vref0, udword vref1, Point& p, SubdivHistory* history);
								bool			ComputeExtraordinaryVertex	(udword face_id, udword vref0, udword vref1, Point& p, SubdivHistory* history);
								bool			ComputeVirtualVertex		(udword face_id, udword vref0, udword vref1, Point& p, SubdivHistory* history);
								bool			ComputeVirtualVertices		(udword face_id, udword vref0, udword vref1, Point* regular, SubdivHistory* histories);

								bool			IsOnBoundary				(udword face_id, udword vref0, udword vref1, udword& op, bool ccw);
								bool			GetOrderedNeighbors			(udword face_id, udword vref0, udword vref1, udword* neighbors, udword* id=null);
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Initializes the subdivision scheme.
		 *	\param		surface		[in] the surface to subdivide
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(SubdivScheme)	bool			Init(IndexedSurface& surface);
		override(SubdivScheme)	bool			ComputeNewVertex(udword face_id, udword vref0, udword vref1, Point& p, SubdivHistory* history);
	};

#endif // ICEBUTTERFLY_H
