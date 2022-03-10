///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to compute minimal bounding spheres.
 *	\file		IceMiniball.h
 *	\author		Pierre Terdiman, original code by Nicolas Capens
 *	\date		June, 29, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMINIBALL_H
#define ICEMINIBALL_H

	class ICEMATHS_API Miniball : public Allocateable
	{
		public:
		// Constructors
							Miniball();
							Miniball(const Miniball& X);
							Miniball(const Point& O);   // Point-Miniball
							Miniball(const Point& O, float R);   // Center and radius (not squared)
							Miniball(const Point& O, const Point& A);   // Miniball through two points
							Miniball(const Point& O, const Point& A, const Point& B);   // Miniball through three points
							Miniball(const Point& O, const Point& A, const Point& B, const Point& C);   // Miniball through four points
		// Destructor
		inline_				~Miniball()	{}

				Point		mCenter;
				float		mRadius;

				Miniball&	operator=(const Miniball& S);

				float		d(const Point& P) const;  // Distance from p to boundary of the Miniball
				float		d2(const Point& P) const;  // Square distance from p to boundary of the Miniball

		static	float		d(const Miniball& S, const Point& P);  // Distance from p to boundary of the Miniball
		static	float		d(const Point& P, const Miniball& S);  // Distance from p to boundary of the Miniball

		static	float		d2(const Miniball& S, const Point& P);  // Square distance from p to boundary of the Miniball
		static	float		d2(const Point& P, const Miniball& S);  // Square distance from p to boundary of the Miniball

		static	Miniball	miniBall(Point P[], unsigned int p);   // Smallest enclosing Miniball
		static	Miniball	smallBall(Point P[], unsigned int p);   // Enclosing Miniball approximation

		private:
		static	Miniball	recurseMini(Point* P[], unsigned int p, unsigned int b = 0);
	};

#endif	// ICEMINIBALL_H
