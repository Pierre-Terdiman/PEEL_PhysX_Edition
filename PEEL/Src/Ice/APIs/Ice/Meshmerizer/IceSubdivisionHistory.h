///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains subdivision history code.
 *	\file		IceSubdivisionHistory.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESUBDIVISIONHISTORY_H
#define ICESUBDIVISIONHISTORY_H

	class MESHMERIZER_API SubdivHistory : public Allocateable
	{
		public:
								SubdivHistory();
								~SubdivHistory();
		// Recording
		inline_	void			SetNbOriginalVerts(udword nb)			{ mNbOriginalVerts = nb;					}
		inline_	SubdivHistory&	RecordNbVerts(udword nb)				{ Add(nb);					return *this;	}
		inline_	SubdivHistory&	RecordVertex(udword index, float coeff)	{ Add(coeff);	Add(index);	return *this;	}

		// Processing
				bool			Optimize();
				bool			Apply(const Point* source, Point* dest);
		// Data access
		inline_	udword			GetNbOriginalVerts()	const	{ return mNbOriginalVerts;	}
		inline_	Container*		GetHistory()			const	{ return mHistory;			}
				udword			GetNbVerts();
		private:
				udword			mNbOriginalVerts;	//!< Number of vertices before subdivision
				udword			mNbVerts;
				Container*		mHistory;
		// Internal methods
				udword			ComputeNbVerts();
				void			Add(udword value);
				void			Add(float value);
	};

#endif // ICESUBDIVISIONHISTORY_H
