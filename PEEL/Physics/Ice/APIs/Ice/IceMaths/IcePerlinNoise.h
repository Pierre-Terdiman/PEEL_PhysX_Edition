///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Perlin noise in 3D from "Texturing & Modeling - a procedural approach"
 *	\file		IcePerlinNoise.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPERLINNOISE_H
#define ICEPERLINNOISE_H

	// Noise function as found on Hugo Elias' website
	inline_	float Noise2D(sdword x, sdword y)
	{
		sdword n = x + y * 57;
		n = (n<<13) ^ n;
		return ( 1.0f - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);    
	}

	// Noise function as found on Hugo Elias' website, modified by Kim Pallister to take a 3rd variable
	inline_	float Noise3D(sdword x, sdword y, sdword z)
	{
		sdword n = x + y * 57 + z * 131;
		n = (n<<13) ^ n;
		return ( 1.0f - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) * 0.000000000931322574615478515625f);    
	}

	// Constants
	#define PERLIN_B	256
	#define PERLIN_BM	255
	#define PERLIN_N	10000

	class ICEMATHS_API PerlinNoise : public Allocateable
	{
		public:
							PerlinNoise();					
							~PerlinNoise();
		// Computations
				float		Compute(const Point& vector)				const;	// Perlin noise
				float		Turbulence(const Point& vector, float freq)	const;	// Perlin's chaos function
		protected:
				udword		mP[PERLIN_B*2 + 2];								// Permutations
				float		mG3[PERLIN_B*2 + 2][3];							// Gradients
	};

#endif // ICEPERLINNOISE_H
