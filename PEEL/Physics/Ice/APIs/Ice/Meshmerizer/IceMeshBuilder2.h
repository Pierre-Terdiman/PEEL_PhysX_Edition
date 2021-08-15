///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to build a clean mesh.
 *	\file		IceMeshBuilder2.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMESHBUILDER2_H
#define ICEMESHBUILDER2_H

	//! X-vertices
	enum VertexCode
	{
		VCODE_VERTEX		= (1<<0),			//!< Geometry
		VCODE_UVW			= (1<<1),			//!< UV-mappings
		VCODE_COLOR			= (1<<2),			//!< Vertex colors
#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
		VCODE_UVW2			= (1<<3),			//!< Second UV-mappings
#endif
		VCODE_FORCEDWORD	= 0x7fffffff
	};

	//! Initialization structure.
	struct MESHMERIZER_API MBCreate
	{
						MBCreate()				{ ZeroMemory(this, SIZEOFOBJECT);	}

		udword			NbVerts;				//!< Number of vertices in the mesh
		udword			NbFaces;				//!< Number of faces in the mesh
		udword			NbTVerts;				//!< Number of texture-vertices in the mesh
		udword			NbCVerts;				//!< Number of vertex-colors in the mesh
#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
		udword			NbTVerts2;				//!< Number of texture-vertices2 in the mesh
#endif
		const Point*	Verts;					//!< Verts in MAX are Point3 == Point
		const Point*	TVerts;					//!< TVerts in MAX are UVVert == Point3 == Point
		const Point*	CVerts;					//!< CVerts in MAX are VertColor == Point3 == Point
#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
		const Point*	TVerts2;				//!< Second mapping channel
#endif
//		udword			OptimizeFlags;			//!< A combination of VertexCode flags, to enable/disable passes

		bool			KillZeroAreaFaces;		//!< Look for zero-area faces and delete them
		bool			UseW;					//!< Use W coord in UVW mappings, else just deals with standard U & V
		bool			ComputeVNorm;			//!< Compute vertex-normals, else leave it to the user
		bool			ComputeFNorm;			//!< Compute face-normals, else leave it to the user
		bool			ComputeNormInfo;		//!< Compute normal-information table (used to recompute'm dynamically according to smoothing groups)
		bool			IndexedGeo;				//!< Ask for indexed geometry
		bool			IndexedUVW;				//!< Ask for indexed UVW
		bool			IndexedColors;			//!< Ask for indexed vertex-colors
		bool			RelativeIndices;		//!< Reset indices from one submesh to another, else use absolute indices
		bool			IsSkin;					//!< True for skins => MeshBuilder does not touch the list of vertices
		bool			WeightNormalWithAngles;	//!< Take angles between edges into account in normal computation
		bool			OptimizeVertexList;		//!< Optimize vertex list for cache coherence
	};

	// Vertex alpha support:
	// I expect colors as points (RGB floating-point values in CVerts) since that's what I get out of MAX.
	// So at first it seems vertex alpha is not supported. Actually it is, here's how:
	//
	// 1) First method:
	// - convert your colors to RGBA *udwords* (since that's what will eventually be in your vertex buffers there's no real loss of precision...)
	// - put this binary data in the Red channel as if it was a float (but don't do the int-to-float conversion of course)
	// - clear the green and blue channels to allow MeshBuilder to reduce the vertex cloud anyway
	// - and that's it, you just have to depack the colors at the other side of the pipeline if needed.
	//
	// It works since I actually only work on the binary representation of the float values without really doing any computation.
	// Here I once again appreciate the power of radix-sort ! With a standard comparison-based sort, as the name suggests, real FPU comparisons
	// would have been made while reducing the cloud, and it probably would have failed since some "floats" would've been read as NaNs or
	// other non-valid float representations..............................................
	//
	// 2) Second method:
	// - Store the alpha in the TVerts list, in the usually unused W.

	struct MESHMERIZER_API MBFaceData : public Allocateable
	{
						MBFaceData() : Index(0), MaterialID(INVALID_ID), SmoothingGroups(1), VRefs(null), TRefs(null), CRefs(null),
#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
						TRefs2(null),
#endif
						CCW(false)	{}

		udword			Index;				//!< Face index in the original (user-controlled) list
		sdword			MaterialID;			//!< Material ID for this face
		udword			SmoothingGroups;	//!< Smoothing groups for this face
		const udword*	VRefs;				//!< Vertex references (3 udwords)
		const udword*	TRefs;				//!< Texture-vertex references (3 udwords)
		const udword*	CRefs;				//!< Vertex-colors references (3 udwords)
#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
		const udword*	TRefs2;				//!< Texture-vertex2 references (3 udwords)
#endif
		bool			CCW;				//!< True if vertex references must be swapped
	};

	class MESHMERIZER_API MeshBuilder2 : public Allocateable
	{
		//! A custom vertex holding 4 references.
		struct MBVertex : public Allocateable
		{
			udword		VRef;				//!< Ref into an array of vertices (mVerts)
			udword		TRef;				//!< Ref into an array of UVW mappings (mTVerts)
			udword		CRef;				//!< Ref into an array of vertex-colors (mCVerts)
#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
			udword		TRef2;				//!< Ref into an array of UVW mappings (mTVerts2)
#endif
		};

		//! A custom face holding all the face-related properties.
		struct MBFace : public Allocateable
		{
			udword		NewRef[3];			//!< Final "vertex" references [==refs to one geometry/UVW/Color/Normal component/vertex]
			udword		Ref[3];				//!< Ref into an array of MBVertices
			sdword		MatID;				//!< Material ID
			udword		SmGroup;			//!< Smoothing group
			Point		Normal;				//!< Face normal vector
			udword		Index;				//!< Original face index from AddFace().
		};

		public:
								MeshBuilder2();
								~MeshBuilder2();
		// Creation methods
				bool			Init(const MBCreate& create);
				bool			AddFace(const MBFaceData& face);
		// Build method
				bool			Build(MBResult& result);
		private:
		// Custom arrays / containers
				Container		mArrayTopology;					//!< Array to store triangle strip runs / face data => topology
				Container		mArrayTopoSize;					//!< Array to store triangle strip lengths / nb of faces => topology size
				Container		mArrayVertsRefs;				//!< Array to store vertices references.
				Container		mArrayTVertsRefs;				//!< Array to store texture-vertices references.
				Container		mArrayColorRefs;				//!< Array to store vertex-colors references.
				Vertices		mArrayVerts;					//!< Array to store vertices.
				Container		mArrayTVerts;					//!< Array to store mapping coordinates. [Container only for UVs without W]
				Vertices		mArrayCVerts;					//!< Array to store vertex-colors.
				Vertices		mArrayVNormals;					//!< Array to store vertex normals.
				Vertices		mArrayFNormals;					//!< Array to store face normals.
				Container		mArrayNormalInfo;				//!< Array to store information about normals.
				Container		mArraySubmeshProperties;		//!< Array to store submesh properties: material ID, smoothing groups, #substrips, #vertex-data.
#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
				Container		mArrayTVertsRefs2;				//!< Array to store texture-vertices references.
				Container		mArrayTVerts2;					//!< Array to store mapping coordinates. [Container only for UVs without W]
#endif
				Container		mMaterialInfo;					//!< Array to derive material informations

				udword			mNbFaces;						//!< Maximum (expected) number of faces (provided by user)
				udword			mNbVerts;						//!< Number of vertices in the original mesh (provided by user)
				udword			mNbTVerts;						//!< Number of mapping coordinates in the original mesh (provided by user)
				udword			mNbCVerts;						//!< Number of vertex colors in the original mesh (provided by user)
#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
				udword			mNbTVerts2;						//!< Number of mapping coordinates2 in the original mesh (provided by user)
#endif
				udword			mNbAvailableFaces;				//!< Number of faces in database (could be <NbFaces if zero-area faces are skipped)
				udword			mNbBuildVertices;				//!< TO BE DOCUMENTED
				udword			mNewVertsIndex;					//!< TO BE DOCUMENTED

				Point*			mVerts;							//!< Original/reduced list of vertices
				Point*			mTVerts;						//!< Original/reduced list of texture-vertices
				Point*			mCVerts;						//!< Original/reduced list of vertex-colors
#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
				Point*			mTVerts2;						//!< Original/reduced list of texture-vertices2
#endif
				MBFace*			mFacesArray;					//!< The original list of faces.
				MBVertex*		mVertexArray;					//!< The list of rebuilt vertices.
		// Faces permutation
				udword*			mOutToIn;						//!< New (consolidated) list to old list mapping
				udword			mCrossIndex;					//!< Running face index used to build mOutToIn
		// Normals computation / smoothing groups
				udword*			mFCounts;						//!< For each vertex, the number of faces sharing that vertex.
				udword*			mVOffsets;						//!< Radix-style offsets of mFCounts.
				udword*			mVertexToFaces;					//!< List of faces sharing each vertex.
				udword			mNbNorm;						//!< Number of normals currently computed in mArrayNormalInfo
		// Build settings
//				udword			mOptimizeFlags;					//!< TO BE DOCUMENTED
				bool			mKillZeroAreaFaces;				//!< true if degenerate triangles must be skipped (default)
				bool			mUseW;							//!< true if W coord must be used, else false (default)
				bool			mComputeVNorm;					//!< true if vertex-normals must be computed, else false (default)
				bool			mComputeFNorm;					//!< true if face-normals must be computed, else false (default)
				bool			mComputeNormInfo;				//!< true if normal information must be computed, else false (default)
				bool			mIndexedGeo;					//!< true if geometry must be exported as indexed (default), else false
				bool			mIndexedUVW;					//!< true if UVW must be exported as indexed, else false (default)
				bool			mIndexedColors;					//!< true if vertex-colors must be exported as indexed, else false (default)
				bool			mRelativeIndices;				//!< true if indices must be reset for each submesh, else false (default)
				bool			mIsSkin;						//!< true if the input mesh is a skin, else false (default)
				bool			mWeightNormalWithAngles;		//!< true if angles between edges must be used as weights in normal computation
				bool			mOptimizeVertexList;			//!< true if consolidated vertex list must further be optimized for cache coherence
		// Internal methods
				MeshBuilder2&	FreeUsedRam();
				bool			FixNullSmoothingGroups();
				bool			OptimizeTopology();
				bool			OptimizeXMappings(udword& nb_xverts, Point*& xverts, VertexCode vcode);
				bool			OptimizeGeometry();
				bool			ComputeNormals();
				bool			SaveXVertices();
				bool			ComputeSubmeshes();
				udword			BuildTrilist(const udword* list, udword nb_faces, sdword mat_id, udword sm_grp);
				udword			RebuildVertexData(const udword* faces, udword nb_faces, Container& array);
	};

#endif // ICEMESHBUILDER2_H
