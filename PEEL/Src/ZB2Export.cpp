///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZB2Export.h"
#include "PINT_Editor.h"
#include "PEEL_MenuBar.h"
//#include "RenderModel.h"
#include "PintShapeRenderer.h"
#include "Zcb2_RenderData.h"
#include "TextureManager.h"
#include "PintVisibilityManager.h"

static const bool gExportAllMaterials = false;	// Else only used ones

/*#define USED	INVALID_ID-1

template<class T0>
static void MarkUsedRenderers(const PtrContainer& shapes)
{
	const udword NbShapes = shapes.GetNbEntries();
	for(udword i=0;i<NbShapes;i++)
	{
		T0* Current = (T0*)shapes.GetEntry(i);
		if(CanExport(Current))
		{
			if(Current->mRenderer && Current->mRenderer->mExportable)
				Current->mRenderer->mID = USED;
		}
	}
}*/

static void _ExportRenderSource(RenderDataChunk* chunk, ZCB2Exporter& database, PtrContainer& textures, udword& render_id, udword& texture_id)
{
	if(!chunk || chunk->GetID()!=INVALID_ID)
		return;

	switch(chunk->GetChunkType())
	{
		case RenderCollectionType:
		{
			RenderDataChunkCollection* RDCChunk = static_cast<RenderDataChunkCollection*>(chunk);
			const udword NbChunks = RDCChunk->mRenderDataChunks.GetNbEntries();
			for(udword i=0;i<NbChunks;i++)
			{
				RenderDataChunk* c = (RenderDataChunk*)RDCChunk->mRenderDataChunks[i];
				_ExportRenderSource(c, database, textures, render_id, texture_id);
			}
		}
		break;

		case ColorRenderDataType:
		{
			ColorRenderDataChunk* CRDChunk = static_cast<ColorRenderDataChunk*>(chunk);

			const ManagedTexture* MT = CRDChunk->mTexture;
			if(MT && MT->mExportID == INVALID_ID)
			{
				if(MT->mSource)
				{
					MT->mExportID = texture_id++;

					//### not a great design/solution here
					textures.AddPtr(MT);

					// TODO: support external files (filename/URL)
					if(1)
					{
						ManagedTextureChunk* MTC = ICE_NEW(ManagedTextureChunk);
						database.RegisterUserDefinedChunk(MTC, TRUE);
						MTC->SetSourceBitmap(*MT->mSource);
						const char* error = MTC->Validate();
						ASSERT(!error);
					}
					else
					{
						// Old codepath using TXMP chunks

						TXMPChunk* txmp = static_cast<TXMPChunk*>(database.CreateChunk('TXMP'));
						txmp->SetSourceBitmap(MT->mSource, null, null);
						const char* error = txmp->Validate();
						ASSERT(!error);
					}
				}
			}

			// Takes care of ColorRenderDataChunk. Only deals with one child here.
			RenderDataChunk* ChildChunk = CRDChunk->mSource;
			_ExportRenderSource(ChildChunk, database, textures, render_id, texture_id);
		}
		break;
	};

	chunk->SetID(render_id++);
	database.RegisterUserDefinedChunk(chunk, FALSE);
}

