///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for Euler angles.
 *	\file		IceEulerAngles.h
 *	\author		Pierre Terdiman
 *	\date		August, 20, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEEULERANGLES_H
#define ICEEULERANGLES_H

	// Forward declarations
	class Matrix3x3;
	class Quat;

	class ICEMATHS_API EulerAngles : public Allocateable
	{
		public:
							EulerAngles(float yaw=0.0f, float pitch=0.0f, float roll=0.0f);
							~EulerAngles();

		inline_	float		GetYaw()						const	{ return mYaw;										}
		inline_	float		GetPitch()						const	{ return mPitch;									}
		inline_	float		GetRoll()						const	{ return mRoll;										}

		inline_	void		SetYawLimits(float min, float max)		{ mYawMin = min;	mYawMax = max;					}
		inline_	void		SetPitchLimits(float min, float max)	{ mPitchMin = min;	mPitchMax = max;				}
		inline_	void		SetRollLimits(float min, float max)		{ mRollMin = min;	mRollMax = max;					}

		inline_	void		SetYawUnlimited()						{ mYawMin = MIN_FLOAT;		mYawMax = MAX_FLOAT;	}
		inline_	void		SetPitchUnlimited()						{ mPitchMin = MIN_FLOAT;	mPitchMax = MAX_FLOAT;	}
		inline_	void		SetRollUnlimited()						{ mRollMin = MIN_FLOAT;		mRollMax = MAX_FLOAT;	}

				void		SetUnlimited()
							{
								SetYawUnlimited();
								SetPitchUnlimited();
								SetRollUnlimited();
							}

				void		SetYaw(float yaw);
				void		SetPitch(float pitch);
				void		SetRoll(float roll);

				void		IncYaw(float step);
				void		IncPitch(float step);
				void		IncRoll(float step);

				operator	Matrix3x3()						const;
				void		Set(const Quat& q, bool roll=false);

		protected:
				float		mYaw;		//!< a.k.a. azimuth
				float		mPitch;		//!< a.k.a. elevation
				float		mRoll;
		// Yaw limits
				float		mYawMin;
				float		mYawMax;
		// Pitch limits
				float		mPitchMin;
				float		mPitchMax;
		// Roll limits
				float		mRollMin;
				float		mRollMax;
		// Internal methods
		inline_	void		ClampYaw();
		inline_	void		ClampPitch();
		inline_	void		ClampRoll();
	};

#endif // ICEEULERANGLES_H
