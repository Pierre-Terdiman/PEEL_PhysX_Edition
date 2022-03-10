///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	This file contains code to compute inertia tensors.
 *	\file		IceInertiaTensor.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEINERTIATENSOR_H
#define ICEINERTIATENSOR_H

	// A simple template mapping a Matrix3x3
	struct ICEMATHS_API InertiaTensor : public Allocateable
	{
		float xx, xy, xz;
		float yx, yy, yz;
		float zx, zy, zz;
	};

	class ICEMATHS_API InertiaTensorComputer : public Allocateable
	{
		public:
							InertiaTensorComputer();
							~InertiaTensorComputer();

				void		Reset();
				bool		Set(float mass, const Point& cog, float xx, float yy, float zz, float xy, float xz, float yz);
				bool		AdjustMass(float newmass);

				bool		Add(const InertiaTensorComputer& it);

				void		Add(const Point& offset, float mass, const Matrix3x3& ig);
				void		AddSphere(const Point& offset, float density, float radius);
				void		AddBox(float density, const Point& min, const Point& max);
				void		AddEllipsoid(const Point& offset, float density, float rx, float ry, float rz);
				void		AddCylinder(const Point& offset, float density, int dir, float r, float l);
				void		AddCone(const Point& offset, float density, int dir, float r, float l);

		inline_	Point		GetCenterOfGravity()			const	{ return mG / -mMass;								}
		inline_	float		GetMass()						const	{ return mMass;										}
		inline_	Matrix3x3	GetInertia()					const	{ return TransI(mI, mMass, GetCenterOfGravity());	}

				bool		Check()							const;
		private:
				float		mMass;
				Matrix3x3	mI;
				Point		mG;

		static	Matrix3x3	TransI(const Matrix3x3& ig, float mass, const Point& offset);
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Compute instantaneous inertia.
	 *	\param		inertia			[out] instantaneous inertia tensor (or inverse tensor)
	 *	\param		rotation		[in] instantaneous rotation
	 *	\param		local_inertia	[in] or inverse local inertia
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION ICEMATHS_API	void	ComputeInstantInertia(Matrix3x3& inertia, const Matrix3x3& rotation, const Point& local_inertia);

#endif	// ICEINERTIATENSOR_H

