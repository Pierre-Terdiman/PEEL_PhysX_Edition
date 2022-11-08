///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZB2Import.h"
#include "PintShapeRenderer.h"
#include "TestScenes.h"
#include "PintObjectsManager.h"
#include "PintVisibilityManager.h"
#include "Zcb2_RenderData.h"
#include "ProgressBar.h"
#include "PEEL.h"

CHECK_CONTAINER_ITEM(ZCB2Factory::ZCB2MeshRecord)

// Create shape renderers
// This is only for shape renderers that haven't been created by the import process, i.e. this is for "implicit" renderers
// that can be derived from collision shapes (and thus haven't been exported, to make files smaller)
// TODO: eventually put that in CreatePintObject.... ?
static void CreateImplicitShapeRenderers(ZCB2Factory* factory)
{
	printf("...creating implicit shape renderers\n");
	const udword NbMeshes = factory->mMeshObjects.GetNbEntries();
	PintShapeRenderer** MeshRenderers = (PintShapeRenderer**)ICE_ALLOC(sizeof(PintShapeRenderer*)*NbMeshes);
	ZeroMemory(MeshRenderers, sizeof(PintShapeRenderer*)*NbMeshes);

	const udword NbActors = factory->GetNbActors();
	const ZCB2Factory::ActorCreate* Actors = factory->GetActors();

	class CreateImplicitShape : public PintShapeEnumerateCallback
	{
		public:
		ZCB2Factory*		mFactory;
		PintShapeRenderer**	mMeshRenderers;
		const udword		mNbMeshes;

		CreateImplicitShape(ZCB2Factory* factory, PintShapeRenderer** mesh_renderers, udword nb_meshes) : mFactory(factory), mMeshRenderers(mesh_renderers), mNbMeshes(nb_meshes)	{}

		virtual	void	ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data)	override
		{
			const PINT_SHAPE_CREATE* CurrentShape = &create;

			if(!CurrentShape->mRenderer)
			{
				switch(CurrentShape->mType)
				{
					case PINT_SHAPE_SPHERE:
					{
						printf(".....creating sphere shape renderer\n");
						const PINT_SPHERE_CREATE* ShapeCreate = static_cast<const PINT_SPHERE_CREATE*>(CurrentShape);
						CurrentShape->mRenderer = CreateSphereRenderer(ShapeCreate->mRadius);
					}
					break;

					case PINT_SHAPE_CAPSULE:
					{
						printf(".....creating capsule shape renderer\n");
						const PINT_CAPSULE_CREATE* ShapeCreate = static_cast<const PINT_CAPSULE_CREATE*>(CurrentShape);
						CurrentShape->mRenderer = CreateCapsuleRenderer(ShapeCreate->mRadius, ShapeCreate->mHalfHeight*2.0f);
					}
					break;

					case PINT_SHAPE_CYLINDER:
					{
						printf(".....creating cylinder shape renderer\n");
						const PINT_CYLINDER_CREATE* ShapeCreate = static_cast<const PINT_CYLINDER_CREATE*>(CurrentShape);
						CurrentShape->mRenderer = CreateCylinderRenderer(ShapeCreate->mRadius, ShapeCreate->mHalfHeight*2.0f);
					}
					break;

					case PINT_SHAPE_BOX:
					{
						printf(".....creating box shape renderer\n");
						const PINT_BOX_CREATE* ShapeCreate = static_cast<const PINT_BOX_CREATE*>(CurrentShape);
						CurrentShape->mRenderer = CreateBoxRenderer(ShapeCreate->mExtents);
					}
					break;

					case PINT_SHAPE_CONVEX:
					{
						printf(".....creating convex shape renderer\n");
						const PINT_CONVEX_CREATE* ShapeCreate = static_cast<const PINT_CONVEX_CREATE*>(CurrentShape);
						CurrentShape->mRenderer = CreateConvexRenderer(ShapeCreate->mNbVerts, ShapeCreate->mVerts);
					}
					break;

					case PINT_SHAPE_MESH:
					{
						printf(".....creating mesh shape renderer\n");
						const PINT_MESH_CREATE* ShapeCreate = static_cast<const PINT_MESH_CREATE*>(CurrentShape);
						CurrentShape->mRenderer = CreateMeshRenderer(ShapeCreate->GetSurface()/*, true*/);
					}
					break;

					case PINT_SHAPE_MESH2:
					{
						printf(".....creating mesh2 shape renderer\n");
						const PINT_MESH_CREATE2* ShapeCreate = static_cast<const PINT_MESH_CREATE2*>(CurrentShape);
						const udword MeshID = udword(size_t(ShapeCreate->mTriangleMesh));	//### I think this is converted to a pointer when creating the zb2 scene later?
						ASSERT(MeshID<mNbMeshes);
						if(mMeshRenderers[MeshID])
						{
//							if(!Color)
								CurrentShape->mRenderer = mMeshRenderers[MeshID];
//							else
//								CurrentShape->mRenderer = CreateColorShapeRenderer(mMeshRenderers[MeshID], *Color);
						}
						else
						{
							const PINT_MESH_DATA_CREATE* Create = (const PINT_MESH_DATA_CREATE*)mFactory->mMeshObjects[MeshID];
							//###TODO here: take color into account
							// Can we share renderers with the same color??
							PintShapeRenderer* R = CreateMeshRenderer(Create->GetSurface());
							mMeshRenderers[MeshID]	= R;

//							if(!Color)
								CurrentShape->mRenderer = R;
//							else
//								CurrentShape->mRenderer = CreateColorShapeRenderer(R, *Color);
						}

						ZCB2Factory::ZCB2MeshRecord* Record = ICE_RESERVE(ZCB2Factory::ZCB2MeshRecord, mFactory->mZCB2MeshRecords);
						Record->mShapeCreate_	= const_cast<PINT_MESH_CREATE2*>(ShapeCreate);
						Record->mMeshID			= MeshID;
					}
					break;

					default:
						ASSERT(0);
						break;
				}

				//#### to revisit
				const RGBAColor* Color = null;
				if(1)
				{
					const udword Nb = mFactory->GetNbShapes();
					const PINT_SHAPE_CREATE** Shapes = mFactory->GetShapes();
					for(udword j=0;j<Nb;j++)
					{
						if(CurrentShape==Shapes[j])
						{
							const udword* Binary = mFactory->mShapeColors.GetEntries() + j*4;
							if(*Binary!=INVALID_ID)
							{
								Color = reinterpret_cast<const RGBAColor*>(Binary);

								CurrentShape->mRenderer = CreateColorShapeRenderer(CurrentShape->mRenderer, *Color);
							}
							break;
						}
					}
				}
				//####
			}
		}
	};

	CreateImplicitShape CB(factory, MeshRenderers, NbMeshes);
	for(udword i=0;i<NbActors;i++)
		Actors[i].mCreate->GetNbShapes(&CB);

	ICE_FREE(MeshRenderers);
}

