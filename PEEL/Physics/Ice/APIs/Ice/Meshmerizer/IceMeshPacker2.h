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
#ifndef ICEMESHPACKER2_H
#define ICEMESHPACKER2_H

	struct MESHMERIZER_API MPVertex
	{
		Point	p;
		Point	uv;
		Point	color;
	};

	class MESHMERIZER_API MeshPacker2
	{
		public:
									MeshPacker2();
									~MeshPacker2();

//				bool				Init(IndexedSurface* surface, Adjacencies* adj, udword nbbits, bool refpredictor, bool compression);
				bool				Init(udword nb_bits, bool ref_predictor, bool uvs, bool vcol);
				bool				AddFace(const IndexedTriangle& tri);
				bool				AddFace(udword ref0, udword ref1, udword ref2);
				bool				AddVertex(const MPVertex& vertex);

				bool				CompressMesh(CustomArray& packed_mesh);
				bool				DecompressMesh(CustomArray& packed_mesh);

		inline_	udword				GetNbVerts()		const	{ return mNbVerts;			}
		inline_	udword				GetNbFaces()		const	{ return mNbFaces;			}
		inline_	IndexedTriangle*	GetFaces()			const	{ return mDepackedFaces;	}
		inline_	Point*				GetVerts()			const	{ return mDepackedPts;		}
		inline_	Point*				GetUVs()			const	{ return mDepackedUVs;		}
		inline_	Point*				GetColors()			const	{ return mDepackedColors;	}

		private:
		// Internal data

			// Surface data
				Container			mFaces;
				Vertices			mVerts;
				Vertices			mUVs;
				Vertices			mColors;

				Adjacencies*		mAdj;				//!< Surface adjacencies

			// Geometry streams
				CustomArray*		mErrors;			//!< Quantized error vectors
				CustomArray*		mVRefs;				//!< Shared vertex (predicted) references
			// Topology streams
				CustomArray*		mEscape;			//!< Escape codes (1 bit/face)
				CustomArray*		mBounds;			//!< Boundaries (2 bits/face)
				CustomArray*		mWinding;			//!< Winding order (1 bit/face)
			//
				sdword*				mQuantized;			//!< Quantized error vectors
				sdword*				mQuantizedUVs;		//!< Quantized error vectors
				sdword*				mQuantizedColors;	//!< Quantized error vectors
				udword				mCurNbVerts;		//!< Current number of vertices

			// Compression data
				bool*				mFMarkers;			//!< Face markers
				bool*				mVMarkers;			//!< Vertex markers. True = has been exported
				udword*				mXRef;				//!< Vertices cross-ref table
				udword*				mPackedVerts;		//!< List of already packed vertices
				udword				mCurNbFaces;		//!< Current number of packed/depacked triangles

			// Decompression data
				udword				mNbVerts;
				udword				mNbFaces;
				IndexedTriangle*	mDepackedFaces;		//!< List of depacked triangles
				Point*				mDepackedPts;
				Point*				mDepackedUVs;
				Point*				mDepackedColors;

		// User-provided data
				udword				mNbBits;
				bool				mRefPredictor;
				bool				mCompressUVs;
				bool				mCompressVCol;

		// Private methods
				void				FreeUsedRam();
				bool				CreateAdjacencies();
				bool				QuantizeVertexCloud(const Container& cloud, Point& dequant_coeff, sdword*& quantized);
				void				OutputVertex(udword vref);
				void				RecoverVertex(udword index);
				void				_CompressVertex(udword link, udword prev_face, udword prev_edge);
				void				_DecompressVertex(udword prev_face, udword prev_edge);
				void				PredictPoint(const sdword* quantized, udword vref_last, udword vref0, udword vref1, sdword& x, sdword& y, sdword& z);
	};

	MESHMERIZER_API bool TestCompression(MultiSurface* surface);
	MESHMERIZER_API bool TestDecompression(MultiSurface* surface);

#endif // ICEMESHPACKER2_H
