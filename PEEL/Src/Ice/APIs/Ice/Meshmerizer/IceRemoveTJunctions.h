///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to remove T-junctions. This is a wrapper around John Ratcliff's code.
 *	\file		IceRemoveTJunctions.h
 *	\author		Pierre Terdiman
 *	\date		December, 12, 2009
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEREMOVETJUNCTIONS_H
#define ICEREMOVETJUNCTIONS_H

	class MESHMERIZER_API REMOVETJUNCTIONSCREATE
	{
		public:
								REMOVETJUNCTIONSCREATE();
		// Input
				float			mEpsilon;
				udword			mVcount;		// input vertice count.
				const float*	mVertices;		// input vertices as PxF32s or...
				udword			mTcount;		// number of input triangles.
				const udword*	mIndices;		// triangle indices.
				const udword*	mIds;			// optional triangle Id numbers.
		// Output
				udword			mTcountOut;		// number of output triangles.
				const udword*	mIndicesOut;	// output triangle indices
				const udword*	mIdsOut;		// output retained id numbers.
	};

	// Removes t-junctions from an input mesh. Does not generate any new data points, but may possible produce additional triangles and new indices.
	FUNCTION MESHMERIZER_API void*	CreateTJunctionsRemover();
	FUNCTION MESHMERIZER_API void	ReleaseTJunctionsRemover(void* ptr);
	FUNCTION MESHMERIZER_API udword RemoveTJunctions(REMOVETJUNCTIONSCREATE& create, void* ptr);	// returns number of triangles output and the descriptor is filled with the appropriate results.

#endif // ICEREMOVETJUNCTIONS_H
