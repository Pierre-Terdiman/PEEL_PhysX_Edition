///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_VEHICLES_H
#define PINT_COMMON_PHYSX3_VEHICLES_H

#include "PINT_CommonPhysX3.h"

#include "foundation/PxPreprocessor.h"
#include "common/PxPhysXCommonConfig.h"
#include "vehicle/PxVehicleSDK.h"
#include "vehicle/PxVehicleUpdate.h"
#include "PxScene.h"
#if PHYSX_SUPPORT_VEHICLE5
	class MyBatchQueryExt;
#else
	#include "PxBatchQueryDesc.h"
#endif

#if PX_DEBUG_VEHICLE_ON
	#define PINT_DEBUG_VEHICLE_ON	1
#endif

#if PINT_DEBUG_VEHICLE_ON
	#include "vehicle/PxVehicleUtilTelemetry.h"
#endif

	class SharedPhysX_Vehicles;

	//Data structure to store reports for each wheel. 
	class SampleVehicleWheelQueryResults
	{
	public:

		//Allocate a buffer of wheel query results for up to maxNumWheels.
		static SampleVehicleWheelQueryResults* allocate(const PxU32 maxNumWheels);

		//Free allocated buffer.
		void free();

		PxWheelQueryResult* addVehicle(const PxU32 numWheels);

	private:

		//One result for each wheel.
		PxWheelQueryResult* mWheelQueryResults;

		//Maximum number of wheels.
		PxU32 mMaxNumWheels;

		//Number of wheels 
		PxU32 mNumWheels;

		SampleVehicleWheelQueryResults()
			: mWheelQueryResults(NULL),mMaxNumWheels(0), mNumWheels(0)
		{
			init();
		}

		~SampleVehicleWheelQueryResults()
		{
		}

		void init()
		{
			mWheelQueryResults=NULL;
			mMaxNumWheels=0;
			mNumWheels=0;
		}
	};

//	#define NB_SWEEP_HITS_PER_WHEEL 1
	#define NB_SWEEP_HITS_PER_WHEEL 4

#if PHYSX_SUPPORT_VEHICLE5
#else
	//Data structure for quick setup of scene queries for suspension raycasts.
	class SampleVehicleSceneQueryData
	{
	public:

		//Allocate scene query data for up to maxNumWheels suspension raycasts.
		static SampleVehicleSceneQueryData* allocate(const PxU32 maxNumWheels, const bool use_sweeps);

		//Free allocated buffer for scene queries of suspension raycasts.
		void free();

		//Create a PxBatchQuery instance that will be used as a single batched raycast of multiple suspension lines of multiple vehicles
		PxBatchQuery* setUpBatchedSceneQuery(PxScene* scene, const bool use_sweeps);

		// For sweeps:

		//Get the buffer of scene query results that will be used by PxVehicleNWSuspensionRaycasts
		PX_FORCE_INLINE	PxSweepQueryResult*		getSweepQueryResultBuffer()				{ return mSweepSqResults;	}
		//Get the number of scene query results that have been allocated for use by PxVehicleNWSuspensionRaycasts
		PX_FORCE_INLINE	PxU32					getSweepQueryResultBufferSize()	const	{ return mNumQueries;		}

		// For raycasts:

		//Get the buffer of scene query results that will be used by PxVehicleNWSuspensionRaycasts
		PX_FORCE_INLINE	PxRaycastQueryResult*	getRaycastQueryResultBuffer()			{ return mRaycastSqResults;	}
		//Get the number of scene query results that have been allocated for use by PxVehicleNWSuspensionRaycasts
		PX_FORCE_INLINE	PxU32					getRaycastQueryResultBufferSize() const	{ return mNumQueries;		}

	private:

		// For sweeps:
		PxSweepQueryResult* mSweepSqResults;		//One result for each wheel.
		PxSweepHit* mSweepSqHitBuffer;				//One hit for each wheel.

		// For raycasts:
		PxRaycastQueryResult* mRaycastSqResults;	//One result for each wheel.		
		PxRaycastHit* mRaycastSqHitBuffer;			//One hit for each wheel.

		PxU32 mNbSqResults;

		//Maximum number of suspension raycasts that can be supported by the allocated buffers 
		//assuming a single query and hit per suspension line.
		PxU32 mNumQueries;
	};
