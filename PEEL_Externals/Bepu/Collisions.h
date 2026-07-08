#pragma once

#include <assert.h>
#include <stdint.h>
#include "InteropMath.h"
#include "Handles.h"
#include "Constraints.h"

namespace Bepu
{
	/// <summary>
	/// Represents how a collidable can interact and move.
	/// </summary>
	enum struct CollidableMobility : uint32_t
	{
		/// <summary>
		/// Marks a collidable as owned by a dynamic body.
		/// </summary>
		Dynamic = 0,
		/// <summary>
		/// Marks a collidable as owned by a kinematic body.
		/// </summary>
		Kinematic = 1,
		/// <summary>
		/// Marks the collidable as an independent immobile collidable.
		/// </summary>
		Static = 2
	};

	/// <summary>
	/// Uses a bitpacked representation to refer to a body or static collidable.
	/// </summary>
	struct CollidableReference
	{
		/// <summary>
		/// Bitpacked representation of the collidable reference.
		/// </summary>
		uint32_t Packed;

		/// <summary>
		/// Gets the mobility state of the owner of this collidable.
		/// </summary>
		CollidableMobility GetMobility() const
		{
			return (CollidableMobility)(Packed >> 30);
		}

		/// <summary>
		/// Gets the body handle of the owner of the collidable referred to by this instance.
		/// </summary>
		BodyHandle GetBodyHandle() const
		{
			assert(GetMobility() == CollidableMobility::Dynamic || GetMobility() == CollidableMobility::Kinematic);
			return BodyHandle{ GetRawHandleValue() };

		}

		/// <summary>
		/// Gets the static handle of the owner of the collidable referred to by this instance.
		/// </summary>
		StaticHandle GetStaticHandle() const
		{
			assert(GetMobility() == CollidableMobility::Static);
			return StaticHandle{ GetRawHandleValue() };
		}

		/// <summary>
		/// Gets the integer value of the handle of the owner of the collidable referred to by this instance.
		/// </summary>
		int32_t GetRawHandleValue() const
		{
			return (int)(Packed & 0x3FFFFFFF);
		}

		static CollidableReference CreateStatic(StaticHandle handle)
		{
			return CollidableReference{ ((uint32_t)CollidableMobility::Static << 30) | handle.Value };
		}
		static CollidableReference CreateDynamic(BodyHandle handle)
		{
			return CollidableReference{ ((uint32_t)CollidableMobility::Dynamic << 30) | handle.Value };
		}
		static CollidableReference CreateKinematic(BodyHandle handle)
		{
			return CollidableReference{ ((uint32_t)CollidableMobility::Kinematic << 30) | handle.Value };
		}
	};

	struct CollidablePair
	{
		CollidableReference A;
		CollidableReference B;
	};

	/// <summary>
	/// Information about a single contact in a convex collidable pair. Convex collidable pairs share one surface basis across the manifold, since the contact surface is guaranteed to be a plane.
	/// </summary>    
	struct ConvexContact
	{
		/// <summary>
		/// Offset from the position of collidable A to the contact position. 
		/// </summary>
		Vector3 Offset;
		/// <summary>
		/// Penetration depth between the two collidables at this contact. Negative values represent separation.
		/// </summary>
		float Depth;
		/// <summary>
		/// Id of the features involved in the collision that generated this contact. If a contact has the same feature id as in a previous frame, it is an indication that the
		/// same parts of the shape contributed to its creation. This is useful for carrying information from frame to frame.
		/// </summary>
		int32_t FeatureId;
	};

	/// <summary>
	/// Contains the data associated with a convex contact manifold.
	/// </summary>
	struct ConvexContactManifold
	{
		/// <summary>
		/// Offset from collidable A to collidable B.
		/// </summary>
		Vector3 OffsetB;
		int32_t Count;

		/// <summary>
		/// Surface normal shared by all contacts. Points from collidable B to collidable A.
		/// </summary>
		Vector3 Normal;

		ConvexContact Contacts[4];

		void ValidateIndex(int contactIndex)
		{
			assert(contactIndex >= 0 && contactIndex < Count);
		}
	};

