#pragma once

#include <stdint.h>
#include <limits>
#include "InteropMath.h"
#include "Handles.h"
#include "Continuity.h"

namespace Bepu
{
	/// <summary>
	/// Description of a collidable used by a body living in the broad phase and able to generate collision pairs.
	/// Collidables with a ShapeIndex that points to nothing (a default constructed <see cref="TypedIndex"/>) are not capable of colliding with anything.
	/// This can be used for a body which needs no collidable representation.
	/// </summary>
	struct Collidable
	{
		/// <summary>
		/// Index of the shape used by the body. While this can be changed, any transition from shapeless->shapeful or shapeful->shapeless must be reported to the broad phase. 
		/// If you need to perform such a transition, consider using <see cref="Bodies.SetShape"/> or <see cref="Bodies.ApplyDescription"/>; those functions update the relevant state.
		/// </summary>
		TypedIndex Shape;
		/// <summary>
		/// Continuous collision detection settings for this collidable. Includes the collision detection mode to use and tuning variables associated with those modes.
		/// </summary>
		ContinuousDetection Continuity;
		/// <summary>
		/// Lower bound on the value of the speculative margin used by the collidable.
		/// </summary>
		/// <remarks>0 tends to be a good default value. Higher values can be chosen if velocity magnitude is a poor proxy for speculative margins, but these cases are rare.
		/// In those cases, try to use the smallest value that still satisfies requirements to avoid creating unnecessary contact constraints.</remarks>
		float MinimumSpeculativeMargin;
		/// <summary>
		/// Upper bound on the value of the speculative margin used by the collidable.
		/// </summary>
		/// <remarks><see cref="float.MaxValue"/> tends to be a good default value for discrete or passive mode collidables. 
		/// The speculative margin will increase in size proportional to velocity magnitude, so having an unlimited maximum won't cost extra if the body isn't moving fast.
		/// <para>Smaller values can be useful for improving performance in chaotic situations where missing a collision is acceptable. When using <see cref="ContinuousDetectionMode.Continuous"/>, a speculative margin larger than the velocity magnitude will result in the sweep test being skipped, so lowering the maximum margin can help avoid ghost collisions.</para>
		/// </remarks>
		float MaximumSpeculativeMargin;

		/// <summary>
		/// Automatically computed size of the margin around the surface of the shape in which contacts can be generated. These contacts will have negative depth and only contribute if the frame's velocities
		/// would push the shapes of a pair into overlap. 
		/// <para>This is automatically set by bounding box prediction each frame, and is bound by the collidable's <see cref="MinimumSpeculativeMargin"/> and <see cref="MaximumSpeculativeMargin"/> values.
		/// The effective speculative margin for a collision pair can also be modified from <see cref="INarrowPhaseCallbacks"/> callbacks.</para>
		/// <para>This should be positive to avoid jittering.</para>
		/// <para>It can also be used as a form of continuous collision detection, but excessively high values combined with fast motion may result in visible 'ghost collision' artifacts. 
		/// For continuous collision detection with less chance of ghost collisions, use <see cref="ContinuousDetectionMode.Continuous"/>.</para>
		/// <para>If using <see cref="ContinuousDetectionMode.Continuous"/>, consider setting <see cref="MaximumSpeculativeMargin"/> to a smaller value to help filter ghost collisions.</para>
		/// <para>For more information, see the ContinuousCollisionDetection.md documentation.</para>
		/// </summary>
		float SpeculativeMargin;
		/// <summary>
		/// Index of the collidable in the broad phase. Used to look up the target location for bounding box scatters. Under normal circumstances, this should not be set externally.
		/// </summary>
		int32_t BroadPhaseIndex;
	};


	/// <summary>
	/// Linear and angular velocity for a body.
	/// </summary>
	struct BodyVelocity
	{
		/// <summary>
		/// Linear velocity associated with the body.
		/// </summary>
		Vector3 Linear;
		int32_t Pad0;
		/// <summary>
		/// Angular velocity associated with the body.
		/// </summary>
		Vector3 Angular;
		int32_t Pad1;

		BodyVelocity(Vector3 linear, Vector3 angular)
		{
			Linear = linear;
			Pad0 = 0;
			Angular = angular;
			Pad1 = 0;
		}

		BodyVelocity(Vector3 linear) : BodyVelocity(linear, Vector3()) {}

		BodyVelocity() : BodyVelocity(Vector3(), Vector3()) {}
	};

	/// <summary>
	/// Describes the pose and velocity of a body.
	/// </summary>
	struct MotionState
	{
		/// <summary>
		/// Pose of the body.
		/// </summary>
		RigidPose Pose;
		/// <summary>
		/// Linear and angular velocity of the body.
		/// </summary>
		BodyVelocity Velocity;
	};

