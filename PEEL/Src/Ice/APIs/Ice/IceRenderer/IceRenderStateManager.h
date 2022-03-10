///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains the render state manager.
 *	\file		IceRenderStateManager.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERENDERSTATEMANAGER_H
#define ICERENDERSTATEMANAGER_H

	// Compilation flags
//	#define	RS_SUPPORT_RUNTIME_CHECKINGS		//!< Check caps at runtime for all render state calls

	// Boolean-related macros
	// Despite we get them as "BOOL", we need to test the caches as "bool".... maybe there's a faster way...
#ifdef _DEBUG
		#define	BRS_CHECK(flag, mask)		mNbBRSCalls++;																		\
											const bool Current = (mCacheFlags&mask)!=0;	if(Current==(flag!=0))	return FALSE;	\
											if(flag) mCacheFlags|=mask;	else mCacheFlags&=~mask;								\
											mNbBRSDone++;
#else
		#define	BRS_CHECK(flag, mask)		const bool Current = (mCacheFlags&mask)!=0;	if(Current==(flag!=0))	return FALSE;	\
											if(flag) mCacheFlags|=mask;	else mCacheFlags&=~mask;
#endif

	// Dword-related macros
#ifdef _DEBUG
		#define	DRS_CHECK(dword, mask)		mNbDRSCalls++;																		\
											if(mCacheDwords[mask]==dword)			return FALSE;	mCacheDwords[mask]=dword;	\
											mNbDRSDone++;
		#define	TS_CHECK(dword, member)		mNbTSCalls++;																		\
											if(IR(mTexCaches[i].member)==IR(dword))	return FALSE;	mTexCaches[i].member=dword;	\
											mNbTSDone++;
#else
		#define	DRS_CHECK(dword, mask)		if(mCacheDwords[mask]==dword)			return FALSE;	mCacheDwords[mask]=dword;
		#define	TS_CHECK(dword, member)		if(IR(mTexCaches[i].member)==IR(dword))	return FALSE;	mTexCaches[i].member=dword;
#endif
		#define	DRS_INVALIDATE(mask)		mCacheDwords[mask]=0x7fffffff;

	// Priority levels macros
	#ifdef RS_PRIORITYLEVELS
		#define	PL_CHECK(level, mask)		if(level<=mPriorityLevels[mask])	return FALSE;
		#define	TPL_CHECK(level, member)	if(level<=mPLTexture[i].member)		return FALSE;
	#else
		#define	PL_CHECK(level, mask)
		#define	TPL_CHECK(level, member)
	#endif

	#ifdef RS_SUPPORT_RUNTIME_CHECKINGS
//		#define RS_PREVENT(cndt, text)		if(mCheckings && (cndt))	SetIceError(text, gEC_InvalidRS, mUpdateLogFile);
//		#define RS_PREVENT(cndt, text)		if(mCheckings && (cndt))	{ SetIceError(text, 0, mUpdateLogFile); return FALSE; }
		#define RS_PREVENT(cndt, text)		if(mCheckings && (cndt))	return FALSE;
	#else
		#define RS_PREVENT(cndt, text)
	#endif

	// Forward declarations
	class RenderStateManager;
	class StateBlock;

#ifdef RS_PRIORITYLEVELS
	typedef BOOL (RenderStateManager::*TextureRSCall)	(udword stage, udword value, PriorityLevel plevel=128);
	typedef	BOOL (RenderStateManager::*DwordRSCall)		(udword value, PriorityLevel plevel=128);
	typedef	BOOL (RenderStateManager::*BoolRSCall)		(BOOL flag, PriorityLevel plevel=128);
#else
	typedef BOOL (RenderStateManager::*TextureRSCall)	(udword stage, udword value);
	typedef	BOOL (RenderStateManager::*DwordRSCall)		(udword value);
	typedef	BOOL (RenderStateManager::*BoolRSCall)		(BOOL flag);
