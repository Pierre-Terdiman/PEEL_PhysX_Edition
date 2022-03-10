///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to create vertex valencies.
 *	\file		IceValency.h
 *	\author		Pierre Terdiman
 *	\date		February, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEVALENCY_H
#define ICEVALENCY_H

	//! Valencies creation structure
	struct MESHMERIZER_API VALENCIESCREATE
	{
		//! Constructor
								VALENCIESCREATE()	{ ZeroMemory(this, SIZEOFOBJECT); }

				udword			NbVerts;		//!< Number of vertices
				udword			NbFaces;		//!< Number of faces
		const	udword*			dFaces;			//!< List of faces (triangle list)
		const	uword*			wFaces;			//!< List of faces (triangle list)
				bool			AdjacentList;	//!< Compute list of adjacent vertices or not
	};

	class MESHMERIZER_API Valencies : public Allocateable
	{
		public:
		//! Constructor
								Valencies();
		//! Destructor
								~Valencies();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes valencies and adjacent vertices.
		 *	After the call, get results with the appropriate accessors.
		 *
		 *	\param		vc		[in] creation structure
		 *	\return		true if success.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool			Compute(const VALENCIESCREATE& vc);

		// Data access
		inline_	udword			GetNbVerts()				const	{ return mNbVerts;						}
		inline_	const udword*	GetValencies()				const	{ return mValencies;					}
		inline_	udword			GetValency(udword i)		const	{ return mValencies[i];					}
		inline_	const udword*	GetOffsets()				const	{ return mOffsets;						}
		inline_	udword			GetOffset(udword i)			const	{ return mOffsets[i];					}
		inline_	const udword*	GetAdjacentVerts()			const	{ return mAdjacentVerts;				}

		inline_	udword			GetNbNeighbors(udword i)	const	{ return mValencies[i];					}
		inline_	const udword*	GetNeighbors(udword i)		const	{ return &mAdjacentVerts[mOffsets[i]];	}

		// Stats
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the number of bytes used.
		 *	\return		number of used bytes
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				udword			GetUsedRam()				const;
		private:
				udword			mNbVerts;		//!< Number of vertices
				udword			mNbAdjVerts;	//!< Total number of adjacent vertices
				udword*			mValencies;		//!< A list of mNbVerts valencies (= number of neighbors)
				udword*			mOffsets;		//!< A list of mNbVerts offsets in mAdjacentVerts
				udword*			mAdjacentVerts;	//!< List of adjacent vertices
	};

#endif // ICEVALENCY_H

