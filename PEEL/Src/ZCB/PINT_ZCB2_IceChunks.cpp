///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PINT_ZCB2.h"
#include "..\PintShapeRenderer.h"
#include "..\Zcb2_RenderData.h"

bool ZCB2FactoryEx::NewScene(const SCENChunk* scene)
{
	OutputConsoleError("Found unhandled legacy SCEN chunk!\n");
	return true;
}

bool ZCB2FactoryEx::NewFreeCamera(const FCAMChunk* camera)
{
	OutputConsoleError("Found unhandled legacy FCAM chunk!\n");
	return true;
}

bool ZCB2FactoryEx::NewTargetCamera(const TCAMChunk* camera)
{
	OutputConsoleError("Found unhandled legacy TCAM chunk!\n");
	return true;
}

#ifndef _WIN64	// "temporary"
bool ZCB2FactoryEx::NewMaterial(const MATLChunk* material)
{
	OutputConsoleError("Found unhandled legacy MATL chunk!\n");

	ZCB2FactoryEx::MaterialData* MD = ICE_RESERVE(ZCB2FactoryEx::MaterialData, mImportHelper_MaterialData);
	MD->mZB2ID = material->GetID();
	MD->mTextureID = material->GetDiffuseMapID();
	MD->mDiffuseColor = material->GetMaterialProps().mDiffuseColor;

/*	bool Found = false;
	const udword NbTextures = mImportHelper_TextureData.GetNbEntries()/(sizeof(ZCB2FactoryEx::TextureData)/sizeof(udword));
	const ZCB2FactoryEx::TextureData* Textures = (const ZCB2FactoryEx::TextureData*)mImportHelper_TextureData.GetEntries();
	for(udword i=0;i<NbTextures;i++)
	{
		if(Textures[i].mZB2ID == material->GetDiffuseMapID())
		{
			Found = true;
			break;
		}
	}*/
	return true;
}
#endif

static bool CheckMeshHasUniqueMaterial(const MESHChunk* mesh, udword& unique_material_id)
{
	const udword NbDIDs = mesh->GetMaterialIDs().GetNbDIDs();
	const udword NbWIDs = mesh->GetMaterialIDs().GetNbWIDs();
	if(NbDIDs && NbWIDs)
		OutputConsoleError("Unexpected: MESH chunk has both word & dword material IDs!\n");
	if(NbDIDs)
	{
		const udword* DIDs = mesh->GetMaterialIDs().GetDIDs();
		const udword UniqueID = DIDs[0];
		for(udword i=1;i<NbDIDs;i++)
		{
			if(DIDs[i]!=UniqueID)
				return false;
		}
		unique_material_id = UniqueID;
	}
	else if(NbWIDs)
	{
		const uword* WIDs = mesh->GetMaterialIDs().GetWIDs();
		const uword UniqueID = WIDs[0];
		for(udword i=1;i<NbWIDs;i++)
		{
			if(WIDs[i]!=UniqueID)
				return false;
		}
		unique_material_id = UniqueID;
	}
	return true;
}

