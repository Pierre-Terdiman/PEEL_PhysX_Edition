#pragma once

#include <stdint.h>
#include "Handles.h"
#include "Continuity.h"
#include "InteropMath.h"

namespace Bepu
{
	/// <summary>
	/// Describes the properties of a static object. When added to a simulation, static objects can collide but have no velocity and will not move in response to forces.
	/// </summary>
	struct StaticDescription
	{
		/// <summary>
		/// Position and orientation of the static.
		/// </summary>
		RigidPose Pose;
		/// <summary>
		/// Shape of the static.
		/// </summary>
		TypedIndex Shape;
		/// <summary>
		/// Continuous collision detection settings for the static.
		/// </summary>
		ContinuousDetection Continuity;

		/// <summary>
		/// Builds a new static description.
		/// </summary>
		/// <param name="pose">Pose of the static collidable.</param>
		/// <param name="shape">Shape of the static.</param>
		/// <param name="continuity">Continuous collision detection settings for the static.</param>
		static StaticDescription Create(RigidPose pose, TypedIndex shape, ContinuousDetection continuity)
		{
			StaticDescription description;
			description.Pose = pose;
			description.Shape = shape;
			description.Continuity = continuity;
			return description;
		}

		/// <summary>
		/// Builds a new static description with <see cref="ContinuousDetectionMode.Discrete"/> continuity.
		/// </summary>
		/// <param name="pose">Pose of the static collidable.</param>
		/// <param name="shape">Shape of the static.</param>
		static StaticDescription Create(RigidPose pose, TypedIndex shape)
		{
			StaticDescription description;
			description.Pose = pose;
			description.Shape = shape;
			description.Continuity = ContinuousDetection::Discrete();
			return description;
		}

		/// <summary>
		/// Builds a new static description.
		/// </summary>
		/// <param name="position">Position of the static.</param>
		/// <param name="orientation">Orientation of the static.</param>
		/// <param name="shape">Shape of the static.</param>
		/// <param name="continuity">Continuous collision detection settings for the static.</param>
		static StaticDescription Create(Vector3 position, Quaternion orientation, TypedIndex shape, ContinuousDetection continuity)
		{
			StaticDescription description;
			description.Pose.Position = position;
			description.Pose.Orientation = orientation;
			description.Shape = shape;
			description.Continuity = continuity;
			return description;
		}

		/// <summary>
		/// Builds a new static description with <see cref="ContinuousDetectionMode.Discrete"/> continuity.
		/// </summary>
		/// <param name="position">Position of the static.</param>
		/// <param name="orientation">Orientation of the static.</param>
		/// <param name="shape">Shape of the static.</param>
		static StaticDescription Create(Vector3 position, Quaternion orientation, TypedIndex shape)
		{
			StaticDescription description;
			description.Pose.Position = position;
			description.Pose.Orientation = orientation;
			description.Shape = shape;
			description.Continuity = ContinuousDetection::Discrete();
			return description;
		}
	};


	/// <summary>
	/// Stores data for a static collidable in the simulation. Statics can be posed and collide, but have no velocity and no dynamic behavior.
	/// </summary>
	/// <remarks>Unlike bodies, statics have a very simple access pattern. Most data is referenced together and there are no extreme high frequency data accesses like there are in the solver.
	/// Everything can be conveniently stored within a single location contiguously.</remarks>
	struct Static
	{
		/// <summary>
		/// Pose of the static collidable.
		/// </summary>
		RigidPose Pose;

		/// <summary>
		/// Continuous collision detection settings for this collidable. Includes the collision detection mode to use and tuning variables associated with those modes.
		/// </summary>
		/// <remarks>Note that statics cannot move, so there is no difference between <see cref="ContinuousDetectionMode.Discrete"/> and <see cref="ContinuousDetectionMode.Passive"/> for them.
		/// Enabling <see cref="ContinuousDetectionMode.Continuous"/> will still require that pairs associated with the static use swept continuous collision detection.</remarks>
		ContinuousDetection Continuity;

		/// <summary>
		/// Index of the shape used by the static. While this can be changed, any transition from shapeless->shapeful or shapeful->shapeless must be reported to the broad phase. 
		/// If you need to perform such a transition, consider using <see cref="Statics.SetShape"/> or Statics.ApplyDescription; those functions update the relevant state.
		/// </summary>
		TypedIndex Shape;
		//Note that statics do not store a 'speculative margin' independently of the contini
		/// <summary>
		/// Index of the collidable in the broad phase. Used to look up the target location for bounding box scatters. Under normal circumstances, this should not be set externally.
		/// </summary>
		int32_t BroadPhaseIndex;
	};
}