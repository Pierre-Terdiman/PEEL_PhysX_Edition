///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains source code from the article "Efficient Creation Of Triangle Strips".
 *	\file		IceStriper.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESTRIPER_H
#define ICESTRIPER_H

	//! Striper creation structure
	struct MESHMERIZER_API STRIPERCREATE
	{
		//! Constructor
										STRIPERCREATE();

				udword					NbFaces;			//!< Number of faces in source topo
				const udword*			DFaces;				//!< List of faces (dwords) or null
				const uword*			WFaces;				//!< List of faces (words) or null
				bool					AskForWords;		//!< true => results are in words (else dwords)
				bool					OneSided;			//!< true => create one-sided strips
				bool					SGIAlgorithm;		//!< true => use the SGI algorithm, pick least connected faces first
				bool					ConnectAllStrips;	//!< true => create a single strip with void faces
	};

	//! Striper result structure
	struct MESHMERIZER_API STRIPERRESULT
	{
				udword					NbStrips;			//!< Number of strips created
				udword*					StripLengths;		//!< Lengths of the strips (NbStrips values)
				void*					StripRuns;			//!< The strips in words or dwords, depends on AskForWords
				bool					AskForWords;		//!< true => results are in words (else dwords)
				float					Ratio;				//!< Total number of indices in strips / (number of faces * 3)
	};

	class MESHMERIZER_API Striper
	{
		public:
										Striper();
										~Striper();

				bool					Init(const STRIPERCREATE& create);
				bool					Compute(STRIPERRESULT& result);
		private:
				Striper&				FreeUsedRam();
				udword					ComputeBestStrip(udword face);
				udword					TrackStrip(udword face, udword oldest, udword middle, udword* strip, udword* faces, bool* tags);
				bool					ConnectAllStrips(STRIPERRESULT& result);

				Adjacencies*			mAdj;				//!< Adjacency structures
#ifndef MSH_ADJACENCIES_INCLUDE_TOPOLOGY
				IndexedSurface*			mTopo;
#endif
				bool*					mTags;				//!< Face markers

				udword					mNbStrips;			//!< The number of strips created for the mesh
				CustomArray*			mStripLengths;		//!< Array to store strip lengths
				CustomArray*			mStripRuns;			//!< Array to store strip indices

				udword					mTotalLength;		//!< The length of the single strip
				CustomArray*			mSingleStrip;		//!< Array to store the single strip

		// Flags
				bool					mAskForWords;
				bool					mOneSided;
				bool					mSGIAlgorithm;
				bool					mConnectAllStrips;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Creates a triangle-list from a triangle strip. The triangle strip
	 *	can contain multiple strips linked together with degenerate faces.
	 *
	 *	\param		nb_indices			[in] number of indices in the strip
	 *	\param		word_strip_indices	[in] array of word indices, or null
	 *	\param		dword_strip_indices	[in] array of dword indices, or null
	 *	\param		word_dest			[out] word triangle list
	 *	\param		dword_dest			[out] dword triangle list
	 *	\param		bool				[in] parity of returned list
	 *	\return		number of triangles in returned list
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION MESHMERIZER_API udword Unstrip(udword nb_indices, const uword* word_strip_indices, const udword* dword_strip_indices, udword* word_dest, udword* dword_dest, bool parity);

#endif // ICESTRIPER_H