bool ZCB2FactoryEx::NewMesh(const MESHChunk* mesh)
{
	OutputConsoleError("Found unhandled legacy MESH chunk!\n");

	MeshDataChunk* Chunk;
	{
		// MESHChunk is created on the stack within ICE then discarded when this function returns,
		// so we must copy the mesh data and make it persistent for the lifetime of the factory.
		// We reuse MeshDataChunk in a bit of an unexpected way to limit the amount of code here.
		// It is kind of beautiful how all these structures written over the course of like 20 years
		// in different projects still actually work quite nicely with each other.

		PINT_MESH_DATA_CREATE mdc;
		mdc.SetSurfaceData(mesh->GetVertices().GetNbPts(), mesh->GetVertices().GetPts(), mesh->GetFaces().GetNbFaces(), mesh->GetFaces().GetDFaces(), mesh->GetFaces().GetWFaces());

		Chunk = ICE_NEW(MeshDataChunk)(mdc);
		mMeshDataChunks.AddPtr(Chunk);
	}

	PINT_MESH_CREATE* Shape;
	{
		Shape = ICE_NEW(PINT_MESH_CREATE);
		Shape->SetSurfaceData(Chunk->mVertexData.GetNbPts(), Chunk->mVertexData.GetPts(), Chunk->mTriangleData.GetNbFaces(), Chunk->mTriangleData.GetDFaces(), Chunk->mTriangleData.GetWFaces());

		bool Found = false;
		RGBAColor DiffuseColor;
		sdword TexID = -1;

		udword UniqueMaterialID;
		if(CheckMeshHasUniqueMaterial(mesh, UniqueMaterialID))
		{
			const udword NbMaterials = mImportHelper_MaterialData.GetNbEntries()/(sizeof(ZCB2FactoryEx::MaterialData)/sizeof(udword));
			const ZCB2FactoryEx::MaterialData* Materials = (const ZCB2FactoryEx::MaterialData*)mImportHelper_MaterialData.GetEntries();
			for(udword i=0;i<NbMaterials;i++)
			{
				if(Materials[i].mZB2ID == UniqueMaterialID)
				{
					Found = true;
					DiffuseColor = Materials[i].mDiffuseColor;
					TexID = Materials[i].mTextureID;
					break;
				}
			}
		}
		else
		{
			OutputConsoleError("Found unhandled multi-materials MESH chunk!\n");
		}

		if(0)
		{
			// Codepath for implicitly created renderers
			if(Found)
				mShapeColors.Add(DiffuseColor.R).Add(DiffuseColor.G).Add(DiffuseColor.B).Add(DiffuseColor.A);
			else
				mShapeColors.Add(INVALID_ID).Add(INVALID_ID).Add(INVALID_ID).Add(INVALID_ID);
		}
		else
		{
			if(Found)
			{
				PintShapeRenderer* Renderer;
				const ManagedTexture* MT = null;
				if(mesh->GetUVs().GetNbPts() && mesh->GetTFaces().GetNbFaces())
				{
					MultiSurface MS;
					MS.IndexedSurface::Init(mesh->GetFaces().GetNbFaces(), mesh->GetVertices().GetNbPts(), mesh->GetVertices().GetPts(), (const IndexedTriangle*)mesh->GetFaces().GetDFaces());
					{
						IndexedSurface* UVSurface = MS.AddExtraSurface(SURFACE_UVS);
						UVSurface->Init(mesh->GetTFaces().GetNbFaces(), mesh->GetUVs().GetNbPts());

						IndexedTriangle* TFaces = (IndexedTriangle*)UVSurface->GetFaces();
						CopyMemory(TFaces, mesh->GetTFaces().GetDFaces(), mesh->GetTFaces().GetNbFaces()*sizeof(IndexedTriangle));

						Point* uvs = (Point*)UVSurface->GetVerts();
						CopyMemory(uvs, mesh->GetUVs().GetPts(), mesh->GetUVs().GetNbPts()*sizeof(Point));
					}

					{
						const udword NbTextures = mImportHelper_TextureData.GetNbEntries()/(sizeof(ZCB2FactoryEx::TextureData)/sizeof(udword));
						const ZCB2FactoryEx::TextureData* Textures = (const ZCB2FactoryEx::TextureData*)mImportHelper_TextureData.GetEntries();
						for(udword i=0;i<NbTextures;i++)
						{
							if(Textures[i].mZB2ID == TexID)
							{
								MT = Textures[i].mTexture;
								break;
							}
						}
					}

					Renderer = CreateMeshRenderer(MS, true);
				}
				else
					Renderer = CreateMeshRenderer(Shape->GetSurface());

				Shape->mRenderer = CreateColorShapeRenderer(Renderer, DiffuseColor, MT);
			}
			else
			{
				Shape->mRenderer = CreateMeshRenderer(Shape->GetSurface());
			}
		}

		//ASSERT(mMeshCreate.GetNbEntries()==chunk.GetID());
		mShapeCreate.AddPtr(Shape);
	}

	{
		PINT_OBJECT_CREATE* Create = ICE_NEW(PINT_OBJECT_CREATE);

		Create->mPosition = mesh->GetPRS().mPos;
		Create->mRotation = mesh->GetPRS().mRot;

		Create->SetShape(Shape);

		//ASSERT(GetNbActors()==Chunk.GetID());

		const bool Hidden = false;
		ActorCreate* NewActor = AddActor(Create, udword(Hidden));
	}
	return true;
}

bool ZCB2FactoryEx::NewTextureMap(const TXMPChunk* txmp)
{
	OutputConsoleError("Found unhandled legacy TXMP chunk!\n");
	return true;
}

bool ZCB2FactoryEx::NewPLight(const PLITChunk* light)
{
	OutputConsoleError("Found unhandled legacy PLIT chunk!\n");
	return true;
}