#endif

	//! Render state base class
	class ICERENDERER_API RenderStateManager : public RenderBase
	{
		public:
										RenderStateManager();
		virtual							~RenderStateManager();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Management methods
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			bool			InitDefaultRenderStates(const RenderCaps& caps, bool reset_all);	// Initialize the default render states
		virtual			bool			ValidateDevice(udword& nb_passes)							= 0;	// Validates the texture cascade.
		inline_			void			SetCheckings(bool flag)				{ mCheckings			= flag;	}
		inline_			void			SetUpdateLog(bool flag)				{ mUpdateLogFile		= flag;	}
		inline_			void			SetStateBlockEmulation(bool flag)	{ mStateBlockEmulation	= flag;	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Transformations
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			BOOL			SetWorldMatrix(const Matrix4x4& world)						= 0;
		virtual			void			GetWorldMatrix(Matrix4x4& world)							= 0;
						void			SetWorldIdentity();
						void			SetWorldMultiplier(const Matrix4x4& mul);
		inline_	const	Matrix4x4*		GetWorldMultiplier()	const	{ return mWorldTransform;	}
		inline_	const	Matrix4x4*		GetLastTransform()		const	{ return mLastTransform;	}
		inline_			void			ResetLastTransform()			{ mLastTransform = null;	}
		inline_			void			SetWorldMatrix_CACHED(const Matrix4x4& world)	{ if(&world!=mLastTransform)	SetWorldMatrix(world);	}

		virtual			BOOL			SetBoneMatrix(udword i, const Matrix4x4& bone)				= 0;
		virtual			void			GetBoneMatrix(udword i, Matrix4x4& bone)					= 0;

		virtual			BOOL			SetViewMatrix(const Matrix4x4& view)						= 0;
		virtual			void			GetViewMatrix(Matrix4x4& view)								= 0;

		virtual			BOOL			SetProjMatrix(const Matrix4x4& proj)						= 0;
		virtual			void			GetProjMatrix(Matrix4x4& proj)								= 0;

		virtual			BOOL			SetTextureMatrix(udword i, const Matrix4x4& texture)		= 0;
		virtual			void			GetTextureMatrix(udword i, Matrix4x4& texture)				= 0;
						void			SetTextureIdentity(udword i);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Caches management
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						void			SetCacheFlags(udword flags);
						void			SetCacheDwords(udword* dwords);
						void			SetCacheTextures(TextureStage* textures);

		inline_			udword			GetCacheFlags				()	const	{ return mCacheFlags;									}
		inline_			udword*			GetCacheDwords				()			{ return mCacheDwords;									}
		inline_			TextureStage*	GetCacheTextures			()			{ return mTexCaches;									}

		inline_			BOOL			GetTexturePerspective		()	const	{ return mCacheFlags&BRS_TEXTUREPERSPECTIVE;			}
		inline_			BOOL			GetZWrite					()	const	{ return mCacheFlags&BRS_ZWRITE;						}
		inline_			BOOL			GetAlphaTest				()	const	{ return mCacheFlags&BRS_ALPHATEST;						}
		inline_			BOOL			GetDithering				()	const	{ return mCacheFlags&BRS_DITHERING;						}
		inline_			BOOL			GetAlphaBlend				()	const	{ return mCacheFlags&BRS_ALPHABLENDING;					}
		inline_			BOOL			GetFog						()	const	{ return mCacheFlags&BRS_FOG;							}
		inline_			BOOL			GetSpecular					()	const	{ return mCacheFlags&BRS_SPECULAR;						}
		inline_			BOOL			GetStencilTest				()	const	{ return mCacheFlags&BRS_STENCILTEST;					}
		inline_			BOOL			GetLighting					()	const	{ return mCacheFlags&BRS_LIGHTING;						}
		inline_			BOOL			GetTexturing				()	const	{ return mCacheFlags&BRS_TEXTURING;						}
		inline_			BOOL			GetLastPixel				()	const	{ return mCacheFlags&BRS_LASTPIXEL;						}
		inline_			BOOL			GetStippledAlpha			()	const	{ return mCacheFlags&BRS_STIPPLEDALPHA;					}
		inline_			BOOL			GetEdgeAntialias			()	const	{ return mCacheFlags&BRS_EDGEANTIALIAS;					}
		inline_			BOOL			GetColorKeyMode				()	const	{ return mCacheFlags&BRS_COLORKEYMODE;					}
		inline_			BOOL			GetColorKeyBlendMode		()	const	{ return mCacheFlags&BRS_COLORKEYBLENDMODE;				}
		inline_			BOOL			GetRangeBasedFog			()	const	{ return mCacheFlags&BRS_RANGEBASEDFOG;					}
		inline_			BOOL			GetClipping					()	const	{ return mCacheFlags&BRS_CLIPPING;						}
		inline_			BOOL			GetScreenExtents			()	const	{ return mCacheFlags&BRS_SCREENEXTENTS;					}
		inline_			BOOL			GetColorVertex				()	const	{ return mCacheFlags&BRS_COLORVERTEX;					}
		inline_			BOOL			GetPerspSpecHigh			()	const	{ return mCacheFlags&BRS_PERSPSPECHIGH;					}
		inline_			BOOL			GetNormalize				()	const	{ return mCacheFlags&BRS_NORMALIZE;						}

		inline_			BOOL			GetSoftwareVertexProcessing	()	const	{ return mCacheFlags&BRS_SOFTWAREVERTEXPROCESSING;		}
		inline_			BOOL			GetPointSprite				()	const	{ return mCacheFlags&BRS_POINTSPRITE;					}
		inline_			BOOL			GetPointScale				()	const	{ return mCacheFlags&BRS_POINTSCALE;					}
		inline_			BOOL			GetMultiSampleAntialias		()	const	{ return mCacheFlags&BRS_MULTISAMPLEANTIALIAS;			}
		inline_			BOOL			GetIndexedVertexBlend		()	const	{ return mCacheFlags&BRS_INDEXEDVERTEXBLEND;			}

		inline_			udword			GetAmbientColor				()	const	{ return mCacheDwords[DRS_AMBIENTCOLOR];				}
		inline_			udword			GetAntialiasMode			()	const	{ return mCacheDwords[DRS_ANTIALIASMODE];				}
		inline_			udword			GetZBufferMode				()	const	{ return mCacheDwords[DRS_ZBUFFERMODE];					}
		inline_			udword			GetZBufferCmpMode			()	const	{ return mCacheDwords[DRS_ZBUFFERCMPMODE];				}
		inline_			udword			GetZBias					()	const	{ return mCacheDwords[DRS_ZBIAS];						}
		inline_			udword			GetLinePattern				()	const	{ return mCacheDwords[DRS_LINEPATTERN];					}
		inline_			udword			GetAlphaRef					()	const	{ return mCacheDwords[DRS_ALPHAREF];					}
		inline_			udword			GetAlphaFunc				()	const	{ return mCacheDwords[DRS_ALPHAFUNC];					}
		inline_			udword			GetSrcBlend					()	const	{ return mCacheDwords[DRS_SRCBLEND];					}
		inline_			udword			GetDstBlend					()	const	{ return mCacheDwords[DRS_DESTBLEND];					}
		inline_			udword			GetDiffuseMaterialSource	()	const	{ return mCacheDwords[DRS_DIFFUSEMATSRC];				}
		inline_			udword			GetSpecularMaterialSource	()	const	{ return mCacheDwords[DRS_SPECULARMATSRC];				}
		inline_			udword			GetAmbientMaterialSource	()	const	{ return mCacheDwords[DRS_AMBIENTMATSRC];				}
		inline_			udword			GetEmissiveMaterialSource	()	const	{ return mCacheDwords[DRS_EMISSIVEMATSRC];				}
		inline_			udword			GetVertexBlend				()	const	{ return mCacheDwords[DRS_VERTEXBLEND];					}
		inline_			udword			GetTextureFactor			()	const	{ return mCacheDwords[DRS_TEXTUREFACTOR];				}
		inline_			udword			GetFillMode					()	const	{ return mCacheDwords[DRS_FILLMODE];					}
		inline_			udword			GetShadeMode				()	const	{ return mCacheDwords[DRS_SHADEMODE];					}
		inline_			udword			GetCullMode					()	const	{ return mCacheDwords[DRS_CULLMODE];					}
		inline_			udword			GetStencilRef				()	const	{ return mCacheDwords[DRS_STENCILREF];					}
		inline_			udword			GetStencilCmpMode			()	const	{ return mCacheDwords[DRS_STENCILCMPMODE];				}
		inline_			udword			GetStencilCmpMask			()	const	{ return mCacheDwords[DRS_STENCILCMPMASK];				}
		inline_			udword			GetStencilWriteMask			()	const	{ return mCacheDwords[DRS_STENCILWRITEMASK];			}
		inline_			udword			GetStencilFailOp			()	const	{ return mCacheDwords[DRS_STENCILFAILOP];				}
		inline_			udword			GetStencilZFailOp			()	const	{ return mCacheDwords[DRS_STENCILZFAILOP];				}
		inline_			udword			GetStencilPassOp			()	const	{ return mCacheDwords[DRS_STENCILPASSOP];				}
		inline_			udword			GetFogVertexMode			()	const	{ return mCacheDwords[DRS_FOGVERTEXMODE];				}
		inline_			udword			GetFogMode					()	const	{ return mCacheDwords[DRS_FOGMODE];						}
		inline_			udword			GetFogDensity				()	const	{ return mCacheDwords[DRS_FOGDENSITY];					}
		inline_			udword			GetFogColor					()	const	{ return mCacheDwords[DRS_FOGCOLOR];					}
		inline_			udword			GetFogStart					()	const	{ return mCacheDwords[DRS_FOGSTART];					}
		inline_			udword			GetFogEnd					()	const	{ return mCacheDwords[DRS_FOGEND];						}
		inline_			udword			GetMaterialProps			()	const	{ return mCacheDwords[DRS_MATERIAL];					}

		inline_			udword			GetPointSize				()	const	{ return mCacheDwords[DRS_POINTSIZE];					}
		inline_			udword			GetPointSizeMin				()	const	{ return mCacheDwords[DRS_POINTSIZEMIN];				}
		inline_			udword			GetPointScaleA				()	const	{ return mCacheDwords[DRS_POINTSCALEA];					}
		inline_			udword			GetPointScaleB				()	const	{ return mCacheDwords[DRS_POINTSCALEB];					}
		inline_			udword			GetPointScaleC				()	const	{ return mCacheDwords[DRS_POINTSCALEC];					}
		inline_			udword			GetMultiSampleMask			()	const	{ return mCacheDwords[DRS_MULTISAMPLEMASK];				}
		inline_			udword			GetPatchEdgeStyle			()	const	{ return mCacheDwords[DRS_PATCHEDGESTYLE];				}
		inline_			udword			GetPatchSegments			()	const	{ return mCacheDwords[DRS_PATCHSEGMENTS];				}
		inline_			udword			GetPointSizeMax				()	const	{ return mCacheDwords[DRS_POINTSIZEMAX];				}
		inline_			udword			GetColorWriteEnable			()	const	{ return mCacheDwords[DRS_COLORWRITEENABLE];			}
		inline_			udword			GetTweenFactor				()	const	{ return mCacheDwords[DRS_TWEENFACTOR];					}
		inline_			udword			GetBlendOp					()	const	{ return mCacheDwords[DRS_BLENDOP];						}

		inline_			udword			GetVertexShader				()	const	{ return mCacheDwords[DRS_VERTEXSHADER];				}
		inline_			udword			GetPixelShader				()	const	{ return mCacheDwords[DRS_PIXELSHADER];					}

/*
		TEXTUREADDRESS		GetTextureAddressU(udword i)			{ return mTextAddrU[i];			}
		TEXTUREADDRESS		GetTextureAddressV(udword i)			{ return mTextAddrV[i];			}
		TEXTUREMINFILTER	GetTextureMinFilter(udword i)			{ return mTextMinFilter[i];		}
		TEXTUREMAGFILTER	GetTextureMagFilter(udword i)			{ return mTextMagFilter[i];		}
		TEXTUREMIPFILTER	GetTextureMipFilter(udword i)			{ return mTextMipFilter[i];		}

		protected:
*/
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Helper methods
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			void			SetDepthRange				(float zmin=0.0f, float zmax=1.0f)		= 0;	// Z mappings
		virtual			void			SetColorUpdate				(BOOL flag)								= 0;

#ifdef RS_PRIORITYLEVELS
						void			SetAlphaBlend(ALPHABLEND srcmode, ALPHABLEND dstmode, PriorityLevel plevel=128);
						void			SetStencilOp(STENCILOP fail, STENCILOP zfail, STENCILOP pass, PriorityLevel plevel=128);
						void			SetMaterial(const Material* mtl=null, PriorityLevel plevel=128);
						BOOL			SetTexturing(BOOL flag=TRUE, PriorityLevel plevel=128);	// Enable/disable texturing
#else
						void			SetAlphaBlend(ALPHABLEND srcmode, ALPHABLEND dstmode);
						void			SetStencilOp(STENCILOP fail, STENCILOP zfail, STENCILOP pass);
						void			SetMaterial(const Material* mtl=null);
						BOOL			SetTexturing(BOOL flag=TRUE);	// Enable/disable texturing
#endif

		inline_			void			ResetTextureCascade()
										{
											//### could use a state block
											for(udword i=0;i<mCaps.mMaxTextureBlendStages;i++)
											{
												SetTextureBitmap(i);
											}
											SetTextureColorOp(0, STAGEOP_DISABLE);
											SetTextureAlphaOp(0, STAGEOP_DISABLE);
										}

		// LLM : Low Level Method
		// HLM : High Level Method

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// User-defined clipping planes
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			udword			MaxUserClippingPlanes()					= 0;	// Get max #clipping planes supported by the driver
		virtual			udword			MaxHardwareUserClippingPlanes()			= 0;	// Get max #clipping planes supported by the hardware
		virtual			void			EnableClipPlane(udword mask=0)			= 0;	// LLM: enable one or many clip planes thanks to a bitmask
		virtual			void			SetClipPlane(udword i, float* equation)	= 0;	// LLM: set the ith clip plane equation
		virtual			void			GetClipPlane(udword i, float* equation)	= 0;	// LLM: get the ith clip plane equation
		virtual			udword			AddClipPlane(const Plane& clipplane)	= 0;	// HLM: add a user-defined clip plane
		virtual			bool			RemoveClipPlane(udword planeid)			= 0;	// HLM: remove a user-defined clip plane
						bool			AllocateClipPlanes();
		protected:
		virtual			void			ValidateClipPlanes()					= 0;	// HLM: validates all active clip planes
		public:
		// Clip-plane emulation using texture masking
						bool			EnableClipTexture(udword stage, const Plane& clip_plane);
						bool			DisableClipTexture();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Material manager
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						Material*		CreateMaterial(const MATERIALCREATE& create, bool sharing=true);
						bool			ReleaseMaterial(Material* mtl);
						bool			ReleaseMaterials();
		inline_			void			InvalidateMaterialCache()		{ mCacheDwords[DRS_MATERIAL]=0; }

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// State block manager
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Creates a new state block.
		 *	\return		the new state block, or null if failed
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						StateBlock*		CreateStateBlock();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Registers a state block.
		 *	The render states are saved and restored, i.e. not actually modified by the registration process.
		 *	\param		sb			[in] the state block to register
		 *	\param		sharing		[in] true to first search the database for a similar state block
		 *	\return		the state block management ID
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						SBID			Register(StateBlock* sb, bool sharing=true);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Applies a recorded state block.
		 *	\param		id		[in] state block ID
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool			ApplyStateBlock(SBID id);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Releases a state block.
		 *	\param		id		[in] state block id
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool			ReleaseStateBlock(SBID& id);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Releases all state blocks.
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool			ReleaseStateBlocks();
		private:
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	State block emulation method.
		 *	\param		sb		[in] the state block to execute
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool			ExecuteStateBlock(StateBlock* sb);
		// To override
		virtual			bool			BeginStateBlock()						{ return false;	}
		virtual			bool			EndStateBlock(udword& id)				{ return false;	}
		virtual			bool			ApplyStateBlock(udword blockid)			{ return false;	}
		virtual			bool			DeleteStateBlock(udword blockid)		{ return false;	}

		public:
#ifdef RS_PRIORITYLEVELS
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Priority levels
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						ubyte			SetPriorityLevel(udword mask, ubyte level)
										{
											ubyte CurrentLevel = mPriorityLevels[mask];
											if(level==255)	return CurrentLevel;
											mPriorityLevels[mask] = level;
											return CurrentLevel;
										}
#endif

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// General Render States
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		inline_			void			SetFrontFaces(FRONTFACE front_face)	{	mFrontFace = front_face;	}
		inline_			FRONTFACE		GetFrontFaces()			const		{	return mFrontFace;			}
		inline			void			FlipFrontFaces()
										{
													if(mFrontFace==FRONTFACE_CW)	mFrontFace = FRONTFACE_CCW;
											else	if(mFrontFace==FRONTFACE_CCW)	mFrontFace = FRONTFACE_CW;
										}
		// Generic render state calls
#ifdef RS_PRIORITYLEVELS
		inline_			BOOL			SetRenderStateBool		(BooleanRenderState id, BOOL flag, PriorityLevel plevel=128)					{ return (this->*mBRSTable[GetBRSIndex(id)])(flag, plevel);	}
		inline_			BOOL			SetRenderStateDword		(DwordRenderState id, udword state, PriorityLevel plevel=128)					{ return (this->*mDRSTable[id])(state, plevel);				}
		inline_			BOOL			SetRenderStateTexture	(TextureRenderState id, udword stage, udword state, PriorityLevel plevel=128)	{ return (this->*mTSTable[id])(stage, state, plevel);		}
#else
		inline_			BOOL			SetRenderStateBool		(BooleanRenderState id, BOOL flag)					{ return (this->*mBRSTable[GetBRSIndex(id)])(flag);	}
		inline_			BOOL			SetRenderStateDword		(DwordRenderState id, udword state)					{ return (this->*mDRSTable[id])(state);				}
		inline_			BOOL			SetRenderStateTexture	(TextureRenderState id, udword stage, udword state)	{ return (this->*mTSTable[id])(stage, state);		}
#endif

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Flag-controlled render states
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef RS_PRIORITYLEVELS
	#define DECLARE_BRS(name, def)	virtual	inline_	BOOL	Set##name(BOOL flag=def, PriorityLevel plevel=128)
#else
	#define DECLARE_BRS(name, def)	virtual	inline_	BOOL	Set##name(BOOL flag=def)
#endif

		DECLARE_BRS(TexturePerspective, TRUE)
		{
			RS_PREVENT(!mCaps.mTextureCaps.mPerspective, "Perspective correction not supported");
			PL_CHECK(plevel, RS_TEXTUREPERSPECTIVE);	BRS_CHECK(flag, BRS_TEXTUREPERSPECTIVE);	return TRUE;
		}

		DECLARE_BRS(ZWrite, TRUE)
		{
			PL_CHECK(plevel, RS_ZWRITE);				BRS_CHECK(flag, BRS_ZWRITE);				return TRUE;
		}

		DECLARE_BRS(AlphaTest, FALSE)
		{
			PL_CHECK(plevel, RS_ALPHATEST);				BRS_CHECK(flag, BRS_ALPHATEST);				return TRUE;
		}

		DECLARE_BRS(Dithering, TRUE)
		{
			RS_PREVENT(!mCaps.mRasterCaps.mDither, "Dithering not supported");
			PL_CHECK(plevel, RS_DITHERING);				BRS_CHECK(flag, BRS_DITHERING);				return TRUE;
		}

		DECLARE_BRS(AlphaBlending, FALSE)
		{
			PL_CHECK(plevel, RS_ALPHABLENDING);			BRS_CHECK(flag, BRS_ALPHABLENDING);			return TRUE;
		}

		DECLARE_BRS(Fog, FALSE)
		{
			PL_CHECK(plevel, RS_FOG);					BRS_CHECK(flag, BRS_FOG);					return TRUE;
		}

		DECLARE_BRS(Specular, FALSE)
		{
			PL_CHECK(plevel, RS_SPECULAR);				BRS_CHECK(flag, BRS_SPECULAR);				return TRUE;
		}

		DECLARE_BRS(StencilTest, FALSE)
		{
			PL_CHECK(plevel, RS_STENCILTEST);			BRS_CHECK(flag, BRS_STENCILTEST);			return TRUE;
		}

		DECLARE_BRS(Lighting, TRUE)
		{
			PL_CHECK(plevel, RS_LIGHTING);				BRS_CHECK(flag, BRS_LIGHTING);				return TRUE;
		}

		DECLARE_BRS(LastPixel, TRUE)
		{
			PL_CHECK(plevel, RS_LASTPIXEL);				BRS_CHECK(flag, BRS_LASTPIXEL);				return TRUE;
		}

		DECLARE_BRS(StippledAlpha, FALSE)
		{
			PL_CHECK(plevel, RS_STIPPLEDALPHA);			BRS_CHECK(flag, BRS_STIPPLEDALPHA);			return TRUE;
		}

		DECLARE_BRS(EdgeAntialias, FALSE)
		{
			PL_CHECK(plevel, RS_EDGEANTIALIAS);			BRS_CHECK(flag, BRS_EDGEANTIALIAS);			return TRUE;
		}

		DECLARE_BRS(ColorKeyMode, FALSE)
		{
			PL_CHECK(plevel, RS_COLORKEYMODE);			BRS_CHECK(flag, BRS_COLORKEYMODE);			return TRUE;
		}

		DECLARE_BRS(ColorKeyBlendMode, FALSE)
		{
			PL_CHECK(plevel, RS_COLORKEYBLENDMODE);		BRS_CHECK(flag, BRS_COLORKEYBLENDMODE);		return TRUE;
		}

		DECLARE_BRS(RangeBasedFog, FALSE)
		{
			PL_CHECK(plevel, RS_RANGEBASEDFOG);			BRS_CHECK(flag, BRS_RANGEBASEDFOG);			return TRUE;
		}

		DECLARE_BRS(Clipping, TRUE)
		{
			PL_CHECK(plevel, RS_CLIPPING);				BRS_CHECK(flag, BRS_CLIPPING);				return TRUE;
		}

		DECLARE_BRS(ScreenExtents, FALSE)
		{
			PL_CHECK(plevel, RS_SCREENEXTENTS);			BRS_CHECK(flag, BRS_SCREENEXTENTS);			return TRUE;
		}

		DECLARE_BRS(ColorVertex, FALSE)
		{
			PL_CHECK(plevel, RS_COLORVERTEX);			BRS_CHECK(flag, BRS_COLORVERTEX);			return TRUE;
		}

		DECLARE_BRS(PerspSpecHigh, TRUE)
		{
			PL_CHECK(plevel, RS_PERSPSPECHIGH);			BRS_CHECK(flag, BRS_PERSPSPECHIGH);			return TRUE;
		}

		DECLARE_BRS(Normalize, FALSE)
		{
			PL_CHECK(plevel, RS_NORMALIZE);				BRS_CHECK(flag, BRS_NORMALIZE);				return TRUE;
		}

		DECLARE_BRS(SoftwareVertexProcessing, FALSE)
		{
			PL_CHECK(plevel, RS_SOFTWAREVERTEXPROCESSING);	BRS_CHECK(flag, BRS_SOFTWAREVERTEXPROCESSING);	return TRUE;
		}

		DECLARE_BRS(PointSprite, FALSE)
		{
			PL_CHECK(plevel, RS_POINTSPRITE);			BRS_CHECK(flag, BRS_POINTSPRITE);			return TRUE;
		}

		DECLARE_BRS(PointScale, FALSE)
		{
			PL_CHECK(plevel, RS_POINTSCALE);			BRS_CHECK(flag, BRS_POINTSCALE);			return TRUE;
		}

		DECLARE_BRS(MultiSampleAntialias, TRUE)
		{
			PL_CHECK(plevel, RS_MULTISAMPLEANTIALIAS);	BRS_CHECK(flag, BRS_MULTISAMPLEANTIALIAS);	return TRUE;
		}

		DECLARE_BRS(IndexedVertexBlend, TRUE)
		{
			PL_CHECK(plevel, RS_INDEXEDVERTEXBLEND);	BRS_CHECK(flag, BRS_INDEXEDVERTEXBLEND);	return TRUE;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Dword-controlled render states
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef RS_PRIORITYLEVELS
	#define DECLARE_DRS(name, type, def)	virtual	inline_	BOOL	Set##name(type value=def, PriorityLevel plevel=128)
#else
	#define DECLARE_DRS(name, type, def)	virtual	inline_	BOOL	Set##name(type value=def)
#endif

		DECLARE_DRS(AmbientColor, udword, 0)
		{
			PL_CHECK(plevel, RS_AMBIENTCOLOR);			DRS_CHECK(value, DRS_AMBIENTCOLOR);		return TRUE;
		}

		DECLARE_DRS(AntialiasMode, ANTIALIASMODE, ANTIALIAS_NONE)
		{
			PL_CHECK(plevel, RS_ANTIALIASMODE);			DRS_CHECK(udword(value), DRS_ANTIALIASMODE);	return TRUE;
		}

		DECLARE_DRS(ZBufferMode, ZBUFFERMODE, ZB_TRUE)
		{
			RS_PREVENT(zm==ZB_USEW && !mCaps.mRasterCaps.mWBuffer, "WBuffer not supported");
			PL_CHECK(plevel, RS_ZBUFFERMODE);			DRS_CHECK(udword(value), DRS_ZBUFFERMODE);		return TRUE;
		}

		DECLARE_DRS(ZBufferCmpMode, CMPFUNC, CMP_LESSEQUAL)
		{
#ifdef RS_SUPPORT_RUNTIME_CHECKINGS
			if(mCheckings)
			{
				RS_PREVENT(cf==CMP_ALWAYS		&& !mCaps.mZCmpCaps.mAlways,		"ZBuffer: CMP_ALWAYS not supported");
				RS_PREVENT(cf==CMP_EQUAL		&& !mCaps.mZCmpCaps.mEqual,			"ZBuffer: CMP_EQUAL not supported");
				RS_PREVENT(cf==CMP_GREATER		&& !mCaps.mZCmpCaps.mGreater,		"ZBuffer: CMP_GREATER not supported");
				RS_PREVENT(cf==CMP_GREATEREQUAL	&& !mCaps.mZCmpCaps.mGreaterEqual,	"ZBuffer: CMP_GREATEREQUAL not supported");
				RS_PREVENT(cf==CMP_LESS			&& !mCaps.mZCmpCaps.mLess,			"ZBuffer: CMP_LESS not supported");
				RS_PREVENT(cf==CMP_LESSEQUAL	&& !mCaps.mZCmpCaps.mLessEqual,		"ZBuffer: CMP_LESSEQUAL not supported");
				RS_PREVENT(cf==CMP_NEVER		&& !mCaps.mZCmpCaps.mNever,			"ZBuffer: CMP_NEVER not supported");
				RS_PREVENT(cf==CMP_NOTEQUAL		&& !mCaps.mZCmpCaps.mNotEqual,		"ZBuffer: CMP_NOTEQUAL not supported");
			}
#endif
			PL_CHECK(plevel, RS_ZBUFFERCMPMODE);		DRS_CHECK(udword(value), DRS_ZBUFFERCMPMODE);	return TRUE;
		}

		DECLARE_DRS(ZBias, udword, 0)
		{
			RS_PREVENT(!mCaps.mRasterCaps.mZBias, "ZBias not supported");
			PL_CHECK(plevel, RS_ZBIAS);					DRS_CHECK(value, DRS_ZBIAS);				return TRUE;
		}

		DECLARE_DRS(LinePattern, udword, 0)
		{
			PL_CHECK(plevel, RS_LINEPATTERN);			DRS_CHECK(value, DRS_LINEPATTERN);			return TRUE;
		}

		DECLARE_DRS(AlphaRef, udword, 0)
		{
			PL_CHECK(plevel, RS_ALPHAREF);				DRS_CHECK(value, DRS_ALPHAREF);				return TRUE;
		}

		DECLARE_DRS(AlphaFunc, CMPFUNC, CMP_ALWAYS)
		{
#ifdef RS_SUPPORT_RUNTIME_CHECKINGS
			if(mCheckings)
			{
				RS_PREVENT(af==CMP_ALWAYS		&& !mCaps.mAlphaCmpCaps.mAlways,		"AlphaFunc: CMP_ALWAYS not supported");
				RS_PREVENT(af==CMP_EQUAL		&& !mCaps.mAlphaCmpCaps.mEqual,			"AlphaFunc: CMP_EQUAL not supported");
				RS_PREVENT(af==CMP_GREATER		&& !mCaps.mAlphaCmpCaps.mGreater,		"AlphaFunc: CMP_GREATER not supported");
				RS_PREVENT(af==CMP_GREATEREQUAL	&& !mCaps.mAlphaCmpCaps.mGreaterEqual,	"AlphaFunc: CMP_GREATEREQUAL not supported");
				RS_PREVENT(af==CMP_LESS			&& !mCaps.mAlphaCmpCaps.mLess,			"AlphaFunc: CMP_LESS not supported");
				RS_PREVENT(af==CMP_LESSEQUAL	&& !mCaps.mAlphaCmpCaps.mLessEqual,		"AlphaFunc: CMP_LESSEQUAL not supported");
				RS_PREVENT(af==CMP_NEVER		&& !mCaps.mAlphaCmpCaps.mNever,			"AlphaFunc: CMP_NEVER not supported");
				RS_PREVENT(af==CMP_NOTEQUAL		&& !mCaps.mAlphaCmpCaps.mNotEqual,		"AlphaFunc: CMP_NOTEQUAL not supported");
			}
#endif
			PL_CHECK(plevel, RS_ALPHAFUNC);				DRS_CHECK(udword(value), DRS_ALPHAFUNC);		return TRUE;
		}

		DECLARE_DRS(SrcBlend, ALPHABLEND, ABLEND_SRCALPHA)
		{
#ifdef RS_SUPPORT_RUNTIME_CHECKINGS
			if(mCheckings)
			{
				RS_PREVENT(mode==ABLEND_ZERO			&& !mCaps.mSrcBlendCaps.mZero,				"SrcBlend: ABLEND_ZERO not supported");
				RS_PREVENT(mode==ABLEND_ONE				&& !mCaps.mSrcBlendCaps.mOne,				"SrcBlend: ABLEND_ONE not supported");
				RS_PREVENT(mode==ABLEND_SRCCOLOR		&& !mCaps.mSrcBlendCaps.mSrcColor,			"SrcBlend: ABLEND_SRCCOLOR not supported");
				RS_PREVENT(mode==ABLEND_INVSRCCOLOR		&& !mCaps.mSrcBlendCaps.mInvSrcColor,		"SrcBlend: ABLEND_INVSRCCOLOR not supported");
				RS_PREVENT(mode==ABLEND_SRCALPHA		&& !mCaps.mSrcBlendCaps.mSrcAlpha,			"SrcBlend: ABLEND_SRCALPHA not supported");
				RS_PREVENT(mode==ABLEND_INVSRCALPHA		&& !mCaps.mSrcBlendCaps.mInvSrcAlpha,		"SrcBlend: ABLEND_INVSRCALPHA not supported");
				RS_PREVENT(mode==ABLEND_DESTALPHA		&& !mCaps.mSrcBlendCaps.mDestAlpha,			"SrcBlend: ABLEND_DESTALPHA not supported");
				RS_PREVENT(mode==ABLEND_INVDESTALPHA	&& !mCaps.mSrcBlendCaps.mInvDestAlpha,		"SrcBlend: ABLEND_INVDESTALPHA not supported");
				RS_PREVENT(mode==ABLEND_DESTCOLOR		&& !mCaps.mSrcBlendCaps.mDestColor,			"SrcBlend: ABLEND_DESTCOLOR not supported");
				RS_PREVENT(mode==ABLEND_INVDESTCOLOR	&& !mCaps.mSrcBlendCaps.mInvDestColor,		"SrcBlend: ABLEND_INVDESTCOLOR not supported");
				RS_PREVENT(mode==ABLEND_SRCALPHASAT		&& !mCaps.mSrcBlendCaps.mSrcAlphaSat,		"SrcBlend: ABLEND_SRCALPHASAT not supported");
				RS_PREVENT(mode==ABLEND_BOTHSRCALPHA	&& !mCaps.mSrcBlendCaps.mBothSrcAlpha,		"SrcBlend: ABLEND_BOTHSRCALPHA not supported");
				RS_PREVENT(mode==ABLEND_BOTHINVSRCALPHA	&& !mCaps.mSrcBlendCaps.mBothInvSrcAlpha,	"SrcBlend: ABLEND_BOTHINVSRCALPHA not supported");
			}
#endif
			PL_CHECK(plevel, RS_SRCBLEND);				DRS_CHECK(udword(value), DRS_SRCBLEND);		return TRUE;
		}

		DECLARE_DRS(DstBlend, ALPHABLEND, ABLEND_INVSRCALPHA)
		{
#ifdef RS_SUPPORT_RUNTIME_CHECKINGS
			if(mCheckings)
			{
				RS_PREVENT(mode==ABLEND_ZERO			&& !mCaps.mDestBlendCaps.mZero,				"DestBlend: ABLEND_ZERO not supported");
				RS_PREVENT(mode==ABLEND_ONE				&& !mCaps.mDestBlendCaps.mOne,				"DestBlend: ABLEND_ONE not supported");
				RS_PREVENT(mode==ABLEND_SRCCOLOR		&& !mCaps.mDestBlendCaps.mSrcColor,			"DestBlend: ABLEND_SRCCOLOR not supported");
				RS_PREVENT(mode==ABLEND_INVSRCCOLOR		&& !mCaps.mDestBlendCaps.mInvSrcColor,		"DestBlend: ABLEND_INVSRCCOLOR not supported");
				RS_PREVENT(mode==ABLEND_SRCALPHA		&& !mCaps.mDestBlendCaps.mSrcAlpha,			"DestBlend: ABLEND_SRCALPHA not supported");
				RS_PREVENT(mode==ABLEND_INVSRCALPHA		&& !mCaps.mDestBlendCaps.mInvSrcAlpha,		"DestBlend: ABLEND_INVSRCALPHA not supported");
				RS_PREVENT(mode==ABLEND_DESTALPHA		&& !mCaps.mDestBlendCaps.mDestAlpha,		"DestBlend: ABLEND_DESTALPHA not supported");
				RS_PREVENT(mode==ABLEND_INVDESTALPHA	&& !mCaps.mDestBlendCaps.mInvDestAlpha,		"DestBlend: ABLEND_INVDESTALPHA not supported");
				RS_PREVENT(mode==ABLEND_DESTCOLOR		&& !mCaps.mDestBlendCaps.mDestColor,		"DestBlend: ABLEND_DESTCOLOR not supported");
				RS_PREVENT(mode==ABLEND_INVDESTCOLOR	&& !mCaps.mDestBlendCaps.mInvDestColor,		"DestBlend: ABLEND_INVDESTCOLOR not supported");
				RS_PREVENT(mode==ABLEND_SRCALPHASAT		&& !mCaps.mDestBlendCaps.mSrcAlphaSat,		"DestBlend: ABLEND_SRCALPHASAT not supported");
				RS_PREVENT(mode==ABLEND_BOTHSRCALPHA	&& !mCaps.mDestBlendCaps.mBothSrcAlpha,		"DestBlend: ABLEND_BOTHSRCALPHA not supported");
				RS_PREVENT(mode==ABLEND_BOTHINVSRCALPHA	&& !mCaps.mDestBlendCaps.mBothInvSrcAlpha,	"DestBlend: ABLEND_BOTHINVSRCALPHA not supported");
			}
#endif
			PL_CHECK(plevel, RS_DESTBLEND);				DRS_CHECK(udword(value), DRS_DESTBLEND);		return TRUE;
		}

		DECLARE_DRS(DiffuseMaterialSource, MCS, MCS_COLOR1)
		{
			PL_CHECK(plevel, RS_DIFFUSEMATSRC);			DRS_CHECK(udword(value), DRS_DIFFUSEMATSRC);	return TRUE;
		}

		DECLARE_DRS(SpecularMaterialSource, MCS, MCS_COLOR2)
		{
			PL_CHECK(plevel, RS_SPECULARMATSRC);		DRS_CHECK(udword(value), DRS_SPECULARMATSRC);return TRUE;
		}

		DECLARE_DRS(AmbientMaterialSource, MCS, MCS_COLOR2)
		{
			PL_CHECK(plevel, RS_AMBIENTMATSRC);			DRS_CHECK(udword(value), DRS_AMBIENTMATSRC);return TRUE;
		}

		DECLARE_DRS(EmissiveMaterialSource, MCS, MCS_MATERIAL)
		{
			PL_CHECK(plevel, RS_EMISSIVEMATSRC);		DRS_CHECK(udword(value), DRS_EMISSIVEMATSRC);return TRUE;
		}

		DECLARE_DRS(VertexBlend, VBLEND, VBLEND_DISABLE)
		{
			PL_CHECK(plevel, RS_VERTEXBLEND);			DRS_CHECK(udword(value), DRS_VERTEXBLEND);	return TRUE;
		}

		DECLARE_DRS(TextureFactor, udword, 0)
		{
			PL_CHECK(plevel, RS_TEXTUREFACTOR);			DRS_CHECK(value, DRS_TEXTUREFACTOR);		return TRUE;
		}

		DECLARE_DRS(FillMode, FILLMODE, FILL_SOLID)
		{
			PL_CHECK(plevel, RS_FILLMODE);				DRS_CHECK(udword(value), DRS_FILLMODE);		return TRUE;
		}

		DECLARE_DRS(ShadeMode, SHADEMODE, SHADE_GOURAUD)
		{
			PL_CHECK(plevel, RS_SHADEMODE);				DRS_CHECK(udword(value), DRS_SHADEMODE);		return TRUE;
		}

		DECLARE_DRS(CullMode, CULLMODE, CULL_CW)
		{
#ifdef RS_SUPPORT_RUNTIME_CHECKINGS
			if(mCheckings)
			{
				RS_PREVENT(cm==CULL_NONE	&& !mCaps.mMiscCaps.mCullNone,	"Backface culling: CULL_NONE not supported");
				RS_PREVENT(cm==CULL_CW		&& !mCaps.mMiscCaps.mCullCW,	"Backface culling: CULL_CW not supported");
				RS_PREVENT(cm==CULL_CCW		&& !mCaps.mMiscCaps.mCullCCW,	"Backface culling: CULL_CCW not supported");
			}
#endif
			PL_CHECK(plevel, RS_CULLMODE);				DRS_CHECK(udword(value), DRS_CULLMODE);		return TRUE;
		}

		DECLARE_DRS(StencilRef, udword, 0)
		{
			PL_CHECK(plevel, RS_STENCILREF);			DRS_CHECK(value, DRS_STENCILREF);				return TRUE;
		}

		DECLARE_DRS(StencilCmpMode, CMPFUNC, CMP_ALWAYS)
		{
		/*	if(mCheckings)
			{
				RS_PREVENT(cf==CMP_ALWAYS		&& !mCaps.AlphaCmpCaps_Always,			"StencilFunc: CMP_ALWAYS not supported");
				RS_PREVENT(cf==CMP_EQUAL		&& !mCaps.AlphaCmpCaps_Equal,			"StencilFunc: CMP_EQUAL not supported");
				RS_PREVENT(cf==CMP_GREATER		&& !mCaps.AlphaCmpCaps_Greater,			"StencilFunc: CMP_GREATER not supported");
				RS_PREVENT(cf==CMP_GREATEREQUAL	&& !mCaps.AlphaCmpCaps_GreaterEqual,	"StencilFunc: CMP_GREATEREQUAL not supported");
				RS_PREVENT(cf==CMP_LESS			&& !mCaps.AlphaCmpCaps_Less,			"StencilFunc: CMP_LESS not supported");
				RS_PREVENT(cf==CMP_LESSEQUAL	&& !mCaps.AlphaCmpCaps_LessEqual,		"StencilFunc: CMP_LESSEQUAL not supported");
				RS_PREVENT(cf==CMP_NEVER		&& !mCaps.AlphaCmpCaps_Never,			"StencilFunc: CMP_NEVER not supported");
				RS_PREVENT(cf==CMP_NOTEQUAL		&& !mCaps.AlphaCmpCaps_NotEqual,		"StencilFunc: CMP_NOTEQUAL not supported");
			}*/
			PL_CHECK(plevel, RS_STENCILCMPMODE);		DRS_CHECK(udword(value), DRS_STENCILCMPMODE);	return TRUE;
		}

		DECLARE_DRS(StencilCmpMask, udword, 0xffffffff)
		{
			PL_CHECK(plevel, RS_STENCILCMPMASK);		DRS_CHECK(udword(value), DRS_STENCILCMPMASK);return TRUE;
		}

		DECLARE_DRS(StencilWriteMask, udword, 0xffffffff)
		{
			PL_CHECK(plevel, RS_STENCILWRITEMASK);		DRS_CHECK(udword(value), DRS_STENCILWRITEMASK);return TRUE;
		}

		DECLARE_DRS(StencilFailOp, STENCILOP, STENCILOP_KEEP)
		{
#ifdef RS_SUPPORT_RUNTIME_CHECKINGS
			if(mCheckings)
			{
				RS_PREVENT(sop==STENCILOP_KEEP		&& !mCaps.mStencilOp.mKeep,		"StencilFailOp: STENCILOP_KEEP not supported!");
				RS_PREVENT(sop==STENCILOP_ZERO		&& !mCaps.mStencilOp.mZero,		"StencilFailOp: STENCILOP_ZERO not supported!");
				RS_PREVENT(sop==STENCILOP_REPLACE	&& !mCaps.mStencilOp.mReplace,	"StencilFailOp: STENCILOP_REPLACE not supported!");
				RS_PREVENT(sop==STENCILOP_INCRSAT	&& !mCaps.mStencilOp.mIncrSat,	"StencilFailOp: STENCILOP_INCRSAT not supported!");
				RS_PREVENT(sop==STENCILOP_DECRSAT	&& !mCaps.mStencilOp.mDecrSat,	"StencilFailOp: STENCILOP_DECRSAT not supported!");
				RS_PREVENT(sop==STENCILOP_INVERT	&& !mCaps.mStencilOp.mInvert,	"StencilFailOp: STENCILOP_INVERT not supported!");
				RS_PREVENT(sop==STENCILOP_INCR		&& !mCaps.mStencilOp.mIncr,		"StencilFailOp: STENCILOP_INCR not supported!");
				RS_PREVENT(sop==STENCILOP_DECR		&& !mCaps.mStencilOp.mDecr,		"StencilFailOp: STENCILOP_DECR not supported!");
			}
#endif
			PL_CHECK(plevel, RS_STENCILFAILOP);			DRS_CHECK(udword(value), DRS_STENCILFAILOP);	return TRUE;
		}

		DECLARE_DRS(StencilZFailOp, STENCILOP, STENCILOP_KEEP)
		{
#ifdef RS_SUPPORT_RUNTIME_CHECKINGS
			if(mCheckings)
			{
				RS_PREVENT(sop==STENCILOP_KEEP		&& !mCaps.mStencilOp.mKeep,		"StencilZFailOp: STENCILOP_KEEP not supported!");
				RS_PREVENT(sop==STENCILOP_ZERO		&& !mCaps.mStencilOp.mZero,		"StencilZFailOp: STENCILOP_ZERO not supported!");
				RS_PREVENT(sop==STENCILOP_REPLACE	&& !mCaps.mStencilOp.mReplace,	"StencilZFailOp: STENCILOP_REPLACE not supported!");
				RS_PREVENT(sop==STENCILOP_INCRSAT	&& !mCaps.mStencilOp.mIncrSat,	"StencilZFailOp: STENCILOP_INCRSAT not supported!");
				RS_PREVENT(sop==STENCILOP_DECRSAT	&& !mCaps.mStencilOp.mDecrSat,	"StencilZFailOp: STENCILOP_DECRSAT not supported!");
				RS_PREVENT(sop==STENCILOP_INVERT	&& !mCaps.mStencilOp.mInvert,	"StencilZFailOp: STENCILOP_INVERT not supported!");
				RS_PREVENT(sop==STENCILOP_INCR		&& !mCaps.mStencilOp.mIncr,		"StencilZFailOp: STENCILOP_INCR not supported!");
				RS_PREVENT(sop==STENCILOP_DECR		&& !mCaps.mStencilOp.mDecr,		"StencilZFailOp: STENCILOP_DECR not supported!");
			}
#endif
			PL_CHECK(plevel, RS_STENCILZFAILOP);		DRS_CHECK(udword(value), DRS_STENCILZFAILOP);	return TRUE;
		}

		DECLARE_DRS(StencilPassOp, STENCILOP, STENCILOP_KEEP)
		{
#ifdef RS_SUPPORT_RUNTIME_CHECKINGS
			if(mCheckings)
			{
				RS_PREVENT(sop==STENCILOP_KEEP		&& !mCaps.mStencilOp.mKeep,		"StencilPassOp: STENCILOP_KEEP not supported!");
				RS_PREVENT(sop==STENCILOP_ZERO		&& !mCaps.mStencilOp.mZero,		"StencilPassOp: STENCILOP_ZERO not supported!");
				RS_PREVENT(sop==STENCILOP_REPLACE	&& !mCaps.mStencilOp.mReplace,	"StencilPassOp: STENCILOP_REPLACE not supported!");
				RS_PREVENT(sop==STENCILOP_INCRSAT	&& !mCaps.mStencilOp.mIncrSat,	"StencilPassOp: STENCILOP_INCRSAT not supported!");
				RS_PREVENT(sop==STENCILOP_DECRSAT	&& !mCaps.mStencilOp.mDecrSat,	"StencilPassOp: STENCILOP_DECRSAT not supported!");
				RS_PREVENT(sop==STENCILOP_INVERT	&& !mCaps.mStencilOp.mInvert,	"StencilPassOp: STENCILOP_INVERT not supported!");
				RS_PREVENT(sop==STENCILOP_INCR		&& !mCaps.mStencilOp.mIncr,		"StencilPassOp: STENCILOP_INCR not supported!");
				RS_PREVENT(sop==STENCILOP_DECR		&& !mCaps.mStencilOp.mDecr,		"StencilPassOp: STENCILOP_DECR not supported!");
			}
#endif
			PL_CHECK(plevel, RS_STENCILPASSOP);			DRS_CHECK(udword(value), DRS_STENCILPASSOP);	return TRUE;
		}

		DECLARE_DRS(FogVertexMode, FOGMODE, FOGMODE_NONE)
		{
			PL_CHECK(plevel, RS_FOGVERTEXMODE);			DRS_CHECK(udword(value), DRS_FOGVERTEXMODE);return TRUE;
		}

		DECLARE_DRS(FogMode, FOGMODE, FOGMODE_NONE)
		{
			PL_CHECK(plevel, RS_FOGMODE);				DRS_CHECK(udword(value), DRS_FOGMODE);			return TRUE;
		}

		DECLARE_DRS(FogDensity, float, 0.0f)
		{
			PL_CHECK(plevel, RS_FOGDENSITY);			DRS_CHECK(*(udword*)&value, DRS_FOGDENSITY);	return TRUE;
		}

		DECLARE_DRS(FogColor, udword, 0)
		{
			PL_CHECK(plevel, RS_FOGCOLOR);				DRS_CHECK(value, DRS_FOGCOLOR);				return TRUE;
		}

		DECLARE_DRS(FogStart, float, 0.0f)
		{
			PL_CHECK(plevel, RS_FOGSTART);				DRS_CHECK(*(udword*)&value, DRS_FOGSTART);	return TRUE;
		}

		DECLARE_DRS(FogEnd, float, 0.0f)
		{
			PL_CHECK(plevel, RS_FOGEND);				DRS_CHECK(*(udword*)&value, DRS_FOGEND);		return TRUE;
		}

		DECLARE_DRS(MaterialProps, const MaterialProps*, null)
		{
			// WARNING: cache must be invalidated when material properties are dynamic....
			PL_CHECK(plevel, RS_MATERIAL);				DRS_CHECK(udword(value), DRS_MATERIAL);			return TRUE;
		}

		DECLARE_DRS(PointSize, float, 1.0f)
		{
			PL_CHECK(plevel, RS_POINTSIZE);				DRS_CHECK(*(udword*)&value, DRS_POINTSIZE);		return TRUE;
		}

		DECLARE_DRS(PointSizeMin, float, 1.0f)
		{
			PL_CHECK(plevel, RS_POINTSIZEMIN);			DRS_CHECK(*(udword*)&value, DRS_POINTSIZEMIN);	return TRUE;
		}

		DECLARE_DRS(PointScaleA, float, 1.0f)
		{
			PL_CHECK(plevel, RS_POINTSCALEA);			DRS_CHECK(*(udword*)&value, DRS_POINTSCALEA);		return TRUE;
		}

		DECLARE_DRS(PointScaleB, float, 0.0f)
		{
			PL_CHECK(plevel, RS_POINTSCALEB);			DRS_CHECK(*(udword*)&value, DRS_POINTSCALEB);		return TRUE;
		}

		DECLARE_DRS(PointScaleC, float, 0.0f)
		{
			PL_CHECK(plevel, RS_POINTSCALEC);			DRS_CHECK(*(udword*)&value, DRS_POINTSCALEC);		return TRUE;
		}

		DECLARE_DRS(MultiSampleMask, udword, 0xffffffff)
		{
			PL_CHECK(plevel, RS_MULTISAMPLEMASK);		DRS_CHECK(value, DRS_MULTISAMPLEMASK);				return TRUE;
		}

		DECLARE_DRS(PatchEdgeStyle, PATCHEDGESTYLE, PES_DISCRETE)
		{
			PL_CHECK(plevel, RS_PATCHEDGESTYLE);		DRS_CHECK(udword(value), DRS_PATCHEDGESTYLE);			return TRUE;
		}

		DECLARE_DRS(PatchSegments, float, 0.0f)
		{
			PL_CHECK(plevel, RS_PATCHSEGMENTS);			DRS_CHECK(*(udword*)&value, DRS_PATCHSEGMENTS);		return TRUE;
		}

		DECLARE_DRS(PointSizeMax, float, 1.0f)
		{
			PL_CHECK(plevel, RS_POINTSIZEMAX);			DRS_CHECK(*(udword*)&value, DRS_POINTSIZEMAX);	return TRUE;
		}

		DECLARE_DRS(ColorWriteEnable, udword, CWE_ALL)
		{
			RS_PREVENT(!mCaps.mMiscCaps.mColorWriteEnable, "Color-write-enable not supported");
			PL_CHECK(plevel, RS_COLORWRITEENABLE);		DRS_CHECK(value, DRS_COLORWRITEENABLE);				return TRUE;
		}

		DECLARE_DRS(TweenFactor, float, 0.0f)
		{
			PL_CHECK(plevel, RS_TWEENFACTOR);			DRS_CHECK(*(udword*)&value, DRS_TWEENFACTOR);		return TRUE;
		}

		DECLARE_DRS(BlendOp, BLENDOP, BLENDOP_ADD)
		{
			PL_CHECK(plevel, RS_BLENDOP);				DRS_CHECK(udword(value), DRS_BLENDOP);					return TRUE;
		}

		DECLARE_DRS(VertexShader, VertexShader*, null)
		{
			PL_CHECK(plevel, RS_VERTEXSHADER);			DRS_CHECK(udword(value), DRS_VERTEXSHADER);			return TRUE;
		}

		DECLARE_DRS(PixelShader, PixelShader*, null)
		{
			PL_CHECK(plevel, RS_PIXELSHADER);			DRS_CHECK(udword(value), DRS_PIXELSHADER);			return TRUE;
		}

		DECLARE_DRS(VertexDeclaration, VertexDeclaration*, null)
		{
			PL_CHECK(plevel, RS_VERTEXDECLARATION);		DRS_CHECK(udword(value), DRS_VERTEXDECLARATION);	return TRUE;
		}

		// Note: color mask render state is obsolete in DX7. Use alpha blending.
		// But it's back in DX8 with SetColorWriteEnable!

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Texture Render States
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef RS_PRIORITYLEVELS
		virtual	inline_	BOOL			SetTextureBitmap			(udword i, void* bitmap=null,		PriorityLevel plevel=128)=0;
		virtual	inline_	BOOL			SetTextureAddress(udword i, TEXTUREADDRESS ta, PriorityLevel plevel=128)	= 0;
#else
		virtual	inline_	BOOL			SetTextureBitmap			(udword i, void* bitmap=null)=0;
		virtual	inline_	BOOL			SetTextureAddress(udword i, TEXTUREADDRESS ta)	= 0;
#endif

#ifdef RS_PRIORITYLEVELS
	#define DECLARE_TRS(name, type)	virtual	inline_	BOOL	Set##name(udword i, type value, PriorityLevel plevel=128)
#else
	#define DECLARE_TRS(name, type)	virtual	inline_	BOOL	Set##name(udword i, type value)
#endif

		DECLARE_TRS(TextureColorOp, STAGEOP)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mColorOp);	TS_CHECK(value, mColorOp);		return TRUE;
		}

		DECLARE_TRS(TextureColorArg1, STAGEARG)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mColorArg1);	TS_CHECK(value, mColorArg1);	return TRUE;
		}

		DECLARE_TRS(TextureColorArg2, STAGEARG)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mColorArg2);	TS_CHECK(value, mColorArg2);	return TRUE;
		}

		DECLARE_TRS(TextureAlphaOp, STAGEOP)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mAlphaOp);	TS_CHECK(value, mAlphaOp);		return TRUE;
		}

		DECLARE_TRS(TextureAlphaArg1, STAGEARG)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mAlphaArg1);	TS_CHECK(value, mAlphaArg1);	return TRUE;
		}

		DECLARE_TRS(TextureAlphaArg2, STAGEARG)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mAlphaArg2);	TS_CHECK(value, mAlphaArg2);	return TRUE;
		}

		DECLARE_TRS(TextureMinFilter, TEXTUREMINFILTER)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mMinFilter);	TS_CHECK(value, mMinFilter);	return TRUE;
		}

		DECLARE_TRS(TextureMagFilter, TEXTUREMAGFILTER)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mMagFilter);	TS_CHECK(value, mMagFilter);	return TRUE;
		}

		DECLARE_TRS(TextureMipFilter, TEXTUREMIPFILTER)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mMipFilter);	TS_CHECK(value, mMipFilter);	return TRUE;
		}

		DECLARE_TRS(TextureCoordIndex, STAGETEXCOORDINDEX)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mCoordIndex);	TS_CHECK(value, mCoordIndex);	return TRUE;
		}

		DECLARE_TRS(TextureTransFlags, TEXTURETRANSFLAGS)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mTransflags);	TS_CHECK(value, mTransFlags);	return TRUE;
		}

		DECLARE_TRS(TextureMipMapLODBias, float)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mMipMapLODBias);	TS_CHECK(value, mMipMapLODBias);	return TRUE;
		}

		DECLARE_TRS(TextureMaxMipMapLevel, udword)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mMaxMipMapLevel);	TS_CHECK(value, mMaxMipMapLevel);	return TRUE;
		}

		DECLARE_TRS(TextureMaxAnisotropy, udword)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mMaxAnisotropy);	TS_CHECK(value, mMaxAnisotropy);		return TRUE;
		}

		DECLARE_TRS(TextureAddressU, TEXTUREADDRESS)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mAddressU);	TS_CHECK(value, mAddressU);	return TRUE;
		}

		DECLARE_TRS(TextureAddressV, TEXTUREADDRESS)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mAddressV);	TS_CHECK(value, mAddressV);	return TRUE;
		}

		DECLARE_TRS(TextureBorder, udword)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mBorder);	TS_CHECK(value, mBorder);	return TRUE;
		}

		DECLARE_TRS(TextureWrapU, BOOL)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mWrapU);	TS_CHECK(value, mWrapU);	return TRUE;
		}

		DECLARE_TRS(TextureWrapV, BOOL)
		{
			RS_PREVENT(i>=mCaps.mMaxTextureBlendStages, "Texture stage index exceeds device caps");
			TPL_CHECK(plevel, mWrapV);	TS_CHECK(value, mWrapV);	return TRUE;
		}

		// Stats