	/// <summary>
	/// Stores the inertia for a body.
	/// </summary>
	/// <remarks>This representation stores the inverse mass and inverse inertia tensor. Most of the high frequency use cases in the engine naturally use the inverse.</remarks>

	struct BodyInertia
	{
		/// <summary>
		/// Inverse of the body's inertia tensor.
		/// </summary>
		Symmetric3x3 InverseInertiaTensor;
		/// <summary>
		/// Inverse of the body's mass.
		/// </summary>
		float InverseMass;
		uint32_t Pad;
	};

	/// <summary>
	/// Stores the local and world views of a body's inertia, packed together for efficient access.
	/// </summary>
	struct BodyInertias
	{
		/// <summary>
		/// Local inertia of the body.
		/// </summary>
		BodyInertia Local;
		/// <summary>
		/// Transformed world inertia of the body. Note that this is only valid between the velocity integration that updates it and the pose integration that follows.
		/// Outside of that execution window, this should be considered undefined.
		/// </summary>
		/// <remarks>
		/// We cache this here because velocity integration wants both the local and world inertias, and any integration happening within the solver will do so without the benefit of sequential loads.
		/// In that context, being able to load a single cache line to grab both local and world inertia helps quite a lot.</remarks>
		BodyInertia World;
	};

	/// <summary>
	/// Stores all body information needed by the solver together.
	/// </summary>
	struct BodyDynamics
	{
		/// <summary>
		/// Pose and velocity information for the body.
		/// </summary>
		MotionState Motion;
		/// <summary>
		/// Inertia information for the body.
		/// </summary>
		BodyInertias Inertia;
	};


	/// <summary>
	/// Describes how a body sleeps, and its current state with respect to sleeping.
	/// </summary>
	struct BodyActivity
	{
		/// <summary>
		/// Threshold of squared velocity under which the body is allowed to go to sleep. This is compared against dot(linearVelocity, linearVelocity) + dot(angularVelocity, angularVelocity).
		/// Setting this to a negative value guarantees the body cannot go to sleep without user action.
		/// </summary>
		float SleepThreshold;
		/// <summary>
		/// The number of time steps that the body must be under the sleep threshold before the body becomes a sleeping candidate.
		/// Note that the body is not guaranteed to go to sleep immediately after meeting this minimum.
		/// </summary>
		uint8_t MinimumTimestepsUnderThreshold;

		//Note that all values beyond this point are runtime set. The user should virtually never need to modify them. 
		//We do not constrain write access by default, instead opting to leave it open for advanced users to mess around with.
		//TODO: If people misuse these, we should internalize them in a case by case basis.

		/// <summary>
		/// If the body is awake, this is the number of time steps that the body has had a velocity below the sleep threshold.
		/// </summary>
		uint8_t TimestepsUnderThresholdCount;
		//Note that this flag is held alongside the other sleeping data, despite the fact that the traversal only needs the SleepCandidate state.
		//This is primarily for simplicity, but also note that the dominant accessor of this field is actually the sleep candidacy computation. Traversal doesn't visit every
		//body every frame, but sleep candidacy analysis does.
		//The reason why this flag exists at all is just to prevent traversal from being aware of the logic behind candidacy managemnt.
		//It doesn't cost anything extra to store this; it fits within the 8 byte layout.
		/// <summary>
		/// True if this body is a candidate for being slept. If all the bodies that it is connected to by constraints are also candidates, this body may go to sleep.
		/// </summary>
		bool SleepCandidate;
	};

	/// <summary>
	/// Describes a collidable and how it should handle continuous collision detection.
	/// </summary>
	struct CollidableDescription
	{
		/// <summary>
		/// Shape of the collidable.
		/// </summary>
		TypedIndex Shape;
		/// <summary>
		/// Continuous collision detection settings used by the collidable.
		/// </summary>
		ContinuousDetection Continuity;
		/// <summary>
		/// Lower bound on the value of the speculative margin used by the collidable.
		/// </summary>
		/// <remarks>0 tends to be a good default value. Higher values can be chosen if velocity magnitude is a poor proxy for speculative margins, but these cases are rare.
		/// In those cases, try to use the smallest value that still satisfies requirements to avoid creating unnecessary contact constraints.</remarks>
		float MinimumSpeculativeMargin;
		/// <summary>
		/// Upper bound on the value of the speculative margin used by the collidable.
		/// </summary>
		/// <remarks><see cref="float.MaxValue"/> tends to be a good default value for discrete or passive mode collidables. 
		/// The speculative margin will increase in size proportional to velocity magnitude, so having an unlimited maximum won't cost extra if the body isn't moving fast.
		/// <para>Smaller values can be useful for improving performance in chaotic situations where missing a collision is acceptable. When using <see cref="ContinuousDetectionMode.Continuous"/>, a speculative margin larger than the velocity magnitude will result in the sweep test being skipped, so lowering the maximum margin can help avoid ghost collisions.</para>
		/// </remarks>
		float MaximumSpeculativeMargin;