template<class T0, class T1>
static void ExportShapes(ZCB2Exporter& database, const PtrContainer& shapes, PtrContainer& textures, udword& shape_id, udword& material_id, udword& render_id, udword& texture_id)
{
	const udword NbShapes = shapes.GetNbEntries();
	for(udword i=0;i<NbShapes;i++)
	{
		T0* Current = (T0*)shapes.GetEntry(i);
		if(CanExport(Current))
		{
			// Material
			if(!gExportAllMaterials)
			{
				MaterialData* M = const_cast<MaterialData*>(Current->mMaterial);
				if(M && M->mID == INVALID_ID)
				{
					M->mID = material_id++;
					database.RegisterUserDefinedChunk(ICE_NEW(MaterialChunk)(*M));
				}
			}

			// Renderer
			if(Current->mRenderer)
			{
				_ExportRenderSource(Current->mRenderer->mRenderSource, database, textures, render_id, texture_id);
/*				RenderDataChunk* RenderChunk = Current->mRenderer->mRenderSource;
				if(RenderChunk)
				{
					struct Export
					{
						static void Chunk(ZCB2Exporter& database, RenderDataChunk* chunk, udword& render_id)
						{
							if(chunk->GetID()==INVALID_ID)
							{
								chunk->SetID(render_id++);
								database.RegisterUserDefinedChunk(chunk, FALSE);
							}
						}
					};

					//
					const ManagedTexture* MT = RenderChunk->GetManagedTexture();
					if(MT)
					{
						if(MT->mExportID == INVALID_ID)
						{
							if(MT->mSource)	//#### temporary
							{
								MT->mExportID = texture_id++;
								TXMPChunk* txmp = static_cast<TXMPChunk*>(database.CreateChunk('TXMP'));
								// TODO: support external files (filename/URL)
								txmp->SetSourceBitmap(MT->mSource, null, null);
								const char* error = txmp->Validate();
								ASSERT(!error);
							}
						}
					}

					// Takes care of ColorRenderDataChunk. Only deals with one child here.
					RenderDataChunk* ChildChunk = RenderChunk->GetChildRenderChunk();
					if(ChildChunk)
						Export::Chunk(database, ChildChunk, render_id);

					Export::Chunk(database, RenderChunk, render_id);
				}*/
			}

			// Shape
			Current->mID = shape_id++;
			database.RegisterUserDefinedChunk(ICE_NEW(T1)(*Current));
		}
	}
}

static void ResetRenderSourceIDs()
{
	const udword NbRenderSources = GetNbRenderSources();
	for(udword i=0;i<NbRenderSources;i++)
	{
		RenderDataChunk* chunk = GetRenderSource(i);
		chunk->SetID(INVALID_ID);
	}
}