#ifdef _DEBUG
		inline_			void					ResetStats()
												{
													mNbBRSCalls			= 0;
													mNbBRSDone			= 0;
													mNbDRSCalls			= 0;
													mNbDRSDone			= 0;
													mNbTSCalls			= 0;
													mNbTSDone			= 0;
													mNbSetViewMatrix	= 0;
													mNbSetProjMatrix	= 0;
													mNbSetWorldMatrix	= 0;
												}
						udword					mNbBRSCalls;		//!< Number of boolean-render-state calls from the app
						udword					mNbBRSDone;			//!< Number of boolean-render-state calls performed
						udword					mNbDRSCalls;		//!< Number of dword-render-state calls from the app
						udword					mNbDRSDone;			//!< Number of dword-render-state calls performed
						udword					mNbTSCalls;			//!< Number of texture-render-state calls from the app
						udword					mNbTSDone;			//!< Number of texture-render-state calls from the app
						udword					mNbSetViewMatrix;	//!< Number of view-matrix setups from the app
						udword					mNbSetProjMatrix;	//!< Number of proj-matrix setups from the app
						udword					mNbSetWorldMatrix;	//!< Number of world-matrix setups from the app
#else
		inline_			void					ResetStats()	{}
#endif

		inline_			void					SetTimeInfo(TimeInfo* time)				{ mTime = time;	}
		inline_			TimeInfo*				GetTimeInfo()					const	{ return mTime;	}
		inline_			const RenderCaps&		GetCaps()						const	{ return mCaps;	}

		virtual			void					SetChunkLODTexGen(udword stage, float xsize, float zsize, float x0, float z0)	{}

		private:
						TextureRSCall			mTSTable[TS_SIZE];
						DwordRSCall				mDRSTable[DRS_SIZE];
						BoolRSCall				mBRSTable[32];
		// State block manager
						XList					mSBManager;
		// Material manager
						XList					mMTLManager;
		// Symbolic lists
						Container				mSLists;
		// Possible time-related info for dynamic render states
						TimeInfo*				mTime;

						bool					CreateRenderStatesSymbolicLists();
		protected:
		// Transforms
						Matrix4x4				mMulWorld;					//!< A user-defined matrix multiplied with world transforms
						const Matrix4x4*		mWorldTransform;			//!< Pointer to previous matrix or null not to use it
						const Matrix4x4*		mLastTransform;				//!< Pointer to last world matrix
		// Render state caches
						udword					mCacheFlags;				//!< Cache for bool-controlled render states
						udword					mCacheDwords[DRS_SIZE];		//!< Caches for dword-controlled render states
						TextureStage*			mTexCaches;					//!< Texture stages caches

						FRONTFACE				mFrontFace;					//!< Temp
