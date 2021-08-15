///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains result structures for MeshBuilder.
 *	\file		IceMeshBuilderResults.h
 *	\author		Pierre Terdiman
 *	\date		May, 8, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMESHBUILDERRESULTS_H
#define ICEMESHBUILDERRESULTS_H

	//! Submesh properties.
	struct MESHMERIZER_API MBSubmesh : public Allocateable
	{
		sdword			MatID;					//!< MaterialID for this submesh
		udword			SmGrp;					//!< Smoothing groups for this submesh
		udword			NbFaces;				//!< Number of faces in this submesh
		udword			NbVerts;				//!< Number of vertices in this submesh
		udword			NbSubstrips;			//!< Number of strips in this submesh
	};

	//! Material properties.
	struct MESHMERIZER_API MBMatInfo : public Allocateable
	{
		sdword			MatID;					//!< This material's ID
		udword			NbFaces;				//!< Number of faces having this material
		udword			NbVerts;				//!< Related number of exported vertices
		udword			NbSubmeshes;			//!< Number of related submeshes
	};

	//! The topology structure.
	struct MESHMERIZER_API MBTopology : public Allocateable
	{
						MBTopology()			{ ZeroMemory(this, SIZEOFOBJECT);	}

		udword			NbFaces;				//!< Total number of faces
		udword			NbSourceFaces;			//!< Number of faces in source mesh
		udword			NbSubmeshes;			//!< Total number of submeshes (e.g. 6 for the reference cube)
		udword*			VRefs;					//!< Vertex references (3 refs for each face) [WARNING: udwords since version 3.4]
		udword*			FacesInSubmesh;			//!< Number of faces in each submesh
		float*			Normals;				//!< Face normals
		MBSubmesh*		SubmeshProperties;		//!< NbSubmeshes structures
		udword*			Map;					//!< Maps new faces to old faces
	};

	//! The geometry structure.
	struct MESHMERIZER_API MBGeometry : public Allocateable
	{
						MBGeometry()			{ ZeroMemory(this, SIZEOFOBJECT);	}
		// Basic data
		udword			NbGeomPts;				//!< Number of vertices in the original mesh
		udword			NbTVerts;				//!< Number of mapping coordinates in the original mesh
		udword			NbColorVerts;			//!< Number of vertex-colors in the original mesh
		//
		udword			NbVerts;				//!< Number of vertices in the final mesh (some may have been duplicated) = sum of all NbVerts in MBSubmeshes
		// Indices
		udword*			VertsRefs;				//!< Vertex indices (only !=null if mIndexedGeo, else vertices are duplicated). Index in Verts.
		udword*			TVertsRefs;				//!< UVW indices (only !=null if mIndexedUVW, else UVWs are duplicated). Index in TVerts.
		udword*			ColorRefs;				//!< Vertex-color indices (only !=null if mIndexedColors, else colors are duplicated). Index in CVerts.
#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
		udword*			TVertsRefs2;			//!< UVW indices (only !=null if mIndexedUVW, else UVWs are duplicated). Index in TVerts.
#endif
		// Vertex data
		float*			Verts;					//!< List of vertices, may be duplicated or not
		float*			TVerts;					//!< List of UV(W) mappings, may be duplicated or not.
		float*			CVerts;					//!< List of vertex colors, may be duplicated or not.
#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
		float*			TVerts2;				//!< List of UV(W) mappings, may be duplicated or not.
#endif
		// Normals
		float*			Normals;				//!< Vertex normals. Can't be indexed.
		udword			NormalInfoSize;			//!< Size of the NormalInfo field (in number of udwords, not in bytes)
		udword*			NormalInfo;				//!< Information used to rebuild normals in realtime. See below.
		//
		bool			UseW;					//!< True if TVerts uses W.
	};

	// More about NormalInfo:
	//
	// NormalInfo contains some extra information used to rebuild vertex-normals in realtime, by averaging
	// a number of face-normals. Each vertex-normal depends on a various number of face-normals. The exact
	// number depends on the mesh topology, but also depends on the smoothing groups.
	//
	// NormalInfo contains data to rebuild one normal/vertex, ie to rebuild NbVerts normals.
	// Format is, for each vertex:
	//		udword		Count				a number of faces
	//		udword		Ref0, Ref1,...		a list of Count face indices
	//
	// To rebuild vertex-normals in realtime you need to:
	// 1) Rebuild all face-normals (which is easy)
	// 2) For each vertex, add Count face-normals according to NormalInfo, then normalize the summed vector.
	//
	// Other techniques exist, of course.

	//! The materials structure.
	struct MESHMERIZER_API MBMaterials : public Allocateable
	{
						MBMaterials() : NbMtls(0), MaterialInfo(null)	{}

		udword			NbMtls;					//!< Number of materials found.
		MBMatInfo*		MaterialInfo;			//!< One structure for each material.
	};

	//! Result structure.
	struct MESHMERIZER_API MBResult : public Allocateable
	{
		MBTopology		Topology;				//!< Result topology.
		MBGeometry		Geometry;				//!< Result geometry
		MBMaterials		Materials;				//!< Result materials
	};

	//
	// Pseudo-code showing how to use the consolidation and the striper result structures:
	// mVB is a DX7 vertex buffer filled thanks to the MBGeometry structure.
	// mResult is the MBResult structure.
	// mStripRuns and mStripLengths are from the STRIPERRESULT structure.
	//
	//	// Get indices
	//	uword* VRefs = mResult->Topology.VRefs;	// NB: since version 3.4 you must actually convert to uwords first!
	//	// Make one API call / material
	//	for(i=0;i<mResult->Materials.NbMtls;i++)
	//	{
	//		// Select correct material
	//		udword MaterialID = mResult->Materials.MaterialInfo[i].MatID;
	//		// Draw mesh
	//		if(mStripRuns)	renderer->DrawIndexedPrimitiveVB(PRIMTYPE_TRISTRIP, mVB, 0, mResult->Geometry.NbVerts, mStripRuns[i], mStripLengths[i]);
	//		else			renderer->DrawIndexedPrimitiveVB(PRIMTYPE_TRILIST, mVB, 0, mResult->Geometry.NbVerts, VRefs, mResult->Materials.MaterialInfo[i].NbFaces*3);
	//		// Update index pointer for trilists
	//		VRefs+=mResult->Materials.MaterialInfo[i].NbFaces*3;
	//	}
	//

#endif	// ICEMESHBUILDERRESULTS_H