///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to build a clean mesh.
 *	\file		IceMeshBuilder.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEMESHBUILDER_H__
#define __ICEMESHBUILDER_H__

/*
	#define	MCU_EDGE_0					1					// Bitmask for first edge
	#define	MCU_EDGE_1					2					// Bitmask for second edge
	#define	MCU_EDGE_2					4					// Bitmask for third edge
*/

	#define MESHBUILDER_REF_NOT_DONE	-1					//!< Default cross reference in RebuildVertexData()

	enum VertexCode
	{
		VCODE_VERTEX		= 0,
		VCODE_UVW			= 1,
		VCODE_COLOR			= 2,
		VCODE_FORCEDWORD	= 0x7fffffff
	};
#ifdef OLD
	//! A custom vertex holding 3 references.
	struct MESHMERIZER_API BuilderVertex
	{
		udword			VRef;								//!< Ref into an array of vertices (mVerts)
		udword			TRef;								//!< Ref into an array of UVW mappings (mReducedTVerts)
		udword			CRef;								//!< Ref into an array of vertex-colors (mCVerts)
	};

	//! A custom face holding all the face-related properties.
	struct MESHMERIZER_API BuilderFace
	{
		udword			NewRef[3];							//!< Final "vertex" references [==refs to one geometry/UVW/Color/Normal component/vertex]
		udword			Ref[3];								//!< Ref into an array of BuilderVertices
		sdword			MatID;								//!< Material ID
		udword			SmGroup;							//!< Smoothing group
		udword			ID;									//!< Final face ID [OBSOLETE]
		Point			Normal;								//!< Face normal vector
		ubyte			EdgeVis;							//!< Edges visibility code
	};

	//! Initialization structure.
	struct MESHMERIZER_API MESHBUILDERCREATE
	{
		MESHBUILDERCREATE()
		{
			NbVerts				= 0;
			NbFaces				= 0;
			NbTVerts			= 0;
			NbCVerts			= 0;
			Verts				= null;
			TVerts				= null;
			CVerts				= null;
			KillZeroAreaFaces	= false;
			UseW				= false;
			ComputeVNorm		= false;
			ComputeFNorm		= false;
			ComputeNormInfo		= false;
			IndexedGeo			= false;
			IndexedUVW			= false;
			RelativeIndices		= false;
		}

		udword			NbVerts;							//!< Number of vertices in the mesh
		udword			NbFaces;							//!< Number of faces in the mesh
		udword			NbTVerts;							//!< Number of texture-vertices in the mesh
		udword			NbCVerts;							//!< Number of vertex-colors in the mesh
		Point*			Verts;								//!< Verts in MAX are Point3 == Point
		Point*			TVerts;								//!< TVerts in MAX are UVVert == Point3 == Point
		Point*			CVerts;								//!< CVerts in MAX are VertColor == Point3 == Point

		bool			KillZeroAreaFaces;					//!< Look for zero-area faces and delete them
		bool			UseW;								//!< Use W coord in UVW mappings, else just deals with standard U & V
		bool			ComputeVNorm;						//!< Compute vertex-normals, else leave it to the user
		bool			ComputeFNorm;						//!< Compute face-normals, else leave it to the user
		bool			ComputeNormInfo;					//!< Compute normal-information table (used to recompute'm dynamically according to smoothing groups)
		bool			IndexedGeo;							//!< Ask for indexed geometry
		bool			IndexedUVW;							//!< Ask for indexed UVW
		bool			RelativeIndices;					//!< Reset indices from one submesh to another, else use absolute indices
/*		bool			CreateStrips;						//!< Create triangle strips, else create triangle lists
		bool			ForceSingleStrip;					//!< Create a degenerated single strip for each submesh
		bool			CreateWireData;						//!< Create and export wire-strips with edges-visibilities
		bool			ProgressiveMesh;					//!< Create and export progressive mesh information
		CustomArray*	ArrayForLogText;					//!< User-array for log text if needed*/
	};