bool ZCB2FactoryEx::NewSLight(const SLITChunk* light)
{
	OutputConsoleError("Found unhandled legacy SLIT chunk!\n");
	return true;
}

bool ZCB2FactoryEx::NewDLight(const DLITChunk* light)
{
	OutputConsoleError("Found unhandled legacy DLIT chunk!\n");
	return true;
}

bool ZCB2FactoryEx::NewBoxGizmo(const BGIZChunk* gizmo)
{
	OutputConsoleError("Found unhandled legacy BGIZ chunk!\n");
	return true;
}

bool ZCB2FactoryEx::NewSphereGizmo(const SGIZChunk* gizmo)
{
	OutputConsoleError("Found unhandled legacy SGIZ chunk!\n");
	return true;
}

bool ZCB2FactoryEx::NewCylinderGizmo(const CGIZChunk* gizmo)
{
	OutputConsoleError("Found unhandled legacy CGIZ chunk!\n");
	return true;
}

bool ZCB2FactoryEx::NewHelper(const HELPChunk* helper)
{
	OutputConsoleError("Found unhandled legacy HELP chunk!\n");
	return true;
}

bool ZCB2FactoryEx::NewSkeleton(const SKELChunk* skeleton)
{
	OutputConsoleError("Found unhandled legacy SKEL chunk!\n");
	return true;
}

bool ZCB2FactoryEx::NewShape(const SHAPChunk* shape)
{
	OutputConsoleError("Found unhandled legacy SHAP chunk!\n");
	return true;
}

bool ZCB2FactoryEx::NewSampler(const SAMPChunk* sampler)
{
	OutputConsoleError("Found unhandled legacy SAMP chunk!\n");
	return true;
}

bool ZCB2FactoryEx::NewRotator(const ROTAChunk* rota)
{
	OutputConsoleError("Found unhandled legacy ROTA chunk!\n");
	return true;
}

#ifndef _WIN64	// "temporary"
bool ZCB2FactoryEx::NewMotion(const MOVEChunk* move)
{
	OutputConsoleError("Found unhandled legacy MOVE chunk!\n");
	return true;
}
#endif

bool ZCB2FactoryEx::NewLegacyChunk(const BaseChunk* chunk)
{
	if(!chunk)
		return false;

	switch(chunk->GetChunkType())
	{
		case 'SCEN':	{	return NewScene			((const SCENChunk*)chunk);	}break;

		case 'FCAM':	{	return NewFreeCamera	((const FCAMChunk*)chunk);	}break;
		case 'TCAM':	{	return NewTargetCamera	((const TCAMChunk*)chunk);	}break;

		case 'MESH':	{	return NewMesh			((const MESHChunk*)chunk);	}break;

		case 'PLIT':	{	return NewPLight		((const PLITChunk*)chunk);	}break;
		case 'SLIT':	{	return NewSLight		((const SLITChunk*)chunk);	}break;
		case 'DLIT':	{	return NewDLight		((const DLITChunk*)chunk);	}break;

#ifndef _WIN64	// "temporary"
		case 'MATL':	{	return NewMaterial		((const MATLChunk*)chunk);	}break;
#endif
		case 'TXMP':	{	return NewTextureMap	((const TXMPChunk*)chunk);	}break;

//		case 'GIZM':	{	return NewGizmo			((const GIZMChunk*)chunk);	}break;
		case 'BGIZ':	{	return NewBoxGizmo		((const BGIZChunk*)chunk);	}break;
		case 'SGIZ':	{	return NewSphereGizmo	((const SGIZChunk*)chunk);	}break;
		case 'CGIZ':	{	return NewCylinderGizmo	((const CGIZChunk*)chunk);	}break;
		case 'HELP':	{	return NewHelper		((const HELPChunk*)chunk);	}break;
		case 'SKEL':	{	return NewSkeleton		((const SKELChunk*)chunk);	}break;

		case 'SHAP':	{	return NewShape			((const SHAPChunk*)chunk);	}break;

		case 'SAMP':	{	return NewSampler		((const SAMPChunk*)chunk);	}break;
		case 'ROTA':	{	return NewRotator		((const ROTAChunk*)chunk);	}break;

#ifndef _WIN64	// "temporary"
		case 'MOVE':	{	return NewMotion		((const MOVEChunk*)chunk);	}break;
#endif
	}
	return true;
}

