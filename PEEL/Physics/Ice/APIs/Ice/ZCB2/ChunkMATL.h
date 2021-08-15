///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	MATL chunk for ZB2 format.
 *	\file		ChunkMATL.h
 *	\author		Pierre Terdiman
 *	\date		August, 29, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CHUNKMATL_H
#define CHUNKMATL_H

#ifndef _WIN64	// "temporary"

	#define MATL_VERSION	1

	enum ZCB2_MATL_Flag
	{
		ZCB2_MATL_TEXTURE_MATRIX	= (1<<0),
		ZCB2_MATL_CROP_VALUES		= (1<<1),
	};

	class ZCB2_API MATLChunk : public ROBJChunk
	{
		DECLARE_CHUNK(MATLChunk, mMATLCore)

		DECLARE_STD_MEMBER(DiffuseMapID,	sdword)
		DECLARE_STD_MEMBER(ReflexionMapID,	sdword)
		DECLARE_STD_MEMBER(MaterialProps,	MaterialProps)
		DECLARE_STD_MEMBER(CullMode,		CULLMODE)

		// Data access
		inline_	bool				HasTextureMatrix()						const	{ return (mMATLCore.mFlags&ZCB2_MATL_TEXTURE_MATRIX)!=0;	}
		inline_	bool				HasCropValues()							const	{ return (mMATLCore.mFlags&ZCB2_MATL_CROP_VALUES)!=0;		}
//		inline_	sdword				GetDiffuseMapID()						const	{ return mDiffuseMapID;								}
//		inline_	sdword				GetReflexionMapID()						const	{ return mReflexionMapID;							}
//		inline_	void				GetMaterialProps(MaterialProps& props)	const	{ props = mProps;									}
//		inline_	CULLMODE			GetCullMode()							const	{ return mCullMode;									}
		inline_	void				GetTextureMatrix(Matrix4x4& matrix)		const	{ matrix = mTextureMatrix;							}
		inline_	void				GetCropValues(CropValues& crop)			const	{ crop = mCrop;										}

		// Chunk definition
//				bool				SetDiffuseMapID(sdword id);
//				bool				SetReflexionMapID(sdword id);
//				bool				SetMaterialProps(const MaterialProps& props);
//				bool				SetCullMode(CULLMODE cullmode);
				bool				SetMatrix(const Matrix4x4& texture_matrix);
				bool				SetCropValues(const CropValues& crop);
		private:
		// Chunk data
//				sdword				mDiffuseMapID;
//				sdword				mReflexionMapID;
//				MaterialProps		mProps;
//				CULLMODE			mCullMode;
				Matrix4x4			mTextureMatrix;
				CropValues			mCrop;
	};

#endif

#endif // CHUNKMATL_H
