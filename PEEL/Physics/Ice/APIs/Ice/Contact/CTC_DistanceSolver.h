///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a distance solver
 *	\file		CTC_DistanceSolver.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCDISTANCESOLVER_H
#define CTCDISTANCESOLVER_H

	class CONTACT_API DistanceSolver
	{
		public:
								DistanceSolver();
		virtual					~DistanceSolver();

				float			Solve(float target_distance);
		virtual	float			ComputeDistance(float current_time)	= 0;

		private:

		inline_	bool			ValidDistance(float current_dist, float target_dist)
								{
									return (fabsf(current_dist - target_dist) < mEpsilon);
								}

				float			Solve(float t0, float t1, float value, float v0, float v1);

				float			mEpsilon;
				udword			mMaxNbIter;
	};

	class CONTACT_API BoxBoxDistanceSolver : public DistanceSolver
	{
		public:
								BoxBoxDistanceSolver();
		virtual					~BoxBoxDistanceSolver();

		virtual	float			ComputeDistance(float current_time);

				DynamicPR		mPose0;
				DynamicPR		mPose1;
				Point			mExtents0;
				Point			mExtents1;
	};

#endif // CTCDISTANCESOLVER_H

