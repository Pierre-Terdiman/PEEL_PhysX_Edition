#pragma once

#include <stdint.h>

namespace Bepu
{
	/// <summary>
	/// Defines how a collidable will handle collision detection in the presence of velocity.
	/// </summary>
	enum struct ContinuousDetectionMode : uint32_t
	{
		/// <summary>
		/// <para>No sweep tests are performed. Default speculative contact generation will occur within the speculative margin.</para>
		/// <para>The collidable's bounding box will not be expanded by velocity beyond the speculative margin.</para>
		/// <para>This is the cheapest mode. If a Discrete mode collidable is moving quickly and the maximum speculative margin is limited,
		/// the fact that its bounding box is not expanded may cause it to miss a collision even with a non-Discrete collidable.</para>
		/// </summary>
		Discrete = 0,
		/// <summary>
		/// <para>No sweep tests are performed. Default speculative contact generation will occur within the speculative margin.</para>
		/// <para>The collidable's bounding box will be expanded by velocity without being limited by the speculative margin.</para>
		/// <para>This is useful when a collidable may move quickly and does not itself require continuous detection, but there exist other collidables with continuous modes 
		/// that should avoid missing collisions.</para>
		/// </summary>
		Passive = 1,
		/// <summary>
		/// <para>Collision detection will start with a sweep test to identify a likely time of impact. Speculative contacts will be generated for the predicted collision.</para>
		/// <para>This mode can capture angular motion with very few ghost collisions. It can, however, miss secondary collisions that would have occurred due to the primary impact's velocity change.</para>
		/// </summary>
		Continuous = 2,
	};

	/// <summary>
	/// Defines how a collidable handles collisions with significant velocity.
	/// </summary>
	struct ContinuousDetection
	{
		/// <summary>
		/// The continuous collision detection mode.
		/// </summary>
		ContinuousDetectionMode Mode;

		/// <summary>
		/// If using <see cref="ContinuousDetectionMode.Continuous"/>, this defines the minimum progress that the sweep test will make when searching for the first time of impact.
		/// Collisions lasting less than <see cref="MinimumSweepTimestep"/> may be missed by the sweep test. Using larger values can significantly increase the performance of sweep tests.
		/// </summary>
		float MinimumSweepTimestep;

		/// <summary>
		/// If using <see cref="ContinuousDetectionMode.Continuous"/>, sweep tests will terminate if the time of impact region has been refined to be smaller than <see cref="SweepConvergenceThreshold"/>.
		/// Values closer to zero will converge more closely to the true time of impact, but for speculative contact generation larger values usually work fine.
		/// Larger values allow the sweep to terminate much earlier and can significantly improve sweep performance.
		/// </summary>
		float SweepConvergenceThreshold;

		/// <summary>
		/// Gets whether the continuous collision detection configuration will permit bounding box expansion beyond the calculated speculative margin.
		/// </summary>
		bool AllowExpansionBeyondSpeculativeMargin() { return Mode > ContinuousDetectionMode::Discrete; }

		/// <summary>
		/// <para>No sweep tests are performed. Default speculative contact generation will occur within the speculative margin.</para>
		/// <para>The collidable's bounding box will not be expanded by velocity beyond the speculative margin.</para>
		/// <para>This can be marginally cheaper than Passive modes if using a limited maximum speculative margin. If a Discrete mode collidable is moving quickly and the maximum speculative margin is limited,
		/// the fact that its bounding box is not expanded may cause it to miss a collision even with a non-Discrete collidable.</para>
		/// <para>Note that Discrete and Passive only differ if maximum speculative margin is restricted.</para>
		/// </summary>
		/// <returns>Detection settings for the given discrete configuration.</returns>
		static ContinuousDetection Discrete()
		{
			ContinuousDetection detection;
			detection.Mode = ContinuousDetectionMode::Discrete;
			detection.MinimumSweepTimestep = 0;
			detection.SweepConvergenceThreshold = 0;
			return detection;
		}

		/// <summary>
		/// <para>No sweep tests are performed. Default speculative contact generation will occur within the speculative margin.</para>
		/// <para>The collidable's bounding box and speculative margin will be expanded by velocity.</para>
		/// <para>This is useful when a collidable may move quickly and does not itself require continuous detection, but there exist other collidables with continuous modes that should avoid missing collisions.</para>
		/// </summary>
		/// <returns>Detection settings for the passive configuration.</returns>
		static ContinuousDetection Passive()
		{
			ContinuousDetection detection;
			detection.Mode = ContinuousDetectionMode::Passive;
			detection.MinimumSweepTimestep = 0;
			detection.SweepConvergenceThreshold = 0;
			return detection;
		}

		/// <summary>
		/// <para>Collision detection will start with a sweep test to identify a likely time of impact. Speculative contacts will be generated for the predicted collision.</para>
		/// <para>This mode can capture angular motion with very few ghost collisions. It can, however, miss secondary collisions that would have occurred due to the primary impact's velocity change.</para>
		/// </summary>
		/// <param name="minimumSweepTimestep">Minimum progress that the sweep test will make when searching for the first time of impact.
		/// Collisions lasting less than MinimumProgress may be missed by the sweep test. Using larger values can significantly increase the performance of sweep tests.</param>
		/// <param name="sweepConvergenceThreshold">Threshold against which the time of impact region is compared for sweep termination. 
		/// If the region has been refined to be smaller than SweepConvergenceThreshold, the sweep will terminate.
		/// Values closer to zero will converge more closely to the true time of impact, but for speculative contact generation larger values usually work fine.
		/// Larger values allow the sweep to terminate much earlier and can significantly improve sweep performance.</param>
		/// <returns>Detection settings for the given continuous configuration.</returns>
		static ContinuousDetection Continuous(float minimumSweepTimestep = 1e-3f, float sweepConvergenceThreshold = 1e-3f)
		{
			ContinuousDetection detection;
			detection.Mode = ContinuousDetectionMode::Continuous;
			detection.MinimumSweepTimestep = minimumSweepTimestep;
			detection.SweepConvergenceThreshold = sweepConvergenceThreshold;
			return detection;
		}
	};
}