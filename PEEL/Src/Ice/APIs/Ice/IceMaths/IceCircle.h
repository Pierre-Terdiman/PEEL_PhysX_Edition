///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for circles.
 *	\file		IceCircle.h
 *	\author		Pierre Terdiman, original code by Dave Eberly
 *	\date		April, 28, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECIRCLE_H
#define ICECIRCLE_H

	class ICEMATHS_API Circle : public Allocateable
	{
		public:
		//! Constructor
		inline_					Circle()		{}
		//! Destructor
		inline_					~Circle()		{}

		inline_	const Point&	U()														const	{ return mU;		}
		inline_	const Point&	V()														const	{ return mV;		}
		inline_	const Point&	N()														const	{ return mN;		}
		inline_	const Point&	Center()												const	{ return mCenter;	}
		inline_	const float&	Radius()												const	{ return mRadius;	}

				float			SqrDistance(const Point& point, Point* closest=null)	const;
		inline_	float			Distance(const Point& point, Point* closest=null)		const	{ return sqrtf(SqrDistance(point, closest));	}

				Point			mU, mV, mN;
				Point			mCenter;
				float			mRadius;
	};

#endif // ICECIRCLE_H
