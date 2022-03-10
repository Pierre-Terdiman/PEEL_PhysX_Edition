#ifndef ICESUN_H
#define ICESUN_H

	class ICETERRAIN_API Sun
	{
		public:
						Sun(float theta=0.0f, float phi=0.0f, float intensity=1.0f); 
									// Theta is angle from zenith direction (Y axis)
									// Theta = 0 (Pos y), Theta = 180 (Neg Y)
									// Phi is angle in CW from x axis in XZ coordinate system.
									// eg. E(pos X) = 0, N(pos Z) = 90, W = 180, S = 270.
						~Sun()	{}

				Point	GetDirection()				const;
				Point	GetColor()					const;
				HPoint	GetColorAndIntensity()		const;
				Point	GetColorWithIntensity()		const;

		inline_	float	GetTheta()					const	{ return mTheta;				}
		inline_	float	GetPhi()					const	{ return mPhi;					}
		inline_	float	GetIntensity()				const	{ return mIntensity;			}

		inline_	void	SetTheta(float theta)				{ mTheta		= theta;		}
		inline_	void	SetPhi(float phi)					{ mPhi			= phi;			}
		inline_	void	SetIntensity(float intensity)		{ mIntensity	= intensity;	}

				void	Interpolate(const Sun& one, const Sun& two, float f);

				Sun		operator + (const Sun& s2)	const
						{
							return Sun(mTheta + s2.mTheta, mPhi + s2.mPhi, mIntensity + s2.mIntensity);
						}
				Sun		operator * (float f)		const
						{
							return Sun(mTheta * f, mPhi * f, mIntensity * f);
						}
		friend	Sun		operator * (float f, const Sun& s)
						{
							return Sun(s.mTheta * f, s.mPhi * f, s.mIntensity * f);
						}
		private:
				float	mTheta;
				float	mPhi;
				float	mIntensity;

		// Internal methods
				Point	ComputeAttenuation(float theta, int turbidity=2)	const;
	};

#endif	// ICESUN_H
