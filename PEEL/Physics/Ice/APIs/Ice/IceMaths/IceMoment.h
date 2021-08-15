///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains moment code.
 *	\file		IceMoment.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMOMENT_H
#define ICEMOMENT_H

	//! Moment of a triangle
	class ICEMATHS_API Moment : public Allocateable
	{
		public:
		//! Constructor
							Moment()		{}
		//! Destructor
							~Moment()		{}

		inline_	void		Clear()					{ ZeroMemory(this, SIZEOFOBJECT);	}
		inline_	void		Mean(Point& m)			{ m = mCentroid;					}
		inline_	void		WeightedMean(Point& m)	{ m = mCentroid / mArea;			}

				void		AddMoment(const Moment& m)
							{
								mCentroid	+= m.mCentroid * m.mArea;
								mCovariance	+= m.mCovariance;
								mArea		+= m.mArea;
							}

				void		Covariance(Matrix3x3& m)
							{
								for(udword i=0;i<3;i++)
								{
									for(udword j=0;j<3;j++)
									{
										m.m[i][j] = mCovariance.m[i][j] - mCentroid[i] * mCentroid[j] / mArea;
									}
								}
							}
		public:
				float		mArea;				//!< Triangle area
				Point		mCentroid;			//!< Triangle center
				Matrix3x3	mCovariance;		//!< Second-order components of the covariance matrix
	};

#endif // ICEMOMENT_H
