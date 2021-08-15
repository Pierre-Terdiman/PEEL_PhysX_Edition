///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains IK-related code.
 *	\file		IceIK.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEIK_H
#define ICEIK_H

	class IKChain;

	class ICEMATHS_API IKBone : public Allocateable
	{
		private:
							IKBone(float length);
							~IKBone();

//		inline_	IKBone*		GetParent()		{ return mParent;		}
//		inline_	IKBone*		GetChild()		{ return mChild;		}
//		inline_	float		GetLength()		{ return mLength;		}
//
//		private:
		public:
				IKBone*		mParent;		//!< Parent bone
				IKBone*		mChild;			//!< Child bone

				float		mLength;		//!< Bone's length

				Point		mPosition;
				Point		mDirection;
				Matrix3x3	mOrientMatrix;
		private:
		// Internal methods
				void		_ForwKineThis(IKBone* parent, const Matrix3x3& parentmat);

		friend	class		IKChain;
	};

	class ICEMATHS_API IKChain : public Allocateable
	{
		public:
							IKChain();
							~IKChain();

				bool		AddBone(float length);
				bool		InvKine(const Point& targetpos);

		// Data access
		inline_	IKBone*		GetStartBone()		const	{ return mStart;		}
		inline_	IKBone*		GetEndBone()		const	{ return mEnd;			}
		inline_	udword		GetNbBones()		const	{ return mNbBones;		}

		private:
				IKBone*		mStart;			//!< First bone
				IKBone*		mEnd;			//!< Current end effector
				Point		mEndPos;		//!< Position of mEnd's end point
				udword		mNbBones;		//!< Number of bones
	};

#endif // ICEIK_H