void ExportZB2(const EditorPlugin& editor, const char* filename)
{
	printf("\n-- ZB2 export: %s\n", filename);

	// Rebuild the ZCB2 database just before export. We need to map current pointers to IDs in a way
	// that is compatible with the ZCB2 importer, i.e. all shapes in one array etc.
	ZCB2Exporter Database;
	PtrContainer TexturesToReset;
	{
		// Database takes ownership of chunks after calling RegisterUserDefinedChunk() on them.
		const SceneData* EditorSceneData = editor.GetEditorSceneData();
		if(EditorSceneData)
			Database.RegisterUserDefinedChunk(ICE_NEW(SceneChunk)(*EditorSceneData));

		const CollisionGroupsData* EditorGroupsData = editor.GetEditorCollisionGroupsData();
		if(EditorGroupsData)
			Database.RegisterUserDefinedChunk(ICE_NEW(CollisionGroupsChunk)(*EditorGroupsData));

		// Export all materials, or just initialize their IDs for partial export
		{
			udword ID = 0;
			const udword NbMaterials = editor.GetNbEditorMaterials();
			for(udword i=0;i<NbMaterials;i++)
			{
				const MaterialData* Current = editor.GetEditorMaterial(i);
				if(gExportAllMaterials)
				{
					if(CanExport(Current))
					{
						Current->mID = ID++;
						Database.RegisterUserDefinedChunk(ICE_NEW(MaterialChunk)(*Current));
					}
				}
				else
					Current->mID = INVALID_ID;
			}
			if(gExportAllMaterials)
				printf("-- ZB2: %d exportable materials\n", ID);
		}

		// ### issue: some renderers reference other renderers and the code below won't be enough
		// We probably need to make things more intrusive here, and maybe sort renderers so that the "leaf" nodes
		// are exported first (for easier loading code)

		ResetRenderSourceIDs();

		// Export all renderers
		// Plan:
		// - first pass to set all renderers to "unused" (=INVALID_ID)
/*		{
//			const udword NbRenderers = GetNbShapeRenderers();
			const udword NbRenderSources = GetNbRenderSources();
			for(udword i=0;i<NbRenderSources;i++)
//			for(udword i=0;i<NbRenderers;i++)
			{
//				PintShapeRenderer* renderer = GetShapeRenderer(i);
				RenderDataChunk* chunk = GetRenderSource(i);
				chunk->SetID(INVALID_ID);
			}
		}*/

/*		// - second pass to parse shapes and mark the renderers they use
		{
			MarkUsedRenderers<SphereShapeData>		(editor.mEditorSphereShapes);
			MarkUsedRenderers<CapsuleShapeData>		(editor.mEditorCapsuleShapes);
			MarkUsedRenderers<CylinderShapeData>	(editor.mEditorCylinderShapes);
			MarkUsedRenderers<BoxShapeData>			(editor.mEditorBoxShapes);
			MarkUsedRenderers<ConvexShapeData>		(editor.mEditorConvexShapes);
			MarkUsedRenderers<MeshShapeData>		(editor.mEditorMeshShapes);
			MarkUsedRenderers<MeshShapeData2>		(editor.mEditorMeshShapes2);
		}

		// - third pass to assign IDs & export renderers
		// => could be merged with second pass but we want the IDs to be assigned in order (for easier loading code)
		{
			udword ID = 0;
			const udword NbRenderers = GetNbShapeRenderers();
			for(udword i=0;i<NbRenderers;i++)
			{
				PintShapeRenderer* Current = GetShapeRenderer(i);
				if(Current->mID==USED)
				{
					Current->mID = ID++;
					ASSERT(0);
					//Database.RegisterUserDefinedChunk(ICE_NEW(MaterialChunk)(*Current));
				}
			}
			printf("-- ZB2: %d exportable renderers\n", ID);
		}
		// - then during shape export, needs to export renderer IDs
		*/

		// Export all meshes
		{
			//###TODO: don't export them if refcount is zero?
			const udword NbMeshes = editor.GetNbEditorMeshes();
			for(udword i=0;i<NbMeshes;i++)
			{
				const MeshData* Current = editor.GetEditorMesh(i);
				Current->mID = i;
				Database.RegisterUserDefinedChunk(ICE_NEW(MeshDataChunk)(*Current));
			}
			printf("-- ZB2: %d exportable meshes\n", NbMeshes);
		}

		// Export all shapes & used materials
		{
			udword ShapeID = 0;
			udword MaterialID = 0;
			udword RenderID = 0;
			udword TextureID = 0;
			ExportShapes<SphereShapeData, SphereShapeChunk>		(Database, editor.GetEditorSphereShapes(), TexturesToReset, ShapeID, MaterialID, RenderID, TextureID);
			ExportShapes<CapsuleShapeData, CapsuleShapeChunk>	(Database, editor.GetEditorCapsuleShapes(), TexturesToReset, ShapeID, MaterialID, RenderID, TextureID);
			ExportShapes<CylinderShapeData, CylinderShapeChunk>	(Database, editor.GetEditorCylinderShapes(), TexturesToReset, ShapeID, MaterialID, RenderID, TextureID);
			ExportShapes<BoxShapeData, BoxShapeChunk>			(Database, editor.GetEditorBoxShapes(), TexturesToReset, ShapeID, MaterialID, RenderID, TextureID);
			ExportShapes<ConvexShapeData, ConvexShapeChunk>		(Database, editor.GetEditorConvexShapes(), TexturesToReset, ShapeID, MaterialID, RenderID, TextureID);
			ExportShapes<MeshShapeData, MeshShapeChunk>			(Database, editor.GetEditorMeshShapes(), TexturesToReset, ShapeID, MaterialID, RenderID, TextureID);
			ExportShapes<MeshShapeData2, MeshShapeChunk2>		(Database, editor.GetEditorMeshShapes2(), TexturesToReset, ShapeID, MaterialID, RenderID, TextureID);
			printf("-- ZB2: %d exportable shapes\n", ShapeID);
			if(!gExportAllMaterials)
				printf("-- ZB2: %d exportable materials\n", MaterialID);
			printf("-- ZB2: %d exportable render sources\n", RenderID);
			printf("-- ZB2: %d exportable textures\n", TextureID);
		}

		// Export all actors
		{
			const udword NbActors = editor.GetNbEditorActors();
			udword ID = 0;
			for(udword i=0;i<NbActors;i++)
			{
				const ActorData* CurrentActor = editor.GetEditorActor(i);
				if(CanExport(CurrentActor))
				{
					CurrentActor->mID = ID++;

					const bool IsRenderable = editor.mVisHelper ? editor.mVisHelper->IsRenderable2(PintActorHandle(CurrentActor)) : true;
					//printf("IsRenderable: %d\n", IsRenderable);
					ActorChunk* NewActor = ICE_NEW(ActorChunk)(*CurrentActor, !IsRenderable);
					ASSERT(NewActor);

					const udword NbShapes = CurrentActor->mShapes.GetNbEntries();
					for(udword j=0;j<NbShapes;j++)
					{
						const ShapeData* CurrentShape = (const ShapeData*)CurrentActor->mShapes.GetEntry(j);
						NewActor->mShapes.Add(CurrentShape->mID);
					}

					Database.RegisterUserDefinedChunk(NewActor);
				}
			}
			printf("-- ZB2: %d exportable actors\n", ID);
		}

		// Export all joints
		{
			const udword NbJoints = editor.GetNbEditorJoints();
			for(udword i=0;i<NbJoints;i++)
			{
				const JointData* CurrentJoint = editor.GetEditorJoint(i);
				CurrentJoint->mID = i;

				JointChunk* NewJoint = null;
				switch(CurrentJoint->mType)
				{
					case PINT_JOINT_SPHERICAL:			{ NewJoint = ICE_NEW(SphericalJointChunk)	(*static_cast<const SphericalJointData*>	(CurrentJoint));	}	break;
					case PINT_JOINT_HINGE:				{ NewJoint = ICE_NEW(HingeJointChunk)		(*static_cast<const HingeJointData*>		(CurrentJoint));	}	break;
					case PINT_JOINT_HINGE2:				{ NewJoint = ICE_NEW(Hinge2JointChunk)		(*static_cast<const Hinge2JointData*>		(CurrentJoint));	}	break;
					case PINT_JOINT_PRISMATIC:			{ NewJoint = ICE_NEW(PrismaticJointChunk)	(*static_cast<const PrismaticJointData*>	(CurrentJoint));	}	break;
					case PINT_JOINT_FIXED:				{ NewJoint = ICE_NEW(FixedJointChunk)		(*static_cast<const FixedJointData*>		(CurrentJoint));	}	break;
					case PINT_JOINT_DISTANCE:			{ NewJoint = ICE_NEW(DistanceJointChunk)	(*static_cast<const DistanceJointData*>		(CurrentJoint));	}	break;
					case PINT_JOINT_D6:					{ NewJoint = ICE_NEW(D6JointChunk)			(*static_cast<const D6JointData*>			(CurrentJoint));	}	break;
					case PINT_JOINT_GEAR:				{ NewJoint = ICE_NEW(GearJointChunk)		(*static_cast<const GearJointData*>			(CurrentJoint));	}	break;
					case PINT_JOINT_RACK_AND_PINION:	{ NewJoint = ICE_NEW(RackJointChunk)		(*static_cast<const RackJointData*>			(CurrentJoint));	}	break;
				};
				ASSERT(NewJoint);
				if(NewJoint)
					Database.RegisterUserDefinedChunk(NewJoint);
			}
			printf("-- ZB2: %d exportable joints\n", NbJoints);
		}

		// Export all aggregates
		{
			const udword NbAggregates = editor.GetNbEditorAggregates();
			for(udword i=0;i<NbAggregates;i++)
			{
				const AggregateData* CurrentAggregate = editor.GetEditorAggregate(i);
				CurrentAggregate->mID = i;

				AggregateChunk* NewAggregate = ICE_NEW(AggregateChunk)(*CurrentAggregate);
				ASSERT(NewAggregate);

				const udword NbActors = CurrentAggregate->mActors.GetNbEntries();
				for(udword j=0;j<NbActors;j++)
				{
					const ActorData* CurrentActor = (const ActorData*)CurrentAggregate->mActors.GetEntry(j);
					NewAggregate->mActors.Add(CurrentActor->mID);
				}

				Database.RegisterUserDefinedChunk(NewAggregate);
			}
			printf("-- ZB2: %d exportable aggregates\n", NbAggregates);
		}
	}

	if(!Database.Export(filename))
	{
		IceCore::MessageBox(0, "OOPS... Export failed!!!", "Error", MB_OK);
	}

	// TODO: reset all IDs (BaseData::mID)
	ResetRenderSourceIDs();
	{
		const udword NbToReset = TexturesToReset.GetNbEntries();
		for(udword i=0;i<NbToReset;i++)
		{
			ManagedTexture* MT = reinterpret_cast<ManagedTexture*>(TexturesToReset[i]);
			MT->mExportID = INVALID_ID;
		}
	}

	AddToRecentFiles(filename, "PEEL");
	UpdateRecentFiles();
}