#endif

	namespace physx
	{
		class PxScene;
		class PxBatchQuery;
		class PxCooking;
		class PxMaterial;
		class PxConvexMesh;
		struct PxVehicleDrivableSurfaceType;
	}

	class SampleVehicleWheelQueryResults;

	class SampleVehicle_VehicleManager
	{
	public:

		enum
		{
//			MAX_NUM_4W_VEHICLES=1,
			MAX_NUM_4W_VEHICLES=8,
		};

		SampleVehicle_VehicleManager();
		~SampleVehicle_VehicleManager();

		void	init(PxPhysics& physics, PxScene& scene, const PxMaterial** drivableSurfaceMaterials, const PxVehicleDrivableSurfaceType* drivableSurfaceTypes, const bool use_sweeps);

		void shutdown();

		//Create a vehicle ready to drive.
		PxVehicleDrive4W* create4WVehicle(SharedPhysX_Vehicles& sharedPhysX, const PxMaterial& material,
										PxConvexMesh* chassisConvexMesh, PxConvexMesh** wheelConvexMeshes4,
										bool useAutoGearFlag, const PINT_VEHICLE_CREATE& desc, bool useSphereWheels);

		PX_FORCE_INLINE	PxU32						getNbVehicles()					const	{ return mNumVehicles;		}
		PX_FORCE_INLINE	PxVehicleWheels*			getVehicle(const PxU32 i)				{ return mVehicles[i];		}
		PX_FORCE_INLINE const PxVehicleWheelQueryResult& getVehicleWheelQueryResults(const PxU32 i) const { return mVehicleWheelQueryResults[i]; }

		//Start the suspension raycasts (always call before calling update)
		void suspensionRaycasts(PxScene* scene, const bool use_sweeps, float sweep_inflation);

		//Update vehicle dynamics and compute forces/torques to apply to sdk rigid bodies.
	#if PINT_DEBUG_VEHICLE_ON
		void updateAndRecordTelemetryData(const PxF32 timestep, const PxVec3& gravity, PxVehicleWheels* focusVehicleNW, PxVehicleTelemetryData* telemetryDataNW);
	#else
		void update(const PxF32 timestep, const PxVec3& gravity);
	#endif

	private:

		//Array of all cars and report data for each car.
		PxVehicleWheels*			mVehicles[MAX_NUM_4W_VEHICLES];
		PxVehicleWheelQueryResult	mVehicleWheelQueryResults[MAX_NUM_4W_VEHICLES];
		PxU32						mNumVehicles;

		//sdk raycasts (for the suspension lines).
#if PHYSX_SUPPORT_VEHICLE5
		public:
		MyBatchQueryExt* mBatchQueryExt;
		private:
#else
		SampleVehicleSceneQueryData* mSqData;
		PxBatchQuery* mSqWheelRaycastBatchQuery;
#endif
		//Reports for each wheel.
		SampleVehicleWheelQueryResults* mWheelQueryResults;

		//Cached simulation data of focus vehicle in 4W mode.
		PxVehicleDriveSimData4W mDriveSimData4W;

		//Friction from combinations of tire and surface types.
		PxVehicleDrivableSurfaceToTireFrictionPairs* mSurfaceTirePairs;

		//Initialise a car back to its start transform and state.
		void resetNWCar(const PxTransform& startTransform, PxVehicleWheels* car);
	};

	class SharedPhysX_Vehicles : public SharedPhysX, public Pint_Vehicle
	{
		public:
									SharedPhysX_Vehicles(const EditableParams& params);
		virtual						~SharedPhysX_Vehicles();

		virtual	Pint_Vehicle*		GetVehicleAPI();

		virtual	PintVehicleHandle	CreateVehicle(PintVehicleData& data, const PINT_VEHICLE_CREATE& vehicle)	override;
		virtual	bool				SetVehicleInput(PintVehicleHandle vehicle, const PINT_VEHICLE_INPUT& input)	override;
		virtual	PintActorHandle		GetVehicleActor(PintVehicleHandle vehicle)							const	override;
		virtual	bool				GetVehicleInfo(PintVehicleHandle vehicle, PintVehicleInfo& info)	const	override;
		virtual	bool				ResetVehicleData(PintVehicleHandle vehicle)									override;
		virtual	bool				AddActor(PintVehicleHandle vehicle, PintActorHandle actor)					override;
		virtual	bool				AddShape(PintVehicleHandle vehicle, const PINT_SHAPE_CREATE& create)		override;
		virtual	void				RenderDebugData(PintRender& renderer)										override;

				void				CloseVehicles();
				void				UpdateVehiclesAndPhysX(float dt);
		private:
				void				UpdateVehicles(float dt);
				PtrContainer		mVehicles;
	};

#endif