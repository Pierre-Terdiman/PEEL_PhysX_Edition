///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	This file contains all ZCB format-related constants.
 *	\file		IceZCBFormat.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEZCBFORMAT_H
#define ICEZCBFORMAT_H

	// Chunk versions
	#define	CHUNK_MAIN_VER			7
	#define	CHUNK_MESH_VER			9
	#define	CHUNK_CAMS_VER			2
	#define	CHUNK_LITE_VER			4
	#define	CHUNK_SHAP_VER			3
	#define	CHUNK_HELP_VER			3
	#define	CHUNK_TEXM_VER			4
	#define	CHUNK_MATL_VER			8
	#define	CHUNK_CTRL_VER			2
	#define	CHUNK_MOVE_VER			4
	#define	CHUNK_ZCB_VER			0x00040000			//!< Keep the low 16 bits null

	// Mesh flags.
	enum ZCBMeshFlag
	{
		ZCB_VFACE					= (1<<0),			//!< Mesh has vertex-faces.
		ZCB_TFACE					= (1<<1),			//!< Mesh has texture-faces.
		ZCB_CFACE					= (1<<2),			//!< Mesh has color-faces.
		ZCB_UVW						= (1<<3),			//!< UVW's are exported
		ZCB_W_DISCARDED				= (1<<4),			//!< W is discarded
		ZCB_VERTEX_COLORS			= (1<<5),			//!< Vertex colors are exported
		ZCB_ONE_BONE_PER_VERTEX		= (1<<6),			//!< Simple skin with one driving bone/vertex
		ZCB_CONVEX_HULL				= (1<<7),			//!< The convex hull has been exported
		ZCB_BOUNDING_SPHERE			= (1<<8),			//!< The bounding sphere has been exported
		ZCB_INERTIA_TENSOR			= (1<<9),			//!< The inertia tensor has been exported
		ZCB_QUANTIZED_VERTICES		= (1<<10),			//!< Vertices have been quantized
		ZCB_WORD_FACES				= (1<<11),			//!< Vertex references within faces are stored as words instead of dwords
		ZCB_COMPRESSED				= (1<<12),			//!< Mesh has been saved in a compression-friendly way
		ZCB_EDGE_VIS				= (1<<13),			//!< Edge visibility has been exported

		ZCB_CONSOLIDATION			= (1<<16),			//!< Mesh has been consolidated
		ZCB_FACE_NORMALS			= (1<<17),			//!< Export normals to faces
		ZCB_VERTEX_NORMALS			= (1<<18),			//!< Export normals to vertices
		ZCB_NORMAL_INFO				= (1<<19),			//!< Export NormalInfo
		ZCB_VERTEX_ALPHA			= (1<<20),			//!< Vertex alpha exported
		ZCB_DWORD_COLORS			= (1<<21),			//!< Vertex colors are stored as RGBA dwords in the red channel of the consolidation
	};

	// Scene flags
	#define ZCB_FILE_MASK			0x0000ffff			//!< Mask reserved bits
	enum ZCBFile
	{
		ZCB_FILE_SCENE				= 0x00001000,		//!< Complete 3D scene
		ZCB_FILE_MOTION				= 0x00001100,		//!< Motion file

		ZCB_FILE_FORCE_DWORD		= 0x7fffffff
	};

	// Object types
	enum ZCBObjType
	{
		ZCB_OBJ_UNDEFINED			= 0,				//!< Undefined object
		ZCB_OBJ_CAMERA				= 1,				//!< A camera
		ZCB_OBJ_LIGHT				= 2,				//!< A light
		ZCB_OBJ_MESH				= 3,				//!< A mesh
		ZCB_OBJ_BPATCH				= 4,				//!< A b-patch
		ZCB_OBJ_CONTROLLER			= 5,				//!< A controller
		ZCB_OBJ_HELPER				= 6,				//!< A helper
		ZCB_OBJ_MATERIAL			= 7,				//!< A material
		ZCB_OBJ_TEXTURE				= 8,				//!< A texture
		ZCB_OBJ_MOTION				= 9,				//!< A character motion
		ZCB_OBJ_SHAPE				= 10,				//!< A shape

		ZCB_OBJ_FORCE_DWORD			= 0x7fffffff
	};

	// Controllers flags
	enum ZCBCtrlType
	{
		ZCB_CTRL_NONE				= 0,				//!< No controller
		ZCB_CTRL_FLOAT				= 1,				//!< Float controller
		ZCB_CTRL_VECTOR				= 2,				//!< Vector controller
		ZCB_CTRL_QUAT				= 3,				//!< Quaternion controller
		ZCB_CTRL_SCALE				= 4,				//!< Scale controller
		ZCB_CTRL_PR					= 5,				//!< PR controller
		ZCB_CTRL_PRS				= 6,				//!< PRS controller
		ZCB_CTRL_VERTEXCLOUD		= 7,				//!< Morph controller

		ZCB_CTYPE_FORCE_DWORD		= 0x7fffffff
	};

	enum ZCBCtrlMode
	{
		ZCB_CTRL_SAMPLES			= 1,				//!< Samples
		ZCB_CTRL_KEYFRAMES			= 2,				//!< Keyframes
		ZCB_CTRL_PROCEDURAL			= 3,				//!< Procedural

		ZCB_CMODE_FORCE_DWORD		= 0x7fffffff
	};

	// Compression flags
	enum ZCBCompression
	{
		ZCB_COMPRESSION_NONE		= 0,				//!< Not compressed
		ZCB_COMPRESSION_ZLIB		= 1,				//!< Compressed with ZLib
		ZCB_COMPRESSION_BZIP2		= 2,				//!< Compressed with BZip2	
		ZCB_COMPRESSION_CUSTOM		= 3,				//!< Custom compression

		ZCB_COMPRESSION_FORCE_DWORD	= 0x7fffffff
	};

	// Primitives
	enum ZCBPrimitiveType
	{
		ZCB_PRIM_UNDEFINED			= 0,
		// Standard primitives
		ZCB_PRIM_BOX				= 1,
		ZCB_PRIM_SPHERE				= 2,
		ZCB_PRIM_GEOSPHERE			= 3,
		ZCB_PRIM_CYLINDER			= 4,
		ZCB_PRIM_CONE				= 5,
		ZCB_PRIM_TORUS				= 6,
		ZCB_PRIM_TUBE				= 7,
		ZCB_PRIM_TEAPOT				= 8,
		ZCB_PRIM_PLANE				= 11,
		// Extended primitives
		ZCB_PRIM_HEDRA				= 9,
		ZCB_PRIM_CAPSULE			= 10,

		ZCB_PRIM_FORCE_DWORD		= 0x7fffffff
	};

	// Shapes
	enum ZCBShapeType
	{
		ZCB_SHAP_UNDEFINED			= 0,
		ZCB_SHAP_SPLINE				= 1,
		ZCB_SHAP_NGON				= 2,
		ZCB_SHAP_DONUT				= 3,
		ZCB_SHAP_STAR				= 4,
		ZCB_SHAP_RECTANGLE			= 5,
		ZCB_SHAP_HELIX				= 6,
		ZCB_SHAP_ELLIPSE			= 7,
		ZCB_SHAP_CIRCLE				= 8,
		ZCB_SHAP_TEXT				= 9,
		ZCB_SHAP_ARC				= 10,

		ZCB_SHAP_FORCE_DWORD		= 0x7fffffff
	};

	// Helpers
	enum ZCBHelperType
	{
		ZCB_HELPER_DUMMY,
		ZCB_HELPER_GIZMO_BOX,
		ZCB_HELPER_GIZMO_CYLINDER,
		ZCB_HELPER_GIZMO_SPHERE,
		ZCB_HELPER_TAPE,
		ZCB_HELPER_GRID,
		ZCB_HELPER_POINT,
		ZCB_HELPER_PROTRACTOR,
		ZCB_HELPER_CROWD,
		ZCB_HELPER_COMPASS,
		ZCB_HELPER_DELEGATE,
		ZCB_HELPER_BILLBOARD,

		ZCB_HELPER_FORCE_DWORD		= 0xffffffff,
	};

	// Camera types from MAX.
	enum ZCBCameraType
	{
		ZCB_CAMERA_FREE			= 0,	//!< Free camera
		ZCB_CAMERA_TARGET		= 1,	//!< Target camera
		ZCB_CAMERA_PARALLEL		= 2,	//!< Parallel camera

		ZCB_CAMERA_FORCE_DWORD	= 0x7fffffff
	};

	// FOV types
	enum ZCBFOVType
	{
		ZCB_FOV_HORIZONTAL	= 0,	//!< Horizontal FOV
		ZCB_FOV_VERTICAL	= 1,	//!< Vertical FOV
		ZCB_FOV_DIAGONAL	= 2,	//!< Diagonal FOV

		ZCB_FOV_FORCE_DWORD	= 0x7fffffff
	};

	// Light types from MAX. Usual rendering APIs only keep Point, Spot and Directional lights.
	enum ZCBLightType
	{
		ZCB_LIGHT_OMNI			= 0,	//!< Omnidirectional		(PointLight)
		ZCB_LIGHT_TSPOT			= 1,	//!< Targeted				(SpotLight)
		ZCB_LIGHT_DIR			= 2,	//!< Directional			(DirectionalLight)
		ZCB_LIGHT_FSPOT			= 3,	//!< Free					(SpotLight)
		ZCB_LIGHT_TDIR			= 4,	//!< Targeted directional	(DirectionalLight)

		ZCB_LIGHT_FORCE_DWORD	= 0x7fffffff
	};

	// Spotlight shape.
	enum ZCBSpotShape
	{
		ZCB_SPOT_RECT			= 0,	//!< Rectangle
		ZCB_SPOT_CIRCLE			= 1,	//!< Circle

		ZCB_SPOT_FORCE_DWORD	= 0x7fffffff
	};

	// Decay type
	enum ZCBDecayType
	{
		ZCB_DECAY_TYPE_NONE			= 0,
		ZCB_DECAY_TYPE_INV			= 1,
		ZCB_DECAY_TYPE_INVSQ		= 2,

		ZCB_DECAY_TYPE_FORCE_DWORD	= 0x7fffffff
	};

	// Transparency types from MAX 3.1. ("Advanced Transparency" in the Material Editor)
	enum ZCBTranspaType
	{
		ZCB_TRANSPA_SUBTRACTIVE	= 0,
		ZCB_TRANSPA_ADDITIVE	= 1,
		ZCB_TRANSPA_FILTER		= 2,

		ZCB_TRANSPA_FORCE_DWORD	= 0x7fffffff
	};

	// Shading mode
	enum ZCBShadingMode
	{
		ZCB_SHADING_BLINN		= 0,
		ZCB_SHADING_PHONG		= 1,
		ZCB_SHADING_METAL		= 2,
		ZCB_SHADING_ANISO		= 3,

		ZCB_SHADING_FORCE_DWORD	= 0x7fffffff
	};

#endif // ICEZCBFORMAT_H