#ifdef RS_PRIORITYLEVELS
		// Priority levels (for render state overrides)
						PriorityLevel			mPriorityLevels[RS_SIZE];	//!< Render states priority levels
						TSPriorityLevel*		mPLTexture;					//!< Texture stages priority levels
#endif
		// Default states
						MaterialProps			mDefaultMaterial;			//!< Default material properties

		// User-defined clipping planes
						Plane*					mActiveUserClipPlanes;
						udword					mNbActiveUserClipPlanes;
		// Render caps
						RenderCaps				mCaps;
		// Settings
						bool					mCheckings;
						bool					mUpdateLogFile;
						bool					mStateBlockEmulation;
	};

	#define DISABLE_ZBUFFER(rd)		udword __OldZBMode = rd->mRS->GetZBufferMode();	rd->mRS->SetZBufferMode(ZB_FALSE);
	#define ENABLE_ZBUFFER(rd)		rd->mRS->SetZBufferMode((ZBUFFERMODE)__OldZBMode);

	#define DISABLE_CULLING(rd)		udword __OldCullMode = rd->mRS->GetCullMode();	rd->mRS->SetCullMode(CULL_NONE);
	#define ENABLE_CULLING(rd)		rd->mRS->SetCullMode((CULLMODE)__OldCullMode);

#endif // ICERENDERSTATEMANAGER_H
