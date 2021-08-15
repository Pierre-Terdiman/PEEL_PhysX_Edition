///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for Point-Triangle intersection
 *	\file		CTC_PointTriangleOverlap.h
 *	\author		Pierre Terdiman
 *	\date		January, 15, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCPOINTTRIANGLEOVERLAP_H
#define CTCPOINTTRIANGLEOVERLAP_H

	// From Mr Gamemaker
	// http://pub137.ezboard.com/fmrgamemakerfrm0.showMessage?topicID=2687.topic
	inline_ BOOL PointInTriangle(const Point& p, const Point& p0, const Point& edge10, const Point& edge20)
	{ 
		const float a = edge10|edge10;
		const float b = edge10|edge20;
		const float c = edge20|edge20;
		const float ac_bb = (a*c)-(b*b);

		const Point vp = p - p0;

		const float d = vp|edge10;
		const float e = vp|edge20;

		const float x = (d*c) - (e*b);
		const float y = (e*a) - (d*b);
		const float z = x + y - ac_bb;

		// Same as: if(x>0.0f && y>0.0f && z<0.0f)	return TRUE;
		//			else							return FALSE;
		return (( IR(z) & ~(IR(x)|IR(y)) ) & SIGN_BITMASK);
	}

	//! Dedicated 2D version
	inline_ BOOL PointInTriangle2D(	float px, float pz,
									float p0x, float p0z,
									float e10x, float e10z,
									float e20x, float e20z)
	{ 
		const float a = e10x*e10x + e10z*e10z;
		const float b = e10x*e20x + e10z*e20z;
		const float c = e20x*e20x + e20z*e20z;
		const float ac_bb = (a*c)-(b*b);

		const float vpx = px - p0x;
		const float vpz = pz - p0z;

		const float d = vpx*e10x + vpz*e10z;
		const float e = vpx*e20x + vpz*e20z;

		const float x = (d*c) - (e*b);
		const float y = (e*a) - (d*b);
		const float z = x + y - ac_bb;

		// Same as: if(x>0.0f && y>0.0f && z<0.0f)	return TRUE;
		//			else							return FALSE;
		return (( IR(z) & ~(IR(x)|IR(y)) ) & SIGN_BITMASK);
	}

#endif // CTCPOINTTRIANGLEOVERLAP_H