bool ImportZB2File(PINT_WORLD_CREATE& desc, const char* filename, ZCB2Factory* factory, ZB2CustomizeCallback* callback)
{
	ASSERT(factory);
	factory->mCustomize = callback;

	printf("...importing %s\n", filename);

	class ProgressBarCB : public ZCB2ImportCallback
	{
		public:
		ProgressBarCB()
		{
		}
		virtual	~ProgressBarCB()
		{
			ReleaseProgressBar();
		}
		virtual	void	OnImportStart(udword nb_chunks)
		{
			CreateProgressBar(nb_chunks, "Importing ZB2 file...");
		}

		virtual	void	OnNewChunk(udword index)
		{
			SetProgress(index);
		}
	};
	ProgressBarCB ProgressBarCallback;
	if(!factory->Import(filename, &ProgressBarCallback))
	{
		OutputConsoleError(_F("Import failed for file %s\n", filename));
		return false;
	}

	desc.mGlobalBounds				= factory->mScene.mGlobalBounds;
	desc.mGravity					= factory->mScene.mGravity;
	desc.mNbSimulateCallsPerFrame	= factory->mScene.mNbSimulateCallsPerFrame;
	desc.mTimestep					= factory->mScene.mTimestep;
//	desc.mCreateDefaultEnvironment	= factory->mScene.mCreateDefaultEnvironment;
	desc.mCreateDefaultEnvironment	= factory->mScene.IsSetCreateDefaultEnv();
	CopyMemory(&desc.mCamera, &factory->mScene.mCamera, PINT_MAX_CAMERA_POSES*sizeof(PintCameraPose));

	class Access : public PINT_WORLD_CREATE
	{
		public:
		void SetNames(const char* name/*, const char* desc*/)
		{
			mTestName = name;
//			mTestDesc = desc;
		}
	};
//	static_cast<Access&>(desc).SetNames(factory->mScene.GetTestName()/*, factory->mScene.GetTestDesc()*/);
	static_cast<Access&>(desc).SetNames(factory->mScene.GetName()/*, factory->mScene.GetTestDesc()*/);
//	if(factory->mScene.mDesc.IsValid())
//		UI_SetSceneDesc(factory->mScene.mDesc);

	printf("......test name: %s\n", desc.GetTestName());

	if(1)
		CreateImplicitShapeRenderers(factory);

	factory->ReleasePintIndependentData();

	return true;
}

