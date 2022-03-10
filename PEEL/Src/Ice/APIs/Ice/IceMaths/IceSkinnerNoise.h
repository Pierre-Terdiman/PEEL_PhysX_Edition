///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A C++ version of Robert Skinner's noise.
 *	\file		IceSkinnerNoise.h
 *	\author		Pierre Terdiman, original code (C) 1989, 1991 Robert Skinner, Craig E. Kolb
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESKINNERNOISE_H
#define ICESKINNERNOISE_H

	class ICEMATHS_API SkinnerNoise : public Allocateable
	{
		public:
							SkinnerNoise();
							~SkinnerNoise();

		// Robert Skinner's Perlin-style "Noise" function
				float		Noise3(Point* point);
		// Vector-valued noise
				void		DNoise3(Point* point, Point* result);
		// Noise-based utils
				float		Chaos(Point* vec, int octaves);
				float		PAChaos(Point* vec, int octaves);
				float		Marble(Point* vec);
				void		VfBm(Point* vec, float omega, float lambda, int octaves, Point* ans);
				float		fBm(Point* vec, float omega, float lambda, int octaves);
				void		Wrinkled(Point* pos, float lambda, float omega, int octaves, Point* res);
				void		Windy(Point* pos, float windscale, float chaoscale, float bumpscale, int octaves, float tscale, float hscale, float offset, Point* res);
	};

#endif // ICESKINNERNOISE_H
