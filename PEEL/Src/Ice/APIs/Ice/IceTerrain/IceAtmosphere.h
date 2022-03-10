/* (c) 2002 Nathaniel Hoffman, Kenneth J. Mitchell and Arcot J. Preetham */

#ifndef ICEATMOSPHERE_H
#define ICEATMOSPHERE_H

	//! Various Atmosphere parameters.
	typedef enum {
		eAtmBetaRayMultiplier,
		eAtmBetaMieMultiplier,
		eAtmInscatteringMultiplier,
		eAtmExtinctionMultiplier,
		eAtmHGg,
	} AtmosphereParams;

	class ICETERRAIN_API Atmosphere
	{
		public:
								Atmosphere(); 
								~Atmosphere()	{}

				void			SetParam(AtmosphereParams param, float fValue);
				float			GetParam(AtmosphereParams param);

		inline_	const Point&	GetBetaRayleigh()				const	{ return m_vBetaRay;		}
		inline_ const Point&	GetBetaDashRayleigh()			const	{ return m_vBetaDashRay;	}
		inline_ const Point&	GetBetaMie()					const	{ return m_vBetaMie;		}
		inline_ const Point&	GetBetaDashMie()				const	{ return m_vBetaDashMie;	}
				
				void			Interpolate(Atmosphere *one, Atmosphere *two, float f);

				Atmosphere		operator + (const Atmosphere&)	const;
				Atmosphere		operator * (float)				const;
		friend	Atmosphere		operator * (float f, const Atmosphere& a)
								{
									Atmosphere res;
									res.m_fBetaRayMultiplier = a.m_fBetaRayMultiplier * f;
									res.m_fBetaMieMultiplier = a.m_fBetaMieMultiplier * f;
									res.m_fInscatteringMultiplier = a.m_fInscatteringMultiplier * f;
									res.m_fExtinctionMultiplier = a.m_fExtinctionMultiplier * f;
									res.m_fHGg = a.m_fHGg * f;
									return res;
								}
		private:
				float		m_fHGg;		// g value in Henyey Greenstein approximation function.

		// Ideally, the following multipliers should all be 1.0, but provided here for study.
		// The final color of an object in atmosphere is sum of inscattering term and extinction term. 
				float		m_fInscatteringMultiplier;	// Multiply inscattering term with this factor.
				float		m_fExtinctionMultiplier;	// Multiply extinction term with this factor.
				float		m_fBetaRayMultiplier;		// Multiply Rayleigh scattering coefficient with this factor
				float		m_fBetaMieMultiplier;		// Multiply Mie scattering coefficient with this factor

				Point		m_vBetaRay;				// Rayleigh scattering coeff
				Point		m_vBetaDashRay;			// Rayleigh Angular scattering coeff without phase term.
				Point		m_vBetaMie;				// Mie scattering coeff
				Point		m_vBetaDashMie;			// Mie Angular scattering coeff without phase term.

		// Internal methods
				void		CalculateScatteringConstants();
	};

//	Atmosphere operator * ( float , const Atmosphere& );

#endif	// ICEATMOSPHERE_H