bool CreateZB2Scene(Pint& pint, const PintCaps& caps, ZCB2Factory* factory, ZB2CreationCallback* callback)
{
	if(!factory)
		return false;

	printf("...creating ZB2 scene for %s\n", pint.GetName());

	const udword NbGroups = factory->mNbDisabledGroups;
	if(NbGroups)
	{
		if(!caps.mSupportCollisionGroups)
			return false;
		pint.SetDisabledGroups(NbGroups, factory->mDisabledGroups);
	}

	// Using this to avoid leaks when an error occurs
	// TODO: add aggregates etc
	struct MemoryReleaser
	{
		MemoryReleaser()
		{
		}
		~MemoryReleaser()
		{
			ICE_FREE(mData.mJoints);
			ICE_FREE(mData.mActors);
			ICE_FREE(mData.mMeshes);
		}

		PintMeshHandle*		AllocateMeshes(udword nb_mesh_objects)
		{
			mData.mMeshes = (PintMeshHandle*)ICE_ALLOC(sizeof(PintMeshHandle)*nb_mesh_objects);
			mData.mNbMeshObjects = nb_mesh_objects;
			return mData.mMeshes;
		}

		ZB2CreatedObjects::Actor*	AllocateActors(udword nb_actors)
		{
			mData.mActors = (ZB2CreatedObjects::Actor*)ICE_ALLOC(sizeof(ZB2CreatedObjects::Actor)*nb_actors);
			mData.mNbActors = nb_actors;
			return mData.mActors;
		}

		PintJointHandle*	AllocateJoints(udword nb_joints)
		{
			mData.mJoints = (PintJointHandle*)ICE_ALLOC(sizeof(PintJointHandle)*nb_joints);
			mData.mNbJoints = nb_joints;
			return mData.mJoints;
		}

		//private:
		ZB2CreatedObjects	mData;
	};
	MemoryReleaser Created;

	// Create mesh objects
	PintMeshHandle* CreatedMeshes = null;
	const udword NbMeshObjects = factory->mMeshObjects.GetNbEntries();
	if(NbMeshObjects)
	{
		//Created.Meshes = (PintMeshHandle*)ICE_ALLOC(sizeof(PintMeshHandle)*NbMeshObjects);
		CreatedMeshes = Created.AllocateMeshes(NbMeshObjects);
		for(udword i=0;i<NbMeshObjects;i++)
		{
			const PINT_MESH_DATA_CREATE* Create = (const PINT_MESH_DATA_CREATE*)factory->mMeshObjects[i];
			const PintMeshHandle h = pint.CreateMeshObject(*Create);
			ASSERT(h);
			//Created.Meshes[i] = h;
			CreatedMeshes[i] = h;
		}
	}

	// Create meshes
	const udword NbMeshRecords = factory->mZCB2MeshRecords.GetNbEntries()/(sizeof(ZCB2Factory::ZCB2MeshRecord)/sizeof(udword));
	const ZCB2Factory::ZCB2MeshRecord* Records = (const ZCB2Factory::ZCB2MeshRecord*)factory->mZCB2MeshRecords.GetEntries();
	for(udword i=0;i<NbMeshRecords;i++)
	{
		const udword MeshID = Records[i].mMeshID;
		ASSERT(MeshID<NbMeshObjects);
		//Records[i].mShapeCreate->mTriangleMesh = Created.Meshes[MeshID];
		Records[i].mShapeCreate_->mTriangleMesh = CreatedMeshes[MeshID];
	}

	// Create actors
	ZB2CreatedObjects::Actor* CreatedActors = null;
	const udword NbActors = factory->GetNbActors();
	if(NbActors)
	{
		//Created.Actors = (PintActorHandle*)ICE_ALLOC(sizeof(PintActorHandle)*NbActors);
		CreatedActors = Created.AllocateActors(NbActors);
		const ZCB2Factory::ActorCreate* Actors = factory->GetActors();
		for(udword i=0;i<NbActors;i++)
		{
			ASSERT(Actors[i].mCreate);

			PintActorHandle h;
			bool UserCreatedActor = false;
			if(callback)
				UserCreatedActor = callback->CreateActor(h, pint, *Actors[i].mCreate);
			if(!UserCreatedActor)
			{
				h = CreatePintObject(pint, *Actors[i].mCreate);
				ASSERT(h);

				const udword Flags = Actors[i].mFlags;
				if(Flags)
					pint.mVisHelper->SetRenderable(h, false);
			}

			//Created.Actors[i] = h;
			CreatedActors[i].mHandle = h;
			CreatedActors[i].mDesc = Actors[i].mCreate;
		}
	}

	// Create joints
	PintJointHandle* CreatedJoints = null;
	const udword NbJoints = factory->mJoints.GetNbEntries();
	if(NbJoints)
	{
		//Created.Joints = (PintJointHandle*)ICE_ALLOC(sizeof(PintJointHandle)*NbJoints);
		CreatedJoints = Created.AllocateJoints(NbJoints);
		for(udword i=0;i<NbJoints;i++)
		{
			PINT_JOINT_CREATE* Create = reinterpret_cast<PINT_JOINT_CREATE*>(factory->mJoints[i]);
			if(Create)
			{
				switch(Create->mType)
				{
					case PINT_JOINT_SPHERICAL:			{ if(!caps.mSupportSphericalJoints)	return false;	}	break;
					case PINT_JOINT_HINGE:				{ if(!caps.mSupportHingeJoints)		return false;	}	break;
					case PINT_JOINT_HINGE2:				{ if(!caps.mSupportHingeJoints)		return false;	}	break;
					case PINT_JOINT_PRISMATIC:			{ if(!caps.mSupportPrismaticJoints)	return false;	}	break;
					case PINT_JOINT_FIXED:				{ if(!caps.mSupportFixedJoints)		return false;	}	break;
					case PINT_JOINT_DISTANCE:			{ if(!caps.mSupportDistanceJoints)	return false;	}	break;
					case PINT_JOINT_D6:					{ if(!caps.mSupportD6Joints)		return false;	}	break;
					case PINT_JOINT_GEAR:				{ if(!caps.mSupportGearJoints)		return false;	}	break;
					case PINT_JOINT_RACK_AND_PINION:	{ if(!caps.mSupportRackJoints)		return false;	}	break;
					case PINT_JOINT_PORTAL:				{ if(!caps.mSupportPortalJoints)	return false;	}	break;
				};

	//			const udword ID0 = factory->mJointsActorIndices[i*2+0];
	//			const udword ID1 = factory->mJointsActorIndices[i*2+1];
				const udword ID0 = udword(size_t(Create->mObject0));
				const udword ID1 = udword(size_t(Create->mObject1));
				ASSERT(ID0<NbActors || ID0==INVALID_ID);
				ASSERT(ID1<NbActors || ID1==INVALID_ID);
				if(ID0!=INVALID_ID)
					//Create->mObject0 = Created.Actors[ID0];
					Create->mObject0 = CreatedActors[ID0].mHandle;
				else
					Create->mObject0 = null;
				if(ID1!=INVALID_ID)
					//Create->mObject1 = Created.Actors[ID1];
					Create->mObject1 = CreatedActors[ID1].mHandle;
				else
					Create->mObject1 = null;

				//

				if(Create->mType==PINT_JOINT_GEAR)
				{
					PINT_GEAR_JOINT_CREATE* gjc = static_cast<PINT_GEAR_JOINT_CREATE*>(Create);

					const udword JointID0 = udword(size_t(gjc->mHinge0));
					const udword JointID1 = udword(size_t(gjc->mHinge1));
					ASSERT(JointID0<i);
					ASSERT(JointID1<i);
					//gjc->mHinge0 = Created.Joints[JointID0];
					gjc->mHinge0 = CreatedJoints[JointID0];
					//gjc->mHinge1 = Created.Joints[JointID1];
					gjc->mHinge1 = CreatedJoints[JointID1];

					//Created.Joints[i] = pint.CreateJoint(*Create);
					CreatedJoints[i] = pint.CreateJoint(*Create);

					// Restore indices for next Pint plugin
					gjc->mHinge0 = PintJointHandle(size_t(JointID0));
					gjc->mHinge1 = PintJointHandle(size_t(JointID1));
				}
				else if(Create->mType==PINT_JOINT_RACK_AND_PINION)
				{
					PINT_RACK_AND_PINION_JOINT_CREATE* rpjc = static_cast<PINT_RACK_AND_PINION_JOINT_CREATE*>(Create);

					const udword JointID0 = udword(size_t(rpjc->mHinge));
					const udword JointID1 = udword(size_t(rpjc->mPrismatic));
					ASSERT(JointID0<i);
					ASSERT(JointID1<i);
					//rpjc->mHinge = Created.Joints[JointID0];
					rpjc->mHinge = CreatedJoints[JointID0];
					//rpjc->mPrismatic = Created.Joints[JointID1];
					rpjc->mPrismatic = CreatedJoints[JointID1];

					//Created.Joints[i] = pint.CreateJoint(*Create);
					CreatedJoints[i] = pint.CreateJoint(*Create);

					// Restore indices for next Pint plugin
					rpjc->mHinge = PintJointHandle(size_t(JointID0));
					rpjc->mPrismatic = PintJointHandle(size_t(JointID1));
				}
				else
				{
					//Created.Joints[i] = pint.CreateJoint(*Create);
					CreatedJoints[i] = pint.CreateJoint(*Create);
				}
				ASSERT(Created.mData.mJoints[i]);

				// Restore indices for next Pint plugin
				Create->mObject0 = PintActorHandle(size_t(ID0));
				Create->mObject1 = PintActorHandle(size_t(ID1));
			}
			else
				//Created.Joints[i] = null;
				CreatedJoints[i] = null;
		}
	}

	// Create aggregates
	const udword NbAggregates = factory->GetNbAggregates();
	if(NbAggregates)
	{
		if(!caps.mSupportAggregates)
			return false;

		const AggregateChunk** Aggregates = factory->GetAggregates();
		for(udword i=0;i<NbAggregates;i++)
		{
			const AggregateChunk* Chunk = Aggregates[i];

			const PintAggregateHandle h = pint.CreateAggregate(Chunk->mMaxSize, Chunk->IsSetSelfCollision());

			const udword NbActors = Chunk->mActors.GetNbEntries();
			for(udword j=0;j<NbActors;j++)
			{
				const udword ActorID = Chunk->mActors[j];
				ASSERT(ActorID<NbActors);

				bool status = pint.AddToAggregate(CreatedActors[ActorID].mHandle, h);
				//bool status = pint.AddToAggregate(Created.Actors[ActorID], h);
				ASSERT(status);
			}

			if(Chunk->IsSetAddedToScene())
			{
				bool status = pint.AddAggregateToScene(h);
				ASSERT(status);
			}
		}
	}

	// User post create notifications
	if(callback)
		callback->NotifyCreatedObjects(pint, Created.mData, factory);

	return true;
}