		/// <summary>
		/// Constructs a new collidable description.
		/// </summary>
		/// <param name="shape">Shape used by the collidable.</param>
		/// <param name="minimumSpeculativeMargin">Lower bound on the value of the speculative margin used by the collidable.</param>
		/// <param name="maximumSpeculativeMargin">Upper bound on the value of the speculative margin used by the collidable.</param>
		/// <param name="continuity">Continuous collision detection settings for the collidable.</param>
		CollidableDescription(TypedIndex shape, float minimumSpeculativeMargin, float maximumSpeculativeMargin, ContinuousDetection continuity)
		{
			Shape = shape;
			MinimumSpeculativeMargin = minimumSpeculativeMargin;
			MaximumSpeculativeMargin = maximumSpeculativeMargin;
			Continuity = continuity;
		}

		/// <summary>
		/// Constructs a new collidable description with <see cref="ContinuousDetectionMode.Discrete"/>.
		/// </summary>
		/// <param name="shape">Shape used by the collidable.</param>
		/// <param name="minimumSpeculativeMargin">Lower bound on the value of the speculative margin used by the collidable.</param>
		/// <param name="maximumSpeculativeMargin">Upper bound on the value of the speculative margin used by the collidable.</param>
		CollidableDescription(TypedIndex shape, float minimumSpeculativeMargin, float maximumSpeculativeMargin)
		{
			Shape = shape;
			MinimumSpeculativeMargin = minimumSpeculativeMargin;
			MaximumSpeculativeMargin = maximumSpeculativeMargin;
			Continuity = ContinuousDetection::Discrete();
		}

		/// <summary>
		/// Constructs a new collidable description. Uses 0 for the <see cref="MinimumSpeculativeMargin"/> .
		/// </summary>
		/// <param name="shape">Shape used by the collidable.</param>
		/// <param name="maximumSpeculativeMargin">Upper bound on the value of the speculative margin used by the collidable.</param>
		/// <param name="continuity">Continuous collision detection settings for the collidable.</param>
		CollidableDescription(TypedIndex shape, float maximumSpeculativeMargin, ContinuousDetection continuity)
		{
			Shape = shape;
			MinimumSpeculativeMargin = 0;
			MaximumSpeculativeMargin = maximumSpeculativeMargin;
			Continuity = continuity;
		}

		/// <summary>
		/// Constructs a new collidable description. Uses 0 for the <see cref="MinimumSpeculativeMargin"/> and <see cref="float.MaxValue"/> for the <see cref="MaximumSpeculativeMargin"/> .
		/// </summary>
		/// <param name="shape">Shape used by the collidable.</param>
		/// <param name="continuity">Continuous collision detection settings for the collidable.</param>
		CollidableDescription(TypedIndex shape, ContinuousDetection continuity)
		{
			Shape = shape;
			MinimumSpeculativeMargin = 0;
			MaximumSpeculativeMargin = std::numeric_limits<float>::max();
			Continuity = continuity;
		}

		/// <summary>
		/// Constructs a new collidable description with <see cref="ContinuousDetectionMode.Passive"/>. Will use a <see cref="MinimumSpeculativeMargin"/> of 0 and a <see cref="MaximumSpeculativeMargin"/> of <see cref="float.MaxValue"/>.
		/// </summary>
		/// <param name="shape">Shape used by the collidable.</param>
		/// <remarks><see cref="ContinuousDetectionMode.Passive"/> and <see cref="ContinuousDetectionMode.Discrete"/> are equivalent in behavior when the <see cref="MaximumSpeculativeMargin"/>  is <see cref="float.MaxValue"/> since they both result in the same (unbounded) expansion of body bounding boxes in response to velocity.</remarks>
		CollidableDescription(TypedIndex shape) : CollidableDescription(shape, 0, std::numeric_limits<float>::max(), ContinuousDetection::Passive())
		{
		}

		/// <summary>
		/// Constructs a new collidable description with <see cref="ContinuousDetectionMode.Discrete"/>. Will use a minimum speculative margin of 0 and the given maximumSpeculativeMargin.
		/// </summary>
		/// <param name="shape">Shape used by the collidable.</param>
		/// <param name="maximumSpeculativeMargin">Maximum speculative margin to be used with the discrete continuity configuration.</param>
		CollidableDescription(TypedIndex shape, float maximumSpeculativeMargin) : CollidableDescription(shape, 0, maximumSpeculativeMargin, ContinuousDetection::Discrete())
		{
		}
	};