	/// <summary>
	/// Information about a single contact in a nonconvex collidable pair.
	/// Nonconvex pairs can have different surface bases at each contact point, since the contact surface is not guaranteed to be a plane.
	/// </summary>
	struct NonconvexContact
	{
		/// <summary>
		/// Offset from the position of collidable A to the contact position. 
		/// </summary>
		Vector3 Offset;
		/// <summary>
		/// Penetration depth between the two collidables at this contact. Negative values represent separation.
		/// </summary>
		float Depth;
		/// <summary>
		/// Surface basis of the contact. If transformed into a rotation matrix, X and Z represent tangent directions and Y represents the contact normal. Points from collidable B to collidable A.
		/// </summary>
		Vector3 Normal;
		/// <summary>
		/// Id of the features involved in the collision that generated this contact. If a contact has the same feature id as in a previous frame, it is an indication that the
		/// same parts of the shape contributed to its creation. This is useful for carrying information from frame to frame.
		/// </summary>
		int32_t FeatureId;
	};

	/// <summary>
	/// Contains the data associated with a nonconvex contact manifold.
	/// </summary>
	struct NonconvexContactManifold
	{
		/// <summary>
		/// Offset from collidable A to collidable B.
		/// </summary>
		Vector3 OffsetB;
		int32_t Count;

		NonconvexContact Contacts[4];
	};

	/// <summary>
	/// Material properties governing the interaction between colliding bodies. Used by the narrow phase to create constraints of the appropriate configuration.
	/// </summary>
	struct PairMaterialProperties
	{
		/// <summary>
		/// Coefficient of friction to apply for the constraint. Maximum friction force will be equal to the normal force times the friction coefficient.
		/// </summary>
		float FrictionCoefficient;
		/// <summary>
		/// Maximum relative velocity along the contact normal at which the collision constraint will recover from penetration. Clamps the velocity goal created from the spring settings.
		/// </summary>
		float MaximumRecoveryVelocity;
		/// <summary>
		/// Defines the constraint's penetration recovery spring properties.
		/// </summary>
		SpringSettings ContactSpringSettings;

		/// <summary>
		/// Constructs a pair's material properties.
		/// </summary>
		/// <param name="frictionCoefficient">Coefficient of friction to apply for the constraint. Maximum friction force will be equal to the normal force times the friction coefficient.</param>
		/// <param name="maximumRecoveryVelocity">Maximum relative velocity along the contact normal at which the collision constraint will recover from penetration. Clamps the velocity goal created from the spring settings.</param>
		/// <param name="springSettings">Defines the constraint's penetration recovery spring properties.</param>
		PairMaterialProperties(float frictionCoefficient, float maximumRecoveryVelocity, SpringSettings springSettings)
		{
			FrictionCoefficient = frictionCoefficient;
			MaximumRecoveryVelocity = maximumRecoveryVelocity;
			ContactSpringSettings = springSettings;
		}

		PairMaterialProperties() : PairMaterialProperties(0, 0, SpringSettings(0, 0)) {}
	};