bool TestBase::ImportZB2File(PINT_WORLD_CREATE& desc, const char* filename, ZB2CustomizeCallback* callback)
{
	DELETESINGLE(mZB2Factory);
//	mZB2Factory = ICE_NEW(ZCB2Factory);
	mZB2Factory = ICE_NEW(ZCB2FactoryEx);

	return ::ImportZB2File(desc, filename, mZB2Factory, callback);
}

bool TestBase::CreateZB2Scene(Pint& pint, const PintCaps& caps, ZB2CreationCallback* callback)
{
	return ::CreateZB2Scene(pint, caps, mZB2Factory, callback);
}



///////////////////////////////////////////////////////////////////////////////

extern	PhysicsTest*	gRunningTest;

void MergeFile(const char* filename)
{
	// ### only for zb2

	if(gRunningTest)
	{
		ZCB2FactoryEx Factory;

		PINT_WORLD_CREATE PWC;	// ### how to merge this?
		::ImportZB2File(PWC, filename, &Factory);

		const udword NbEngines = GetNbEngines();
		for(udword i=0;i<NbEngines;i++)
		{
			if(!ValidateEngine(i))
				continue;

			Pint* Engine = GetEngine(i);
			ASSERT(Engine);

			PintCaps Caps;
			Engine->GetCaps(Caps);

			bool b = ::CreateZB2Scene(*Engine, Caps, &Factory);
		}
	}
}
