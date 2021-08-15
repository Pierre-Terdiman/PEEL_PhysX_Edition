///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains mesh compression code.
 *	\file		IceMeshPacker.h
 *	\author		Pierre Terdiman
 *	\date		August, 28, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMESHPACKER_H
#define ICEMESHPACKER_H

#ifdef OBSOLETE_MESH_PACKER

	class MESHMERIZER_API MeshPacker
	{
		public:
											MeshPacker();
											~MeshPacker();
		// 
						bool				Init(IndexedSurface* surface, Adjacencies* adj, udword nbbits, bool refpredictor, bool compression);

						bool				CompressMesh(CustomArray* packedmesh);
						bool				DecompressMesh(CustomArray* packedmesh);
		private:
		// Internal data

			// Surface data
						Adjacencies*		mAdj;				//!< Shortcut to surface adjacencies

			// Geometry streams
						CustomArray*		mErrors;			//!< Quantized error vectors
						CustomArray*		mVRefs;				//!< Shared vertex (predicted) references
			// Topology streams
						CustomArray*		mEscape;			//!< Escape codes (1 bit/face)
						CustomArray*		mBounds;			//!< Boundaries (2 bits/face)
						CustomArray*		mWinding;			//!< Winding order (1 bit/face)
			//
						sdword*				mQuantized;			//!< Quantized error vectors
						udword				mCurNbVerts;		//!< Current number of vertices

			// Compression data
						bool*				mFMarkers;			//!< Face markers
						bool*				mVMarkers;			//!< Vertex markers
						udword*				mXRef;				//!< Vertices cross-ref table
						udword*				mPackedVerts;		//!< List of already packed vertices

			// Decompression data
						udword				mCurNbFaces;		//!< Current number of depacked triangles
						IndexedTriangle*	mDepackedFaces;		//!< List of depacked triangles

		// User-provided data
						IndexedSurface*		mSurface;
						udword				mNbBits;
						bool				mRefPredictor;

		// Private methods
						void				_CompressVertex(udword link, udword prevface, udword prevedge);
						void				_DecompressVertex(udword prevface, udword prevedge);
						void				PredictPoint(udword vreflast, udword vref0, udword vref1, sdword& x, sdword& y, sdword& z);
	};
#endif

#endif // ICEMESHPACKER_H
