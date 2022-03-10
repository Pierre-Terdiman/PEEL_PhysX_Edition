///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to import ZCB files.
 *	\file		IceZCBBreaker.cpp
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an importer for ZCB files.
 *	- "ZCB" means "Zappy's Custom Binary"
 *	- at first it was only a test format, to check exported MAX data was ok
 *	- then people started to use it as-is !
 *	- so I kept it alive, but you may want to replace it with your own format in the end
 *
 *	\class		ZCBBreaker
 *	\author		Pierre Terdiman
 *	\version	1.0
 *	\date		2000-2009
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "StdAfx.h"

#include "IceZCBFormat.h"
#include "IceZCBBreaker.h"

//using namespace IceRenderManager;

// Define this to make IDs unique among 3D objects/materials/textures
#define ZCB_MAKE_UNIQUE_IDS

// Lame implementation of a progress bar.
#define ZCB_PROGRESS_BAR

#define ZCB_CHECKALLOC(x)	{ if(!x) { ZCBImportError("Out of memory.", ZCB_ERROR_OUT_OF_MEMORY); return false;} }
#define ZCB_CHECKVERSION(file_version, current_version, chunk_name)																									\
		{		if(file_version>current_version)	{ ZCBImportError("Obsolete code! Get the latest ZCB reader.", ZCB_ERROR_UNKNOWN_VERSION);	return false;	}	\
		else	if(file_version<current_version)	ZCBImportError(_F("Found obsolete %s chunk. Please resave the file.", chunk_name), ZCB_ERROR_OBSOLETE_FILE);	}

// Ugly but...
static bool gHasPivot = false;
static udword gZCBVersion = 0;

#ifdef ZCB_PROGRESS_BAR
/*static ProgressBarParams* gParams = null;
static void InitProgressBar()
{
	gParams = ICE_NEW(ProgressBarParams);
}

static void ReleaseProgressBar()
{
	DELETESINGLE(gParams);
}*/
static GUIProgressBar* gProgressBar = null;
static void InitProgressBar()
{
	GUIRequestInterface* it = GetGUIRequestInterface();
	if(it)
		gProgressBar = it->CreateProgressBar();
}

static void ReleaseProgressBar()
{
	GUIRequestInterface* it = GetGUIRequestInterface();
	if(it)
		it->DeleteProgressBar(gProgressBar);
	gProgressBar = null;
}
#endif

static bool Cleanup()
{
#ifdef ZCB_PROGRESS_BAR
/*	gParams->mRequestCode = RQ_DELETE_PROGRESS_BAR;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);
	ReleaseProgressBar();*/
	ReleaseProgressBar();
#endif
	return false;
}

static void UpdateProgressBar0()
{
#ifdef ZCB_PROGRESS_BAR
//	gParams->mRequestCode = RQ_UPDATE_PROGRESS_BAR0;
//	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);
	if(gProgressBar)
		gProgressBar->Update0();
#endif
}

static void UpdateProgressBar1()
{
#ifdef ZCB_PROGRESS_BAR
//	gParams->mRequestCode = RQ_UPDATE_PROGRESS_BAR1;
//	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);
	if(gProgressBar)
		gProgressBar->Update1();
#endif
}

static void PatchTextureID(sdword& id)
{
#ifdef ZCB_MAKE_UNIQUE_IDS
	if(id!=-1)	id+=0x10000000;
#endif
}

static void PatchMaterialID(sdword& id)
{
#ifdef ZCB_MAKE_UNIQUE_IDS
	if(id!=-1)	id+=0x20000000;
#endif
}

