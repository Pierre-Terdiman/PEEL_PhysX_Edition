///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for angle-axis.
 *	\file		IceAngleAxis.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEANGLEAXIS_H
#define ICEANGLEAXIS_H

	class ICEMATHS_API	AngleAxis : public Allocateable
	{
		public:

		//! Empty constructor.
							AngleAxis()													{}
		//! Constructor from floats.
							AngleAxis(float a, float x, float y, float z)				{ mAngle = a; mAxis.Set(x, y, z);	}
		//! Constructor from an angle and a vector.
							AngleAxis(float a, const Point& p) : mAngle(a), mAxis(p)	{ mAxis.Normalize();				}
		//! Constructor from a quaternion.
							AngleAxis(const Quat& q);
		//! Destructor
							~AngleAxis()												{}

		//! Clear the angle axis
		inline	AngleAxis&	Zero()														{ mAngle = 0.0; mAxis.Set(0.0f, 0.0f, 0.0f);	return *this;	}
		//! Assignment from floats.
		inline	AngleAxis&	Set(float angle, float x, float y, float z)					{ mAngle = angle; mAxis.Set(x, y, z);			return *this;	}
		//! Assignment from an angle and a vector.
		inline	AngleAxis&	Set(float angle, const Point& axis)							{ mAngle = angle; mAxis = axis;					return *this;	}
		//! Assignment from a quaternion.
				AngleAxis&	Set(const Quat& q);
		//! Sets the angle component only.
		inline	AngleAxis&	SetAngle(float angle)										{ mAngle = angle;								return *this;	}
		//! Sets the axis component only, from a Point.
		inline	AngleAxis&	SetAxis(const Point& axis)									{ mAxis = axis;									return *this;	}
		//! Sets the axis component only, from floats.
		inline	AngleAxis&	SetAxis(float x, float y, float z)							{ mAxis.Set(x, y, z);							return *this;	}

		//! Get the number of revolutions.
		inline	sdword		GetRevNum()													{ return (sdword)floor(0.5f * mAngle * INVPI);					}
		//! Set the number of revolutions.
		inline	void		SetRevNum(sdword revcount)									{ mAngle += revcount * TWOPI;									}
		//! Returns the axis' square magnitude.
		inline	float		SquareMagnitude()											{ return mAxis.SquareMagnitude();								}
		//! Returns the axis' magnitude.
		inline	float		Magnitude()													{ return mAxis.Magnitude();										}
		//! Normalizes the axis.
		inline	AngleAxis&	Normalize()													{ mAxis.Normalize();							return *this;	}
//		inline	bool		IsNormalized()												{ return mAxis.IsNormalized(); }

		// Cast operators
		//! Cast an AngleAxis to a Quat.
		inline				operator Quat()						const					{ return Quat(cosf(mAngle*0.5f), mAxis*sinf(mAngle*0.5f));		}
		//! Cast an AngleAxis to a Matrix4x4.
		inline				operator Matrix4x4()				const					{ return (Matrix4x4) (Quat) (*this);							}
		//! Cast an AngleAxis to a Matrix3x3.
		inline				operator Matrix3x3()				const					{ return (Matrix3x3) (Quat) (*this);							}

		// Arithmetic operators
		//! Operator
		inline	AngleAxis	operator+(const AngleAxis& a)		const					{ return AngleAxis(mAngle + a.mAngle, mAxis + a.mAxis);			}
		//! Operator
		inline	AngleAxis	operator-(const AngleAxis& a)		const					{ return AngleAxis(mAngle - a.mAngle, mAxis - a.mAxis);			}
		//! Operator
		inline	AngleAxis	operator*(float s)					const					{ return AngleAxis(mAngle * s, mAxis * s);						}
		//! Operator
friend	inline	AngleAxis	operator*(float s, const AngleAxis& a)						{ return AngleAxis(s * a.mAngle, s * a.mAxis );					}

		//! Operator
		inline	AngleAxis	operator/(float s)					const					{ s = 1.0f / s; return AngleAxis(mAngle * s, mAxis * s);		}
		//! Operator
		inline	float		operator|(const AngleAxis& a)		const					{ return mAxis | a.mAxis; }		// This is a mere dot product with the Axis component only!

		//! Operator
		inline	AngleAxis&	operator+=(const AngleAxis& a)								{ mAngle += a.mAngle; mAxis += a.mAxis;			return *this;	}
		//! Operator
		inline	AngleAxis&	operator-=(const AngleAxis& a)								{ mAngle -= a.mAngle; mAxis -= a.mAxis;			return *this;	}
		//! Operator
		inline	AngleAxis&	operator*=(float s)											{ mAngle *= s; mAxis *= s;						return *this;	}
		//! Operator
		inline	AngleAxis&	operator/=(float s)											{ s = 1.0f / s; mAngle *= s; mAxis *= s;		return *this;	}

		public:
		// Members
				float		mAngle;		//!< The angle.
				Point		mAxis;		//!< The Axis. Must be a unit vector.
	};

#endif // ICEANGLEAXIS_H
