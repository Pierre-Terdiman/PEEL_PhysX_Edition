///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef LOADER_SHAPE_PROFILE_H
#define LOADER_SHAPE_PROFILE_H

	struct Pt : public Allocateable
	{
		inline_	Pt()									{}
		inline_	Pt(sdword x, sdword y) : mX(x), mY(y)	{}
		sdword	mX;
		sdword	mY;
	};

	enum AnchorType
	{
		ANCHOR_NEUTRAL,
		ANCHOR_MALE,
		ANCHOR_FEMALE,

		ANCHOR_FORCE_DWORD	= 0x7fffffff
	};

	struct Seg : public Allocateable
	{
		Pt			mMidPointOffset;
		AnchorType	mAnchor;
		bool		mUsed;
	};

	class ShapeProfile : public Allocateable
	{
		public:
				ShapeProfile(udword nb, const Pt* pts);
				~ShapeProfile();

//		AABB	mBounds;
		udword	mNbPts;
		Pt*		mPts;
		Seg*	mSegments;

		inline_	Pt ComputeMidPoint(udword index)	const
		{
			ASSERT(index<mNbPts);
			const Pt& p0 = mPts[index];
			const Pt& p1 = mPts[(index+1)%mNbPts];
			return Pt((p0.mX + p1.mX)/2, (p0.mY + p1.mY)/2);
		}

		inline_	Pt ComputeSegmentHandle(udword index)	const
		{
			const Pt Midpoint = ComputeMidPoint(index);
			const Pt Offset = mSegments[index].mMidPointOffset;
			return Pt(Midpoint.mX + Offset.mX, Midpoint.mY + Offset.mY);
		}
	};

#endif