	/// <summary>
	/// Defines the callbacks invoked during narrow phase collision detection execution.
	/// </summary>
	struct NarrowPhaseCallbacks
	{
		/// <summary>
		/// Called after the simulation is created. Can be null.
		/// </summary>
		/// <param name="simulationHandle">Handle of the simulation owning these callbacks.</param>
		void (*InitializeFunction)(SimulationHandle simulationHandle);
		/// <summary>
		/// Called when the simulation is being torn down. Can be null.
		/// </summary>
		/// <param name="simulationHandle">Handle of the simulation owning these callbacks.</param>
		void (*DisposeFunction)(SimulationHandle simulationHandle);
		/// <summary>
		/// Called for each pair of collidables with overlapping bounding boxes found by the broad phase.
		/// </summary>
		/// <param name="simulationHandle">Handle of the simulation owning these callbacks.</param>
		/// <param name="workerIndex">Index of the worker within the thread dispatcher that's running this callback.</param>
		/// <param name="a">First collidable in the pair.</param>
		/// <param name="b">Second collidable in the pair.</param>
		/// <param name="speculativeMargin">Speculative contact margin for the pair. Calculated ahead of time, but can be overridden.</param>
		/// <returns>True if the collision detection should run for this pair, false otherwise.</returns>
		bool (*AllowContactGenerationFunction)(SimulationHandle simulationHandle, int32_t workerIndex, CollidableReference a, CollidableReference b, float* speculativeMargin);
		/// <summary>
		/// For pairs involving compound collidables (any type that has children, e.g. Compound, BigCompound, and Mesh), this is invoked for each pair of children with overlapping bounds.
		/// </summary>
		/// <param name="simulationHandle">Handle of the simulation owning these callbacks.</param>
		/// <param name="workerIndex">Index of the worker within the thread dispatcher that's running this callback.</param>
		/// <param name="collidablePair">References to the parent collidables in this pair.</param>
		/// <param name="childIndexA">Index of the child belonging to the first collidable in the pair.</param>
		/// <param name="childIndexB">Index of the child belonging to the second collidable in the pair.</param>
		/// <returns>True if the collision detection should run for these children, false otherwise.</returns>
		bool (*AllowContactGenerationBetweenChildrenFunction)(SimulationHandle simulationHandle, int32_t workerIndex, CollidablePair collidablePair, int32_t childIndexA, int32_t childIndexB);
		/// <summary>
		/// Called after contacts have been found for a collidable pair that resulted in a convex manifold.
		/// </summary>
		/// <param name="simulationHandle">Handle of the simulation owning these callbacks.</param>
		/// <param name="workerIndex">Index of the worker within the thread dispatcher that's running this callback.</param>
		/// <param name="collidablePair">References to the parent collidables in this pair.</param>
		/// <param name="contactManifold">Contacts identified between the pair.</param>
		/// <param name="materialProperties">Contact constraint material properties to use for the constraint, if any.</param>
		/// <returns>True if a contact constraint should be created for this contact manifold, false otherwise.</returns>
		bool (*ConfigureConvexContactManifoldFunction)(SimulationHandle simulationHandle, int32_t workerIndex, CollidablePair collidablePair, ConvexContactManifold* contactManifold, PairMaterialProperties* materialProperties);
		/// <summary>
		/// Called after contacts have been found for a collidable pair that resulted in a nonconvex manifold.
		/// </summary>
		/// <param name="simulationHandle">Handle of the simulation owning these callbacks.</param>
		/// <param name="workerIndex">Index of the worker within the thread dispatcher that's running this callback.</param>
		/// <param name="collidablePair">References to the parent collidables in this pair.</param>
		/// <param name="contactManifold">Contacts identified between the pair.</param>
		/// <param name="materialProperties">Contact constraint material properties to use for the constraint, if any.</param>
		/// <returns>True if a contact constraint should be created for this contact manifold, false otherwise.</returns>
		bool (*ConfigureNonconvexContactManifoldFunction)(SimulationHandle simulationHandle, int32_t workerIndex, CollidablePair collidablePair, NonconvexContactManifold* contactManifold, PairMaterialProperties* materialProperties);
		/// <summary>
		/// Called for contacts identified between children in a compound-involving pair prior to being processed into the top level contact manifold.
		/// </summary>
		/// <param name="simulationHandle">Handle of the simulation owning these callbacks.</param>
		/// <param name="workerIndex">Index of the worker within the thread dispatcher that's running this callback.</param>
		/// <param name="collidablePair">References to the parent collidables in this pair.</param>
		/// <param name="childIndexA">Index of the child belonging to the first collidable in the pair.</param>
		/// <param name="childIndexB">Index of the child belonging to the second collidable in the pair.</param>
		/// <param name="contactManifold">Contacts identified between the pair.</param>
		/// <returns>True if the contacts in this child pair should be considered for constraint generation, false otherwise.</returns>
		/// <remarks>Note that all children are required to be convex, so there is no nonconvex version of this callback.</remarks>
		bool (*ConfigureChildContactManifoldFunction)(SimulationHandle simulationHandle, int32_t workerIndex, CollidablePair collidablePair, int32_t childIndexA, int32_t childIndexB, ConvexContactManifold* contactManifold);
	};

}