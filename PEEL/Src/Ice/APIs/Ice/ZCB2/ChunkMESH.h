///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	MESH chunk for ZB2 format.
 *	\file		ChunkMESH.h
 *	\author		Pierre Terdiman
 *	\date		August, 29, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CHUNKMESH_H
#define CHUNKMESH_H

	#define MESH_VERSION	1

	enum ZCB2_MESH_Flag
	{
		ZCB2_MESH_CAST_SHADOWS		= (1<<0),
		ZCB2_MESH_RECEIVE_SHADOWS	= (1<<1),
		ZCB2_MESH_MOTION_BLUR		= (1<<2),
		ZCB2_MESH_USER_POINT		= (1<<3),
	};

	class ZCB2_API MESHChunk : public PRSChunk
	{
		DECLARE_CHUNK(MESHChunk, mMESHCore)

		DECLARE_SUBCHUNK(Vertices,			PNTSChunk)
		DECLARE_SUBCHUNK(UVs,				PNTSChunk)
		DECLARE_SUBCHUNK(VertexColors,		PNTSChunk)
		DECLARE_SUBCHUNK(Faces,				FACEChunk)
		DECLARE_SUBCHUNK(TFaces,			FACEChunk)
		DECLARE_SUBCHUNK(CFaces,			FACEChunk)
		DECLARE_SUBCHUNK(MaterialIDs,		XIDSChunk)
		DECLARE_STD_ARRAY(SmoothingGroups,	udword)
		DECLARE_STD_MEMBER(CSID,			ubyte)
		DECLARE_STD_MEMBER(UserPoint,		Point)

		DECLARE_STD_FLAG(CastShadows,		ZCB2_MESH_CAST_SHADOWS)
		DECLARE_STD_FLAG(ReceiveShadows,	ZCB2_MESH_RECEIVE_SHADOWS)
		DECLARE_STD_FLAG(MotionBlur,		ZCB2_MESH_MOTION_BLUR)
	};

#endif // CHUNKMESH_H
