///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	This file contains code to compute "screwings" (in Stéphane Redon-parlance).
 *	\file		IceScrewing.h
 *	\author		Pierre Terdiman
 *	\date		September, 17, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESCREWING_H
#define ICESCREWING_H

	class ICEMATHS_API Screwing : public Allocateable
	{
		public:
		inline_						Screwing()					{					}
		inline_						~Screwing()					{					}

				void				Init(const Point& velocity, const Point& omega, const Point& pivot);

		inline_	const Point&		GetAxis()			const	{ return mU;		}
		inline_	const Point&		GetOrigin()			const	{ return mO;		}
		inline_	float				GetRotation()		const	{ return mW;		}
		inline_	float				GetTranslation()	const	{ return mS;		}

		inline_	const Matrix4x4&	GetP()				const	{ return mP;		}
		inline_	const Matrix4x4&	GetInvP()			const	{ return mInvP;		}

		inline_ const Point&		GetVelocity()		const	{ return mVelocity;	}
		inline_ const Point&		GetOmega()			const	{ return mOmega;	}
		inline_ const Point&		GetPivot()			const	{ return mPivot;	}

		inline_	void				TransformPoint(Point& dest, const Point& world_pt, float t=1.0f)	const
									{
										// Transform to screw space
										TransformPoint4x3(dest, world_pt, mP);	// dest = world_pt * mP;

										// Perform screwing
										ComputeCosSin(t);
										// Prevent aliasing
										float Cos = mCachedCos;
										float Sin = mCachedSin;

										Point tmp;
										tmp.x = dest.x * Cos - dest.y * Sin;
										tmp.y = dest.x * Sin + dest.y * Cos;
										tmp.z = (mS*t) + dest.z;

										// Transform back to world space
										TransformPoint4x3(dest, tmp, mInvP);	// dest = tmp * mInvP;
									}

		inline_	void				TransformNormal(Point& dest, const Point& world_pt, float t=1.0f)	const
									{
										// Transform to screw space
										TransformPoint3x3(dest, world_pt, mP);	// dest = world_pt * Matrix3x3(mP);

										// Perform screwing
										ComputeCosSin(t);
										// Prevent aliasing
										float Cos = mCachedCos;
										float Sin = mCachedSin;

										Point tmp;
										tmp.x = dest.x * Cos - dest.y * Sin;
										tmp.y = dest.x * Sin + dest.y * Cos;
										tmp.z = dest.z;

										// Transform back to world space
										TransformPoint3x3(dest, tmp, mInvP);	// dest = tmp * Matrix3x3(mInvP);
									}

				void				TransformRot(Quat& dest, const Quat& source, float t) const;

		inline_	float				ComputeSquaredLength(const Point& world_pt)	const
									{
										// taille du chemin hélicoïdal dl = sqrt(s^2 + w^2(x^2 + y^2) * dt
										// s,w = screwing params
										// x,y = coordonnées de p dans le repère local du screwing

										// Transform to screw space
										Point s;
										TransformPoint4x3(s, world_pt, mP);

										return mS*mS + mW*mW*(s.x*s.x + s.y*s.y);
									}

		inline_	float				ComputeLength(const Point& world_pt)	const
									{
										return sqrtf(ComputeSquaredLength(world_pt));
									}
		private:
		// Input data - we keep them around so that we can compute relative velocities from the screwing.
		// This isn't part of the real screwing as described by Redon.
				Point				mVelocity;
				Point				mOmega;
				Point				mPivot;
		// Output data - typical screwing parameters in Redon's papers.
				Point				mU;			//!< Screwing axis
				Point				mO;			//!< Screwing origin
				float				mW;			//!< Amount of rotation
				float				mS;			//!< Amount of translation
		// Precomputed helpers
				Matrix4x4			mP;			//!< World-space to Screw-space
				Matrix4x4			mInvP;		//!< Screw-space to World-space

		inline_	void				ComputeCosSin(float t)	const
									{
										// Use the angle (since the cache is shared, time is not enough)
										const float Angle = mW*t;
										if(Angle!=mCachedAngle)
										{
											mCachedAngle = Angle;

											FSinCos(mCachedCos, mCachedSin, Angle);
										}
									}

		static	float				mCachedAngle;
		static	float				mCachedCos;
		static	float				mCachedSin;
	};

#endif	// ICESCREWING_H