	/// <summary>
	/// Describes the thresholds for a body going to sleep.
	/// </summary>
	struct BodyActivityDescription
	{
		/// <summary>
		/// Threshold of squared velocity under which the body is allowed to go to sleep. This is compared against dot(linearVelocity, linearVelocity) + dot(angularVelocity, angularVelocity).
		/// </summary>
		float SleepThreshold;
		/// <summary>
		/// The number of time steps that the body must be under the sleep threshold before the body becomes a sleep candidate.
		/// Note that the body is not guaranteed to go to sleep immediately after meeting this minimum.
		/// </summary>
		uint8_t MinimumTimestepCountUnderThreshold;

		/// <summary>
		/// Creates a body activity description.
		/// </summary>
		/// <param name="sleepThreshold">Threshold of squared velocity under which the body is allowed to go to sleep. This is compared against dot(linearVelocity, linearVelocity) + dot(angularVelocity, angularVelocity).</param>
		/// <param name="minimumTimestepCountUnderThreshold">The number of time steps that the body must be under the sleep threshold before the body becomes a sleep candidate.
		/// Note that the body is not guaranteed to go to sleep immediately after meeting this minimum.</param>
		BodyActivityDescription(float sleepThreshold, uint8_t minimumTimestepCountUnderThreshold = 32)
		{
			SleepThreshold = sleepThreshold;
			MinimumTimestepCountUnderThreshold = minimumTimestepCountUnderThreshold;
		}
	};

	/// <summary>
	/// Describes a body's state.
	/// </summary>
	struct BodyDescription
	{
		/// <summary>
		/// Position and orientation of the body.
		/// </summary>
		RigidPose Pose;
		/// <summary>
		/// Linear and angular velocity of the body.
		/// </summary>
		BodyVelocity Velocity;
		/// <summary>
		/// Mass and inertia tensor of the body.
		/// </summary>
		BodyInertia LocalInertia;
		/// <summary>
		/// Shape and collision detection settings for the body.
		/// </summary>
		CollidableDescription Collidable;
		/// <summary>
		/// Sleeping settings for the body.
		/// </summary>
		BodyActivityDescription Activity;

		/// <summary>
		/// Creates a dynamic body description.
		/// </summary>
		/// <param name="pose">Pose of the body.</param>
		/// <param name="velocity">Initial velocity of the body.</param>
		/// <param name="inertia">Local inertia of the body.</param>
		/// <param name="collidable">Collidable to associate with the body.</param>
		/// <param name="activity">Activity settings for the body.</param>
		/// <returns>Constructed description for the body.</returns>
		static BodyDescription CreateDynamic(RigidPose pose, BodyVelocity velocity, BodyInertia inertia, CollidableDescription collidable, BodyActivityDescription activity)
		{
			return BodyDescription{ pose, velocity, inertia, collidable, activity };
		}

		/// <summary>
		/// Creates a dynamic body description with zero initial velocity.
		/// </summary>
		/// <param name="pose">Pose of the body.</param>
		/// <param name="inertia">Local inertia of the body.</param>
		/// <param name="collidable">Collidable to associate with the body.</param>
		/// <param name="activity">Activity settings for the body.</param>
		/// <returns>Constructed description for the body.</returns>
		static BodyDescription CreateDynamic(RigidPose pose, BodyInertia inertia, CollidableDescription collidable, BodyActivityDescription activity)
		{
			return BodyDescription{ pose, BodyVelocity(), inertia, collidable, activity };
		}

		/// <summary>
		/// Creates a kinematic body description.
		/// </summary>
		/// <param name="pose">Pose of the body.</param>
		/// <param name="velocity">Initial velocity of the body.</param>
		/// <param name="collidable">Collidable to associate with the body.</param>
		/// <param name="activity">Activity settings for the body.</param>
		/// <returns>Constructed description for the body.</returns>
		static BodyDescription CreateKinematic(RigidPose pose, BodyVelocity velocity, CollidableDescription collidable, BodyActivityDescription activity)
		{
			return BodyDescription{ pose, velocity, {}, collidable, activity };
		}

		/// <summary>
		/// Creates a kinematic body description with zero initial velocity.
		/// </summary>
		/// <param name="pose">Pose of the body.</param>
		/// <param name="collidable">Collidable to associate with the body.</param>
		/// <param name="activity">Activity settings for the body.</param>
		/// <returns>Constructed description for the body.</returns>
		static BodyDescription CreateKinematic(RigidPose pose, CollidableDescription collidable, BodyActivityDescription activity)
		{
			return BodyDescription{ pose, {}, {}, collidable, activity };
		}
	};


	struct BodyConstraintReference
	{
		ConstraintHandle ConnectingConstraintHandle;
		int32_t BodyIndexInConstraint;
	};
}