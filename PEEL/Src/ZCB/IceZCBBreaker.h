///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to import ZCB files.
 *	\file		IceZCBBreaker.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Usage:
  1) Use ZCBBreaker as a base class and override all app-dependent methods. They'll get called during the import.
  2) Call the Import(filename) method.

  That's it!
  ...poorly designed, but does the job...
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEZCBBREAKER_H
#define ICEZCBBREAKER_H

	#ifndef ICERENDERMANAGER_API
	#define ICERENDERMANAGER_API
	#endif

#define USE_STREAM_INTERFACE
#ifdef USE_STREAM_INTERFACE
	typedef BufferReadStream	ImportArray;
#else
	typedef CustomArray			ImportArray;
#endif


	enum ZCBError
	{
		ZCB_ERROR_OK				= 0,
		ZCB_ERROR_FILE_NOT_FOUND	= 1,
		ZCB_ERROR_OUT_OF_MEMORY		= 2,
		ZCB_ERROR_INVALID_FILE		= 3,
		ZCB_ERROR_CANT_DEPACK		= 4,
		ZCB_ERROR_UNKNOWN_VERSION	= 5,
		ZCB_ERROR_CHUNK_NOT_FOUND	= 6,
		ZCB_ERROR_OBSOLETE_FILE		= 7,

		ZCB_ERROR_FORCE_DWORD		= 0x7fffffff
	};

	class ICERENDERMANAGER_API ZCBComponents// : public Allocateable
	{
		public:
									ZCBComponents();
		virtual						~ZCBComponents();
		// ZCB file components
				udword				mNbMeshes;			//!< Number of geomobjects (meshes) found
				udword				mNbDerived;			//!< Number of derived objects found (actually number of skins)
				udword				mNbCameras;			//!< Number of cameras found
				udword				mNbLights;			//!< Number of lights found
				udword				mNbShapes;			//!< Number of shapes found
				udword				mNbHelpers;			//!< Number of helpers found
				udword				mNbControllers;		//!< Number of controllers found
				udword				mNbMaterials;		//!< Number of materials found
				udword				mNbTextures;		//!< Number of textures found
				udword				mNbUnknowns;		//!< Number of unknown nodes found
				udword				mNbInvalidNodes;	//!< Number of invalid nodes found
	};

	class ICERENDERMANAGER_API ZCBSceneInfo : public ZCBComponents, public Allocateable
	{
		public:
									ZCBSceneInfo();
		virtual						~ZCBSceneInfo();
		// Time-related info
				udword				mFirstFrame;		//!< Timeline's first frame number
				udword				mLastFrame;			//!< Timeline's last frame number
				udword				mFrameRate;			//!< Global frame rate
				udword				mDeltaTime;			//!< Ticks per frame
		// Environment
				Point				mBackColor;			//!< Background color
				Point				mAmbientColor;		//!< Global ambient color
		// Scene info
				String				mSceneInfo;			//!< The scene information string
				String				mSceneHelpText;		//!< A possible help string (describing controls for current scene, etc)
				udword				mTesselation;		//!< Global scene tesselation
				udword				mLightingMode;		//!< Scene lighting mode
		// Scene physics
				Point				mGravity;			//!< Global world gravity
				float				mRestitution;		//!< Global world restitution
				float				mStaticFriction;	//!< Global world static friction
				float				mFriction;			//!< Global world friction
				bool				mGroundPlane;		//!< Use a ground plane or not
				bool				mCollisionDetection;//!< Allow/ban collision detection
		// Global scale
				float				mGlobalScale;		//!< Global scaling used for exporting
	};

	class ICERENDERMANAGER_API ZCBBaseInfo : public Allocateable
	{
		public:
									ZCBBaseInfo();
		virtual						~ZCBBaseInfo();

		// Database information
				String				mName;				//!< Object's name
				sdword				mID;				//!< Object's ID
				sdword				mParentID;			//!< Parent's ID
				sdword				mTargetID;			//!< Target ID
				sdword				mMasterID;			//!< Master ID
				bool				mGroup;				//!< true if the object belongs to a group
				bool				mIsHidden;			//!< true if the object was hidden
		// Position/Rotation/Scale = PRS
				PRS					mPrs;
		// Pivot
				Point				mPivotPos;			//!< Global position
				Quat				mPivotRot;			//!< Global rotation
		// Rendering information
				udword				mWireColor;			//!< The wireframe color
				bool				mLocalPRS;			//!< true for local PRS
				bool				mD3DCompliant;		//!< true if converted to D3D frame
		// User properties
				String				mUserProps;			//!< The user-defined properties
		// Extra physics properties
				Point				mLinearVelocity;	//!< Initial linear velocity
				Point				mAngularVelocity;	//!< Initial angular velocity
				float				mDensity;
				float				mMass;
				udword				mSamplingDensity;
				bool				mResetPivot;
				bool				mIsCollidable;
				bool				mLockPivot;

				bool				Import(ImportArray& importer);
	};

	class ICERENDERMANAGER_API ZCBCameraInfo : public ZCBBaseInfo
	{
		public:
									ZCBCameraInfo();
		virtual						~ZCBCameraInfo();

		// Camera parameters
				ZCBCameraType		mType;					//!< Camera type
				bool				mOrthoCam;				//!< Camera type: ortographic (true) or perspective (false)
				float				mFOV;					//!< Field-Of-View (degrees) or Width for ortho cams		[Animatable]
				ZCBFOVType			mFOVType;				//!< FOV Type
				float				mNearClip;				//!< Near/hither clip										[Animatable]
				float				mFarClip;				//!< Far/yon clip											[Animatable]
				float				mTDist;					//!< Distance to target										[Animatable]
				long				mHLineDisplay;			//!< Horizon Line Display
				float				mEnvNearRange;			//!< Environment near range									[Animatable]
				float				mEnvFarRange;			//!< Environment far range									[Animatable]
				long				mEnvDisplay;			//!< Environment display
				long				mManualClip;			//!< Manual clip
	};

	// NB: mIsSpot and mIsDir are dumped from MAX, but they may not match D3D's definitions...
	class ICERENDERMANAGER_API ZCBLightInfo : public ZCBBaseInfo
	{
		public:
									ZCBLightInfo();
		virtual						~ZCBLightInfo();

		// Light parameters
				ZCBLightType		mLightType;						//!< Light's type
				bool				mIsSpot;						//!< Is the light a spotlight?
				bool				mIsDir;							//!< Is the light a directional?
				Point				mColor;							//!< Light's color											[Animatable]
				float				mIntensity;						//!< Light's intensity										[Animatable]
				float				mContrast;						//!< Light's contrast										[Animatable]
				float				mDiffuseSoft;					//!< Light's diffuse soft									[Animatable]
				bool				mLightUsed;						//!< Is the light used?
				bool				mAffectDiffuse;					//!< Does the light affect diffuse?
				bool				mAffectSpecular;				//!< Does the light affect specular?
				bool				mUseAttenNear;					//
				bool				mAttenNearDisplay;				//
				bool				mUseAtten;						//!< Is attenuation used?
				bool				mShowAtten;						//
				float				mNearAttenStart;				//!< Near atten start										[Animatable]
				float				mNearAttenEnd;					//!< Near atten end											[Animatable]
				float				mAttenStart;					//!< Atten start											[Animatable]
				float				mAttenEnd;						//!< Atten end (use that as a range for non-dir lights)		[Animatable]
				char				mDecayType;						//!< Light's decay type
				float				mHotSpot;						//!< Light's hotspot										[Animatable]
				float				mFallsize;						//!< Light's falloff										[Animatable]
				float				mAspect;						//!< Light's aspect											[Animatable]
				ZCBSpotShape		mSpotShape;						//!< Light's spot shape
				long				mOvershoot;						//!< Light's overshoot
				bool				mConeDisplay;					//
				float				mTDist;							//!< Distance to target										[Animatable]
				long				mShadowType;					//!< Light's shadow type
				long				mAbsMapBias;					//!< Light's absolute map bias
				float				mRayBias;						//!< Raytrace bias											[Animatable]
				float				mMapBias;						//!< Map bias												[Animatable]
				float				mMapRange;						//!< Map range												[Animatable]
				long				mMapSize;						//!< Map size												[Animatable]
				bool				mCastShadows;					//!< Cast shadows
				float				mShadowDensity;					//!< Shadow density											[Animatable]
				Point				mShadowColor;					//!< Shadow color											[Animatable]
				bool				mLightAffectsShadow;			//!< Light affects shadow or not
				sdword				mProjMapID;						//!< Projector texture map
				sdword				mShadowProjMapID;				//!< Shadow projector texture map
	};

	//! A possible texture matrix.
	struct ICERENDERMANAGER_API TextureMatrix : public Allocateable
	{
		TextureMatrix()
		{
			ZeroMemory(m, 3*4*sizeof(float));
			m[0][0] = m[1][1] = m[2][2] = 1.0f;
		}
		//! Warning: that's a direct dump from MAX (untransformed)
		float m[4][3];
	};

	//! Crop values. Useful for T-pages. Warning: equations to use them are weird.
	struct ICERENDERMANAGER_API TextureCrop : public Allocateable
	{
		TextureCrop() : mOffsetU(0.0f), mOffsetV(0.0f), mScaleU(1.0f), mScaleV(1.0f)	{}

		float	mOffsetU;	//!< Offset for U
		float	mOffsetV;	//!< Offset for V
		float	mScaleU;	//!< Scale for U
		float	mScaleV;	//!< Scale for V
	};

	class ICERENDERMANAGER_API ZCBMaterialInfo : public Allocateable
	{
		public:
									ZCBMaterialInfo();
		virtual						~ZCBMaterialInfo();

				bool				HasExtendedSelfIllum()	const
									{
										if(		mSelfIllumOn
											||	IR(mSelfIllumValue)
											||	IR(mSelfIllumColor.x)
											||	IR(mSelfIllumColor.y)
											||	IR(mSelfIllumColor.z)
											)
										return true;
										else return false;
									}
		// Database information
				String				mName;								//!< Material name
				sdword				mID;								//!< Material ID
		// Texture IDs
				sdword				mAmbientMapID;						//!< Ambient texture map (seems not to exist anymore in MAX 3)
				sdword				mDiffuseMapID;						//!< Diffuse texture map
				sdword				mSpecularMapID;						//!< Specular texture map
				sdword				mShininessMapID;					//!< Shininess texture map
				sdword				mShiningStrengthMapID;				//!< Shining Strength texture map
				sdword				mSelfIllumMapID;					//!< Self Illum texture map
				sdword				mOpacityMapID;						//!< Opacity texture map
				sdword				mFilterMapID;						//!< Filter texture map
				sdword				mBumpMapID;							//!< Bump texture map
				sdword				mReflexionMapID;					//!< Reflexion texture map
				sdword				mRefractionMapID;					//!< Refraction texture map
				sdword				mDisplacementMapID;					//!< Displacement texture map
		// Amounts
				float				mAmbientCoeff;						//!< Ambient texture %									[Animatable]
				float				mDiffuseCoeff;						//!< Diffuse tetxure %									[Animatable]
				float				mSpecularCoeff;						//!< Specular texture %									[Animatable]
				float				mShininessCoeff;					//!< Shininess texture %								[Animatable]
				float				mShiningStrengthCoeff;				//!< Shining Strength texture %							[Animatable]
				float				mSelfIllumCoeff;					//!< Self Illum texture %								[Animatable]
				float				mOpacityCoeff;						//!< Opacity texture %									[Animatable]
				float				mFilterCoeff;						//!< Filter texture %									[Animatable]
				float				mBumpCoeff;							//!< Bump texture %										[Animatable]
				float				mReflexionCoeff;					//!< Reflexion texture %								[Animatable]
				float				mRefractionCoeff;					//!< Refraction texture %								[Animatable]
				float				mDisplacementCoeff;					//!< Displacement texture %								[Animatable]
		// Colors
				Point				mMtlAmbientColor;					//!< Ambient Color										[Animatable]
				Point				mMtlDiffuseColor;					//!< Diffuse Color										[Animatable]
				Point				mMtlSpecularColor;					//!< Specular Color										[Animatable]
				Point				mMtlFilterColor;					//!< Filter Color										[Animatable]
		// Static properties
				ZCBShadingMode		mShading;							//!< Material Shading
				bool				mSoften;							//!< MaterialSoften
				bool				mFaceMap;							//!< MaterialFaceMap
				bool				mTwoSided;							//!< true for two-sided materials
				bool				mWire;								//!< true for wireframe mode
				bool				mWireUnits;							//!< Wire Units
				bool				mFalloffOut;						//!< MaterialFalloffOut
				ZCBTranspaType		mTransparencyType;					//!< MaterialTransparencyType
		// Dynamic properties
				float				mShininess;							//!< MaterialShininess									[Animatable]
				float				mShiningStrength;					//!< MaterialShiningStrength							[Animatable]
				float				mSelfIllum;							//!< MaterialSelfIllum									[Animatable]
				float				mOpacity;							//!< MaterialOpacity									[Animatable]
				float				mOpaFalloff;						//!< MaterialOpacityFalloff								[Animatable]
				float				mWireSize;							//!< MaterialWireSize									[Animatable]
				float				mIOR;								//!< MaterialIOR										[Animatable]
		// Dynamic properties
				float				mBounce;							//!< Bounce												[Animatable]
				float				mStaticFriction;					//!< Static Friction									[Animatable]
				float				mSlidingFriction;					//!< Sliding Friction									[Animatable]
		// Crop values & texture matrix (for diffuse map)
		// Valid if mDiffuseMapID!=INVALID_ID
				TextureCrop			mCValues_Diffuse;					//!< Cropping values
				TextureMatrix		mTMtx_Diffuse;						//!< Texture matrix
				bool				mWrapU_Diffuse;
				bool				mMirrorU_Diffuse;
				bool				mWrapV_Diffuse;
				bool				mMirrorV_Diffuse;
		// Crop values & texture matrix (for filter map)
		// Valid if mFilterMapID!=INVALID_ID
				TextureCrop			mCValues_Filter;					//!< Cropping values
				TextureMatrix		mTMtx_Filter;						//!< Texture matrix
				bool				mWrapU_Filter;
				bool				mMirrorU_Filter;
				bool				mWrapV_Filter;
				bool				mMirrorV_Filter;
		// Extended self-illum
				bool				mSelfIllumOn;						//!< true => use SelfIllumColor, else use SelfIllumValue
				float				mSelfIllumValue;					//!< SelfIllum value
				Point				mSelfIllumColor;					//!< SelfIllum color
		// Flexporter specific
				String				mShaderFile;						//!< Shader filename
				String				mUserProps;							//!< User properties
				sdword				mDecalMapID;						//!< Decal texture map
				sdword				mDetailMapID;						//!< Detail texture map
				sdword				mBillboardMapID;					//!< Billboard texture map
	};

	class ICERENDERMANAGER_API ZCBTextureInfo : public Allocateable
	{
		public:
									ZCBTextureInfo();
		virtual						~ZCBTextureInfo();

		// Database information
				String				mName;							//!< Texture name & path
				sdword				mID;							//!< Texture ID
		// Crop values & texture matrix
				TextureCrop			mCValues;						//!< Cropping values
				TextureMatrix		mTMtx;							//!< Texture matrix
				bool				mWrapU;
				bool				mMirrorU;
				bool				mWrapV;
				bool				mMirrorV;
		// Bitmap data
				udword				mWidth;							//!< Texture's width
				udword				mHeight;						//!< Texture's height
				ubyte*				mBitmap;						//!< Texture in R,G,B,A order. (always contains Alpha)
				bool				mHasAlpha;						//!< True => Alpha is valid.
				bool				mIsBitmapIncluded;				//!< True if previous fields are valid, else must be read from disk
		// External source
				bool				mExternalSourceHasAlpha;		//!< True => external source has alpha
	};

	struct ICERENDERMANAGER_API FaceProperties : public Allocateable
	{
		sdword	MatID;		//!< Material ID
		udword	Smg;		//!< Smoothing groups
		ubyte	EdgeVis;	//!< Edge visibility code
	};

	struct ICERENDERMANAGER_API BasicBone : public Allocateable
	{
		udword	CSID;		//!< Character Studio ID
		udword	pCSID;		//!< Parent's CSID
	};

	class ICERENDERMANAGER_API ZCBNativeMeshInfo
	{
		public:
									ZCBNativeMeshInfo();
		virtual						~ZCBNativeMeshInfo();

		// Useful figures
				udword				mNbFaces;					//!< Number of faces in MAX native data
				udword				mNbVerts;					//!< Number of vertices.
				udword				mNbTVerts;					//!< Number of texture-vertices (mapping coordinates)
				udword				mNbCVerts;					//!< Number of vertex-colors
		// ZCB Flags
				udword				mFlags;						//!< See ZCB flags
		// Topologies
				udword*				mFaces;						//!< List of faces
				udword*				mTFaces;					//!< List of texture-faces
				udword*				mCFaces;					//!< List of color-faces
				FaceProperties*		mFaceProperties;			//!< List of face properties
		// Geometries
				Point*				mVerts;						//!< List of vertices (null for skins, use offset vectors)
				Point*				mTVerts;					//!< List of texture-vertices
				Point*				mCVerts;					//!< List of vertex-colors
				float*				mVertexAlpha;				//!< List of mNbVerts vertex alpha
		// Skin-related information
				udword*				mBonesNb;					//!< Number of bones for each vertex, or null if one bone/vertex. (only for skins)
				udword*				mBonesID;					//!< IDs of driving bones. (only for skins)
				Point*				mOffsetVectors;				//!< Character Studio's offset vectors. (only for skins)
				float*				mWeights;					//!< Bones weights when there's more than one bone/vertex, else null. (only for skins)
		// Skeleton-related information
				udword				mNbBones;					//!< Number of bones in the skeleton below (only for skins)
				BasicBone*			mSkeleton;					//!< Skeletal information (only for skins)
		// Possible convex hull
				udword				mNbHullVerts;				//!< Number of vertices in the convex hull
				udword				mNbHullFaces;				//!< Number of faces in the convex hull
				Point*				mHullVerts;					//!< Convex hull vertices
				udword*				mHullFaces;					//!< Convex hull faces
		// Possible bounding sphere
				Point				mBSCenter;					//!< Bounding sphere center
				float				mBSRadius;					//!< Bounding sphere radius
		// Possible volume integrals
				Point				mCOM;						//!< Center Of Mass
				float				mComputedMass;				//!< Total mass
				float				mInertiaTensor[3][3];		//!< Inertia tensor (mass matrix) relative to the origin
				float				mCOMInertiaTensor[3][3];	//!< Inertia tensor (mass matrix) relative to the COM
		// Winding order
				bool				mParity;					//!< Faces are CW or CCW.
	};

	class ICERENDERMANAGER_API ZCBPrimitiveParams : public Allocateable
	{
		public:
									ZCBPrimitiveParams() : mType(ZCB_PRIM_UNDEFINED)	{}
		virtual						~ZCBPrimitiveParams()								{}

				ZCBPrimitiveType	mType;
	};

	class ICERENDERMANAGER_API ZCBBoxParams : public ZCBPrimitiveParams
	{
		public:
									ZCBBoxParams()	{ mType = ZCB_PRIM_BOX;	}
		virtual						~ZCBBoxParams()	{}

				float				mLength;
				float				mWidth;
				float				mHeight;
				int					mWSegs;
				int					mLSegs;
				int					mHSegs;
				BOOL				mGenUVS;
	};

	class ICERENDERMANAGER_API ZCBSphereParams : public ZCBPrimitiveParams
	{
		public:
									ZCBSphereParams()	{ mType = ZCB_PRIM_SPHERE;	}
		virtual						~ZCBSphereParams()	{}

				float				mRadius;
				int					mSegments;
				BOOL				mSmooth;
				float				mHemisphere;
				BOOL				mSquash;	// else Chop
				float				mSliceFrom;
				float				mSliceTo;
				BOOL				mSliceOn;
				BOOL				mRecenter;	// Base to pivot
				BOOL				mGenUVS;
	};

	class ICERENDERMANAGER_API ZCBGeosphereParams : public ZCBPrimitiveParams
	{
		public:
									ZCBGeosphereParams()	{ mType = ZCB_PRIM_GEOSPHERE;	}
		virtual						~ZCBGeosphereParams()	{}

				float				mRadius;
				int					mSegments;
				int					mGenType;	// Between 0 & 2
				BOOL				mHemisphere;
				BOOL				mSmooth;
				BOOL				mRecenter;
				BOOL				mGenUVS;
	};

	class ICERENDERMANAGER_API ZCBCylinderParams : public ZCBPrimitiveParams
	{
		public:
									ZCBCylinderParams()		{ mType = ZCB_PRIM_CYLINDER;	}
		virtual						~ZCBCylinderParams()	{}

				float				mRadius;
				float				mHeight;
				int					mHSegs;
				int					mCapSegs;
				int					mSides;
				BOOL				mSmooth;
				BOOL				mSliceOn;
				float				mSliceFrom;
				float				mSliceTo;
				BOOL				mGenUVS;
	};

	class ICERENDERMANAGER_API ZCBConeParams : public ZCBPrimitiveParams
	{
		public:
									ZCBConeParams()		{ mType = ZCB_PRIM_CONE;	}
		virtual						~ZCBConeParams()	{}

				float				mRadius1;
				float				mRadius2;
				float				mHeight;
				int					mHSegs;
				int					mCapSegs;
				int					mSides;
				BOOL				mSmooth;
				BOOL				mSliceOn;
				float				mSliceFrom;
				float				mSliceTo;
				BOOL				mGenUVS;
	};

	class ICERENDERMANAGER_API ZCBTorusParams : public ZCBPrimitiveParams
	{
		public:
									ZCBTorusParams()	{ mType = ZCB_PRIM_TORUS;	}
		virtual						~ZCBTorusParams()	{}

				float				mRadius1;
				float				mRadius2;
				float				mRotation;
				float				mTwist;
				int					mSegments;
				int					mSides;
				BOOL				mSmooth;
				BOOL				mSliceOn;
				float				mSliceFrom;
				float				mSliceTo;
				BOOL				mGenUVS;
	};

	class ICERENDERMANAGER_API ZCBTubeParams : public ZCBPrimitiveParams
	{
		public:
									ZCBTubeParams()		{ mType = ZCB_PRIM_TUBE;	}
		virtual						~ZCBTubeParams()	{}

				float				mRadius1;
				float				mRadius2;
				float				mHeight;
				int					mSegments;
				int					mCapSegs;
				int					mSides;
				BOOL				mSmooth;
				BOOL				mSliceOn;
				float				mSliceFrom;
				float				mSliceTo;
				BOOL				mGenUVS;
	};

	class ICERENDERMANAGER_API ZCBTeapotParams : public ZCBPrimitiveParams
	{
		public:
									ZCBTeapotParams()	{ mType = ZCB_PRIM_TEAPOT;	}
		virtual						~ZCBTeapotParams()	{}
		// Not implemented
	};

	class ICERENDERMANAGER_API ZCBPlaneParams : public ZCBPrimitiveParams
	{
		public:
									ZCBPlaneParams()	{ mType = ZCB_PRIM_PLANE;	}
		virtual						~ZCBPlaneParams()	{}

				float				mLength;
				float				mWidth;
				int					mWidthSegs;
				int					mLengthSegs;
				float				mDensity;
				float				mScale;
				BOOL				mGenUVS;
	};

	class ICERENDERMANAGER_API ZCBHedraParams : public ZCBPrimitiveParams
	{
		public:
									ZCBHedraParams()	{ mType = ZCB_PRIM_HEDRA;	}
		virtual						~ZCBHedraParams()	{}

				float				mRadius;
				int					mFamily;
				float				mP;
				float				mQ;
				float				mScaleP;
				float				mScaleQ;
				float				mScaleR;
				int					mVertices;
				BOOL				mGenUVS;
	};

	class ICERENDERMANAGER_API ZCBCapsuleParams : public ZCBPrimitiveParams
	{
		public:
									ZCBCapsuleParams()	{ mType = ZCB_PRIM_CAPSULE;	}
		virtual						~ZCBCapsuleParams()	{}

				float				mRadius;
				float				mHeight;
				int					mCenters;
				int					mSides;
				int					mHSegs;
				BOOL				mSmooth;
				BOOL				mSliceOn;
				float				mSliceFrom;
				float				mSliceTo;
				BOOL				mGenUVS;
	};

	class ICERENDERMANAGER_API ZCBMeshInfo : public ZCBBaseInfo, public ZCBNativeMeshInfo
	{
		public:
									ZCBMeshInfo();
		virtual						~ZCBMeshInfo();

		// Parameters
				bool				mIsCollapsed;		//!< true => mesh comes from a collapsed modifier stack
				bool				mIsSkeleton;		//!< true => mesh is a BIPED part (i.e. a bone)
				bool				mIsInstance;		//!< true => mesh is an instance from another mesh
				bool				mIsTarget;			//!< true => mesh is a target node (camera, spot or directional target)
				bool				mIsConvertible;		//!< false => mesh can't be converted to triangles (and the native format is unsupported)
				bool				mIsSkin;			//!< true => the mesh is a PHYSIQUE skin
				bool				mCastShadows;		//!< true => a shadow volume can be built from the mesh
				bool				mReceiveShadows;	//!< true => mesh can receive shadows
				bool				mMotionBlur;		//!< true => mesh should be motion blured

		// Biped-related information (valid if mIsSkeleton==true)
				udword				mCharID;			//!< Owner character's ID
				udword				mCSID;				//!< CSID code.
		// Mesh data
				MBResult*			mCleanMesh;			//!< Mesh after consolidation
		// Lightmapper data
				udword				mNbColors;			//!< Number of computed colors
				Point*				mColors;			//!< Computed colors
		// Primitive data
				ZCBPrimitiveParams*	mPrimParams;
	};

	class ICERENDERMANAGER_API ZCBShapeInfo : public ZCBBaseInfo
	{
		public:
									ZCBShapeInfo();
		virtual						~ZCBShapeInfo();

				ZCBShapeType		mType;					//!< Type of shape
		// Shape parameters
				udword				mNbLines;				//!< Number of polylines
				udword*				mNbVerts;				//!< Number of vertices for each polyline
				bool*				mClosed;				//!< Closed/open polylines
				Point*				mVerts;					//!< List of vertices
				udword				mTotalNbVerts;			//!< Total number of vertices in the list (=sum of all mNbVerts)
				sdword				mMatID;					//!< Shape's material ID
	};

	class ICERENDERMANAGER_API ZCBHelperInfo : public ZCBBaseInfo
	{
		public:
									ZCBHelperInfo();
		virtual						~ZCBHelperInfo();

		// Helper parameters
				ZCBHelperType		mHelperType;		//!< Type of helper
				bool				mIsGroupHead;		//!< True for group heads
				float				mLength;			//!< For BoxGizmo
				float				mWidth;				//!< For BoxGizmo
				float				mHeight;			//!< For BoxGizmo/CylinderGizmo
				float				mRadius;			//!< For CylinderGizmo/SphereGizmo
				bool				mHemi;				//!< For SphereGizmo
				Container			mGroupedObjects;	//!< IDs of grouped objects [not available in old files, so use mIsGroupHead instead]
	};

	class ICERENDERMANAGER_API ZCBControllerInfo : public Allocateable
	{
		public:
									ZCBControllerInfo();
		virtual						~ZCBControllerInfo();

				String				mField;					//!< Controlled field
				sdword				mObjectID;				//!< Controller's ID
				sdword				mOwnerID;				//!< Owner object's ID
				ZCBObjType			mOwnerType;				//!< Owner object's type

				ZCBCtrlType			mCtrlType;				//!< Controlled type (float, quat, etc)
				ZCBCtrlMode			mCtrlMode;				//!< Controller mode (sampling, keyframes, etc)

		// Sampling-related values
				udword				mNbSamples;				//!< Number of samples
				udword				mSamplingRate;			//!< Sampling rate
				void*				mSamples;				//!< Sampled data
				udword				mNbVertices;			//!< Number of vertices for morph controllers
	};

	class ICERENDERMANAGER_API ZCBMotionInfo : public Allocateable
	{
		public:
									ZCBMotionInfo()		{}
		virtual						~ZCBMotionInfo()	{}

				String				mName;				//!< Motion's name
				String				mType;				//!< Motion's type
				udword				mCharID;			//!< Owner's character ID
				udword				mNbBones;			//!< Number of bones involved
				udword				mNbVBones;			//!< Number of virtual bones involved
				const void*			mData;				//!< Motion data
				bool				mLocalPRS;			//!< True for relative PRS
				bool				mD3DCompliant;		//!< True if converted to D3D frame
	};

	class ICERENDERMANAGER_API ZCBBreaker : public ZCBComponents
	{
		public:
									ZCBBreaker();
		virtual						~ZCBBreaker();

		// Import method
		virtual	bool				Import(const char* filename);
				bool				Import(const VirtualFile& file);

		// Application-dependent import methods, called by the importer
		virtual	bool				NewScene				(const ZCBSceneInfo& scene)				= 0;
		virtual	bool				NewCamera				(const ZCBCameraInfo& camera)			= 0;
		virtual	bool				NewLight				(const ZCBLightInfo& light)				= 0;
		virtual	bool				NewMaterial				(const ZCBMaterialInfo& material)		= 0;
		virtual	bool				NewTexture				(const ZCBTextureInfo& texture)			= 0;
		virtual	bool				NewMesh					(const ZCBMeshInfo& mesh)				= 0;
		virtual	bool				NewShape				(const ZCBShapeInfo& shape)				= 0;
		virtual	bool				NewHelper				(const ZCBHelperInfo& helper)			= 0;
		virtual	bool				NewController			(const ZCBControllerInfo& controller)	= 0;
		virtual	bool				NewMotion				(const ZCBMotionInfo& motion)			= 0;

		// Application-dependent error handler
		virtual	bool				ZCBImportError			(const char* error_text, udword error_code)	= 0;

		// Application-dependent log method
		virtual	void				ZCBLog					(LPSTR fmt, ...)							= 0;

		// Free used memory
				void				ReleaseRam();
		protected:
				ZCBFile				mFileType;
				ImportArray*		mImportArray;
		private:
				udword				mZCBGlobalVersion;
				udword				mMAINVersion;
				udword				mMOVEVersion;
				udword				mCAMSVersion;
				udword				mLITEVersion;
				udword				mMATLVersion;
				udword				mTEXMVersion;
				udword				mMESHVersion;
				udword				mCTRLVersion;
				udword				mSHAPVersion;
				udword				mHELPVersion;

				udword				mMeshOffset;
				udword				mCameraOffset;
				udword				mLightOffset;
				udword				mShapeOffset;
				udword				mHelperOffset;
				udword				mTexmapOffset;
				udword				mMaterialOffset;
				udword				mControllerOffset;
				udword				mMotionOffset;

				bool				Import();

				bool				ImportCameras			(ImportArray& importer);
				bool				ImportLights			(ImportArray& importer);
				bool				ImportMaterials			(ImportArray& importer);
				bool				ImportTextures			(ImportArray& importer);
				bool				ImportMeshes			(ImportArray& importer);
				bool				ImportShapes			(ImportArray& importer);
				bool				ImportHelpers			(ImportArray& importer);
				bool				ImportControllers		(ImportArray& importer);
				bool				ImportMotion			(ImportArray& importer);

				bool				ImportVertices			(ZCBMeshInfo& curmesh, ImportArray& importer);
				bool				ImportSkinData			(ZCBMeshInfo& curmesh, ImportArray& importer);
				bool				ImportTextureVertices	(ZCBMeshInfo& curmesh, ImportArray& importer);
				bool				ImportVertexColors		(ZCBMeshInfo& curmesh, ImportArray& importer);
				bool				ImportFaces				(ZCBMeshInfo& curmesh, ImportArray& importer);
				bool				ImportExtraStuff		(ZCBMeshInfo& curmesh, ImportArray& importer);
				bool				ImportConsolidated		(ZCBMeshInfo& curmesh, ImportArray& importer);
				bool				ImportLightingData		(ZCBMeshInfo& curmesh, ImportArray& importer);

				bool				ImportCroppingValues	(TextureCrop& cvalues, TextureMatrix& tmtx, ImportArray& importer, bool& wrap_u, bool& mirror_u, bool& wrap_v, bool& mirror_v);
	};

/*	class ICERENDERMANAGER_API ZCBConverter : public Allocateable
	{
		public:
									ZCBConverter();
									~ZCBConverter();

				bool				Convert(const char* filename);
	};
*/

#endif // ICEZCBBREAKER_H