static void PatchControllerID(sdword& id)
{
#ifdef ZCB_MAKE_UNIQUE_IDS
	if(id!=-1)	id+=0x40000000;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBComponents::ZCBComponents()
{
	mNbMeshes		= 0;
	mNbDerived		= 0;
	mNbCameras		= 0;
	mNbLights		= 0;
	mNbShapes		= 0;
	mNbHelpers		= 0;
	mNbControllers	= 0;
	mNbMaterials	= 0;
	mNbTextures		= 0;
	mNbUnknowns		= 0;
	mNbInvalidNodes	= 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBComponents::~ZCBComponents()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBSceneInfo::ZCBSceneInfo() : mFirstFrame(0), mLastFrame(0), mFrameRate(0), mDeltaTime(0), mGlobalScale(1.0f)
{
	mBackColor		.Zero();
	mAmbientColor	.Zero();
	mGravity.Zero();
	mRestitution		= 0.0f;
	mStaticFriction		= 0.0f;
	mFriction			= 0.0f;
	mGroundPlane		= false;
	mCollisionDetection	= true;
	mTesselation		= INVALID_ID;
	mLightingMode		= INVALID_ID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBSceneInfo::~ZCBSceneInfo()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBBaseInfo::ZCBBaseInfo() : mID(INVALID_ID), mParentID(INVALID_ID), mTargetID(INVALID_ID), mMasterID(INVALID_ID)
{
	mPrs.Identity();
	mWireColor			= 0x7fffffff;
	mLocalPRS			= false;
	mD3DCompliant		= false;
	mGroup				= false;
	mIsHidden			= false;

	mLinearVelocity.Zero();
	mAngularVelocity.Zero();
	mDensity			= 0.0f;
	mMass				= 0.0f;
	mSamplingDensity	= 0;
	mResetPivot			= false;
	mIsCollidable		= true;
	mLockPivot			= false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBBaseInfo::~ZCBBaseInfo()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBCameraInfo::ZCBCameraInfo()
{
	mType			= ZCB_CAMERA_TARGET;
	mOrthoCam		= false;
	mFOV			= 0.0f;
	mFOVType		= ZCB_FOV_HORIZONTAL;
	mNearClip		= 0.0f;
	mFarClip		= 0.0f;
	mTDist			= 0.0f;
	mHLineDisplay	= 0;
	mEnvNearRange	= 0.0f;
	mEnvFarRange	= 0.0f;
	mEnvDisplay		= 0;
	mManualClip		= 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBCameraInfo::~ZCBCameraInfo()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBLightInfo::ZCBLightInfo()
{
	// ...
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBLightInfo::~ZCBLightInfo()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBMaterialInfo::ZCBMaterialInfo()
{
	mID						= INVALID_ID;

	mAmbientMapID			= INVALID_ID;
	mDiffuseMapID			= INVALID_ID;
	mSpecularMapID			= INVALID_ID;
	mShininessMapID			= INVALID_ID;
	mShiningStrengthMapID	= INVALID_ID;
	mSelfIllumMapID			= INVALID_ID;
	mOpacityMapID			= INVALID_ID;
	mFilterMapID			= INVALID_ID;
	mBumpMapID				= INVALID_ID;
	mReflexionMapID			= INVALID_ID;
	mRefractionMapID		= INVALID_ID;
	mDisplacementMapID		= INVALID_ID;
	mDecalMapID				= INVALID_ID;
	mDetailMapID			= INVALID_ID;
	mBillboardMapID			= INVALID_ID;

	mAmbientCoeff			= 1.0f;
	mDiffuseCoeff			= 1.0f;
	mSpecularCoeff			= 1.0f;
	mShininessCoeff			= 1.0f;
	mShiningStrengthCoeff	= 1.0f;
	mSelfIllumCoeff			= 1.0f;
	mOpacityCoeff			= 1.0f;
	mFilterCoeff			= 1.0f;
	mBumpCoeff				= 1.0f;
	mReflexionCoeff			= 1.0f;
	mRefractionCoeff		= 1.0f;
	mDisplacementCoeff		= 1.0f;

	mMtlAmbientColor		.Zero();
	mMtlDiffuseColor		.Zero();
	mMtlSpecularColor		.Zero();
	mMtlFilterColor			.Zero();

	mShading				= ZCB_SHADING_FORCE_DWORD;
	mSoften					= false;
	mFaceMap				= false;
	mTwoSided				= false;
	mWire					= false;
	mWireUnits				= false;
	mFalloffOut				= false;
	mTransparencyType		= ZCB_TRANSPA_FORCE_DWORD;

	mShininess				= 0.0f;
	mShiningStrength		= 0.0f;
	mSelfIllum				= 0.0f;
	mOpacity				= 1.0f;
	mOpaFalloff				= 0.0f;
	mWireSize				= 0.0f;
	mIOR					= 0.0f;

	mBounce					= 0.0f;
	mStaticFriction			= 0.0f;
	mSlidingFriction		= 0.0f;

	mSelfIllumOn			= false;
	mSelfIllumValue			= 0.0f;
	mSelfIllumColor			.Zero();

	mWrapU_Diffuse			= true;
	mMirrorU_Diffuse		= false;
	mWrapV_Diffuse			= true;
	mMirrorV_Diffuse		= false;

	mWrapU_Filter			= true;
	mMirrorU_Filter			= false;
	mWrapV_Filter			= true;
	mMirrorV_Filter			= false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBMaterialInfo::~ZCBMaterialInfo()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBTextureInfo::ZCBTextureInfo() :
	mID						(INVALID_ID),
	mWidth					(0),
	mHeight					(0),
	mBitmap					(null),
	mHasAlpha				(false),
	mIsBitmapIncluded		(false),
	mExternalSourceHasAlpha	(false),
	mWrapU					(true),
	mMirrorU				(false),
	mWrapV					(true),
	mMirrorV				(false)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBTextureInfo::~ZCBTextureInfo()
{
	ICE_FREE(mBitmap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBNativeMeshInfo::ZCBNativeMeshInfo()
{
	mNbFaces		= 0;
	mNbVerts		= 0;
	mNbTVerts		= 0;
	mNbCVerts		= 0;
	mFlags			= 0;
	mFaces			= null;
	mTFaces			= null;
	mCFaces			= null;
	mFaceProperties	= null;
	mVerts			= null;
	mTVerts			= null;
	mCVerts			= null;
	mVertexAlpha	= null;
	mParity			= false;

	mBonesNb		= null;
	mBonesID		= null;
	mOffsetVectors	= null;
	mWeights		= null;
	mSkeleton		= null;
	mNbBones		= 0;

	mNbHullVerts	= 0;
	mNbHullFaces	= 0;
	mHullVerts		= null;
	mHullFaces		= null;

	mBSCenter.Zero();
	mBSRadius		= 0.0f;

	mCOM.Zero();
	mComputedMass	= 0.0f;
	ZeroMemory(mInertiaTensor, 9*sizeof(float));
	ZeroMemory(mCOMInertiaTensor, 9*sizeof(float));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBNativeMeshInfo::~ZCBNativeMeshInfo()
{
	ICE_FREE(mHullFaces);
	DELETEARRAY(mHullVerts);

	DELETEARRAY(mSkeleton);
	ICE_FREE(mWeights);
	DELETEARRAY(mOffsetVectors);
	ICE_FREE(mBonesID);
	ICE_FREE(mBonesNb);

	ICE_FREE(mFaces);
	ICE_FREE(mTFaces);
	ICE_FREE(mCFaces);
	DELETEARRAY(mFaceProperties);
	DELETEARRAY(mVerts);
	DELETEARRAY(mTVerts);
	DELETEARRAY(mCVerts);
	ICE_FREE(mVertexAlpha);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBMeshInfo::ZCBMeshInfo()
{
	mIsCollapsed	= false;
	mIsSkeleton		= false;
	mIsInstance		= false;
	mIsTarget		= false;
	mIsConvertible	= false;
	mIsSkin			= false;
	mCastShadows	= false;
	mReceiveShadows	= false;
	mMotionBlur		= false;

	mCharID			= INVALID_ID;
	mCSID			= INVALID_ID;

	mCleanMesh		= null;

	mNbColors		= 0;
	mColors			= null;

	mPrimParams		= null;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBMeshInfo::~ZCBMeshInfo()
{
	DELETESINGLE(mPrimParams);
	DELETESINGLE(mCleanMesh);
	DELETEARRAY(mColors);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBShapeInfo::ZCBShapeInfo() : mType(ZCB_SHAP_UNDEFINED), mNbLines(0), mNbVerts(null), mClosed(null), mVerts(null), mTotalNbVerts(0), mMatID(INVALID_ID)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBShapeInfo::~ZCBShapeInfo()
{
	DELETEARRAY(mVerts);
	ICE_FREE(mClosed);
	ICE_FREE(mNbVerts);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBHelperInfo::ZCBHelperInfo() : mIsGroupHead(false)
{
	mHelperType		= ZCB_HELPER_DUMMY;
	mIsGroupHead	= false;
	mLength			= 0.0f;
	mWidth			= 0.0f;
	mHeight			= 0.0f;
	mRadius			= 0.0f;
	mHemi			= false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBHelperInfo::~ZCBHelperInfo()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBControllerInfo::ZCBControllerInfo()
{
	mObjectID		= INVALID_ID;
	mOwnerID		= INVALID_ID;
	mOwnerType		= ZCB_OBJ_UNDEFINED;

	mCtrlType		= ZCB_CTRL_NONE;
	mCtrlMode		= ZCB_CTRL_SAMPLES;

	mNbSamples		= 0;
	mSamplingRate	= 0;
	mSamples		= null;
	mNbVertices		= 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBControllerInfo::~ZCBControllerInfo()
{
	ICE_FREE(mSamples);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBBreaker::ZCBBreaker() : mFileType(ZCB_FILE_FORCE_DWORD), mImportArray(null), mZCBGlobalVersion(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZCBBreaker::~ZCBBreaker()
{
	ReleaseRam();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Frees used memory.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ZCBBreaker::ReleaseRam()
{
	DELETESINGLE(mImportArray);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports a ZCB scene.
 *	\param		filename	[in] the scene's filename
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::Import(const char* filename)
{
	// Find the file somewhere in registered paths
	VirtualFileHandle FoundFile;

	// Check the file exists
	if(!FindFile(filename, &FoundFile))
	{
		ZCBImportError("File not found.", ZCB_ERROR_FILE_NOT_FOUND);
		return false;
	}

	// Release possibly already existing array
	ReleaseRam();

#ifdef USE_STREAM_INTERFACE
//	mImportArray = ICE_NEW(VirtualFile)(FoundFile);

		VirtualFile File(FoundFile);
		udword Length;
		ubyte* tmp = File.Load(Length);
		mImportArray = ICE_NEW(BufferReadStream)(tmp, Length);
#else
	// Create a new array
//		IceFile File(FoundFile, "rb");
		VirtualFile File(FoundFile);
		udword Length;
		ubyte* tmp = File.Load(Length);
		mImportArray = ICE_NEW(CustomArray)(Length, tmp);
		mImportArray->Reset();

//	mImportArray = ICE_NEW(CustomArray)(FoundFile);
#endif
	ZCB_CHECKALLOC(mImportArray);

	// Parse the array
	return Import();
}

bool ZCBBreaker::Import(const VirtualFile& file)
{
	// Release possibly already existing array
	ReleaseRam();

#ifdef USE_STREAM_INTERFACE
//	mImportArray = ICE_NEW(VirtualFile)(FoundFile);

//		VirtualFile File(handle);
		udword Length;
		ubyte* tmp = file.Load(Length);
		mImportArray = ICE_NEW(BufferReadStream)(tmp, Length);
#else
	// Create a new array
//		IceFile File(FoundFile, "rb");
		VirtualFile File(handle);
		udword Length;
		ubyte* tmp = File.Load(Length);
		mImportArray = ICE_NEW(CustomArray)(Length, tmp);
		mImportArray->Reset();

//	mImportArray = ICE_NEW(CustomArray)(FoundFile);
#endif
	ZCB_CHECKALLOC(mImportArray);

	// Parse the array
	return Import();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports a ZCB scene.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::Import()
{
	// Check format signature
	const ubyte b1 = mImportArray->ReadByte();
	const ubyte b2 = mImportArray->ReadByte();
	const ubyte b3 = mImportArray->ReadByte();

	// The three first bytes must be "ZCB"
	if(b1!='Z' || b2!='C' || b3!='B')
	{
		ZCBImportError("Invalid ZCB file.", ZCB_ERROR_INVALID_FILE);
		return false;
	}

	// The fourth byte can be '!' for normal files and 'P' for packed files.
	const ubyte b4 = mImportArray->ReadByte();
	if(b4=='P')
	{
#ifdef USE_STREAM_INTERFACE
		ASSERT(0);
		return false;
#else
		// This is a packed ZCB file. ZCB files are packed with ZLib or BZip2 only, so that anyone can read them back.
		const udword Compression	= mImportArray->ReadDword();	// Compression scheme
		const udword OriginalSize	= mImportArray->ReadDword();	// Size of the forthcoming depacked buffer
		const udword PackedSize		= mImportArray->ReadDword();	// Size of the packed buffer

		// Get some bytes for the depacked buffer
		ubyte* Depacked = (ubyte*)ICE_ALLOC_TMP(sizeof(ubyte)*OriginalSize);
		ZCB_CHECKALLOC(Depacked);

		if(Compression==ZCB_COMPRESSION_ZLIB)
		{
#ifdef FLEXINESDK_H
			if(!Decompress(Depacked, OriginalSize, mImportArray->GetAddress(), PackedSize, "ZLIB"))	return false;
#else
	#ifdef _ZLIB_H
			// Use ZLib to depack the file
			int ErrorCode = uncompress((Bytef*)Depacked, (uLongf*)&OriginalSize, (Bytef*)mImportArray->GetAddress(), PackedSize);
			if(ErrorCode!=Z_OK)
			{
				ZCBImportError("Depacking failed.", ZCB_ERROR_CANT_DEPACK);
				return false;
			}
	#else
			ZCBImportError("ZLIB support not available.", ZCB_ERROR_CANT_DEPACK);
			return false;
	#endif
#endif
		}
		else if(Compression==ZCB_COMPRESSION_BZIP2)
		{
#ifdef FLEXINESDK_H
			if(!Decompress(Depacked, OriginalSize, mImportArray->GetAddress(), PackedSize, "BZIP2"))	return false;
#else
	#ifdef _BZLIB_H
			// Use BZip2 to depack the file
			int ErrorCode = BZ2_bzBuffToBuffDecompress((char*)Depacked, &OriginalSize, (char*)mImportArray->GetAddress(), PackedSize, 0, 0);
			if(ErrorCode!=BZ_OK)
			{
				ZCBImportError("Depacking failed.", ZCB_ERROR_CANT_DEPACK);
				return false;
			}
	#else
			ZCBImportError("BZIP2 support not available.", ZCB_ERROR_CANT_DEPACK);
			return false;
	#endif
#endif
		}
		else
		{
			ZCBImportError("Depacking failed.", ZCB_ERROR_CANT_DEPACK);
			return false;
		}

		// Release packed buffer
		ReleaseRam();

		mImportArray = ICE_NEW(CustomArray)(OriginalSize, Depacked);
		ZCB_CHECKALLOC(mImportArray);
		mImportArray->Reset();

		// Release depacked buffer
		ICE_FREE(Depacked);

		// And wrap it up
		return Import();
#endif
	}
	else if(b4=='!')
	{
		// This is a valid ZCB file. Get the type.
		const udword Data = mImportArray->ReadDword();
		// Since 1.14 we need to mask out reserved bits & get back the global version
		mZCBGlobalVersion = Data>>16;
		mFileType = ZCBFile(Data & ZCB_FILE_MASK);
		gZCBVersion = mZCBGlobalVersion;
	}
	else
	{
		// Uh... unknown header... corrupted file ?
		ZCBImportError("Invalid ZCB file.", ZCB_ERROR_INVALID_FILE);
		return false;
	}

	// Import the file
	if(mFileType==ZCB_FILE_SCENE)
	{
#ifdef ZCB_PROGRESS_BAR
/*		InitProgressBar();

		gParams->mRequestCode = RQ_CREATE_PROGRESS_BAR;
		GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);

		gParams->mMin = 0;
		gParams->mMax = 9;
		gParams->mRequestCode = RQ_SET_RANGE_PROGRESS_BAR0;
		GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);

		gParams->mText = "Importing file...";
		gParams->mRequestCode = RQ_SET_TEXT_PROGRESS_BAR0;
		GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);*/

		InitProgressBar();
		if(gProgressBar)
		{
			gProgressBar->SetRange0(0, 9);
			gProgressBar->SetText0("Importing file...");
		}

#endif
		// Find MAIN chunk
		if(mImportArray->GetChunk(mZCBGlobalVersion ? "MAINCHUNK" : "MAIN"))
		{
			// Get chunk version
			mMAINVersion	= mImportArray->ReadDword();
			ZCB_CHECKVERSION(mMAINVersion, CHUNK_MAIN_VER, "MAIN");

			// Ugly but....
			if(mMAINVersion>=2)	gHasPivot = true;
			else				gHasPivot = false;

			// Fill a scene structure
			ZCBSceneInfo	SceneInfo;
			// Time-related info
			SceneInfo.mFirstFrame		= mImportArray->ReadDword();
			SceneInfo.mLastFrame		= mImportArray->ReadDword();
			SceneInfo.mFrameRate		= mImportArray->ReadDword();
			SceneInfo.mDeltaTime		= mImportArray->ReadDword();
			// Background color
			SceneInfo.mBackColor.x		= mImportArray->ReadFloat();
			SceneInfo.mBackColor.y		= mImportArray->ReadFloat();
			SceneInfo.mBackColor.z		= mImportArray->ReadFloat();
			// Global ambient color
			SceneInfo.mAmbientColor.x	= mImportArray->ReadFloat();
			SceneInfo.mAmbientColor.y	= mImportArray->ReadFloat();
			SceneInfo.mAmbientColor.z	= mImportArray->ReadFloat();

			// Scene info
			if(mMAINVersion>=2)	SceneInfo.mSceneInfo.Set((const char*)mImportArray->ReadString());
			if(mMAINVersion>=4)
			{
				SceneInfo.mSceneHelpText.Set((const char*)mImportArray->ReadString());

				if(mMAINVersion>=5)
				{
					SceneInfo.mTesselation = mImportArray->ReadDword();
				}
				if(mMAINVersion>=6)
				{
					SceneInfo.mLightingMode = mImportArray->ReadDword();
				}

				// Scene physics
				SceneInfo.mGravity.x = mImportArray->ReadFloat();
				SceneInfo.mGravity.y = mImportArray->ReadFloat();
				SceneInfo.mGravity.z = mImportArray->ReadFloat();
				SceneInfo.mRestitution = mImportArray->ReadFloat();
				SceneInfo.mStaticFriction = mImportArray->ReadFloat();
				SceneInfo.mFriction = mImportArray->ReadFloat();
				SceneInfo.mGroundPlane = mImportArray->ReadBool();
				SceneInfo.mCollisionDetection = mImportArray->ReadBool();
			}

			if(mMAINVersion>=3)	SceneInfo.mGlobalScale = mImportArray->ReadFloat();

			if(mMAINVersion>=7)
			{
				mMeshOffset			= mImportArray->ReadDword();
				mCameraOffset		= mImportArray->ReadDword();
				mLightOffset		= mImportArray->ReadDword();
				mShapeOffset		= mImportArray->ReadDword();
				mHelperOffset		= mImportArray->ReadDword();
				mTexmapOffset		= mImportArray->ReadDword();
				mMaterialOffset		= mImportArray->ReadDword();
				mControllerOffset	= mImportArray->ReadDword();
				mMotionOffset		= mImportArray->ReadDword();
			}
			else
			{
				mMeshOffset			= INVALID_ID;
				mCameraOffset		= INVALID_ID;
				mLightOffset		= INVALID_ID;
				mShapeOffset		= INVALID_ID;
				mHelperOffset		= INVALID_ID;
				mTexmapOffset		= INVALID_ID;
				mMaterialOffset		= INVALID_ID;
				mControllerOffset	= INVALID_ID;
				mMotionOffset		= INVALID_ID;
			}

			// Get number of expected elements
			mNbMeshes		= SceneInfo.mNbMeshes		= mImportArray->ReadDword();
			mNbDerived		= SceneInfo.mNbDerived		= mImportArray->ReadDword();
			mNbCameras		= SceneInfo.mNbCameras		= mImportArray->ReadDword();
			mNbLights		= SceneInfo.mNbLights		= mImportArray->ReadDword();
			mNbShapes		= SceneInfo.mNbShapes		= mImportArray->ReadDword();
			mNbHelpers		= SceneInfo.mNbHelpers		= mImportArray->ReadDword();
			mNbControllers	= SceneInfo.mNbControllers	= mImportArray->ReadDword();
			mNbMaterials	= SceneInfo.mNbMaterials	= mImportArray->ReadDword();
			mNbTextures		= SceneInfo.mNbTextures		= mImportArray->ReadDword();
			mNbUnknowns		= SceneInfo.mNbUnknowns		= mImportArray->ReadDword();
			mNbInvalidNodes	= SceneInfo.mNbInvalidNodes	= mImportArray->ReadDword();

			// Call the app
			NewScene(SceneInfo);

			UpdateProgressBar0();
		}
		else
		{
			ZCBImportError("Chunk MAIN not found!", ZCB_ERROR_CHUNK_NOT_FOUND);
			return Cleanup();
		}

		// Import everything
		ImportMotion			(*mImportArray);
		if(!ImportCameras		(*mImportArray))	return Cleanup();	UpdateProgressBar0();
		if(!ImportLights		(*mImportArray))	return Cleanup();	UpdateProgressBar0();
		if(!ImportTextures		(*mImportArray))	return Cleanup();	UpdateProgressBar0();
		if(!ImportMaterials		(*mImportArray))	return Cleanup();	UpdateProgressBar0();
		if(!ImportMeshes		(*mImportArray))	return Cleanup();	UpdateProgressBar0();
		if(!ImportShapes		(*mImportArray))	return Cleanup();	UpdateProgressBar0();
		if(!ImportHelpers		(*mImportArray))	return Cleanup();	UpdateProgressBar0();
		if(!ImportControllers	(*mImportArray))	return Cleanup();	UpdateProgressBar0();

		Cleanup();
	}
	else if(mFileType==ZCB_FILE_MOTION)
	{
		if(!ImportMotion		(*mImportArray))	return false;
	}
	else
	{
		ZCBImportError("Invalid ZCB file.", ZCB_ERROR_INVALID_FILE);
		return false;
	}

	// Release array
	DELETESINGLE(mImportArray);

	return true;
}

#define LOAD_CHUNK(offset, str0, str1)										\
	bool Status;															\
	if(offset==INVALID_ID)													\
	{																		\
		Status = importer.GetChunk(mZCBGlobalVersion ? str0 : str1)!=null;	\
	}																		\
	else																	\
	{																		\
		if(!offset)	Status = false;											\
		else																\
		{																	\
			Status = true;													\
			importer.Reset(offset);											\
			importer.ReadDword();											\
			importer.ReadDword();											\
			importer.ReadByte();											\
		}																	\
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports a BIPED motion
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportMotion(ImportArray& importer)
{
	// Find MOVE chunk
	LOAD_CHUNK(mMotionOffset, "MOVECHUNK", "MOVE")
	if(Status)
	{
		// Get version number back
		mMOVEVersion	= importer.ReadDword();
		ZCB_CHECKVERSION(mMOVEVersion, CHUNK_MOVE_VER, "MOVE");

		// Log message
		ZCBLog("Importing BIPED motion...\n");

		// Fill a motion structure
		ZCBMotionInfo Mot;

		Mot.mCharID		= importer.ReadDword();						// LinkID
		Mot.mNbBones	= importer.ReadDword();						// Nb bones
		Mot.mNbVBones	= importer.ReadDword();						// Nb virtual bones
		// Since version 2
		if(mMOVEVersion>=2)	Mot.mLocalPRS = importer.ReadBool();		// Local/global PRS
		else				Mot.mLocalPRS = true;						// Arbitrary
		// Since version 4
		if(mMOVEVersion>=4)	Mot.mD3DCompliant = importer.ReadBool();	// D3D compatible
		else				Mot.mD3DCompliant = false;
		// Since version 3
		if(mMOVEVersion>=3)	Mot.mName.Set((const char*)importer.ReadString());		// The motion's name
		else				Mot.mName.Set((const char*)"MotionName");
		if(mMOVEVersion>=3)	Mot.mType.Set((const char*)importer.ReadString());		// The motion's type
		else				Mot.mType.Set((const char*)"MotionType");
		Mot.mData		= importer.GetAddress();				// Motion data

		// Call the app
		NewMotion(Mot);
	}
	else
	{
		// MOVE chunk is mandatory for motion files only
		if(mFileType==ZCB_FILE_MOTION)
		{
			ZCBImportError("Chunk MOVE not found!", ZCB_ERROR_CHUNK_NOT_FOUND);
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports basic object information.
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBaseInfo::Import(ImportArray& importer)
{
	// Database information
	mName.Set((const char*)importer.ReadString());		// The object's name
	mID					= importer.ReadDword();			// The object's ID
	mParentID			= importer.ReadDword();			// The parent's ID
	if(gZCBVersion>=0x2)
	{
		mMasterID		= importer.ReadDword();			// ID of master object (ex: we're an instance)
		mTargetID		= importer.ReadDword();			// ID of target object (ex: we're a camera)
	}
	else
	{
		// In old versions we only had a "link ID" whose role depended on context
		mTargetID		= importer.ReadDword();			// The link ID (target nodes, master objects, etc)
		mMasterID		= mTargetID;
	}
	mGroup				= importer.ReadBool();			// true if the object belongs to a group
	if(gZCBVersion>=0x3)
	{
		mIsHidden		= importer.ReadBool();
	}
	// PRS
	mPrs.mPos.x			= importer.ReadFloat();			// Position x
	mPrs.mPos.y			= importer.ReadFloat();			// Position y
	mPrs.mPos.z			= importer.ReadFloat();			// Position z

	mPrs.mRot.p.x		= importer.ReadFloat();			// Rotation x
	mPrs.mRot.p.y		= importer.ReadFloat();			// Rotation y
	mPrs.mRot.p.z		= importer.ReadFloat();			// Rotation z
	mPrs.mRot.w			= importer.ReadFloat();			// Rotation w

	mPrs.mScale.x		= importer.ReadFloat();			// Scale x
	mPrs.mScale.y		= importer.ReadFloat();			// Scale y
	mPrs.mScale.z		= importer.ReadFloat();			// Scale z

	// Rendering information
	mWireColor			= importer.ReadDword();			// The wireframe color
	mLocalPRS			= importer.ReadBool();			// true for local PRS
	mD3DCompliant		= importer.ReadBool();			// true if converted to D3D frame

	if(gZCBVersion>=0x3)
	{
		mDensity			= importer.ReadFloat();
		mMass				= importer.ReadFloat();
		mSamplingDensity	= importer.ReadDword();
		mResetPivot			= importer.ReadBool();
		mIsCollidable		= importer.ReadBool();
		mLockPivot			= importer.ReadBool();
	}
	if(gZCBVersion>=0x4)
	{
		mLinearVelocity.x	= importer.ReadFloat();
		mLinearVelocity.y	= importer.ReadFloat();
		mLinearVelocity.z	= importer.ReadFloat();
		mAngularVelocity.x	= importer.ReadFloat();
		mAngularVelocity.y	= importer.ReadFloat();
		mAngularVelocity.z	= importer.ReadFloat();
	}

	// User-properties
	mUserProps.Set((const char*)importer.ReadString());	// The user-defined properties

	// Get pivot
	if(gHasPivot)
	{
		mPivotPos.x		= importer.ReadFloat();			// Position x
		mPivotPos.y		= importer.ReadFloat();			// Position y
		mPivotPos.z		= importer.ReadFloat();			// Position z

		mPivotRot.p.x	= importer.ReadFloat();			// Rotation x
		mPivotRot.p.y	= importer.ReadFloat();			// Rotation y
		mPivotRot.p.z	= importer.ReadFloat();			// Rotation z
		mPivotRot.w		= importer.ReadFloat();			// Rotation w
	}
	else
	{
		mPivotPos.Zero();
		mPivotRot.Identity();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the cameras.
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportCameras(ImportArray& importer)
{
#ifdef ZCB_PROGRESS_BAR
/*	gParams->mText = "Importing cameras...";
	gParams->mRequestCode = RQ_SET_TEXT_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);

	gParams->mMin = 0;
	gParams->mMax = mNbCameras;
	gParams->mRequestCode = RQ_SET_RANGE_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);*/

	if(gProgressBar)
	{
		gProgressBar->SetText1("Importing cameras...");
		gProgressBar->SetRange1(0, mNbCameras);
	}
#endif
	// Are there any cameras to import?
	if(mNbCameras)
	{
		// Find CAMS chunk
		LOAD_CHUNK(mCameraOffset, "CAMSCHUNK", "CAMS")
		if(Status)
//		if(importer.GetChunk(mZCBGlobalVersion ? "CAMSCHUNK" : "CAMS"))
		{
			// Get version number back
			mCAMSVersion	= importer.ReadDword();
			ZCB_CHECKVERSION(mCAMSVersion, CHUNK_CAMS_VER, "CAMS");

			// Log message
			ZCBLog("Importing %d cameras...\n", mNbCameras);

			// Import all cameras
			for(udword n=0;n<mNbCameras;n++)
			{
				// Fill a camera structure
				ZCBCameraInfo CurCam;

				// Base info
				CurCam.Import(importer);

				// Get camera information back
				CurCam.mOrthoCam		= importer.ReadBool();		// Camera type: ortographic (true) or perspective (false)
				CurCam.mFOV				= importer.ReadFloat();		// Field-Of-View (degrees) or Width for ortho cams
				CurCam.mNearClip		= importer.ReadFloat();		// Near/hither clip
				CurCam.mFarClip			= importer.ReadFloat();		// Far/yon clip
				CurCam.mTDist			= importer.ReadFloat();		// Distance to target
				CurCam.mHLineDisplay	= importer.ReadDword();		// Horizon Line Display
				CurCam.mEnvNearRange	= importer.ReadFloat();		// Environment near range
				CurCam.mEnvFarRange		= importer.ReadFloat();		// Environment far range
				CurCam.mEnvDisplay		= importer.ReadDword();		// Environment display
				CurCam.mManualClip		= importer.ReadDword();		// Manual clip

				if(mCAMSVersion>=2)
				{
					CurCam.mFOVType		= (ZCBFOVType)importer.ReadDword();		// FOV type
					CurCam.mType		= (ZCBCameraType)importer.ReadDword();	// Camera type
				}

				// Call the app
				NewCamera(CurCam);

				UpdateProgressBar1();
			}
		}
		else
		{
			ZCBImportError("Chunk CAMS not found!", ZCB_ERROR_CHUNK_NOT_FOUND);
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the lights.
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportLights(ImportArray& importer)
{
#ifdef ZCB_PROGRESS_BAR
/*	gParams->mText = "Importing lights...";
	gParams->mRequestCode = RQ_SET_TEXT_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);

	gParams->mMin = 0;
	gParams->mMax = mNbLights;
	gParams->mRequestCode = RQ_SET_RANGE_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);*/
	if(gProgressBar)
	{
		gProgressBar->SetText1("Importing lights...");
		gProgressBar->SetRange1(0, mNbLights);
	}
#endif
	// Are there any lights to import?
	if(mNbLights)
	{
		// Find LITE chunk
		LOAD_CHUNK(mLightOffset, "LITECHUNK", "LITE")
		if(Status)
//		if(importer.GetChunk(mZCBGlobalVersion ? "LITECHUNK" : "LITE"))
		{
			// Get version number back
			mLITEVersion	= importer.ReadDword();
			ZCB_CHECKVERSION(mLITEVersion, CHUNK_LITE_VER, "LITE");

			// Log message
			ZCBLog("Importing %d lights...\n", mNbLights);

			for(udword n=0;n<mNbLights;n++)
			{
				// Fill a light structure
				ZCBLightInfo CurLight;

				// Base info
				CurLight.Import(importer);

				// Get light information back
				CurLight.mLightType			= (ZCBLightType)importer.ReadDword();	// Light's type
				CurLight.mIsSpot			= importer.ReadBool();					// Is the light a spotlight?
				CurLight.mIsDir				= importer.ReadBool();					// Is the light a directional?
				CurLight.mColor.x			= importer.ReadFloat();					// Light's color
				CurLight.mColor.y			= importer.ReadFloat();					//
				CurLight.mColor.z			= importer.ReadFloat();					//
				CurLight.mIntensity			= importer.ReadFloat();					// Light's intensity
				CurLight.mContrast			= importer.ReadFloat();					// Light's contrast
				CurLight.mDiffuseSoft		= importer.ReadFloat();					// Light's diffuse soft
				CurLight.mLightUsed			= importer.ReadBool();					// Is the light used?
				CurLight.mAffectDiffuse		= importer.ReadBool();					// Does the light affect diffuse?
				CurLight.mAffectSpecular	= importer.ReadBool();					// Does the light affect specular?
				CurLight.mUseAttenNear		= importer.ReadBool();					//
				CurLight.mAttenNearDisplay	= importer.ReadBool();					//
				CurLight.mUseAtten			= importer.ReadBool();					// Is attenuation used?
				CurLight.mShowAtten			= importer.ReadBool();					//
				CurLight.mNearAttenStart	= importer.ReadFloat();					// Near atten start
				CurLight.mNearAttenEnd		= importer.ReadFloat();					// Near atten end
				CurLight.mAttenStart		= importer.ReadFloat();					// Atten start
				CurLight.mAttenEnd			= importer.ReadFloat();					// Atten end (use that as a range for non-dir lights)
				CurLight.mDecayType			= importer.ReadByte();					// Light's decay type
				CurLight.mHotSpot			= importer.ReadFloat();					// Light's hotspot
				CurLight.mFallsize			= importer.ReadFloat();					// Light's falloff
				CurLight.mAspect			= importer.ReadFloat();					// Light's aspect
				CurLight.mSpotShape			= (ZCBSpotShape)importer.ReadDword();	// Light's spot shape
				CurLight.mOvershoot			= importer.ReadDword();					// Light's overshoot
				CurLight.mConeDisplay		= importer.ReadBool();					//
				CurLight.mTDist				= importer.ReadFloat();					// Distance to target
				CurLight.mShadowType		= importer.ReadDword();					// Light's shadow type
				CurLight.mAbsMapBias		= importer.ReadDword();					// Light's absolute map bias
				CurLight.mRayBias			= importer.ReadFloat();					// Raytrace bias
				CurLight.mMapBias			= importer.ReadFloat();					// Map bias
				CurLight.mMapRange			= importer.ReadFloat();					// Map range
				CurLight.mMapSize			= importer.ReadDword();					// Map size

				if(mLITEVersion>=2)	CurLight.mCastShadows	= importer.ReadBool();	// Cast shadows
				else				CurLight.mCastShadows	= false;

				if(mLITEVersion>=3)
				{
					CurLight.mShadowDensity			= importer.ReadFloat();		// Shadow density
					CurLight.mShadowColor.x			= importer.ReadFloat();		// Shadow color
					CurLight.mShadowColor.y			= importer.ReadFloat();		// Shadow color
					CurLight.mShadowColor.z			= importer.ReadFloat();		// Shadow color
					CurLight.mLightAffectsShadow	= importer.ReadBool();		// Light affects shadow or not
				}
				else
				{
					CurLight.mShadowDensity			= 0.0f;
					CurLight.mShadowColor.x			= 0.0f;
					CurLight.mShadowColor.y			= 0.0f;
					CurLight.mShadowColor.z			= 0.0f;
					CurLight.mLightAffectsShadow	= false;
				}

				if(mLITEVersion>=4)
				{
					CurLight.mProjMapID			= importer.ReadDword();
					CurLight.mShadowProjMapID	= importer.ReadDword();
					PatchTextureID(CurLight.mProjMapID);
					PatchTextureID(CurLight.mShadowProjMapID);
				}
				else
				{
					CurLight.mProjMapID			= -1;
					CurLight.mShadowProjMapID	= -1;
				}

				// Call the app
				NewLight(CurLight);

				UpdateProgressBar1();
			}
		}
		else
		{
			ZCBImportError("Chunk LITE not found!", ZCB_ERROR_CHUNK_NOT_FOUND);
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the cropping values & texture matrix.
 *	\param		cvalues		[out] a place to store the cropping values
 *	\param		tmtx		[out] a place to store the texture matrix
 *	\param		importer	[in] the imported array.
 *	\param		wrap_u		[out] 
 *	\param		mirror_u	[out] 
 *	\param		wrap_v		[out] 
 *	\param		mirror_v	[out] 
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportCroppingValues(TextureCrop& cvalues, TextureMatrix& tmtx, ImportArray& importer, bool& wrap_u, bool& mirror_u, bool& wrap_v, bool& mirror_v)
{
	// Get cropping values back
	cvalues.mOffsetU	= importer.ReadFloat();
	cvalues.mOffsetV	= importer.ReadFloat();
	cvalues.mScaleU		= importer.ReadFloat();
	cvalues.mScaleV		= importer.ReadFloat();

	// Get texture matrix back
	tmtx.m[0][0]		= importer.ReadFloat();
	tmtx.m[0][1]		= importer.ReadFloat();
	tmtx.m[0][2]		= importer.ReadFloat();

	tmtx.m[1][0]		= importer.ReadFloat();
	tmtx.m[1][1]		= importer.ReadFloat();
	tmtx.m[1][2]		= importer.ReadFloat();

	tmtx.m[2][0]		= importer.ReadFloat();
	tmtx.m[2][1]		= importer.ReadFloat();
	tmtx.m[2][2]		= importer.ReadFloat();

	tmtx.m[3][0]		= importer.ReadFloat();
	tmtx.m[3][1]		= importer.ReadFloat();
	tmtx.m[3][2]		= importer.ReadFloat();

	// Export texture tiling
	// CHUNK_TEXM_VER >= 3
	// CHUNK_MATL_VER >= 4
	if(mTEXMVersion >= 3 || mMATLVersion >= 4)
	{
		wrap_u		= importer.ReadBool();
		mirror_u	= importer.ReadBool();
		wrap_v		= importer.ReadBool();
		mirror_v	= importer.ReadBool();
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the materials.
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportMaterials(ImportArray& importer)
{
#ifdef ZCB_PROGRESS_BAR
/*	gParams->mText = "Importing materials...";
	gParams->mRequestCode = RQ_SET_TEXT_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);

	gParams->mMin = 0;
	gParams->mMax = mNbMaterials;
	gParams->mRequestCode = RQ_SET_RANGE_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);*/
	if(gProgressBar)
	{
		gProgressBar->SetText1("Importing materials...");
		gProgressBar->SetRange1(0, mNbMaterials);
	}
#endif
	// Are there any materials to import?
	if(mNbMaterials)
	{
		// Find MATL chunk
		LOAD_CHUNK(mMaterialOffset, "MATLCHUNK", "MATL")
		if(Status)
//		if(importer.GetChunk(mZCBGlobalVersion ? "MATLCHUNK" : "MATL"))
		{
			// Get version number back
			mMATLVersion	= importer.ReadDword();
			ZCB_CHECKVERSION(mMATLVersion, CHUNK_MATL_VER, "MATL");
//IceCore::MessageBox(null, _F("MATL version %d", mMATLVersion), "Report", MB_OK);			
			// Log message
			ZCBLog("Importing %d materials...\n", mNbMaterials);

			// Import all materials
			for(udword n=0;n<mNbMaterials;n++)
			{
				// Fill a material structure
				ZCBMaterialInfo CurMaterial;

				// Database information
				CurMaterial.mName.Set((const char*)importer.ReadString());	// Material name
				CurMaterial.mID	= importer.ReadDword();						// Material ID
//Log("Imported material: %s - %d\n", CurMaterial.mName, CurMaterial.mID);
				PatchMaterialID(CurMaterial.mID);

				// Texture IDs
				CurMaterial.mAmbientMapID			= importer.ReadDword();	PatchTextureID(CurMaterial.mAmbientMapID);
				CurMaterial.mDiffuseMapID			= importer.ReadDword();	PatchTextureID(CurMaterial.mDiffuseMapID);
				CurMaterial.mSpecularMapID			= importer.ReadDword();	PatchTextureID(CurMaterial.mSpecularMapID);
				CurMaterial.mShininessMapID			= importer.ReadDword();	PatchTextureID(CurMaterial.mShininessMapID);
				CurMaterial.mShiningStrengthMapID	= importer.ReadDword();	PatchTextureID(CurMaterial.mShiningStrengthMapID);
				CurMaterial.mSelfIllumMapID			= importer.ReadDword();	PatchTextureID(CurMaterial.mSelfIllumMapID);
				CurMaterial.mOpacityMapID			= importer.ReadDword();	PatchTextureID(CurMaterial.mOpacityMapID);
				CurMaterial.mFilterMapID			= importer.ReadDword();	PatchTextureID(CurMaterial.mFilterMapID);
				CurMaterial.mBumpMapID				= importer.ReadDword();	PatchTextureID(CurMaterial.mBumpMapID);
				CurMaterial.mReflexionMapID			= importer.ReadDword();	PatchTextureID(CurMaterial.mReflexionMapID);
				CurMaterial.mRefractionMapID		= importer.ReadDword();	PatchTextureID(CurMaterial.mRefractionMapID);
				CurMaterial.mDisplacementMapID		= importer.ReadDword();	PatchTextureID(CurMaterial.mDisplacementMapID);

				// Amounts
				CurMaterial.mAmbientCoeff			= importer.ReadFloat();
				CurMaterial.mDiffuseCoeff			= importer.ReadFloat();
				CurMaterial.mSpecularCoeff			= importer.ReadFloat();
				CurMaterial.mShininessCoeff			= importer.ReadFloat();
				CurMaterial.mShiningStrengthCoeff	= importer.ReadFloat();
				CurMaterial.mSelfIllumCoeff			= importer.ReadFloat();
				CurMaterial.mOpacityCoeff			= importer.ReadFloat();
				CurMaterial.mFilterCoeff			= importer.ReadFloat();
				CurMaterial.mBumpCoeff				= importer.ReadFloat();
				CurMaterial.mReflexionCoeff			= importer.ReadFloat();
				CurMaterial.mRefractionCoeff		= importer.ReadFloat();
				CurMaterial.mDisplacementCoeff		= importer.ReadFloat();

				// Colors
				CurMaterial.mMtlAmbientColor.x		= importer.ReadFloat();
				CurMaterial.mMtlAmbientColor.y		= importer.ReadFloat();
				CurMaterial.mMtlAmbientColor.z		= importer.ReadFloat();

				CurMaterial.mMtlDiffuseColor.x		= importer.ReadFloat();
				CurMaterial.mMtlDiffuseColor.y		= importer.ReadFloat();
				CurMaterial.mMtlDiffuseColor.z		= importer.ReadFloat();

				CurMaterial.mMtlSpecularColor.x		= importer.ReadFloat();
				CurMaterial.mMtlSpecularColor.y		= importer.ReadFloat();
				CurMaterial.mMtlSpecularColor.z		= importer.ReadFloat();

				CurMaterial.mMtlFilterColor.x		= importer.ReadFloat();
				CurMaterial.mMtlFilterColor.y		= importer.ReadFloat();
				CurMaterial.mMtlFilterColor.z		= importer.ReadFloat();

				// Static properties
				CurMaterial.mShading				= (ZCBShadingMode)importer.ReadDword();
				CurMaterial.mSoften					= importer.ReadBool();
				CurMaterial.mFaceMap				= importer.ReadBool();
				CurMaterial.mTwoSided				= importer.ReadBool();
				CurMaterial.mWire					= importer.ReadBool();
				CurMaterial.mWireUnits				= importer.ReadBool();
				CurMaterial.mFalloffOut				= importer.ReadBool();
				CurMaterial.mTransparencyType		= (ZCBTranspaType)importer.ReadDword();

				// Dynamic properties
				CurMaterial.mShininess				= importer.ReadFloat();
				CurMaterial.mShiningStrength		= importer.ReadFloat();
				CurMaterial.mSelfIllum				= importer.ReadFloat();
				CurMaterial.mOpacity				= importer.ReadFloat();
				CurMaterial.mOpaFalloff				= importer.ReadFloat();
				CurMaterial.mWireSize				= importer.ReadFloat();
				CurMaterial.mIOR					= importer.ReadFloat();

				CurMaterial.mBounce					= importer.ReadFloat();
				CurMaterial.mStaticFriction			= importer.ReadFloat();
				CurMaterial.mSlidingFriction		= importer.ReadFloat();

				// Cropping values & texture matrix
				if(mMATLVersion>=2)
					ImportCroppingValues(CurMaterial.mCValues_Diffuse, CurMaterial.mTMtx_Diffuse, importer, CurMaterial.mWrapU_Diffuse, CurMaterial.mMirrorU_Diffuse, CurMaterial.mWrapV_Diffuse, CurMaterial.mMirrorV_Diffuse);
				if(mMATLVersion>=8)
					ImportCroppingValues(CurMaterial.mCValues_Filter, CurMaterial.mTMtx_Filter, importer, CurMaterial.mWrapU_Filter, CurMaterial.mMirrorU_Filter, CurMaterial.mWrapV_Filter, CurMaterial.mMirrorV_Filter);

				// Extended self-illum
				if(mMATLVersion>=3)
				{
					CurMaterial.mSelfIllumOn		= importer.ReadBool();
					CurMaterial.mSelfIllumValue		= importer.ReadFloat();
					CurMaterial.mSelfIllumColor.x	= importer.ReadFloat();
					CurMaterial.mSelfIllumColor.y	= importer.ReadFloat();
					CurMaterial.mSelfIllumColor.z	= importer.ReadFloat();
				}

				// Flexporter specific
				if(mMATLVersion>=5)
				{
//Log("Shader address: %d\n", importer.GetAddress());
					const char* ShaderFile = (const char*)importer.ReadString();
					CurMaterial.mShaderFile.Set(ShaderFile);

//Log("UserProps address: %d\n", importer.GetAddress());
					const char* UserProps = (const char*)importer.ReadString();
/*Log("Next address: %d\n", importer.GetAddress());
char Buf[21];
ubyte* Ad = (ubyte*)importer.GetAddress();
for(int ppp=0;ppp<20;ppp++)	Buf[ppp] = Ad[ppp];
Buf[20]=0;
Log("Next bytes: %s\n", Buf);
*/
					CurMaterial.mUserProps.Set(UserProps);
//Log("%s - %s\n", CurMaterial.mShaderFile, CurMaterial.mUserProps);
				}

				if(mMATLVersion>=6)
				{
					CurMaterial.mDecalMapID = importer.ReadDword();
					CurMaterial.mDetailMapID = importer.ReadDword();
				}
				if(mMATLVersion>=7)
				{
					CurMaterial.mBillboardMapID = importer.ReadDword();
				}

				// Call the app
				NewMaterial(CurMaterial);

				UpdateProgressBar1();
			}
		}
		else
		{
			ZCBImportError("Chunk MATL not found!", ZCB_ERROR_CHUNK_NOT_FOUND);
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the textures.
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportTextures(ImportArray& importer)
{
#ifdef ZCB_PROGRESS_BAR
/*	gParams->mText = "Importing textures...";
	gParams->mRequestCode = RQ_SET_TEXT_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);

	gParams->mMin = 0;
	gParams->mMax = mNbTextures;
	gParams->mRequestCode = RQ_SET_RANGE_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);*/
	if(gProgressBar)
	{
		gProgressBar->SetText1("Importing textures...");
		gProgressBar->SetRange1(0, mNbTextures);
	}
#endif
	// Are there any textures to import?
	if(mNbTextures)
	{
		// Find TEXM chunk
		LOAD_CHUNK(mTexmapOffset, "TEXMCHUNK", "TEXM")
		if(Status)
//		if(importer.GetChunk(mZCBGlobalVersion ? "TEXMCHUNK" : "TEXM"))
		{
			// Get version number back
			mTEXMVersion	= importer.ReadDword();
			ZCB_CHECKVERSION(mTEXMVersion, CHUNK_TEXM_VER, "TEXM");

			// Log message
			ZCBLog("Importing %d textures...\n", mNbTextures);

			// Import all textures
			for(udword n=0;n<mNbTextures;n++)
			{
				// Fill a texture structure
				ZCBTextureInfo CurTexture;

				// Database information
				CurTexture.mName.Set((const char*)importer.ReadString());	// Texture path
				CurTexture.mID = importer.ReadDword();						// Texture ID
				PatchTextureID(CurTexture.mID);

				// Get bitmap data
				ubyte Code = 1;	// Default for version <=1
				if(mTEXMVersion>1)	Code = importer.ReadByte();
//				CurTexture.mIsBitmapIncluded = Version>1 ? (importer.ReadBool()) : true;
				CurTexture.mIsBitmapIncluded = Code!=0;

				if(Code)
				{
					// Get texture information back
					CurTexture.mWidth		= importer.ReadDword();
					CurTexture.mHeight		= importer.ReadDword();
					CurTexture.mHasAlpha	= importer.ReadBool();

					// Get bytes for a RGBA texture
					CurTexture.mBitmap		= (ubyte*)ICE_ALLOC(sizeof(ubyte)*CurTexture.mWidth*CurTexture.mHeight*4);
					ZCB_CHECKALLOC(CurTexture.mBitmap);

					if(Code==1)
					{
						// => RGBA texture
						for(udword i=0;i<CurTexture.mWidth*CurTexture.mHeight;i++)
						{
							CurTexture.mBitmap[i*4+0] = importer.ReadByte();	// Red
							CurTexture.mBitmap[i*4+1] = importer.ReadByte();	// Green
							CurTexture.mBitmap[i*4+2] = importer.ReadByte();	// Blue
							CurTexture.mBitmap[i*4+3] = CurTexture.mHasAlpha ? importer.ReadByte() : PIXEL_OPAQUE;
						}
					}
					else
					{
						// => Quantized RGB texture
						ubyte Palette[768];
						for(udword i=0;i<768;i++)	Palette[i] = importer.ReadByte();
						//
						for(udword i=0;i<CurTexture.mWidth*CurTexture.mHeight;i++)
						{
							const ubyte ColorIndex = importer.ReadByte();
							CurTexture.mBitmap[i*4+0] = Palette[ColorIndex*3+0];
							CurTexture.mBitmap[i*4+1] = Palette[ColorIndex*3+1];
							CurTexture.mBitmap[i*4+2] = Palette[ColorIndex*3+2];
							CurTexture.mBitmap[i*4+3] = PIXEL_OPAQUE;
						}
					}
				}
				else
				{
					if(mTEXMVersion>3)
					{
						CurTexture.mExternalSourceHasAlpha = importer.ReadBool();
					}
				}

				// Cropping values & texture matrix
				ImportCroppingValues(CurTexture.mCValues, CurTexture.mTMtx, importer, CurTexture.mWrapU, CurTexture.mMirrorU, CurTexture.mWrapV, CurTexture.mMirrorV);

				// Call the app
				NewTexture(CurTexture);

				UpdateProgressBar1();
			}
		}
		else
		{
			ZCBImportError("Chunk TEXM not found!", ZCB_ERROR_CHUNK_NOT_FOUND);
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the meshes.
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportMeshes(ImportArray& importer)
{
	// Are there any meshes to import?
	udword Total = mNbMeshes + mNbDerived;

#ifdef ZCB_PROGRESS_BAR
/*	gParams->mText = "Importing meshes...";
	gParams->mRequestCode = RQ_SET_TEXT_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);

	gParams->mMin = 0;
	gParams->mMax = Total;
	gParams->mRequestCode = RQ_SET_RANGE_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);*/
	if(gProgressBar)
	{
		gProgressBar->SetText1("Importing meshes...");
		gProgressBar->SetRange1(0, Total);
	}
#endif
	if(Total)
	{
		// Find MESH chunk
		LOAD_CHUNK(mMeshOffset, "MESHCHUNK", "MESH")
		if(Status)
//		if(importer.GetChunk(mZCBGlobalVersion ? "MESHCHUNK" : "MESH"))
		{
			// Get version number back
			mMESHVersion	= importer.ReadDword();
			ZCB_CHECKVERSION(mMESHVersion, CHUNK_MESH_VER, "MESH");

			// Log message
			ZCBLog("Importing %d meshes...\n", Total);

			// Import all meshes
			for(udword n=0;n<Total;n++)
			{
				// Fill a mesh structure
				ZCBMeshInfo CurMesh;

				// Base info
				CurMesh.Import(importer);

				// Get mesh information back
				CurMesh.mIsCollapsed	= importer.ReadBool();	// true if the object has been collapsed
				CurMesh.mIsSkeleton		= importer.ReadBool();	// true for BIPED parts
				CurMesh.mIsInstance		= importer.ReadBool();	// true for instances
				CurMesh.mIsTarget		= importer.ReadBool();	// true for target objects
				CurMesh.mIsConvertible	= importer.ReadBool();	// true for valid objects
				CurMesh.mIsSkin			= importer.ReadBool();	// true for PHYSIQUE skins
				if(mMESHVersion>=4)
					CurMesh.mCastShadows= importer.ReadBool();	// true if the mesh can cast its shadow

				if(mMESHVersion>=9)
				{
					CurMesh.mReceiveShadows	= importer.ReadBool();
					CurMesh.mMotionBlur		= importer.ReadBool();
				}

				// Get skin's character ID
				if(mMESHVersion>=5 && CurMesh.mIsSkin)
				{
					CurMesh.mCharID		= importer.ReadDword();		// the owner's character ID
				}

				// Get BIPED parts information if needed
				if(CurMesh.mIsSkeleton)
				{
					CurMesh.mCharID		= importer.ReadDword();		// the owner's character ID
					CurMesh.mCSID		= importer.ReadDword();		// the CSID
				}

				// Get data back for non-instance meshes
				if(!CurMesh.mIsInstance)
				{
					// Get primitive parameters
					if(mMESHVersion>=8)
					{
						udword PrimitiveType = importer.ReadDword();
						switch(PrimitiveType)
						{
							case ZCB_PRIM_BOX:
							{
								ZCBBoxParams* BP = ICE_NEW(ZCBBoxParams);
								CurMesh.mPrimParams = BP;
								BP->mLength	= importer.ReadFloat();
								BP->mWidth	= importer.ReadFloat();
								BP->mHeight	= importer.ReadFloat();
								BP->mWSegs	= importer.ReadDword();
								BP->mLSegs	= importer.ReadDword();
								BP->mHSegs	= importer.ReadDword();
								BP->mGenUVS	= importer.ReadDword();
							}
							break;

							case ZCB_PRIM_SPHERE:
							{
								ZCBSphereParams* SP = ICE_NEW(ZCBSphereParams);
								CurMesh.mPrimParams = SP;
								SP->mRadius		= importer.ReadFloat();
								SP->mSegments	= importer.ReadDword();
								SP->mSmooth		= importer.ReadDword();
								SP->mHemisphere	= importer.ReadFloat();
								SP->mSquash		= importer.ReadDword();
								SP->mSliceFrom	= importer.ReadFloat();
								SP->mSliceTo	= importer.ReadFloat();
								SP->mSliceOn	= importer.ReadDword();
								SP->mRecenter	= importer.ReadDword();
								SP->mGenUVS		= importer.ReadDword();
							}
							break;

							case ZCB_PRIM_GEOSPHERE:
							{
								ZCBGeosphereParams* GP = ICE_NEW(ZCBGeosphereParams);
								CurMesh.mPrimParams = GP;
								GP->mRadius		= importer.ReadFloat();
								GP->mSegments	= importer.ReadDword();
								GP->mGenType	= importer.ReadDword();
								GP->mHemisphere	= importer.ReadDword();
								GP->mSmooth		= importer.ReadDword();
								GP->mRecenter	= importer.ReadDword();
								GP->mGenUVS		= importer.ReadDword();
							}
							break;

							case ZCB_PRIM_CYLINDER:
							{
								ZCBCylinderParams* CP = ICE_NEW(ZCBCylinderParams);
								CurMesh.mPrimParams = CP;
								CP->mRadius		= importer.ReadFloat();
								CP->mHeight		= importer.ReadFloat();
								CP->mHSegs		= importer.ReadDword();
								CP->mCapSegs	= importer.ReadDword();
								CP->mSides		= importer.ReadDword();
								CP->mSmooth		= importer.ReadDword();
								CP->mSliceOn	= importer.ReadDword();
								CP->mSliceFrom	= importer.ReadFloat();
								CP->mSliceTo	= importer.ReadFloat();
								CP->mGenUVS		= importer.ReadDword();
							}
							break;

							case ZCB_PRIM_CONE:
							{
								ZCBConeParams* CP = ICE_NEW(ZCBConeParams);
								CurMesh.mPrimParams = CP;
								CP->mRadius1	= importer.ReadFloat();
								CP->mRadius2	= importer.ReadFloat();
								CP->mHeight		= importer.ReadFloat();
								CP->mHSegs		= importer.ReadDword();
								CP->mCapSegs	= importer.ReadDword();
								CP->mSides		= importer.ReadDword();
								CP->mSmooth		= importer.ReadDword();
								CP->mSliceOn	= importer.ReadDword();
								CP->mSliceFrom	= importer.ReadFloat();
								CP->mSliceTo	= importer.ReadFloat();
								CP->mGenUVS		= importer.ReadDword();
							}
							break;

							case ZCB_PRIM_TORUS:
							{
								ZCBTorusParams* TP = ICE_NEW(ZCBTorusParams);
								CurMesh.mPrimParams = TP;
								TP->mRadius1	= importer.ReadFloat();
								TP->mRadius2	= importer.ReadFloat();
								TP->mRotation	= importer.ReadFloat();
								TP->mTwist		= importer.ReadFloat();
								TP->mSegments	= importer.ReadDword();
								TP->mSides		= importer.ReadDword();
								TP->mSmooth		= importer.ReadDword();
								TP->mSliceOn	= importer.ReadDword();
								TP->mSliceFrom	= importer.ReadFloat();
								TP->mSliceTo	= importer.ReadFloat();
								TP->mGenUVS		= importer.ReadDword();
							}
							break;

							case ZCB_PRIM_TUBE:
							{
								ZCBTubeParams* TP = ICE_NEW(ZCBTubeParams);
								CurMesh.mPrimParams = TP;
								TP->mRadius1	= importer.ReadFloat();
								TP->mRadius2	= importer.ReadFloat();
								TP->mHeight		= importer.ReadFloat();
								TP->mSegments	= importer.ReadDword();
								TP->mCapSegs	= importer.ReadDword();
								TP->mSides		= importer.ReadDword();
								TP->mSmooth		= importer.ReadDword();
								TP->mSliceOn	= importer.ReadDword();
								TP->mSliceFrom	= importer.ReadFloat();
								TP->mSliceTo	= importer.ReadFloat();
								TP->mGenUVS		= importer.ReadDword();
							}
							break;

							case ZCB_PRIM_TEAPOT:
							{
							}
							break;

							case ZCB_PRIM_PLANE:
							{
								ZCBPlaneParams* PP = ICE_NEW(ZCBPlaneParams);
								CurMesh.mPrimParams = PP;
								PP->mLength		= importer.ReadFloat();
								PP->mWidth		= importer.ReadFloat();
								PP->mWidthSegs	= importer.ReadDword();
								PP->mLengthSegs	= importer.ReadDword();
								PP->mDensity	= importer.ReadFloat();
								PP->mScale		= importer.ReadFloat();
								PP->mGenUVS		= importer.ReadDword();
							}
							break;

							case ZCB_PRIM_HEDRA:
							{
								ZCBHedraParams* HP = ICE_NEW(ZCBHedraParams);
								CurMesh.mPrimParams = HP;
								HP->mRadius		= importer.ReadFloat();
								HP->mFamily		= importer.ReadDword();
								HP->mP			= importer.ReadFloat();
								HP->mQ			= importer.ReadFloat();
								HP->mScaleP		= importer.ReadFloat();
								HP->mScaleQ		= importer.ReadFloat();
								HP->mScaleR		= importer.ReadFloat();
								HP->mVertices	= importer.ReadDword();
								HP->mGenUVS		= importer.ReadDword();
							}
							break;

							case ZCB_PRIM_CAPSULE:
							{
								ZCBCapsuleParams* CP = ICE_NEW(ZCBCapsuleParams);
								CurMesh.mPrimParams = CP;
								CP->mRadius		= importer.ReadFloat();
								CP->mHeight		= importer.ReadFloat();
								CP->mCenters	= importer.ReadDword();
								CP->mSides		= importer.ReadDword();
								CP->mHSegs		= importer.ReadDword();
								CP->mSmooth		= importer.ReadDword();
								CP->mSliceOn	= importer.ReadDword();
								CP->mSliceFrom	= importer.ReadFloat();
								CP->mSliceTo	= importer.ReadFloat();
								CP->mGenUVS		= importer.ReadDword();
							}
							break;
						}
					}

					CurMesh.mNbFaces	= importer.ReadDword();		// Number of faces
					CurMesh.mNbVerts	= importer.ReadDword();		// Number of vertices
					CurMesh.mNbTVerts	= importer.ReadDword();		// Number of texture-vertices
					CurMesh.mNbCVerts	= importer.ReadDword();		// Number of vertex-colors
					CurMesh.mFlags		= importer.ReadDword();		// Flags
					CurMesh.mParity		= importer.ReadBool();		// Mesh parity

					// Get data for skins / non-skins
					if(!CurMesh.mIsSkin)	ImportVertices			(CurMesh, importer);
					else					ImportSkinData			(CurMesh, importer);

					// Native texture vertices
											ImportTextureVertices	(CurMesh, importer);

					// Native vertex-colors
											ImportVertexColors		(CurMesh, importer);

					// Native faces
											ImportFaces				(CurMesh, importer);

					// Extra stuff
											ImportExtraStuff		(CurMesh, importer);

					// Consolidated mesh
											ImportConsolidated		(CurMesh, importer);
				}

				// Lightmapper data
				ImportLightingData(CurMesh, importer);

				// Call the app
				NewMesh(CurMesh);

				// Free consolidation ram
				if(CurMesh.mCleanMesh)
				{
					MBMaterials& Mtl = CurMesh.mCleanMesh->Materials;
					DELETEARRAY(Mtl.MaterialInfo);
					MBGeometry& Geo = CurMesh.mCleanMesh->Geometry;
					ICE_FREE(Geo.NormalInfo);
					ICE_FREE(Geo.CVerts);
					ICE_FREE(Geo.Normals);
					ICE_FREE(Geo.TVerts);
					ICE_FREE(Geo.TVertsRefs);
					ICE_FREE(Geo.Verts);
					ICE_FREE(Geo.VertsRefs);
					MBTopology& Topo = CurMesh.mCleanMesh->Topology;
					ICE_FREE(Topo.Normals);
					ICE_FREE(Topo.VRefs);
					ICE_FREE(Topo.FacesInSubmesh);
					DELETEARRAY(Topo.SubmeshProperties);

					DELETESINGLE(CurMesh.mCleanMesh);
				}

				UpdateProgressBar1();
			}
		}
		else
		{
			ZCBImportError("Chunk MESH not found!", ZCB_ERROR_CHUNK_NOT_FOUND);
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the vertices.
 *	\param		curmesh			[in] current mesh structure
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportVertices(ZCBMeshInfo& curmesh, ImportArray& importer)
{
	// Checkings
	if(!curmesh.mNbVerts)	return true;

	// Get some bytes for vertices
	curmesh.mVerts = ICE_NEW(Point)[curmesh.mNbVerts];
	ZCB_CHECKALLOC(curmesh.mVerts);

	// Get vertices back
	if(curmesh.mFlags&ZCB_QUANTIZED_VERTICES)
	{
		// Get dequantization coeffs
		const float DequantCoeffX = importer.ReadFloat();
		const float DequantCoeffY = importer.ReadFloat();
		const float DequantCoeffZ = importer.ReadFloat();
		// Get quantized vertices
		for(udword i=0;i<curmesh.mNbVerts;i++)
		{
			const sword x = importer.ReadWord();
			const sword y = importer.ReadWord();
			const sword z = importer.ReadWord();
			// Dequantize
			curmesh.mVerts[i].x = float(x) * DequantCoeffX;
			curmesh.mVerts[i].y = float(y) * DequantCoeffY;
			curmesh.mVerts[i].z = float(z) * DequantCoeffZ;
		}
	}
	else
	{
		// Get vertices
/*		for(udword i=0;i<curmesh.mNbVerts;i++)
		{
			const float x = importer.ReadFloat();
			const float y = importer.ReadFloat();
			const float z = importer.ReadFloat();
			curmesh.mVerts[i].x = x;
			curmesh.mVerts[i].y = y;
			curmesh.mVerts[i].z = z;
		}*/
		importer.ReadBuffer(curmesh.mVerts, sizeof(float)*3*curmesh.mNbVerts);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the skin data.
 *	\param		curmesh			[in] current mesh structure
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportSkinData(ZCBMeshInfo& curmesh, ImportArray& importer)
{
	// A skin can be a simple or a complex skin. (err, that's *my* wordlist...)
	// - simple: each vertex is linked to a single bone
	// - complex: each vertex is linked to many bones
	if(curmesh.mFlags & ZCB_ONE_BONE_PER_VERTEX)
	{
		// The skin has one bone/vertex. Hence:
		// - We have N vertices and N bones
		// - N offset vectors
		// - N bones ID
		// - mBonesNb remains null
		// - mWeights remains null

		// Get offset vectors back
		curmesh.mOffsetVectors	= ICE_NEW(Point)[curmesh.mNbVerts];
		ZCB_CHECKALLOC(curmesh.mOffsetVectors);

		for(udword i=0;i<curmesh.mNbVerts;i++)
		{
			curmesh.mOffsetVectors[i].x = importer.ReadFloat();
			curmesh.mOffsetVectors[i].y = importer.ReadFloat();
			curmesh.mOffsetVectors[i].z = importer.ReadFloat();

			// Fix what was missing in Flexporter before v1.13
			if(curmesh.mD3DCompliant && mMESHVersion<6)	TSwap(curmesh.mOffsetVectors[i].y, curmesh.mOffsetVectors[i].z);
		}

		// Get bones ID back
		curmesh.mBonesID		= (udword*)ICE_ALLOC(sizeof(udword)*curmesh.mNbVerts);
		ZCB_CHECKALLOC(curmesh.mBonesID);

		for(udword i=0;i<curmesh.mNbVerts;i++)
		{
			curmesh.mBonesID[i] = importer.ReadDword();
		}
	}
	else
	{
		// The skin has many bones/vertex. Hence:
		// - We have N vertices and M bones
		// - We have N numbers of bones, stored in mBonesNb
		// - M is the sum of all those number of bones
		// - We have M offset vectors
		// - We have M bones ID
		// - We have M weights stored in mWeights

		// Get number of bones / vertex, compute total number of bones
		curmesh.mBonesNb		= (udword*)ICE_ALLOC(sizeof(udword)*curmesh.mNbVerts);
		ZCB_CHECKALLOC(curmesh.mBonesNb);

		udword Sum=0;
		for(udword i=0;i<curmesh.mNbVerts;i++)
		{
			curmesh.mBonesNb[i] = importer.ReadDword();
			Sum+=curmesh.mBonesNb[i];
		}

		// Get bones ID back
		curmesh.mBonesID		= (udword*)ICE_ALLOC(sizeof(udword)*Sum);
		ZCB_CHECKALLOC(curmesh.mBonesID);

		for(udword i=0;i<Sum;i++)
		{
			curmesh.mBonesID[i] = importer.ReadDword();
		}

		// Get weights back
		curmesh.mWeights		= (float*)ICE_ALLOC(sizeof(float)*Sum);
		ZCB_CHECKALLOC(curmesh.mWeights);

		for(udword i=0;i<Sum;i++)
		{
			curmesh.mWeights[i] = importer.ReadFloat();
		}

		// Get offset vectors back
		curmesh.mOffsetVectors	= ICE_NEW(Point)[Sum];
		ZCB_CHECKALLOC(curmesh.mOffsetVectors);

		for(udword i=0;i<Sum;i++)
		{
			curmesh.mOffsetVectors[i].x = importer.ReadFloat();
			curmesh.mOffsetVectors[i].y = importer.ReadFloat();
			curmesh.mOffsetVectors[i].z = importer.ReadFloat();

			// Fix what was missing in Flexporter before v1.13
			if(curmesh.mD3DCompliant && mMESHVersion<6)	TSwap(curmesh.mOffsetVectors[i].y, curmesh.mOffsetVectors[i].z);
		}
	}

	// Get skeletal information back. This just gives the skeleton structure in a simple way, so that you
	// can discard the actual BIPED parts and still use the skin (=derived object) alone.
	curmesh.mNbBones = importer.ReadDword();

	curmesh.mSkeleton = ICE_NEW(BasicBone)[curmesh.mNbBones];
	ZCB_CHECKALLOC(curmesh.mSkeleton);

	for(udword i=0;i<curmesh.mNbBones;i++)
	{
		curmesh.mSkeleton[i].CSID	= importer.ReadDword();	// CSID
		curmesh.mSkeleton[i].pCSID	= importer.ReadDword();	// parent's CSID
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the texture-vertices.
 *	\param		curmesh			[in] current mesh structure
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportTextureVertices(ZCBMeshInfo& curmesh, ImportArray& importer)
{
	// Checkings
	if(!curmesh.mNbTVerts)	return true;

//	if(curmesh.mFlags & ZCB_UVW)	// ### apparently we save them regardless of this flag
	{
		// Get some bytes for texture vertices
		curmesh.mTVerts = ICE_NEW(Point)[curmesh.mNbTVerts];
		ZCB_CHECKALLOC(curmesh.mTVerts);

		// Get texture-vertices back
		if(curmesh.mFlags&ZCB_QUANTIZED_VERTICES)
		{
			// Get dequantization coeffs
			const float DequantCoeffX = importer.ReadFloat();
			const float DequantCoeffY = importer.ReadFloat();
			const float DequantCoeffZ = (curmesh.mFlags & ZCB_W_DISCARDED) ? 0.0f : importer.ReadFloat();

			// Get quantized vertices
			for(udword i=0;i<curmesh.mNbTVerts;i++)
			{
				const sword x = importer.ReadWord();
				const sword y = importer.ReadWord();
				const sword z = (curmesh.mFlags & ZCB_W_DISCARDED) ? 0 : importer.ReadWord();
				// Dequantize
				curmesh.mTVerts[i].x = float(x) * DequantCoeffX;
				curmesh.mTVerts[i].y = float(y) * DequantCoeffY;
				curmesh.mTVerts[i].z = float(z) * DequantCoeffZ;
			}
		}
		else
		{
			// Get texture-vertices
			if(1)
			{
				for(udword i=0;i<curmesh.mNbTVerts;i++)
				{
					curmesh.mTVerts[i].x = importer.ReadFloat();
					curmesh.mTVerts[i].y = importer.ReadFloat();
					curmesh.mTVerts[i].z = (curmesh.mFlags & ZCB_W_DISCARDED) ? 0.0f : importer.ReadFloat();
				}
			}
			else
			{
				if(curmesh.mFlags & ZCB_W_DISCARDED)
				{
					importer.ReadBuffer(curmesh.mTVerts, sizeof(float)*2*curmesh.mNbTVerts);
					const float* Src = (const float*)curmesh.mTVerts;
					float* Dst = (float*)curmesh.mTVerts;
					for(udword i=0;i<curmesh.mNbTVerts;i++)
					{
						const udword Index = curmesh.mNbTVerts-1-i;
						Dst[Index*3+0] = Src[Index*2+0];
						Dst[Index*3+1] = Src[Index*2+1];
						Dst[Index*3+2] = 0.0f;
					}
				}
				else
				{
					importer.ReadBuffer(curmesh.mTVerts, sizeof(float)*3*curmesh.mNbTVerts);
				}
			}
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the vertex-colors.
 *	\param		curmesh			[in] current mesh structure
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportVertexColors(ZCBMeshInfo& curmesh, ImportArray& importer)
{
	// Checkings
	if(!curmesh.mNbCVerts)	return true;

//	if(curmesh.mFlags & ZCB_VERTEX_COLORS)	// ### apparently we save them regardless of this flag
	{
		// Get some bytes for vertex-colors
		curmesh.mCVerts = ICE_NEW(Point)[curmesh.mNbCVerts];
		ZCB_CHECKALLOC(curmesh.mCVerts);

		// Get vertex-colors back
		if(curmesh.mFlags&ZCB_QUANTIZED_VERTICES)
		{
			// Get dequantization coeffs
			const float DequantCoeffX = importer.ReadFloat();
			const float DequantCoeffY = importer.ReadFloat();
			const float DequantCoeffZ = importer.ReadFloat();

			// Get quantized vertices
			for(udword i=0;i<curmesh.mNbCVerts;i++)
			{
				const sword x = importer.ReadWord();
				const sword y = importer.ReadWord();
				const sword z = importer.ReadWord();
				// Dequantize
				curmesh.mCVerts[i].x = float(x) * DequantCoeffX;
				curmesh.mCVerts[i].y = float(y) * DequantCoeffY;
				curmesh.mCVerts[i].z = float(z) * DequantCoeffZ;
			}
		}
		else
		{
			// Get vertex-colors
/*			for(udword i=0;i<curmesh.mNbCVerts;i++)
			{
				curmesh.mCVerts[i].x = importer.ReadFloat();
				curmesh.mCVerts[i].y = importer.ReadFloat();
				curmesh.mCVerts[i].z = importer.ReadFloat();
			}*/
			importer.ReadBuffer(curmesh.mCVerts, sizeof(float)*3*curmesh.mNbCVerts);
		}

		// Vertex alpha
		if(curmesh.mFlags&ZCB_VERTEX_ALPHA)
		{
			curmesh.mVertexAlpha = (float*)ICE_ALLOC(sizeof(float)*curmesh.mNbVerts);
			for(udword i=0;i<curmesh.mNbVerts;i++)
			{
				curmesh.mVertexAlpha[i] = importer.ReadFloat();
			}
		}
	}
	return true;
}

static void ImportFaces(ImportArray& importer, udword* faces, bool word_faces, udword nb_faces)
{
	if(faces)
	{
		if(1)
		{
			// PT: november, 25, 2006: can't remember why there was a cast to sword but it creates bugs
			// when indices cross 0x8000...
			for(udword j=0;j<nb_faces*3;j++)
//				faces[j] = word_faces ? (sword)importer.ReadWord() : importer.ReadDword();
				faces[j] = word_faces ? importer.ReadWord() : importer.ReadDword();
		}
		else
		{
			const udword NbIndices = nb_faces * 3;
			const udword TotalSize = word_faces ? (NbIndices*sizeof(uword)) : (NbIndices*sizeof(udword));
			importer.ReadBuffer(faces, TotalSize);
			if(word_faces)
			{
				// 0x1234123412341234
				// 0x00001234000012340000123400001234
				const uword* Src = (const uword*)faces;
				uword* Dst = (uword*)faces;
				for(udword i=0;i<NbIndices;i++)
				{
					const udword Index = NbIndices-1-i;
					Dst[Index*2+1] = 0;
					Dst[Index*2+0] = Src[Index];
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the topologies.
 *	\param		curmesh			[in] current mesh structure
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportFaces(ZCBMeshInfo& curmesh, ImportArray& importer)
{
	// Get number of faces
	udword NbFaces = curmesh.mNbFaces;

	// Get some bytes for faces
	if(NbFaces)
	{
		if(curmesh.mNbVerts)	{ curmesh.mFaces = (udword*)ICE_ALLOC(sizeof(udword)*NbFaces*3);	ZCB_CHECKALLOC(curmesh.mFaces); }
		if(curmesh.mNbTVerts)	{ curmesh.mTFaces = (udword*)ICE_ALLOC(sizeof(udword)*NbFaces*3);	ZCB_CHECKALLOC(curmesh.mTFaces); }
		if(curmesh.mNbCVerts)	{ curmesh.mCFaces = (udword*)ICE_ALLOC(sizeof(udword)*NbFaces*3);	ZCB_CHECKALLOC(curmesh.mCFaces); }
	}

	// Get faces
	const bool UseWordFaces = (curmesh.mFlags&ZCB_WORD_FACES)!=0;
	::ImportFaces(importer, curmesh.mFaces, UseWordFaces, NbFaces);

	// Get texture faces
	::ImportFaces(importer, curmesh.mTFaces, UseWordFaces, NbFaces);

	// Get color faces
	::ImportFaces(importer, curmesh.mCFaces, UseWordFaces, NbFaces);

	// Get face properties
	if(NbFaces)
	{
		curmesh.mFaceProperties = ICE_NEW(FaceProperties)[NbFaces];
		ZCB_CHECKALLOC(curmesh.mFaceProperties);
	}

	for(udword j=0;j<NbFaces;j++)	{ curmesh.mFaceProperties[j].MatID	= importer.ReadDword();	PatchMaterialID(curmesh.mFaceProperties[j].MatID);	}
	for(udword j=0;j<NbFaces;j++)	curmesh.mFaceProperties[j].Smg		= importer.ReadDword();
	for(udword j=0;j<NbFaces;j++)	curmesh.mFaceProperties[j].EdgeVis	= (curmesh.mFlags&ZCB_EDGE_VIS) ? importer.ReadByte() : 0;

	// Undo delta compression
	if(curmesh.mFlags&ZCB_COMPRESSED)
	{
		if(curmesh.mFaces)	UnDelta(curmesh.mFaces, NbFaces*3, 4);
		if(curmesh.mTFaces)	UnDelta(curmesh.mTFaces, NbFaces*3, 4);
		if(curmesh.mCFaces)	UnDelta(curmesh.mCFaces, NbFaces*3, 4);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the volume integrals and other extra stuff.
 *	\param		curmesh			[in] current mesh structure
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportExtraStuff(ZCBMeshInfo& curmesh, ImportArray& importer)
{
	// Get convex hull back
	if(curmesh.mFlags & ZCB_CONVEX_HULL)
	{
		curmesh.mNbHullVerts = importer.ReadDword();
		curmesh.mNbHullFaces = importer.ReadDword();

		// Get hull vertices
		curmesh.mHullVerts	= ICE_NEW(Point)[curmesh.mNbHullVerts];
		ZCB_CHECKALLOC(curmesh.mHullVerts);

		for(udword i=0;i<curmesh.mNbHullVerts;i++)
		{
			curmesh.mHullVerts[i].x = importer.ReadFloat();
			curmesh.mHullVerts[i].y = importer.ReadFloat();
			curmesh.mHullVerts[i].z = importer.ReadFloat();
		}

		// Get hull faces
		curmesh.mHullFaces	= (udword*)ICE_ALLOC(sizeof(udword)*curmesh.mNbHullFaces*3);
		ZCB_CHECKALLOC(curmesh.mHullFaces);

		for(udword i=0;i<curmesh.mNbHullFaces;i++)
		{
			curmesh.mHullFaces[i*3+0] = importer.ReadDword();
			curmesh.mHullFaces[i*3+1] = importer.ReadDword();
			curmesh.mHullFaces[i*3+2] = importer.ReadDword();
		}
	}

	// Get bounding sphere back
	if(curmesh.mFlags & ZCB_BOUNDING_SPHERE)
	{
		curmesh.mBSCenter.x = importer.ReadFloat();
		curmesh.mBSCenter.y = importer.ReadFloat();
		curmesh.mBSCenter.z = importer.ReadFloat();
		curmesh.mBSRadius	= importer.ReadFloat();
	}

	// Get volume integrals back
	if(curmesh.mFlags & ZCB_INERTIA_TENSOR)
	{
		// Center of mass
		curmesh.mCOM.x = importer.ReadFloat();
		curmesh.mCOM.y = importer.ReadFloat();
		curmesh.mCOM.z = importer.ReadFloat();

		// Mass
		curmesh.mComputedMass = importer.ReadFloat();

		// Inertia tensor
		curmesh.mInertiaTensor[0][0] = importer.ReadFloat();
		curmesh.mInertiaTensor[0][1] = importer.ReadFloat();
		curmesh.mInertiaTensor[0][2] = importer.ReadFloat();
		curmesh.mInertiaTensor[1][0] = importer.ReadFloat();
		curmesh.mInertiaTensor[1][1] = importer.ReadFloat();
		curmesh.mInertiaTensor[1][2] = importer.ReadFloat();
		curmesh.mInertiaTensor[2][0] = importer.ReadFloat();
		curmesh.mInertiaTensor[2][1] = importer.ReadFloat();
		curmesh.mInertiaTensor[2][2] = importer.ReadFloat();

		// COM inertia tensor
		curmesh.mCOMInertiaTensor[0][0] = importer.ReadFloat();
		curmesh.mCOMInertiaTensor[0][1] = importer.ReadFloat();
		curmesh.mCOMInertiaTensor[0][2] = importer.ReadFloat();
		curmesh.mCOMInertiaTensor[1][0] = importer.ReadFloat();
		curmesh.mCOMInertiaTensor[1][1] = importer.ReadFloat();
		curmesh.mCOMInertiaTensor[1][2] = importer.ReadFloat();
		curmesh.mCOMInertiaTensor[2][0] = importer.ReadFloat();
		curmesh.mCOMInertiaTensor[2][1] = importer.ReadFloat();
		curmesh.mCOMInertiaTensor[2][2] = importer.ReadFloat();
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the consolidation results.
 *	\param		curmesh			[in] current mesh structure
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportConsolidated(ZCBMeshInfo& curmesh, ImportArray& importer)
{
	// Get consolidated mesh back
	if(curmesh.mFlags & ZCB_CONSOLIDATION)
	{
		curmesh.mCleanMesh = ICE_NEW(MBResult);
		ZCB_CHECKALLOC(curmesh.mCleanMesh);

		// Get topology back
		{
			MBTopology& Topo = curmesh.mCleanMesh->Topology;

			if(mMESHVersion<7)
			{
				Topo.NbFaces		= importer.ReadWord();
				Topo.NbSubmeshes	= importer.ReadWord();
			}
			else
			{
				Topo.NbFaces		= importer.ReadDword();
				Topo.NbSubmeshes	= importer.ReadDword();
			}

			// Submeshes
			Topo.SubmeshProperties = ICE_NEW(MBSubmesh)[Topo.NbSubmeshes];
			ZCB_CHECKALLOC(Topo.SubmeshProperties);

			for(udword i=0;i<Topo.NbSubmeshes;i++)
			{
				MBSubmesh* CurSM = &Topo.SubmeshProperties[i];

				CurSM->MatID		= (sdword)importer.ReadDword();
				PatchMaterialID(CurSM->MatID);
				CurSM->SmGrp		= importer.ReadDword();
				CurSM->NbFaces		= importer.ReadDword();
				CurSM->NbVerts		= importer.ReadDword();
				CurSM->NbSubstrips	= importer.ReadDword();
			}

			// Connectivity
			Topo.FacesInSubmesh = (udword*)ICE_ALLOC(sizeof(udword)*Topo.NbSubmeshes);
			ZCB_CHECKALLOC(Topo.FacesInSubmesh);

			Topo.VRefs = (udword*)ICE_ALLOC(sizeof(udword)*Topo.NbFaces*3);
			ZCB_CHECKALLOC(Topo.VRefs);

			udword* VRefs = Topo.VRefs;
			for(udword i=0;i<Topo.NbSubmeshes;i++)
			{
				if(mMESHVersion<7)	Topo.FacesInSubmesh[i] = importer.ReadWord();
				else				Topo.FacesInSubmesh[i] = importer.ReadDword();

				for(udword j=0;j<Topo.FacesInSubmesh[i];j++)
				{
					if(mMESHVersion<7)
					{
						*VRefs++ = importer.ReadWord();
						*VRefs++ = importer.ReadWord();
						*VRefs++ = importer.ReadWord();
					}
					else
					{
						*VRefs++ = importer.ReadDword();
						*VRefs++ = importer.ReadDword();
						*VRefs++ = importer.ReadDword();
					}
				}
			}

			// Face normals
			if(curmesh.mFlags & ZCB_FACE_NORMALS)
			{
				Topo.Normals = (float*)ICE_ALLOC(sizeof(float)*Topo.NbFaces*3);
				ZCB_CHECKALLOC(Topo.Normals);

				for(udword i=0;i<Topo.NbFaces;i++)
				{
					Topo.Normals[i*3+0] = importer.ReadFloat();
					Topo.Normals[i*3+1] = importer.ReadFloat();
					Topo.Normals[i*3+2] = importer.ReadFloat();
				}
			}
		}

		// Get geometry back
		{
			MBGeometry& Geo = curmesh.mCleanMesh->Geometry;

			if(mMESHVersion<7)
			{
				Geo.NbGeomPts	= importer.ReadWord();
				Geo.NbVerts		= importer.ReadWord();
				Geo.NbTVerts	= importer.ReadWord();
			}
			else
			{
				Geo.NbGeomPts	= importer.ReadDword();
				Geo.NbVerts		= importer.ReadDword();
				Geo.NbTVerts	= importer.ReadDword();
			}

			// Indexed geometry
			Geo.VertsRefs = (udword*)ICE_ALLOC(sizeof(udword)*Geo.NbVerts);
			ZCB_CHECKALLOC(Geo.VertsRefs);

			for(udword i=0;i<Geo.NbVerts;i++)
			{
				Geo.VertsRefs[i] = importer.ReadDword();
			}

			// Vertices
			udword _NbVerts = importer.ReadDword();
			Geo.Verts = (float*)ICE_ALLOC(sizeof(float)*_NbVerts*3);
			ZCB_CHECKALLOC(Geo.Verts);

			for(udword i=0;i<_NbVerts;i++)
			{
				Geo.Verts[i*3+0] = importer.ReadFloat();
				Geo.Verts[i*3+1] = importer.ReadFloat();
				Geo.Verts[i*3+2] = importer.ReadFloat();
			}

			// Indexed UVWs
			if(curmesh.mFlags & ZCB_UVW)
			{
				Geo.TVertsRefs = (udword*)ICE_ALLOC(sizeof(udword)*Geo.NbVerts);
				ZCB_CHECKALLOC(Geo.TVertsRefs);

				for(udword i=0;i<Geo.NbVerts;i++)
				{
					Geo.TVertsRefs[i] = importer.ReadDword();
				}

				// UVWs
				udword _NbTVerts = importer.ReadDword();
				Geo.TVerts = (float*)ICE_ALLOC(sizeof(float)*_NbTVerts*3);
				ZCB_CHECKALLOC(Geo.TVerts);

				float* p = Geo.TVerts;
				for(udword i=0;i<_NbTVerts;i++)
				{
					*p++ = importer.ReadFloat();
					*p++ = importer.ReadFloat();
					if(!(curmesh.mFlags & ZCB_W_DISCARDED))	
					{
						*p++ = importer.ReadFloat();
					}
				}
			}

			// Normals
			if(curmesh.mFlags & ZCB_VERTEX_NORMALS)
			{
				udword NbNormals = importer.ReadDword();
				Geo.Normals = (float*)ICE_ALLOC(sizeof(float)*NbNormals*3);
				ZCB_CHECKALLOC(Geo.Normals);

				for(udword i=0;i<NbNormals;i++)
				{
					Geo.Normals[i*3+0] = importer.ReadFloat();
					Geo.Normals[i*3+1] = importer.ReadFloat();
					Geo.Normals[i*3+2] = importer.ReadFloat();
				}
			}

			// Vertex colors
			if(curmesh.mFlags & ZCB_VERTEX_COLORS)
			{
				udword NbVtxColors = importer.ReadDword();

				if(curmesh.mFlags & ZCB_DWORD_COLORS)
				{
					Geo.CVerts = (float*)ICE_ALLOC(sizeof(float)*NbVtxColors*3);
					ZCB_CHECKALLOC(Geo.CVerts);

					for(udword i=0;i<NbVtxColors;i++)
					{
						Geo.CVerts[i*3+0] = importer.ReadFloat();	// actually a dword RGBA color
						Geo.CVerts[i*3+1] = 0.0f;
						Geo.CVerts[i*3+2] = 0.0f;
					}
				}
				else
				{
					Geo.CVerts = (float*)ICE_ALLOC(sizeof(float)*NbVtxColors*3);
					ZCB_CHECKALLOC(Geo.CVerts);

					for(udword i=0;i<NbVtxColors;i++)
					{
						Geo.CVerts[i*3+0] = importer.ReadFloat();
						Geo.CVerts[i*3+1] = importer.ReadFloat();
						Geo.CVerts[i*3+2] = importer.ReadFloat();
					}
				}
			}

			// NormalInfo
			if(curmesh.mFlags & ZCB_NORMAL_INFO)
			{
				Geo.NormalInfoSize = importer.ReadDword();
				Geo.NormalInfo = (udword*)ICE_ALLOC(sizeof(udword)*Geo.NormalInfoSize);
				ZCB_CHECKALLOC(Geo.NormalInfo);

				for(udword i=0;i<Geo.NormalInfoSize;i++)
				{
					Geo.NormalInfo[i] = importer.ReadDword();
				}
			}
		}

		// Materials
		{
			MBMaterials& Mtl = curmesh.mCleanMesh->Materials;

			Mtl.NbMtls = importer.ReadDword();
			Mtl.MaterialInfo	= ICE_NEW(MBMatInfo)[Mtl.NbMtls];
			ZCB_CHECKALLOC(Mtl.MaterialInfo);

			for(udword i=0;i<Mtl.NbMtls;i++)
			{
				MBMatInfo* CurMtl = &Mtl.MaterialInfo[i];

				CurMtl->MatID		= (sdword)importer.ReadDword();
				PatchMaterialID(CurMtl->MatID);
				CurMtl->NbFaces		= importer.ReadDword();
				CurMtl->NbVerts		= importer.ReadDword();
				CurMtl->NbSubmeshes	= importer.ReadDword();
			}
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the lighting data.
 *	\param		curmesh			[in] current mesh structure
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportLightingData(ZCBMeshInfo& curmesh, ImportArray& importer)
{
	// Get number of precomputed colors
	curmesh.mNbColors = importer.ReadDword();

	// Get them back if needed
	if(curmesh.mNbColors)
	{
		curmesh.mColors = ICE_NEW(Point)[curmesh.mNbColors];
		ZCB_CHECKALLOC(curmesh.mColors);

		for(udword i=0;i<curmesh.mNbColors;i++)
		{
			curmesh.mColors[i].x = importer.ReadFloat();
			curmesh.mColors[i].y = importer.ReadFloat();
			curmesh.mColors[i].z = importer.ReadFloat();
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the controllers.
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportControllers(ImportArray& importer)
{
#ifdef ZCB_PROGRESS_BAR
/*	gParams->mText = "Importing controllers...";
	gParams->mRequestCode = RQ_SET_TEXT_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);

	gParams->mMin = 0;
	gParams->mMax = mNbControllers;
	gParams->mRequestCode = RQ_SET_RANGE_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);*/
	if(gProgressBar)
	{
		gProgressBar->SetText1("Importing controllers...");
		gProgressBar->SetRange1(0, mNbControllers);
	}
#endif
	// Are there any controllers to import?
	if(mNbControllers)
	{
		// Find CTRL chunk
		LOAD_CHUNK(mControllerOffset, "CTRLCHUNK", "CTRL")
		if(Status)
//		if(importer.GetChunk(mZCBGlobalVersion ? "CTRLCHUNK" : "CTRL"))
		{
			// Get version number back
			mCTRLVersion		= importer.ReadDword();
			ZCB_CHECKVERSION(mCTRLVersion, CHUNK_CTRL_VER, "CTRL");

			// Log message
			ZCBLog("Importing %d controllers...\n", mNbControllers);

			// Import all controllers
			for(udword n=0;n<mNbControllers;n++)
			{
				// Fill a controller structure
				ZCBControllerInfo CurController;

				CurController.mField.Set((const char*)importer.ReadString());	// Get the field back
				CurController.mObjectID	= importer.ReadDword();					// Controller's ID
				PatchControllerID(CurController.mObjectID);
				CurController.mOwnerID	= importer.ReadDword();					// Owner's ID
				if(mCTRLVersion>=2)
				{
					// Needed since now we export controllers for camera/material parameters, etc
					CurController.mOwnerType = (ZCBObjType)importer.ReadDword();	// Owner's type
					if(CurController.mOwnerType==ZCB_OBJ_MATERIAL)	PatchMaterialID(CurController.mOwnerID);
					if(CurController.mOwnerType==ZCB_OBJ_TEXTURE)	PatchTextureID(CurController.mOwnerID);
				}
				else
				{
					// Old versions only exported mesh controllers
					CurController.mOwnerType = ZCB_OBJ_MESH;
				}

				CurController.mCtrlType	= (ZCBCtrlType)importer.ReadDword();		// Controller type
				CurController.mCtrlMode	= (ZCBCtrlMode)importer.ReadDword();		// Controller mode

				if(CurController.mCtrlMode==ZCB_CTRL_SAMPLES)
				{
					if(CurController.mCtrlType==ZCB_CTRL_VERTEXCLOUD)	CurController.mNbVertices = importer.ReadDword();

					CurController.mNbSamples	= importer.ReadDword();
					CurController.mSamplingRate	= importer.ReadDword();

					// Use built-in types only
					switch(CurController.mCtrlType)
					{
						case ZCB_CTRL_FLOAT:
						{
							float* Samples = (float*)ICE_ALLOC(sizeof(float)*CurController.mNbSamples);
							ZCB_CHECKALLOC(Samples);
							for(udword i=0;i<CurController.mNbSamples;i++)
							{
								Samples[i]	= importer.ReadFloat();
							}
							CurController.mSamples = Samples;
						}
						break;

						case ZCB_CTRL_VECTOR:
						{
							Point* Samples = (Point*)ICE_ALLOC(sizeof(float)*3*CurController.mNbSamples);
							ZCB_CHECKALLOC(Samples);
							for(udword i=0;i<CurController.mNbSamples;i++)
							{
								Samples[i].x	= importer.ReadFloat();
								Samples[i].y	= importer.ReadFloat();
								Samples[i].z	= importer.ReadFloat();
							}
							CurController.mSamples = Samples;
						}
						break;

						case ZCB_CTRL_QUAT:
						{
							Quat* Samples = (Quat*)ICE_ALLOC(sizeof(float)*4*CurController.mNbSamples);
							ZCB_CHECKALLOC(Samples);
							for(udword i=0;i<CurController.mNbSamples;i++)
							{
								Samples[i].p.x	= importer.ReadFloat();
								Samples[i].p.y	= importer.ReadFloat();
								Samples[i].p.z	= importer.ReadFloat();
								Samples[i].w	= importer.ReadFloat();
							}
							CurController.mSamples = Samples;
						}
						break;

						case ZCB_CTRL_PR:
						{
							PR* Samples = (PR*)ICE_ALLOC(sizeof(float)*7*CurController.mNbSamples);
							ZCB_CHECKALLOC(Samples);
							for(udword i=0;i<CurController.mNbSamples;i++)
							{
								Samples[i].mPos.x	= importer.ReadFloat();
								Samples[i].mPos.y	= importer.ReadFloat();
								Samples[i].mPos.z	= importer.ReadFloat();
								Samples[i].mRot.p.x	= importer.ReadFloat();
								Samples[i].mRot.p.y	= importer.ReadFloat();
								Samples[i].mRot.p.z	= importer.ReadFloat();
								Samples[i].mRot.w	= importer.ReadFloat();
							}
							CurController.mSamples = Samples;
						}
						break;

						case ZCB_CTRL_PRS:
						{
							PRS* Samples = (PRS*)ICE_ALLOC(sizeof(float)*10*CurController.mNbSamples);
							ZCB_CHECKALLOC(Samples);
							for(udword i=0;i<CurController.mNbSamples;i++)
							{
								Samples[i].mPos.x	= importer.ReadFloat();
								Samples[i].mPos.y	= importer.ReadFloat();
								Samples[i].mPos.z	= importer.ReadFloat();
								Samples[i].mRot.p.x	= importer.ReadFloat();
								Samples[i].mRot.p.y	= importer.ReadFloat();
								Samples[i].mRot.p.z	= importer.ReadFloat();
								Samples[i].mRot.w	= importer.ReadFloat();
								Samples[i].mScale.x	= importer.ReadFloat();
								Samples[i].mScale.y	= importer.ReadFloat();
								Samples[i].mScale.z	= importer.ReadFloat();
							}
							CurController.mSamples = Samples;
						}
						break;

						case ZCB_CTRL_VERTEXCLOUD:
						{
							float* Samples = (float*)ICE_ALLOC(sizeof(float)*3*CurController.mNbSamples*CurController.mNbVertices);
							ZCB_CHECKALLOC(Samples);
							float* Pool = (float*)Samples;
							for(udword i=0;i<CurController.mNbSamples;i++)
							{
								for(udword j=0;j<CurController.mNbVertices;j++)
								{
									*Pool++	= importer.ReadFloat();
									*Pool++	= importer.ReadFloat();
									*Pool++	= importer.ReadFloat();
								}
							}
							CurController.mSamples = Samples;
						}
						break;
					}
				}
				else return false;	// only sampling for now.....

				// Call the app
				NewController(CurController);

				UpdateProgressBar1();
			}
		}
		else
		{
			ZCBImportError("Chunk CTRL not found!", ZCB_ERROR_CHUNK_NOT_FOUND);
			return false;
		}
	}
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the shapes.
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportShapes(ImportArray& importer)
{
#ifdef ZCB_PROGRESS_BAR
/*	gParams->mText = "Importing shapes...";
	gParams->mRequestCode = RQ_SET_TEXT_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);

	gParams->mMin = 0;
	gParams->mMax = mNbShapes;
	gParams->mRequestCode = RQ_SET_RANGE_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);*/
	if(gProgressBar)
	{
		gProgressBar->SetText1("Importing shapes...");
		gProgressBar->SetRange1(0, mNbShapes);
	}
#endif
	// Are there any shapes to import?
	if(mNbShapes)
	{
		// Find SHAP chunk
		LOAD_CHUNK(mShapeOffset, "SHAPCHUNK", "SHAP")
		if(Status)
//		if(importer.GetChunk(mZCBGlobalVersion ? "SHAPCHUNK" : "SHAP"))
		{
			// Get version number back
			mSHAPVersion	= importer.ReadDword();
			ZCB_CHECKVERSION(mSHAPVersion, CHUNK_SHAP_VER, "SHAP");

			// Log message
			ZCBLog("Importing %d shapes...\n", mNbShapes);

			// Import all shapes
			for(udword n=0;n<mNbShapes;n++)
			{
				// Fill a shape structure
				ZCBShapeInfo CurShape;

				// Base info
				CurShape.Import(importer);

				// Get type
				if(mSHAPVersion>=3)	CurShape.mType = (ZCBShapeType)importer.ReadDword();
				else			CurShape.mType = ZCB_SHAP_UNDEFINED;

				// Get shape information back
				CurShape.mNbLines	= importer.ReadDword();	// Number of lines
				if(CHUNK_SHAP_VER>=2)
				{
					CurShape.mMatID = importer.ReadDword();	// Material ID
					PatchMaterialID(CurShape.mMatID);
				}
				if(CurShape.mNbLines)
				{
					CurShape.mNbVerts = (udword*)ICE_ALLOC(sizeof(udword)*CurShape.mNbLines);	ZCB_CHECKALLOC(CurShape.mNbVerts);
					CurShape.mClosed = (bool*)ICE_ALLOC(sizeof(bool)*CurShape.mNbLines);		ZCB_CHECKALLOC(CurShape.mClosed);

					// Get all polylines
					CustomArray	Vertices;
					CurShape.mTotalNbVerts = 0;
					for(udword i=0;i<CurShape.mNbLines;i++)
					{
						udword NbVerts = importer.ReadDword();	// Number of vertices in current line
						bool Closed = importer.ReadBool();		// Closed/open status
						for(udword j=0;j<NbVerts;j++)
						{
							float x = importer.ReadFloat();
							float y = importer.ReadFloat();
							float z = importer.ReadFloat();
							Vertices.Store(x).Store(y).Store(z);
						}
						CurShape.mNbVerts[i] = NbVerts;
						CurShape.mClosed[i] = Closed;
						CurShape.mTotalNbVerts+=NbVerts;
					}
					CurShape.mVerts = ICE_NEW(Point)[CurShape.mTotalNbVerts];
					ZCB_CHECKALLOC(CurShape.mVerts);
					CopyMemory(CurShape.mVerts, Vertices.Collapse(), CurShape.mTotalNbVerts*sizeof(Point));
				}

				// Call the app
				NewShape(CurShape);

				UpdateProgressBar1();
			}
		}
		else
		{
			ZCBImportError("Chunk SHAP not found!", ZCB_ERROR_CHUNK_NOT_FOUND);
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Imports the helpers.
 *	\param		importer		[in] the imported array.
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ZCBBreaker::ImportHelpers(ImportArray& importer)
{
#ifdef ZCB_PROGRESS_BAR
/*	gParams->mText = "Importing helpers...";
	gParams->mRequestCode = RQ_SET_TEXT_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);

	gParams->mMin = 0;
	gParams->mMax = mNbHelpers;
	gParams->mRequestCode = RQ_SET_RANGE_PROGRESS_BAR1;
	GetCallbacksManager()->ExecCallbacks(ICCB_REQUEST, ICCB_REQUEST, gParams);*/
	if(gProgressBar)
	{
		gProgressBar->SetText1("Importing helpers...");
		gProgressBar->SetRange1(0, mNbHelpers);
	}
#endif
	// Are there any helpers to import?
	if(mNbHelpers)
	{
		// Find HELP chunk
		LOAD_CHUNK(mHelperOffset, "HELPCHUNK", "HELP")
		if(Status)
//		if(importer.GetChunk(mZCBGlobalVersion ? "HELPCHUNK" : "HELP"))
		{
			// Get version number back
			mHELPVersion	= importer.ReadDword();
			ZCB_CHECKVERSION(mHELPVersion, CHUNK_HELP_VER, "HELP");

			// Log message
			ZCBLog("Importing %d helpers...\n", mNbHelpers);

			// Import all helpers
			for(udword n=0;n<mNbHelpers;n++)
			{
				// Fill a helper structure
				ZCBHelperInfo CurHelper;

				// Base info
				CurHelper.Import(importer);

				// Get helper information back
				if(mHELPVersion<2)	CurHelper.mIsGroupHead	= importer.ReadBool();
				else
				{
					CurHelper.mHelperType	= (ZCBHelperType)importer.ReadDword();
					CurHelper.mIsGroupHead	= importer.ReadBool();

					if(CurHelper.mIsGroupHead && mHELPVersion>=3)
					{
						// Get the list of grouped objects
						udword NbGroupedObjects = importer.ReadDword();
						CurHelper.mGroupedObjects.SetSize(NbGroupedObjects);
						for(udword i=0;i<NbGroupedObjects;i++)
						{
							udword ID = importer.ReadDword();
							CurHelper.mGroupedObjects.Add(ID);
						}
					}

					// Get gizmo data
					switch(CurHelper.mHelperType)
					{
						case ZCB_HELPER_GIZMO_BOX:
						{
							CurHelper.mLength	= importer.ReadFloat();
							CurHelper.mWidth	= importer.ReadFloat();
							CurHelper.mHeight	= importer.ReadFloat();
						}
						break;

						case ZCB_HELPER_GIZMO_SPHERE:
						{
							CurHelper.mRadius	= importer.ReadFloat();
							CurHelper.mHemi		= importer.ReadBool();
						}
						break;

						case ZCB_HELPER_GIZMO_CYLINDER:
						{
							CurHelper.mRadius	= importer.ReadFloat();
							CurHelper.mHeight	= importer.ReadFloat();
						}
						break;

						case ZCB_HELPER_BILLBOARD:
						{
							float Size		= importer.ReadFloat();
							float Length	= importer.ReadFloat();
							bool ScreenAlign = importer.ReadBool();
						}
						break;
					}
				}

				// Call the app
				NewHelper(CurHelper);

				UpdateProgressBar1();
			}
		}
		else
		{
			ZCBImportError("Chunk HELP not found!", ZCB_ERROR_CHUNK_NOT_FOUND);
			return false;
		}
	}
	return true;
}






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
WARNING:
- the offsets must be patched as well!
*/

#ifdef REMOVED
ZCBConverter::ZCBConverter()
{
}

ZCBConverter::~ZCBConverter()
{
}

bool ZCBConverter::Convert(const char* filename)
{
	if(!filename)	return false;

	// Find the file somewhere in registered paths
	VirtualFileHandle FoundFile;

	// Check the file exists
	if(!FindFile(filename, &FoundFile))
		return false;

	VirtualFile File(FoundFile);
	udword Length;
	ubyte* tmp = File.Load(Length);
	ImportArray* mImportArray = ICE_NEW(BufferReadStream)(tmp, Length);

	// Check format signature
	const ubyte b1 = mImportArray->ReadByte();
	const ubyte b2 = mImportArray->ReadByte();
	const ubyte b3 = mImportArray->ReadByte();

	// The three first bytes must be "ZCB"
	if(b1!='Z' || b2!='C' || b3!='B')
		return false;

	// The fourth byte can be '!' for normal files and 'P' for packed files.
	const ubyte b4 = mImportArray->ReadByte();
	if(b4!='!')
		return false;

	// This is a valid ZCB file. Get the type.
	const udword Data = mImportArray->ReadDword();
	// Since 1.14 we need to mask out reserved bits & get back the global version
udword
	mZCBGlobalVersion = Data>>16;
ZCBFile
	mFileType = ZCBFile(Data & ZCB_FILE_MASK);
	if(mFileType!=ZCB_FILE_SCENE)
		return false;

	gZCBVersion = mZCBGlobalVersion;

	// Find MAIN chunk
	if(!mImportArray->GetChunk(mZCBGlobalVersion ? "MAINCHUNK" : "MAIN"))
		return false;

	// Get chunk version
udword
	mMAINVersion	= mImportArray->ReadDword();
//	ZCB_CHECKVERSION(mMAINVersion, CHUNK_MAIN_VER, "MAIN");

	// Ugly but....
	if(mMAINVersion>=2)	gHasPivot = true;
	else				gHasPivot = false;

	// Fill a scene structure
	ZCBSceneInfo	SceneInfo;
	// Time-related info
	SceneInfo.mFirstFrame		= mImportArray->ReadDword();
	SceneInfo.mLastFrame		= mImportArray->ReadDword();
	SceneInfo.mFrameRate		= mImportArray->ReadDword();
	SceneInfo.mDeltaTime		= mImportArray->ReadDword();
	// Background color
	SceneInfo.mBackColor.x		= mImportArray->ReadFloat();
	SceneInfo.mBackColor.y		= mImportArray->ReadFloat();
	SceneInfo.mBackColor.z		= mImportArray->ReadFloat();
	// Global ambient color
	SceneInfo.mAmbientColor.x	= mImportArray->ReadFloat();
	SceneInfo.mAmbientColor.y	= mImportArray->ReadFloat();
	SceneInfo.mAmbientColor.z	= mImportArray->ReadFloat();

	// Scene info
	if(mMAINVersion>=2)	SceneInfo.mSceneInfo.Set((const char*)mImportArray->ReadString());
	if(mMAINVersion>=4)
	{
		SceneInfo.mSceneHelpText.Set((const char*)mImportArray->ReadString());

		if(mMAINVersion>=5)
		{
			SceneInfo.mTesselation = mImportArray->ReadDword();
		}
		if(mMAINVersion>=6)
		{
			SceneInfo.mLightingMode = mImportArray->ReadDword();
		}

		// Scene physics
		SceneInfo.mGravity.x = mImportArray->ReadFloat();
		SceneInfo.mGravity.y = mImportArray->ReadFloat();
		SceneInfo.mGravity.z = mImportArray->ReadFloat();
		SceneInfo.mRestitution = mImportArray->ReadFloat();
		SceneInfo.mStaticFriction = mImportArray->ReadFloat();
		SceneInfo.mFriction = mImportArray->ReadFloat();
		SceneInfo.mGroundPlane = mImportArray->ReadBool();
		SceneInfo.mCollisionDetection = mImportArray->ReadBool();
	}

	if(mMAINVersion>=3)	SceneInfo.mGlobalScale = mImportArray->ReadFloat();

	if(mMAINVersion<7)
		return false;

udword
		mMeshOffset			= mImportArray->ReadDword();
udword
		mCameraOffset		= mImportArray->ReadDword();
udword
		mLightOffset		= mImportArray->ReadDword();
udword
		mShapeOffset		= mImportArray->ReadDword();
udword
		mHelperOffset		= mImportArray->ReadDword();
udword
		mTexmapOffset		= mImportArray->ReadDword();
udword
		mMaterialOffset		= mImportArray->ReadDword();
udword
		mControllerOffset	= mImportArray->ReadDword();
udword
		mMotionOffset		= mImportArray->ReadDword();

	// Get number of expected elements
udword
	mNbMeshes		= SceneInfo.mNbMeshes		= mImportArray->ReadDword();
udword
	mNbDerived		= SceneInfo.mNbDerived		= mImportArray->ReadDword();
udword
	mNbCameras		= SceneInfo.mNbCameras		= mImportArray->ReadDword();
udword
	mNbLights		= SceneInfo.mNbLights		= mImportArray->ReadDword();
udword
	mNbShapes		= SceneInfo.mNbShapes		= mImportArray->ReadDword();
udword
	mNbHelpers		= SceneInfo.mNbHelpers		= mImportArray->ReadDword();
udword
	mNbControllers	= SceneInfo.mNbControllers	= mImportArray->ReadDword();
udword
	mNbMaterials	= SceneInfo.mNbMaterials	= mImportArray->ReadDword();
udword
	mNbTextures		= SceneInfo.mNbTextures		= mImportArray->ReadDword();
udword
	mNbUnknowns		= SceneInfo.mNbUnknowns		= mImportArray->ReadDword();
udword
	mNbInvalidNodes	= SceneInfo.mNbInvalidNodes	= mImportArray->ReadDword();




	// Are there any textures to import?
	if(mNbTextures)
	{
		// Find TEXM chunk
		LOAD_CHUNK(mTexmapOffset, "TEXMCHUNK", "TEXM")
		if(!Status)
			return false;

		// Get version number back
		mTEXMVersion	= importer.ReadDword();
//		ZCB_CHECKVERSION(mTEXMVersion, CHUNK_TEXM_VER, "TEXM");

		// Import all textures
		for(udword n=0;n<mNbTextures;n++)
		{
			// Fill a texture structure
			ZCBTextureInfo CurTexture;

			// Database information
			CurTexture.mName.Set((const char*)importer.ReadString());	// Texture path
			CurTexture.mID = importer.ReadDword();						// Texture ID
//			PatchTextureID(CurTexture.mID);

			// Get bitmap data
			ubyte Code = 1;	// Default for version <=1
			if(mTEXMVersion>1)	Code = importer.ReadByte();
			CurTexture.mIsBitmapIncluded = Code!=0;

			if(Code)
			{
				// Get texture information back
				CurTexture.mWidth		= importer.ReadDword();
				CurTexture.mHeight		= importer.ReadDword();
				CurTexture.mHasAlpha	= importer.ReadBool();

				// Get bytes for a RGBA texture
				CurTexture.mBitmap		= (ubyte*)ICE_ALLOC(sizeof(ubyte)*CurTexture.mWidth*CurTexture.mHeight*4);

				ASSERT(Code==1);
				if(Code==1)
				{
					// => RGBA texture
					for(udword i=0;i<CurTexture.mWidth*CurTexture.mHeight;i++)
					{
						CurTexture.mBitmap[i*4+0] = importer.ReadByte();	// Red
						CurTexture.mBitmap[i*4+1] = importer.ReadByte();	// Green
						CurTexture.mBitmap[i*4+2] = importer.ReadByte();	// Blue
						CurTexture.mBitmap[i*4+3] = CurTexture.mHasAlpha ? importer.ReadByte() : PIXEL_OPAQUE;
					}
				}

				ICE_FREE(CurTexture.mBitmap);
			}

			// Cropping values & texture matrix
			ImportCroppingValues(CurTexture.mCValues, CurTexture.mTMtx, importer, CurTexture.mWrapU, CurTexture.mMirrorU, CurTexture.mWrapV, CurTexture.mMirrorV);
		}
	}



	// Release array
	DELETESINGLE(mImportArray);

	return true;
}
#endif