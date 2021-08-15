///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to find various roots.
 *	\file		IceRoots.h
 *	\author		Ken Turkowski
 *	\date		1997-1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEROOTS_H
#define ICEROOTS_H

	#ifdef DOUBLE_PRECISION
	# define ICE_FLOAT double
	#else
	# define ICE_FLOAT float
	#endif

	FUNCTION ICEMATHS_API	long	FindCubicRoots(const ICE_FLOAT coeff[4], ICE_FLOAT x[3]);
	FUNCTION ICEMATHS_API	long	FindQuadraticRoots(const ICE_FLOAT coeff[3], ICE_FLOAT re[2], ICE_FLOAT im[2]);
	FUNCTION ICEMATHS_API	void	FindPolynomialRoots(const ICE_FLOAT	*a,			// Coefficients
														ICE_FLOAT		*u,			// Real component of each root
														ICE_FLOAT		*v,			// Imaginary component of each root
														ICE_FLOAT		*conv,		// Convergence constant associated with each root
														register long	n,			// Degree of polynomial (order-1)
														long			maxiter,	// Maximum number of iterations
														long			fig			// The number of decimal figures to be computed
													);
	FUNCTION ICEMATHS_API	long	FindZerosOfFunction(ICE_FLOAT	(*func)(ICE_FLOAT),	// The function whose zeros are to be found
														ICE_FLOAT	*root,				// The roots
														long		n,					// The number of roots sought
														long		maxiter,			// Maximum number of iterations
														ICE_FLOAT	epsr,				// Relative convergence criterion on successive iterates
														ICE_FLOAT	epsf,				// Absolute convergence criterion on the function values
														ICE_FLOAT	epsmr,				// Closeness of multiple roots (see mrspr)
														ICE_FLOAT	mrspr				// The spread for multiple roots, that is, if |rt - root[i]| < epsmr,
																						// where root[i] is a previously found root,
																						// then rt is replaced by rt + mrspr.
													);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Computes real quadratic roots for equ. ax^2 + bx + c = 0
	 *
	 *	\param		a	[in] a coeff
	 *	\param		b	[in] b coeff
	 *	\param		c	[in] c coeff
	 *	\param		x0	[out] smallest real root
	 *	\param		x1	[out] largest real root
	 *	\return		number of real roots
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline_ udword FindRealQuadraticRoots(float a, float b, float c, float& x0, float& x1)
	{
		if(a==0.0f)
		{
			// bx + c = 0
			if(b!=0.0f)
			{
				x0 = -c / b;
				return 1;
			}
			else return 0;
		}

		// Compute delta
		float Delta = b*b - 4.0f*a*c;
		if(Delta < 0.0f)	return 0;	// Complex roots

		if(Delta==0.0f)
		{
			x0 = -b / (2.0f * a);
			return 1;
		}
		else
		{
			Delta = sqrtf(Delta);

			b = -b;
			a = 0.5f / a;

			b *= a;
			Delta *= a;

			x0 = b - Delta;
			x1 = b + Delta;

			if(x0>x1)	TSwap(x0,x1);

			return 2;
		}
	}

#endif // ICEROOTS_H