//for FLEXPORTER CARBON-COPY (remove the MESHMERIZER_API)
	//! Submesh properties.
	struct MESHMERIZER_API MESHBUILDERSUBMESH
	{
		sdword			MatID;								//!< MaterialID for this submesh
		udword			SmGrp;								//!< Smoothing groups for this submesh
		udword			NbFaces;							//!< Number of faces in this submesh
		udword			NbVerts;							//!< Number of vertices in this submesh
		udword			NbSubstrips;						//!< Number of strips in this submesh
	};

	//! Material properties.
	struct MESHMERIZER_API MESHBUILDERMATINFO
	{
		sdword			MatID;								//!< This material's ID
		udword			NbFaces;							//!< Number of faces having this material
		udword			NbVerts;							//!< Related number of exported vertices
		udword			NbSubmeshes;						//!< Number of related submeshes
	};

	//! The topology structure.
	struct MESHMERIZER_API MESHBUILDERTOPOLOGY
	{
		MESHBUILDERTOPOLOGY()
		{
			VRefs				= null;
			FacesInSubmesh		= null;
			Normals				= null;
			SubmeshProperties	= null;
		}
		uword				NbFaces;						//!< Total number of faces
		uword				NbSubmeshes;					//!< Total number of submeshes (e.g. 6 for the reference cube)
		union {
			uword*			StripRuns;						//!< Obsolete
			uword*			VRefs;							//!< Vertex references (3 refs for each face)
		};
		union {
			uword*			StripLengths;					//!< Obsolete
			uword*			FacesInSubmesh;					//!< Number of faces in each submesh
		};
		float*				Normals;						//!< Face normals
		MESHBUILDERSUBMESH*	SubmeshProperties;				//!< NbSubmeshes structures
	};

	//! The geometry structure.
	struct MESHMERIZER_API MESHBUILDERGEOMETRY
	{
		MESHBUILDERGEOMETRY()
		{
			VertsRefs	= null;
			TVertsRefs	= null;
			Verts		= null;
			TVerts		= null;
			Normals		= null;
			CVerts		= null;
			NormalInfo	= null;
		}
		// Basic data
		uword				NbGeomPts;						//!< Number of vertices in the original mesh
		uword				NbTVerts;						//!< Number of mapping coordinates in the original mesh
		uword				NbVerts;						//!< Number of vertices in the final mesh (some may have been duplicated) = sum of all NbVerts in MESHBUILDERSUBMESHes
		// Indices
		udword*				VertsRefs;						//!< Vertex indices (only !=null if mIndexedGeo, else vertices are duplicated). Index in Verts.
		udword*				TVertsRefs;						//!< UVW indices (only !=null if mIndexedUVW, else UVWs are duplicated). Index in TVerts.
		// Vertex data
		float*				Verts;							//!< List of vertices, may be duplicated or not
		float*				TVerts;							//!< List of UV(W) mappings, may be duplicated or not.
		float*				Normals;						//!< Vertex normals. Can't be indexed.
		float*				CVerts;							//!< List of vertex colors. Can't be indexed.
		udword				NormalInfoSize;					//!< Size of the NormalInfo field.
		udword*				NormalInfo;						//!< Information used to rebuild normals in realtime. See below.
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
	struct MESHMERIZER_API MESHBUILDERMATERIALS
	{
		MESHBUILDERMATERIALS()
		{
			MaterialInfo	= null;
		}
		udword				NbMtls;							//!< Number of materials found.
		MESHBUILDERMATINFO*	MaterialInfo;					//!< One structure for each material.
	};

	//! Result structure.
	struct MESHMERIZER_API MESHBUILDERRESULT
	{
		//! Constructor
		MESHBUILDERRESULT()
		{
			WireframeData	= null;
			ProgressiveMesh	= null;
		}
		void*					WireframeData;				//!< Obsolete.
		MESHBUILDERTOPOLOGY		Topology;					//!< Result topology.
		MESHBUILDERGEOMETRY		Geometry;					//!< Result geometry
		MESHBUILDERMATERIALS	Materials;					//!< Result materials
		udword*					ProgressiveMesh;			//!< Obsolete.
	};

	//
	// Pseudo-code showing how to use the consolidation and the striper result structures:
	// mVB is a DX7 vertex buffer filled thanks to the MESHBUILDERGEOMETRY structure.
	// mResult is the MESHBUILDERRESULT structure.
	// mStripRuns and mStripLengths are from the STRIPERRESULT structure.
	//
	//	// Get indices
	//	uword* VRefs = mResult->Topology.VRefs;
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

//~for FLEXPORTER CARBON-COPY

	class MESHMERIZER_API MeshBuilder
	{
		public:
		// Constructor/destructor
							MeshBuilder();
							~MeshBuilder();

		// Creation methods
		bool				Init(MESHBUILDERCREATE& create);
		bool				AddFace(sdword materialID, udword smgroup, ubyte edgevis, udword* vrefs, udword* trefs, udword* crefs, bool ccw=false);

		// Build method
		bool				Build(MESHBUILDERRESULT& result);

		private:
		// Custom arrays
		CustomArray			mArrayTopology;					//!< Array to store triangle strip runs / face data => topology
		CustomArray			mArrayTopoSize;					//!< Array to store triangle strip lengths / nb of faces => topology size
		CustomArray			mArrayVertsRefs;				//!< Array to store triangle strip vertices references.
		CustomArray			mArrayTVertsRefs;				//!< Array to store triangle strip texture-vertices references.
		CustomArray			mArrayVerts;					//!< Array to store triangle strip vertices.
		CustomArray			mArrayTVerts;					//!< Array to store triangle strip mapping coordinates.
		CustomArray			mArrayCVerts;					//!< Array to store triangle strip vertex-colors.
		CustomArray			mArrayVNormals;					//!< Array to store triangle strip vertex normals.
		CustomArray			mArrayFNormals;					//!< Array to store triangle strip face normals.
		CustomArray			mArrayNormalInfo;				//!< Array to store information about normals.
		CustomArray			mArraySubmeshProperties;		//!< Array to store submesh properties: material ID, smoothing groups, #substrips, #vertex-data.
//		CustomArray			mArrayWire;						//!< Array to store line strips.
//		CustomArray			mArrayProgMesh;					//!< Array to store progressive mesh information

		CustomArray			mMaterialInfo;					//!< TO BE DOCUMENTED

		udword				mNbFaces;						//!< Maximum (expected) number of faces (provided by user)
		udword				mNbVerts;						//!< Number of vertices in the original mesh (provided by user)
		udword				mNbTVerts;						//!< Number of mapping coordinates in the original mesh (provided by user)
		udword				mNbCVerts;						//!< Number of vertex colors in the original mesh (provided by user)
//		udword				mCurrentVertex;					//!< TO BE DOCUMENTED
		udword				mNbAvailableFaces;				//!< Number of faces in database (could be <NbFaces if zero-area faces are skipped)
		udword				mNbBuildVertices;				//!< TO BE DOCUMENTED
		udword				mNewVertsIndex;					//!< TO BE DOCUMENTED
/*		udword				mNbEdges;						//!< Number of edges in the object/scene (computed)
		udword				mNbConnectedFaces;				//!< Number of faces in extracted submesh
*/
		Point*				mVerts;							//!< Original list of vertices
		Point*				mTVerts;						//!< Reduced list of texture-vertices
		Point*				mCVerts;						//!< Reduced list of vertex-colors

		BuilderFace*		mFacesArray;					//!< The original list of faces.
		BuilderVertex*		mVertexArray;					//!< The list of rebuilt vertices.

		// Normals computation / smoothing groups
		udword*				mFCounts;						//!< For each vertex, the number of faces sharing that vertex.
		udword*				mVOffsets;						//!< Radix-style offsets of mFCounts.
		udword*				mVertexToFaces;					//!< List of faces sharing each vertex.
		udword*				mFacesCrossRefs;				//!< TO BE DOCUMENTED
		udword				mNbNorm;						//!< TO BE DOCUMENTED
		udword				mCrossIndex;					//!< TO BE DOCUMENTED

		// Build settings
		bool				mKillZeroAreaFaces;				//!< true if degenerated triangles must be skipped (default)
		bool				mUseW;							//!< true if W coord must be used, else false (default)
		bool				mComputeVNorm;					//!< true if vertex-normals must be computed, else false (default)
		bool				mComputeFNorm;					//!< true if face-normals must be computed, else false (default)
		bool				mComputeNormInfo;				//!< true if normal information must be computed, else false (default)
		bool				mIndexedGeo;					//!< true if geometry must be exported as indexed (default), else false
		bool				mIndexedUVW;					//!< true if UVW must be exported as indexed, else false (default)
		bool				mRelativeIndices;				//!< true if indices must be reset for each submesh, else false (default)

/*		bool				mValidateTopology;				//!< true if topology-related extraction must be performed, else false (default)
		bool				mValidateUVW;					//!< true if UVW-related extraction must be performed, else false (default)
		bool				mValidateRGB;					//!< true if RGB-related extraction must be performed, else false (default)
		bool				mValidateWire;					//!< true if wire-strips must be computed, else false (default)
		bool				mCreateStrips;					//!< true if triangle strips must be computed, else false (default)
		bool				mForceSingleStrip;				//!< true if a single strip must be created for each submesh, else false (default)
		bool				mProgressiveMesh;				//!< true if progressive mesh information must be computed, else false (default)
*/
		// Internal init methods
		MeshBuilder&		FreeUsedRam();
		bool				SetNbFaces(udword nb);
		bool				SetNbVerts(udword nb, Point* verts);
		bool				SetNbTVerts(udword nb, Point* tverts);
		bool				SetNbCVerts(udword nb, Point* cverts);

		// Cleaning room
		bool				OptimizeTopology();
		bool				OptimizeXMappings(udword& nbxverts, Point*& xverts, VertexCode vcode);
		bool				OptimizeGeometry();

		bool				ComputeNormals();

		bool				SaveXVertices();

		bool				ComputeSubmeshes();
		udword				BuildTrilist(udword* list, udword nbfaces, sdword matid, udword smgrp);
		udword				RebuildVertexData(udword* faces, udword nbfaces, CustomArray* array);
		MeshBuilder&		StoreVertex(udword* crosslist, udword ref, udword grp, udword fnb, udword& newvertsindex, udword id, CustomArray* array);
	};
#endif

#endif // __ICEMESHBUILDER_H__

