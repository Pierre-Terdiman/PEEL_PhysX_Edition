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

bool ImportZB2File(PINT_WORLD_CREATE& desc, const char* filename, ZCB2Factory* mZB2Factory, ZB2CustomizeCallback* callback)
{
	ASSERT(mZB2Factory);
	mZB2Factory->mCustomize = callback;

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
	if(!mZB2Factory->Import(filename, &ProgressBarCallback))
	{
		OutputConsoleError(_F("Import failed for file %s\n", filename));
		return false;
	}

	desc.mGlobalBounds				= mZB2Factory->mScene.mGlobalBounds;
	desc.mGravity					= mZB2Factory->mScene.mGravity;
	desc.mNbSimulateCallsPerFrame	= mZB2Factory->mScene.mNbSimulateCallsPerFrame;
	desc.mTimestep					= mZB2Factory->mScene.mTimestep;
//	desc.mCreateDefaultEnvironment	= mZB2Factory->mScene.mCreateDefaultEnvironment;
	desc.mCreateDefaultEnvironment	= mZB2Factory->mScene.IsSetCreateDefaultEnv();
	CopyMemory(&desc.mCamera, &mZB2Factory->mScene.mCamera, PINT_MAX_CAMERA_POSES*sizeof(PintCameraPose));

	class Access : public PINT_WORLD_CREATE
	{
		public:
		void SetNames(const char* name/*, const char* desc*/)
		{
			mTestName = name;
//			mTestDesc = desc;
		}
	};
//	static_cast<Access&>(desc).SetNames(mZB2Factory->mScene.GetTestName()/*, mZB2Factory->mScene.GetTestDesc()*/);
	static_cast<Access&>(desc).SetNames(mZB2Factory->mScene.GetName()/*, mZB2Factory->mScene.GetTestDesc()*/);
//	if(mZB2Factory->mScene.mDesc.IsValid())
//		UI_SetSceneDesc(mZB2Factory->mScene.mDesc);

	printf("......test name: %s\n", desc.GetTestName());

	// Create shape renderers
	// This is only for shape renderers that haven't been created by the import process, i.e. this is for "implicit" renderers
	// that can be derived from collision shapes (and thus haven't been exported, to make files smaller)
	// TODO: eventually put that in CreatePintObject.... ?
	if(1)
	{
		printf("...creating implicit shape renderers\n");
		const udword NbMeshes = mZB2Factory->mMeshObjects.GetNbEntries();
		PintShapeRenderer** MeshRenderers = (PintShapeRenderer**)ICE_ALLOC(sizeof(PintShapeRenderer*)*NbMeshes);
		ZeroMemory(MeshRenderers, sizeof(PintShapeRenderer*)*NbMeshes);

		const udword NbActors = mZB2Factory->mActors.GetNbEntries();
		for(udword i=0;i<NbActors;i++)
		{
			const PINT_OBJECT_CREATE* Create = (const PINT_OBJECT_CREATE*)mZB2Factory->mActors[i];
			const PINT_SHAPE_CREATE* CurrentShape = Create->mShapes;
			while(CurrentShape)
			{
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
							const udword MeshID = udword(ShapeCreate->mTriangleMesh);
							ASSERT(MeshID<NbMeshes);
							if(MeshRenderers[MeshID])
							{
//								if(!Color)
									CurrentShape->mRenderer = MeshRenderers[MeshID];
//								else
//									CurrentShape->mRenderer = CreateColorShapeRenderer(MeshRenderers[MeshID], *Color);
							}
							else
							{
								const PINT_MESH_DATA_CREATE* Create = (const PINT_MESH_DATA_CREATE*)mZB2Factory->mMeshObjects[MeshID];
								//###TODO here: take color into account
								// Can we share renderers with the same color??
								PintShapeRenderer* R = CreateMeshRenderer(Create->GetSurface());
								MeshRenderers[MeshID]	= R;

//								if(!Color)
									CurrentShape->mRenderer = R;
//								else
//									CurrentShape->mRenderer = CreateColorShapeRenderer(R, *Color);
							}

							ZCB2Factory::ZCB2MeshRecord* Record = ICE_RESERVE(ZCB2Factory::ZCB2MeshRecord, mZB2Factory->mZCB2MeshRecords);
							Record->mShapeCreate	= const_cast<PINT_MESH_CREATE2*>(ShapeCreate);
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
						udword Nb = mZB2Factory->mShapes.GetNbEntries();
						for(udword j=0;j<Nb;j++)
						{
							if(CurrentShape==mZB2Factory->mShapes[j])
							{
								const udword* Binary = mZB2Factory->mShapeColors.GetEntries() + j*4;
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
				CurrentShape = CurrentShape->mNext;
			}
		}
		ICE_FREE(MeshRenderers);
	}

	mZB2Factory->ReleasePintIndependentData();

	return true;
}

bool CreateZB2Scene(Pint& pint, const PintCaps& caps, ZCB2Factory* mZB2Factory, ZB2CreationCallback* callback)
{
	if(!mZB2Factory)
		return false;

	printf("...creating ZB2 scene for %s\n", pint.GetName());

	const udword NbGroups = mZB2Factory->mNbDisabledGroups;
	if(NbGroups)
	{
		if(!caps.mSupportCollisionGroups)
			return false;
		pint.SetDisabledGroups(NbGroups, mZB2Factory->mDisabledGroups);
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

		PintActorHandle*	AllocateActors(udword nb_actors)
		{
			mData.mActors = (PintActorHandle*)ICE_ALLOC(sizeof(PintActorHandle)*nb_actors);
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

	PintMeshHandle* CreatedMeshes = null;
	const udword NbMeshObjects = mZB2Factory->mMeshObjects.GetNbEntries();
	if(NbMeshObjects)
	{
		//Created.Meshes = (PintMeshHandle*)ICE_ALLOC(sizeof(PintMeshHandle)*NbMeshObjects);
		CreatedMeshes = Created.AllocateMeshes(NbMeshObjects);
		for(udword i=0;i<NbMeshObjects;i++)
		{
			const PINT_MESH_DATA_CREATE* Create = (const PINT_MESH_DATA_CREATE*)mZB2Factory->mMeshObjects[i];
			const PintMeshHandle h = pint.CreateMeshObject(*Create);
			ASSERT(h);
			//Created.Meshes[i] = h;
			CreatedMeshes[i] = h;
		}
	}

	const udword NbMeshRecords = mZB2Factory->mZCB2MeshRecords.GetNbEntries()/(sizeof(ZCB2Factory::ZCB2MeshRecord)/sizeof(udword));
	const ZCB2Factory::ZCB2MeshRecord* Records = (const ZCB2Factory::ZCB2MeshRecord*)mZB2Factory->mZCB2MeshRecords.GetEntries();
	for(udword i=0;i<NbMeshRecords;i++)
	{
		const udword MeshID = Records[i].mMeshID;
		ASSERT(MeshID<NbMeshObjects);
		//Records[i].mShapeCreate->mTriangleMesh = Created.Meshes[MeshID];
		Records[i].mShapeCreate->mTriangleMesh = CreatedMeshes[MeshID];
	}

	PintActorHandle* CreatedActors = null;
	const udword NbActors = mZB2Factory->mActors.GetNbEntries();
	if(NbActors)
	{
		//Created.Actors = (PintActorHandle*)ICE_ALLOC(sizeof(PintActorHandle)*NbActors);
		CreatedActors = Created.AllocateActors(NbActors);
		for(udword i=0;i<NbActors;i++)
		{
			const PINT_OBJECT_CREATE* Create = (const PINT_OBJECT_CREATE*)mZB2Factory->mActors[i];
			const PintActorHandle h = CreatePintObject(pint, *Create);
			ASSERT(h);
			//Created.Actors[i] = h;
			CreatedActors[i] = h;
			const udword Flags = mZB2Factory->mActorsFlags[i];
			if(Flags)
				pint.mVisHelper->SetRenderable(h, false);
		}
	}

	PintJointHandle* CreatedJoints = null;
	const udword NbJoints = mZB2Factory->mJoints.GetNbEntries();
	if(NbJoints)
	{
		//Created.Joints = (PintJointHandle*)ICE_ALLOC(sizeof(PintJointHandle)*NbJoints);
		CreatedJoints = Created.AllocateJoints(NbJoints);
		for(udword i=0;i<NbJoints;i++)
		{
			PINT_JOINT_CREATE* Create = reinterpret_cast<PINT_JOINT_CREATE*>(mZB2Factory->mJoints[i]);
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

	//			const udword ID0 = mZB2Factory->mJointsActorIndices[i*2+0];
	//			const udword ID1 = mZB2Factory->mJointsActorIndices[i*2+1];
				const udword ID0 = udword(size_t(Create->mObject0));
				const udword ID1 = udword(size_t(Create->mObject1));
				ASSERT(ID0<NbActors || ID0==INVALID_ID);
				ASSERT(ID1<NbActors || ID1==INVALID_ID);
				if(ID0!=INVALID_ID)
					//Create->mObject0 = Created.Actors[ID0];
					Create->mObject0 = CreatedActors[ID0];
				else
					Create->mObject0 = null;
				if(ID1!=INVALID_ID)
					//Create->mObject1 = Created.Actors[ID1];
					Create->mObject1 = CreatedActors[ID1];
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
					gjc->mHinge0 = PintJointHandle(JointID0);
					gjc->mHinge1 = PintJointHandle(JointID1);
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
					rpjc->mHinge = PintJointHandle(JointID0);
					rpjc->mPrismatic = PintJointHandle(JointID1);
				}
				else
				{
					//Created.Joints[i] = pint.CreateJoint(*Create);
					CreatedJoints[i] = pint.CreateJoint(*Create);
				}
				ASSERT(Created.mData.mJoints[i]);

				// Restore indices for next Pint plugin
				Create->mObject0 = PintActorHandle(ID0);
				Create->mObject1 = PintActorHandle(ID1);
			}
			else
				//Created.Joints[i] = null;
				CreatedJoints[i] = null;
		}
	}

	const udword NbAggregates = mZB2Factory->mAggregateChunks.GetNbEntries();
	if(NbAggregates)
	{
		if(!caps.mSupportAggregates)
			return false;

		for(udword i=0;i<NbAggregates;i++)
		{
			const AggregateChunk* Chunk = (const AggregateChunk*)mZB2Factory->mAggregateChunks[i];

			const PintAggregateHandle h = pint.CreateAggregate(Chunk->mMaxSize, Chunk->IsSetSelfCollision());

			const udword NbActors = Chunk->mActors.GetNbEntries();
			for(udword j=0;j<NbActors;j++)
			{
				const udword ActorID = Chunk->mActors[j];
				ASSERT(ActorID<NbActors);

				bool status = pint.AddToAggregate(CreatedActors[ActorID], h);
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

	if(callback)
		callback->NotifyCreatedObjects(pint, Created.mData, mZB2Factory);

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
