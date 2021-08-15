///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	LITE chunk for ZB2 format.
 *	\file		ChunkLITE.h
 *	\author		Pierre Terdiman
 *	\date		August, 29, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CHUNKLITE_H
#define CHUNKLITE_H

	#define LITE_VERSION	1

	enum ZCB2_LITE_Flag
	{
		ZCB2_LITE_CAST_SHADOWS	= (1<<0),
		ZCB2_LITE_USED			= (1<<1),
	};

	class ZCB2_API LITEChunk : public PRSChunk
	{
		DECLARE_CHUNK(LITEChunk, mLITECore)

		DECLARE_STD_MEMBER(DiffuseColor,	RGBAColor)
		DECLARE_STD_MEMBER(AmbientColor,	RGBAColor)
		DECLARE_STD_MEMBER(SpecularColor,	RGBAColor)

		DECLARE_STD_FLAG(CastShadows,	ZCB2_LITE_CAST_SHADOWS)
		DECLARE_STD_FLAG(LightUsed,		ZCB2_LITE_USED)

/*		// Data access
		inline_			const RGBAColor&	GetDiffuseColor()	const	{ return mDiffuseColor;		}
		inline_			const RGBAColor&	GetSpecularColor()	const	{ return mSpecularColor;	}
		inline_			const RGBAColor&	GetAmbientColor()	const	{ return mAmbientColor;		}

		inline_			bool				GetCastShadows()	const	{ return (mLITECore.mFlags&ZCB2_LITE_CAST_SHADOWS)!=0;	}
		inline_			bool				GetLightUsed()		const	{ return (mLITECore.mFlags&ZCB2_LITE_USED)!=0;			}

		// Chunk definition
						bool				SetDiffuseColor(const RGBAColor& color);
						bool				SetSpecularColor(const RGBAColor& color);
						bool				SetAmbientColor(const RGBAColor& color);
						bool				SetCastShadows(bool flag);
						bool				SetLightUsed(bool flag);
		private:
		// Chunk data
						RGBAColor			mDiffuseColor;
						RGBAColor			mSpecularColor;
						RGBAColor			mAmbientColor;*/
	};

#endif // CHUNKLITE_H
