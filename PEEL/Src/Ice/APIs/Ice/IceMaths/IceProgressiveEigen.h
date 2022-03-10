///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for incremental computation of eigen values.
 *	\file		IceProgressiveEigen.h
 *	\author		Pierre Terdiman
 *	\date		October, 3, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPROGRESSIVEEIGEN_H
#define ICEPROGRESSIVEEIGEN_H

	class ICEMATHS_API ProgressiveEigen : public Allocateable
	{
		public:
									ProgressiveEigen();
									~ProgressiveEigen();

				bool				ComputeBestSortingAxis(udword nb, const Point* list);
		// Data access
		inline_	udword				GetFirstAxis()			const		{ return mFirstAxis;		}
		inline_	udword				GetSecondAxis()			const		{ return mSecondAxis;		}
		inline_	udword				GetThirdAxis()			const		{ return mThirdAxis;		}
		inline_	udword				GetUpdateFrequency()	const		{ return mUpdateFrequency;	}
		inline_	const Point*		GetEigenValues()		const		{ return &mEVals;			}
		inline_	const Matrix3x3*	GetEigenVectors()		const		{ return &mEVects;			}
		// Settings
		inline_	void				SetUpdateFrequency(udword freq)		{ mUpdateFrequency = freq;	}

		protected:
		// Result axes
				udword				mFirstAxis;					//!< Current best sorting axis (0=X, 1=Y, 2=Z)
				udword				mSecondAxis;				//!< Second best axis
				udword				mThirdAxis;					//!< Worst axis
		// Result eigen stuff
				Point				mEVals;						//!< Eigenvalues
				Matrix3x3			mEVects;					//!< Eigenvectors
		// Settings
				udword				mUpdateFrequency;			//!< Number of frames after which a new axis is selected. (0 = never)
				udword				mTimestamp;					//!< Inner timestamp
				udword				mNbCachedPoints;			//!< Number of cached points
				Point*				mCachedPoints;				//!< List of cached points
		// Cached variables
				float				mOneOverNb;					//!< Precomputed value
				Point				mMean;						//!< Cached mean point
				float				mSumXX, mSumXY, mSumXZ;		//!< Cached computation
				float				mSumYY, mSumYZ, mSumZZ;		//!< Cached computation
				udword				mFirstIndex, mLastIndex;	//!< Cached indices
				udword				mStep;						//!< Computation step
		// Internal methods
				bool				SortAxes();
	};

#endif	// ICEPROGRESSIVEEIGEN_H

