#pragma once
#include "Handles.h"
#include "Bodies.h"

namespace Bepu
{

	/// <summary>
	/// Defines how a pose integrator should handle angular velocity integration.
	/// </summary>
	enum struct AngularIntegrationMode
	{
		/// <summary>
		/// Angular velocity is directly integrated and does not change as the body pose changes. Does not conserve angular momentum.
		/// </summary>
		Nonconserving = 0,
		/// <summary>
		/// Approximately conserves angular momentum by updating the angular velocity according to the change in orientation. Does a decent job for gyroscopes, but angular velocities will tend to drift towards a minimal inertia axis.
		/// </summary>
		ConserveMomentum = 1,
		/// <summary>
		/// Approximately conserves angular momentum by including an implicit gyroscopic torque. Best option for Dzhanibekov effect simulation, but applies a damping effect that can make gyroscopes less useful.
		/// </summary>
		ConserveMomentumWithGyroscopicTorque = 2,
	};

	/// <summary>
	/// Defines pose integrator state and callbacks.
	/// </summary>
	struct PoseIntegratorCallbacks
	{
		/// <summary>
		/// How the pose integrator should handle angular velocity integration.
		/// </summary>
		AngularIntegrationMode AngularIntegrationMode;
		/// <summary>
		/// Whether the integrator should use only one step for unconstrained bodies when using a substepping solver.
		/// If true, unconstrained bodies use a single step of length equal to the dt provided to <see cref="Simulation.Timestep"/>. 
		/// If false, unconstrained bodies will be integrated with the same number of substeps as the constrained bodies in the solver.
		/// </summary>
		bool AllowSubstepsForUnconstrainedBodies;
		/// <summary>
		/// Whether the velocity integration callback should be called for kinematic bodies.
		/// If true, <see cref="IntegrateVelocity"/> will be called for bundles including kinematic bodies.
		/// If false, kinematic bodies will just continue using whatever velocity they have set.
		/// Most use cases should set this to false.
		/// </summary>
		bool IntegrateVelocityForKinematics;
		/// <summary>
		/// Whether to use a scalar or vectorized integrator callback. If true, <see cref="IntegrateVelocityScalar"/> will be used.
		/// The scalar callback has much higher overhead due to the required data transpositions.
		/// If false, <see cref="IntegrateVelocitySIMD128"/> or <see cref="IntegrateVelocitySIMD256"/> will be called. 
		/// Use <see cref="GetSIMDWidth"/> to know which vectorized callback would be invoked.
		/// </summary>
		bool UseScalarCallback;
		/// <summary>
		/// If true, velocity integration is performed by a builtin managed-side gravity + damping implementation
		/// (configured through SetBuiltinPoseIntegratorState) and none of the integration function pointers are used.
		/// This avoids all native callback transitions in the integration kernel.
		/// </summary>
		bool UseBuiltinGravityDampingCallback;

		/// <summary>
		/// Called after the simulation is created.
		/// </summary>
		/// <param name="simulation">Simulation to which these callbacks belong.</param>
		void (*Initialize)(SimulationHandle simulation);
		/// <summary>
		/// Called before each simulation stage which could execute velocity integration.
		/// </summary>
		/// <param name="simulation">Simulation to which these callbacks belong.</param>
		/// <param name="dt">Timestep duration that subsequent velocity integrations will be invoked with.</param>
		void (*PrepareForIntegration)(SimulationHandle simulation, float dt);
		//There is technically no need to expose all three of these in the interop type as separate fields; we may want to change that.
		//Right now, we're doing it just so that the signature is more explicit... but that could be better handled on the native side.

		/// <summary>
		/// Called for every active body during each integration pass when <see cref="UseScalarCallback"/> is true.
		/// </summary>
		/// <param name="simulation">Simulation to which these callbacks belong.</param>
		/// <param name="bodyIndex">Current index of the body being integrated in the active body set. This is distinct from the <see cref="BodyHandle"/>; the body index can change over time.</param>
		/// <param name="position">Current position of the body.</param>
		/// <param name="orientation">Current orientation of the body.</param>
		/// <param name="localInertia">Inertia properties of the body in its local space.</param>
		/// <param name="workerIndex">Index of the thread worker processing this callback.</param>
		/// <param name="dt">Timestep duration that subsequent velocity integrations will be invoked with.</param>
		/// <param name="velocity">Velocity of the body to be updated by this callback.</param>
		void (*IntegrateVelocityScalar)(SimulationHandle simulation, int32_t bodyIndex, Vector3 position, Quaternion orientation, BodyInertia localInertia, int32_t workerIndex, float dt, BodyVelocity* velocity);
		/// <summary>
		/// Called for every active body bundle during each integration pass when <see cref="UseScalarCallback"/> is false and SIMD width is 128.
		/// </summary>
		/// <param name="simulation">Simulation to which these callbacks belong.</param>
		/// <param name="bodyIndices">Current indices of the body bundle being integrated in the active body set. This is distinct from the <see cref="BodyHandle"/>; the body index can change over time.</param>
		/// <param name="position">Current positions of the body bundle.</param>
		/// <param name="orientation">Current orientations of the body bundle.</param>
		/// <param name="localInertia">Inertia properties of the body bundle in their local space.</param>
		/// <param name="workerIndex">Index of the thread worker processing this callback.</param>
		/// <param name="dt">Timestep duration that subsequent velocity integrations will be invoked with.</param>
		/// <param name="velocity">Velocity of the body bundle to be updated by this callback.</param>
		void (*IntegrateVelocitySIMD128)(SimulationHandle simulation, Vector128I bodyIndices, Vector3SIMD128* positions, QuaternionSIMD128* orientations, BodyInertiaSIMD128* localInertias, Vector128I integrationMask, int32_t workerIndex, Vector128F dt, BodyVelocitySIMD128* bodyVelocities);
		/// <summary>
		/// Called for every active body bundle during each integration pass when <see cref="UseScalarCallback"/> is false and SIMD width is 256.
		/// </summary>
		/// <param name="simulation">Simulation to which these callbacks belong.</param>
		/// <param name="bodyIndices">Current indices of the body bundle being integrated in the active body set. This is distinct from the <see cref="BodyHandle"/>; the body index can change over time.</param>
		/// <param name="position">Current positions of the body bundle.</param>
		/// <param name="orientation">Current orientations of the body bundle.</param>
		/// <param name="localInertia">Inertia properties of the body bundle in their local space.</param>
		/// <param name="workerIndex">Index of the thread worker processing this callback.</param>
		/// <param name="dt">Timestep duration that subsequent velocity integrations will be invoked with.</param>
		/// <param name="velocity">Velocity of the body bundle to be updated by this callback.</param>
		void (*IntegrateVelocitySIMD256)(SimulationHandle simulation, Vector256I bodyIndices, Vector3SIMD256* positions, QuaternionSIMD256* orientations, BodyInertiaSIMD256* localInertias, Vector256I integrationMask, int32_t workerIndex, Vector256F dt, BodyVelocitySIMD256* bodyVelocities);
	};
}