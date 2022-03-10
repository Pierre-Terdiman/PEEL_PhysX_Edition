///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for PRS. PRS = Position, Rotation, Scale.
 *	\file		IcePRS.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPRS_H
#define ICEPRS_H

	// Forward declarations
	class PR;

	class ICEMATHS_API PRS : public Allocateable
	{
		public:
		//! Constructor
		inline_			PRS()																				{}
		//! Constructor
		inline_			PRS(const Point& p, const Quat& r, const Point& s) : mPos(p), mRot(r), mScale(s)	{}
		//! Constructor
//						PRS(const PR& pr);
		//! Copy constructor
		inline_			PRS(const PRS& prs) : mPos(prs.mPos), mRot(prs.mRot), mScale(prs.mScale)			{}
		//! Destructor
		inline_			~PRS()																				{}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Sets identity PRS.
		 *	\return		Self-Reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				PRS&	Identity()
						{
							mPos.Zero();
							mRot.Identity();
							mScale = Point(1.0f, 1.0f, 1.0f);
							return *this;
						}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Checks whether the PRS is the identity PRS or not.
		 *	\return		true for identity PRS
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool	IsIdentity()	const
						{
							if(IR(mPos.x))				return false;
							if(IR(mPos.y))				return false;
							if(IR(mPos.z))				return false;
							if(IR(mRot.p.x))			return false;
							if(IR(mRot.p.y))			return false;
							if(IR(mRot.p.z))			return false;
							if(IR(mRot.w)!=IEEE_1_0)	return false;
							if(IR(mScale.x)!=IEEE_1_0)	return false;
							if(IR(mScale.y)!=IEEE_1_0)	return false;
							if(IR(mScale.z)!=IEEE_1_0)	return false;
							return true;
						}

		//! Operator for "if(PRS!=PRS)"
		inline_	bool	operator!=(const PRS& prs)	const
						{
							if(mPos!=prs.mPos)		return true;
							if(mRot!=prs.mRot)		return true;
							if(mScale!=prs.mScale)	return true;
							return false;
						}

		//! Cast a PRS to a PR.
						operator PR()	const;

				Point	mPos;
				Quat	mRot;
				Point	mScale;
	};

	class ICEMATHS_API PR : public Allocateable
	{
		public:
		//! Constructor
		inline_			PR()													{}
		//! Identity constructor		
		inline_			PR(IDENTITY) : mPos(0.0f, 0.0f, 0.0f), mRot(Idt)		{}
		//! Constructor
		inline_			PR(const Point& p, const Quat& r) : mPos(p), mRot(r)	{}
		//! Copy constructor
		inline_			PR(const PR& pr) : mPos(pr.mPos), mRot(pr.mRot)			{}
		//! Copy constructor
//		inline_			PR(const PRS& prs) : mPos(prs.mPos), mRot(prs.mRot)		{}	// Drop the scale
		//! Destructor
		inline_			~PR()													{}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Sets identity PR.
		 *	\return		Self-Reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				PR&		Identity()
						{
							mPos.Zero();
							mRot.Identity();
							return *this;
						}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Checks whether the PR is the identity PR or not.
		 *	\return		true for identity PR
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool	IsIdentity()	const
						{
							if(IR(mPos.x))				return false;
							if(IR(mPos.y))				return false;
							if(IR(mPos.z))				return false;
							if(IR(mRot.p.x))			return false;
							if(IR(mRot.p.y))			return false;
							if(IR(mRot.p.z))			return false;
							if(IR(mRot.w)!=IEEE_1_0)	return false;
							return true;
						}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Transforms PR data (not PRS) into a MAX matrix.
		 *	This is the same as:
		 *
		 *	mat = (Matrix4x4)pr->Rot;
		 *	mat.Transpose();
		 *	mat.SetTrans(pr->Pos);
		 *
		 *	But collapsing all those operations in a dedicated method is faster.
		 *
		 *	\param		mat			[out] the destination matrix
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				void	MakeMAXMatrix(Matrix4x4& mat)	const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Transforms PR data (not PRS) into a matrix.
		 *	This is the same as:
		 *
		 *	mat = (Matrix4x4)pr->Rot;
		 *	mat.SetTrans(pr->Pos);
		 *
		 *	i.e. same as
		 *
		 *	mat = (Matrix4x4)*pr;
		 *
		 *	But collapsing all those operations in a dedicated method is faster.
		 *
		 *	\param		mat			[out] the destination matrix
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				void	MakeMatrix(Matrix4x4& mat)	const;

		//!< Lerps
				void	Lerp(const PR& pr0, const PR& pr1, float t);

		//! Inverts the PR.
		inline_	void	Invert()
						{
							const Matrix3x3 tmp = mRot;
							mRot.Invert();
							mPos = Point(-(mPos|tmp[0]), -(mPos|tmp[1]), -(mPos|tmp[2]));
						}

		//! Operator for PR Plus = PR + PR.
		inline_	PR		operator+(const PR& p)			const	{ return PR(mPos + p.mPos, mRot * p.mRot);		}
		//! Operator for PR Minus = PR - PR.
		inline_	PR		operator-(const PR& p)			const	{ return PR(mPos - p.mPos, mRot / p.mRot);		}

		//! Operator for PR += PR.
		inline_	PR&		operator+=(const PR& p)					{ mPos+=p.mPos;	mRot*=p.mRot;	return *this;	}
		//! Operator for PR -= PR.
		inline_	PR&		operator-=(const PR& p)					{ mPos-=p.mPos;	mRot/=p.mRot;	return *this;	}

		//! Operator for PR *= PR.
		inline_	void	operator*=(const PR& father_pr)
						{
							// Rotation
							mRot = father_pr.mRot * mRot;

							// Translation
//							Matrix4x4 TmpMat = father_pr;
//							mPos *= TmpMat;

							// Same but faster:
							const Matrix3x3 TmpMat = father_pr.mRot;
							const float xp = mPos.x * TmpMat.m[0][0] + mPos.y * TmpMat.m[1][0] + mPos.z * TmpMat.m[2][0] + father_pr.mPos.x;
							const float yp = mPos.x * TmpMat.m[0][1] + mPos.y * TmpMat.m[1][1] + mPos.z * TmpMat.m[2][1] + father_pr.mPos.y;
							const float zp = mPos.x * TmpMat.m[0][2] + mPos.y * TmpMat.m[1][2] + mPos.z * TmpMat.m[2][2] + father_pr.mPos.z;
							mPos.x = xp;
							mPos.y = yp;
							mPos.z = zp;
/*
							const float* Run = &TmpMat.m[0][0];
							float xp = father_pr.mPos.x;
							float yp = father_pr.mPos.y;
							float zp = father_pr.mPos.z;
							xp += mPos.x * (*Run++);
							yp += mPos.x * (*Run++);
							zp += mPos.x * (*Run++);
							xp += mPos.y * (*Run++);
							yp += mPos.y * (*Run++);
							zp += mPos.y * (*Run++);
							xp += mPos.z * (*Run++);
							yp += mPos.z * (*Run++);
							zp += mPos.z * (*Run++);
							mPos.x = xp;
							mPos.y = yp;
							mPos.z = zp;*/
						}

		//! Operator for PR /= PR.
		inline_	void	operator/=(const PR& father_pr)
						{
							// Rotation
							mRot = mRot * father_pr.mRot;

							// Translation
							Matrix4x4 TmpMat = *this;
							mPos = father_pr.mPos * TmpMat;
						}

		//! Cast a PR to a Matrix4x4.
		inline_			operator Matrix4x4()		const
						{
							return Matrix4x4(Matrix3x3(mRot), mPos);
						}

		//! Cast a PR to a PRS.
		inline_			operator PRS()				const
						{
							return PRS(mPos, mRot, Point(1.0f, 1.0f, 1.0f));
						}

		//! Operator for "if(PR!=PR)"
		inline_	bool	operator!=(const PR& pr)	const
						{
							if(mPos!=pr.mPos)		return true;
							if(mRot!=pr.mRot)		return true;
							return false;
						}

				Point	mPos;
				Quat	mRot;
	};

#endif // ICEPRS_H
