///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	FACE chunk for ZB2 format.
 *	\file		ChunkFACE.h
 *	\author		Pierre Terdiman
 *	\date		September, 11, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CHUNKFACE_H
#define CHUNKFACE_H

	#define FACE_VERSION	1

	enum ZCB2_FACE_Flag
	{
		ZCB2_FACE_USE_WORDS	= (1<<0),
		ZCB2_FACE_NO_DELTA	= (1<<1),
	};

	class ZCB2_API FACEChunk : public BaseChunk
	{
		DECLARE_CHUNK(FACEChunk, mFACECore)

		// Data access
		inline_	udword			GetNbFaces()	const	{ return mNbFaces;	}
		inline_	const udword*	GetDFaces()		const	{ return mDFaces;	}
		inline_	const uword*	GetWFaces()		const	{ return mWFaces;	}

		// Chunk definition

		// Global setup
				bool			SetFaces(udword nb_faces, const udword* dfaces, const uword* wfaces);
		// Iterative setup
				bool			SetNbFaces(udword nb_faces, bool use_words);
				bool			SetFace(udword i, udword ref0, udword ref1, udword ref2);
				bool			SetFace(udword i, uword ref0, uword ref1, uword ref2);
		private:
		// Chunk data
				udword			mNbFaces;
				udword*			mDFaces;
				uword*			mWFaces;
	};

#endif // CHUNKFACE_H
