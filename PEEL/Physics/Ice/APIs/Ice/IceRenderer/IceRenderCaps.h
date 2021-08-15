///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains caps-related code.
 *	\file		IceRenderCaps.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERENDERCAPS_H
#define ICERENDERCAPS_H

	//! Driver caps
	struct ICERENDERER_API DriverCaps
	{
					DriverCaps();
		// DX8
		bool		mCanGetScanline;				//!< Display hardware is capable of returning the current scan line.
		bool		mCanCalibrateGamma;				//!< The system has a gamma calibrator installed.
		bool		mCanFullscreenGamma;			//!< The driver supports dynamic gamma ramp adjustment in full-screen mode.
		bool		mCanRenderWindowed;				//!< The driver is capable of rendering in windowed mode.
		bool		mNo2DDuring3DScene;				//!< 2-D operations cannot be performed between calls to BeginScene and EndScene.
		// DX9
		bool		mAutoMipmap;
	};

	//! Device caps
	struct ICERENDERER_API DeviceCaps
	{
					DeviceCaps();
		// DX7
		bool		mFloatTLVertex;					//!< Device accepts floating point for post-transform vertex data.
		bool		mSortDecreasingZ;				//!< Device needs data sorted for decreasing depth.
		bool		mSortExact;						//!< Device needs data sorted exactly.
		bool		mSortIncreasingZ;				//!< Device needs data sorted for increasing depth.
//		bool		mStridedVertices;				//!< Device supports strided vertex data for transformation and lighting in hardware.

		// DX8
		bool		mNPatches;						//!< Device supports N patches.
		bool		mPureDevice;					//!< Device can support rasterization, transform, lighting, and shading in hardware.
		bool		mQuinticRTPatches;				//!< Device supports quintic béziers and B-splines.
		bool		mRTPatches;						//!< Device supports rectangular and triangular patches.
		bool		mRTPatchHandleZero;				//!< When this device capability is set, the hardware architecture does not require caching of
													//!< any information, and uncached patches (handle zero) will be drawn as efficiently as cached
													//!< ones. Note that setting D3DDEVCAPS_RTPATCHHANDLEZERO does not mean that a patch with handle
													//!< zero can be drawn. A handle-zero patch can always be drawn whether this cap is set or not.
		// DX7-DX8
		bool		mCanBltSysToNonLocal;			//!< Device supports blits from system-memory textures to nonlocal video-memory textures.
		bool		mCanRenderAfterFlip;			//!< Device can queue rendering commands after a page flip. Applications do not change their behavior
													//!< if this flag is set; this capability simply means that the device is relatively fast.
		bool		mDrawPrimTLVertex;				//!< Device exports a DrawPrimitive-aware hardware abstraction layer (HAL).
		bool		mExecuteSystemMemory;			//!< Device can use execute buffers from system memory.
		bool		mExecuteVideoMemory;			//!< Device can use execute buffers from video memory.
		bool		mHWRasterization;				//!< Device has hardware acceleration for scene rasterization.
		bool		mHWTransformAndLight;			//!< Device supports transformation and lighting in hardware.
		bool		mSeparateTextureMemories;		//!< Device uses discrete texture memory pools for each stage. Textures must be assigned to texture
													//!< stages explicitly at the time of creation by setting the dwTextureStage member of the
													//!< DDSURFACEDESC2 structure to the appropriate stage identifier.
													//!< => Device is texturing from separate memory pools.
		bool		mTextureNonLocalVidMem;			//!< Device can retrieve textures from nonlocal video (AGP) memory.
		bool		mTextureSystemMemory;			//!< Device can retrieve textures from system memory.
		bool		mTextureVideoMemory;			//!< Device can retrieve textures from device memory.
		bool		mTLVertexSystemMemory;			//!< Device can use buffers from system memory for transformed and lit vertices.
		bool		mTLVertexVideoMemory;			//!< Device can use buffers from video memory for transformed and lit vertices.
	};

	//! Misc caps
	struct ICERENDERER_API MiscCaps
	{
					MiscCaps();
		// DX7
		bool		mOpenGLConformant;				//!< The device conforms to the OpenGL standard.
		bool		mMaskPlanes;					//!< The device can perform a bitmask of color planes.

		// DX8
		bool		mBlendop;						//!< Device supports the alpha-blending operations defined in the BLENDOP enumerated type.
		bool		mClipPlaneScaledPoints;			//!< Device correctly clips scaled points of size greater than 1.0 to user-defined clipping planes.
		bool		mClipTLVerts;					//!< Device clips post-transformed vertex primitives.
		bool		mColorWriteEnable;				//!< Device supports per-channel writes for the render target color buffer through the RS_COLORWRITEENABLE state.
		bool		mTSSARGTemp;					//!< Device supports D3DTA_TEMP for temporary register.

		// DX7-DX8
		bool		mCullCCW;						//!< Device supports CULL_CCW culling.
		bool		mCullCW;						//!< Device supports CULL_CW culling.
		bool		mCullNone;						//!< Device supports CULL_NONE culling.
		bool		mLinePatternRep;				//!< The driver can handle values other than 1 in the wRepeatFactor member of the D3DLINEPATTERN structure. (This applies only to line-drawing primitives)
		bool		mMaskZ;							//!< Device can enable and disable modification of the depth buffer on pixel operations.
	};

	//! Comparison capabilities.
	struct ICERENDERER_API CmpCaps
	{
		// DX7-DX8
		bool		mAlways;						//!< Always pass the z test.
		bool		mEqual;							//!< Pass the z test if the new z equals the current z.
		bool		mGreater;						//!< Pass the z test if the new z is greater than the current z.
		bool		mGreaterEqual;					//!< Pass the z test if the new z is greater than or equal to the current z.
		bool		mLess;							//!< Pass the z test if the new z is less than the current z.
		bool		mLessEqual;						//!< Pass the z test if the new z is less than or equal to the current z.
		bool		mNever;							//!< Always fail the z test.
		bool		mNotEqual;						//!< Pass the z test if the new z does not equal the current z.
	};

	//! Blending capabilities.
	struct ICERENDERER_API BlendCaps
	{
		// DX7-DX8
		bool		mBothInvSrcAlpha;				//!< Source blend factor is (1-As, 1-As, 1-As, 1-As), and destination blend factor is (As, As, As, As); the destination blend selection is overridden.
		bool		mBothSrcAlpha;					//!< The driver supports the BLEND_BOTHSRCALPHA blend mode. (This blend mode is obsolete)
		bool		mDestAlpha;						//!< Blend factor is (Ad, Ad, Ad, Ad).
		bool		mDestColor;						//!< Blend factor is (Rd, Gd, Bd, Ad).
		bool		mInvDestAlpha;					//!< Blend factor is (1-Ad, 1-Ad, 1-Ad, 1-Ad).
		bool		mInvDestColor;					//!< Blend factor is (1-Rd, 1-Gd, 1-Bd, 1-Ad).
		bool		mInvSrcAlpha;					//!< Blend factor is (1-As, 1-As, 1-As, 1-As).
		bool		mInvSrcColor;					//!< Blend factor is (1-Rd, 1-Gd, 1-Bd, 1-Ad).
		bool		mOne;							//!< Blend factor is (1, 1, 1, 1).
		bool		mSrcAlpha;						//!< Blend factor is (As, As, As, As).
		bool		mSrcAlphaSat;					//!< Blend factor is (f, f, f, 1); f = min(As, 1-Ad).
		bool		mSrcColor;						//!< Blend factor is (Rs, Gs, Bs, As).
		bool		mZero;							//!< Blend factor is (0, 0, 0, 0).
	};

	//! Shading operations capabilities.
	struct ICERENDERER_API ShadeCaps
	{
		// Shading operations capabilities. It is assumed, in general, that if a device supports a given command (such as D3DOP_TRIANGLE) at all, it supports the
		// D3DSHADE_FLAT mode (as specified in the D3DSHADEMODE enumerated type). This flag specifies whether the driver can also support Gouraud and Phong shading and whether
		// alpha color components are supported for each of the three color-generation modes. When alpha components are not supported in a given mode, the alpha value of colors
		// generated in that mode is implicitly 255. This is the maximum possible alpha (that is, the alpha component is at full intensity). Phong shading is not currently supported. 
		// With the monochromatic shading modes, the blue channel of the specular component is interpreted as a white intensity. (This is controlled by the
		// D3DRENDERSTATE_MONOENABLE render state.) 
		// The color, specular highlights, fog, and alpha interpolants of a triangle each have capability flags that an application can use to find out how they are implemented
		// by the device driver. These are modified by the shading mode, color model, and whether the alpha component of a color is blended or stippled. For more information,
		// see 3-D Primitives. 
		//
		// This member can be one or more of the following: 
		//	D3DPSHADECAPS_ALPHAFLATBLEND 
		//	D3DPSHADECAPS_ALPHAFLATSTIPPLED				Device can support an alpha component for flat blended and stippled transparency, respectively
		//												(the D3DSHADE_FLAT state for the D3DSHADEMODE enumerated type). In these modes, the alpha color component for a primitive
		//												is provided as part of the color for the first vertex of the primitive. 
		//	D3DPSHADECAPS_ALPHAGOURAUDBLEND 
		//	D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED			Device can support an alpha component for Gouraud-blended and stippled transparency, respectively
		//												(the D3DSHADE_GOURAUD state for the D3DSHADEMODE enumerated type). In these modes, the alpha color component of a primitive
		//												is provided at vertices and interpolated across a face along with the other color components. 
		//	D3DPSHADECAPS_ALPHAPHONGBLEND 
		//	D3DPSHADECAPS_ALPHAPHONGSTIPPLED			Device can support an alpha component for Phong-blended and stippled transparency, respectively
		//												(the D3DSHADE_PHONG state for the D3DSHADEMODE enumerated type). In these modes, vertex parameters are reevaluated on
		//												a per-pixel basis, applying lighting effects for the red, green, and blue color components. Phong shading is not currently
		//												supported. 
		//	D3DPSHADECAPS_COLORFLATMONO 
		//	D3DPSHADECAPS_COLORFLATRGB					Device can support colored flat shading in the D3DCOLOR_MONO and D3DCOLOR_RGB color models, respectively. In these modes,
		//												the color component for a primitive is provided as part of the color for the first vertex of the primitive. In the
		//												monochromatic lighting model, only the blue component of the color is interpolated; in the RGB lighting model, the red,
		//												green, and blue components are interpolated. 
		//	D3DPSHADECAPS_COLORGOURAUDMONO 
		//	D3DPSHADECAPS_COLORGOURAUDRGB				Device can support colored Gouraud shading in the D3DCOLOR_MONO and D3DCOLOR_RGB color models, respectively. In these modes,
		//												the color component for a primitive is provided at vertices and interpolated across a face along with the other color
		//												components. In the monochromatic lighting model, only the blue component of the color is interpolated; in the RGB lighting
		//												model, the red, green, and blue components are interpolated. 
		//	D3DPSHADECAPS_COLORPHONGMONO 
		//	D3DPSHADECAPS_COLORPHONGRGB					Device can support colored Phong shading in the D3DCOLOR_MONO and D3DCOLOR_RGB color models, respectively. In these modes,
		//												vertex parameters are reevaluated on a per-pixel basis. Lighting effects are applied for the red, green, and blue color
		//												components in the RGB model, and for the blue component only for the monochromatic model. Phong shading is not currently
		//												supported. 
		//	D3DPSHADECAPS_FOGFLAT 
		//	D3DPSHADECAPS_FOGGOURAUD 
		//	D3DPSHADECAPS_FOGPHONG						Device can support fog in the flat, Gouraud, and Phong shading modes, respectively. Phong shading is not currently supported. 
		//	D3DPSHADECAPS_SPECULARFLATMONO 
		//	D3DPSHADECAPS_SPECULARFLATRGB				Device can support specular highlights in flat shading in the D3DCOLOR_MONO and D3DCOLOR_RGB color models, respectively. 
		//	D3DPSHADECAPS_SPECULARGOURAUDMONO 
		//	D3DPSHADECAPS_SPECULARGOURAUDRGB			Device can support specular highlights in Gouraud shading in the D3DCOLOR_MONO and D3DCOLOR_RGB color models, respectively. 
		//	D3DPSHADECAPS_SPECULARPHONGMONO 
		//	D3DPSHADECAPS_SPECULARPHONGRGB				Device can support specular highlights in Phong shading in the D3DCOLOR_MONO and D3DCOLOR_RGB color models, respectively.
		//												Phong shading is not currently supported.

					ShadeCaps();
		// DX7
		bool		mAlphaFlatBlend;
		bool		mAlphaFlatStippled;
		bool		mAlphaGouraudStippled;
		bool		mAlphaPhongBlend;
		bool		mAlphaPhongStippled;
		bool		mColorFlatMono;
		bool		mColorFlatRGB;
		bool		mColorGouraudMono;
		bool		mColorPhongMono;
		bool		mColorPhongRGB;
		bool		mFogFlat;
		bool		mFogPhong;
		bool		mSpecularFlatMono;
		bool		mSpecularFlatRGB;
		bool		mSpecularGouraudMono;
		bool		mSpecularPhongMono;
		bool		mSpecularPhongRGB;

		// DX7-DX8
		bool		mAlphaGouraudBlend;
		bool		mColorGouraudRGB;
		bool		mFogGouraud;
		bool		mSpecularGouraudRGB;
	};

	//! Raster-drawing capabilities.
	struct ICERENDERER_API RasterCaps
	{
		// Information on raster-drawing capabilities. This member can be one or more of the following: 
		//	D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT		The device supports antialiasing that is dependent on the sort order of the polygons (back-to-front or front-to-back).
		//												The application must draw polygons in the right order for antialiasing to occur. For more information, see the
		//												D3DANTIALIASMODE enumerated type. 
		//	D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT		The device supports antialiasing that is not dependent on the sort order of the polygons. For more information,
		//												see the D3DANTIALIASMODE enumerated type. 
		//	D3DPRASTERCAPS_FOGRANGE						The device supports range-based fog. In range-based fog, the distance of an object from the viewer is used to
		//												compute fog effects, not the depth of the object (that is, the z-coordinate) in the scene. For more information,
		//												see Range-based Fog. 
		//	D3DPRASTERCAPS_FOGTABLE						The device calculates the fog value by referring to a lookup table containing fog values that are indexed to the
		//												depth of a given pixel. 
		//	D3DPRASTERCAPS_FOGVERTEX					The device calculates the fog value during the lighting operation, places the value into the alpha component of the
		//												D3DCOLOR value given for the specular member of the D3DTLVERTEX structure and interpolates the fog value during rasterization. 
		//	D3DPRASTERCAPS_MIPMAPLODBIAS				The device supports level-of-detail (LOD) bias adjustments. These bias adjustments enable an application to make a
		//												mipmap appear crisper or less sharp than it normally would. For more information about LOD bias in mipmaps, see
		//												D3DTSS_MIPMAPLODBIAS. 
		//	D3DPRASTERCAPS_PAT							The driver can perform patterned drawing (lines or fills with D3DRENDERSTATE_LINEPATTERN or one of the
		//												D3DRENDERSTATE_STIPPLEPATTERN render states) for the primitive being queried. 
		//	D3DPRASTERCAPS_ROP2							The device can support raster operations other than R2_COPYPEN. 
		//	D3DPRASTERCAPS_STIPPLE						The device can stipple polygons to simulate translucency. 
		//	D3DPRASTERCAPS_SUBPIXEL						The device performs subpixel placement of z, color, and texture data, rather than working with the nearest integer
		//												pixel coordinate. This helps avoid bleed-through due to z imprecision and jitter of color and texture values for pixels.
		//												There is no corresponding state that can be enabled and disabled; the device either performs subpixel placement, or it
		//												does not. This bit is present only so that the you can better determine what the rendering quality will be. 
		//	D3DPRASTERCAPS_SUBPIXELX					The device is subpixel-accurate along the x-axis only and is clamped to an integer y-axis scan line. For information about
		//												subpixel accuracy, see D3DPRASTERCAPS_SUBPIXEL. 
		//	D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT	The device supports translucency that is not dependent on the sort order of the polygons. For more information, see
		//												D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT. 
		//	D3DPRASTERCAPS_WBUFFER						The device supports depth buffering using w. For more information, see Depth Buffers. 
		//	D3DPRASTERCAPS_WFOG							The device supports w-based fog. W-based fog is used when a perspective projection matrix is specified, but affine
		//												projections still use z-based fog. The system considers a projection matrix that contains a nonzero value in the [3][4]
		//												element to be a perspective projection matrix. 
		//	D3DPRASTERCAPS_XOR							The device can support XOR operations. If this flag is not set but D3DPRIM_RASTER_ROP2 is set, XOR operations must still
		//												be supported. 
		//	D3DPRASTERCAPS_ZBIAS						The device supports z-bias values. These are integer values assigned to polygons that allow physically coplanar polygons
		//												to appear separate. For more information, see D3DRENDERSTATE_ZBIAS in the D3DRENDERSTATETYPE enumerated type. 
		//	D3DPRASTERCAPS_ZBUFFERLESSHSR				The device can perform hidden-surface removal (HSR) without requiring the application to sort polygons and without
		//												requiring the allocation of a depth-buffer. This leaves more video memory for textures. The method used to perform
		//												hidden-surface removal is hardware-dependent and is transparent to the application. 
		//												Z-bufferless HSR is performed if no depth-buffer surface is attached to the rendering-target surface and the depth-buffer
		//												comparison test is enabled (that is, when the state value associated with the D3DRENDERSTATE_ZENABLE enumeration constant
		//												is set to TRUE). 
					RasterCaps();
		// DX7
		bool		mAntialiasSortDependent;
		bool		mAntialiasSortIndependent;
		bool		mRop2;
		bool		mStipple;
		bool		mSubpixel;
		bool		mSubpixelX;
		bool		mTranslucentSortIndependent;
		bool		mXor;

		// DX8
		bool		mColorPerspective;				//!< Device iterates colors perspective correct. 
		bool		mStretchBltMultiSample;			//!< Device provides limited multisample support through a stretch-blt implementation.
													//!< When this capability is set, D3DRS_MULTISAMPLEANTIALIAS cannot be turned on and off in
													//!< the middle of a scene. Multisample masking cannot be performed if this flag is set.
		// DX7-DX8
		bool		mAnisotropy;					//!< Device supports anisotropic filtering.
		bool		mAntialiasEdges;				//!< Device can anti-alias lines forming the convex outline of objects. For more information, see DRS_EDGEANTIALIAS.
		bool		mDither;						//!< Device can dither to improve color resolution.
		bool		mFogRange;
		bool		mFogTable;
		bool		mFogVertex;
		bool		mMipMapLodBias;
		bool		mPat;
		bool		mWBuffer;
		bool		mWFog;
		bool		mZBias;
		bool		mZBufferlessHSR;
		bool		mZFog;							//!< Device supports z-based fog.
		bool		mZTest;							//!< Device can perform z-test operations. This effectively renders a primitive and indicates
													//!< whether any z pixels have been rendered.
	};

	//! Miscellaneous texture-mapping capabilities.
	struct ICERENDERER_API TextureCaps
	{
		//	D3DPTEXTURECAPS_ALPHA						Supports RGBA textures in the D3DTBLEND_DECAL and D3DTBLEND_MODULATE texture filtering modes. If this
		//												capability is not set, only RGB textures are supported in those modes. Regardless of the setting of this
		//												flag, alpha must always be supported in D3DTBLEND_DECALMASK, D3DTBLEND_DECALALPHA, and D3DTBLEND_MODULATEALPHA
		//												filtering modes whenever those filtering modes are available.
		//	D3DPTEXTURECAPS_ALPHAPALETTE				Supports palettized texture surfaces whose palettes contain alpha information (see DDPCAPS_ALPHA in the
		//												DDCAPS structure).
		//	D3DPTEXTURECAPS_BORDER						Superseded by D3DPTADDRESSCAPS_BORDER.
		//	D3DPTEXTURECAPS_COLORKEYBLEND				The device supports alpha-blended colorkeying through the use of the D3DRENDERSTATE_COLORKEYBLENDENABLE
		//												render state.
		//	D3DPTEXTURECAPS_CUBEMAP						Supports cubic environment mapping. This capability flag was introduced with DirectX 7.0
		//	D3DPTEXTURECAPS_NONPOW2CONDITIONAL			Conditionally supports the use of textures with dimensions that are not powers of 2. A device that
		//												exposes this capability can use such a texture if all of the following requirements are met.
		//												The texture addressing mode for the texture stage is set to D3DTADDRESS_CLAMP.
		//												Texture wrapping for the texture stage is disabled (D3DRENDERSTATE_WRAPn set to 0).
		//												Mipmapping is not in use. (Mipmapped textures must have dimensions that are powers of 2.)
		//	D3DPTEXTURECAPS_PERSPECTIVE					Perspective correction is supported.
		//	D3DPTEXTURECAPS_POW2						All nonmipmapped textures must have widths and heights specified as powers of 2. (Mipmapped textures
		//												must always have dimensions that are powers of 2.)
		//	D3DPTEXTURECAPS_PROJECTED					Supports the D3DTTFF_PROJECTED texture transformation flag. When applied, the device divides transformed
		//												texture coordinates by the last texture coordinate.
		//	D3DPTEXTURECAPS_SQUAREONLY					All textures must be square.
		//	D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE	Texture indices are not scaled by the texture size prior to interpolation.
		//	D3DPTEXTURECAPS_TRANSPARENCY				Texture transparency is supported. (Only those texels that are not the current transparent color are drawn.)

					TextureCaps();
		// DX7
		bool		mBorder;
		bool		mColorKeyBlend;
		bool		mTransparency;

		// DX8
		bool		mCubemapPow2;					//!< Device requires that cube texture maps have dimensions specified as powers of 2.
		bool		mMipCubemap;					//!< Device supports mipmapped cube textures.
		bool		mMipmap;						//!< Device supports mipmapped textures.
		bool		mMipVolumeMap;					//!< Device supports mipmapped volume textures.
		bool		mVolumeMap;						//!< Device supports volume textures.
		bool		mVolumeMapPow2;					//!< Device requires that volume texture maps have dimensions specified as powers of 2.

		// DX7-DX8
		bool		mAlpha;
		bool		mAlphaPalette;
		bool		mCubeMap;
		bool		mNonPow2Conditional;
		bool		mPerspective;
		bool		mPow2;
		bool		mProjected;
		bool		mSquareOnly;
		bool		mTexRepeatNotScaledBySize;
	};

	//! Texture-filtering capabilities.
	struct ICERENDERER_API TexFilterCaps
	{
		// Texture-filtering capabilities. General texture-filtering flags reflect which texture-filtering modes are supported and can be set for the D3DTSS_MAGFILTER,
		// D3DTSS_MINFILTER, or D3DTSS_MIPFILTER texture-stage states. Per-stage filtering capabilities reflect which filtering modes are supported for texture stages when
		// performing multiple-texture blending with the IDirect3DDevice7 interface. This member can be any combination of the following general and per-stage texture-filtering
		// flags: 
		// General texture-filtering flags 
		//	D3DPTFILTERCAPS_LINEAR						Bilinear filtering. Chooses the texel that has the nearest coordinates, then performs a weighted average with the four
		//												surrounding texels to determine the final color. This applies to both zooming in and zooming out. If either zooming in or
		//												zooming out is supported, both must be supported. 
		//	D3DPTFILTERCAPS_LINEARMIPLINEAR				Trilinear interpolation between mipmaps. Performs bilinear filtering on the two nearest mipmaps, then interpolates linearly
		//												between the two colors to determine a final color. 
		//	D3DPTFILTERCAPS_LINEARMIPNEAREST			Linear interpolation between two point-sample mipmaps. Chooses the nearest texel from the two closest mipmap levels,
		//												then performs linear interpolation between them. 
		//	D3DPTFILTERCAPS_MIPLINEAR					Nearest mipmapping, with bilinear filtering applied to the result. Chooses the texel from the appropriate mipmap that has
		//												the nearest coordinates, then performs a weighted average with the four surrounding texels to determine the final color. 
		//	D3DPTFILTERCAPS_MIPNEAREST					Nearest mipmapping. Chooses the texel from the appropriate mipmap with coordinates nearest to the desired pixel value. 
		//	D3DPTFILTERCAPS_NEAREST						Point sampling. The texel with coordinates nearest to the desired pixel value is used. This applies to both zooming in and
		//												zooming out. If either zooming in or zooming out is supported, both must be supported. 
		// Per-stage texture filtering flags 
		//	D3DPTFILTERCAPS_MAGFAFLATCUBIC				The device supports per-stage flat cubic filtering for magnifying textures. The flat cubic magnification filter is
		//												represented by the D3DTFG_FLATCUBIC member of the D3DTEXTUREMAGFILTER enumerated type. 
		//	D3DPTFILTERCAPS_MAGFANISOTROPIC				The device supports per-stage anisotropic filtering for magnifying textures. The anisotropic magnification filter is
		//												represented by the D3DTFG_ANISOTROPIC member of the D3DTEXTUREMAGFILTER enumerated type. 
		//	D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC			The device supports the per-stage Gaussian cubic filtering for magnifying textures. The Gaussian cubic magnification filter
		//												is represented by the D3DTFG_GAUSSIANCUBIC member of the D3DTEXTUREMAGFILTER enumerated type. 
		//	D3DPTFILTERCAPS_MAGFLINEAR					The device supports per-stage bilinear interpolation filtering for magnifying textures. The bilinear interpolation
		//												magnification filter is represented by the D3DTFG_LINEAR member of the D3DTEXTUREMAGFILTER enumerated type. 
		//	D3DPTFILTERCAPS_MAGFPOINT					The device supports per-stage point-sample filtering for magnifying textures. The point-sample magnification filter is
		//												represented by the D3DTFG_POINT member of the D3DTEXTUREMAGFILTER enumerated type. 
		//	D3DPTFILTERCAPS_MINFANISOTROPIC				The device supports per-stage anisotropic filtering for minifying textures. The anisotropic minification filter is
		//												represented by the D3DTFN_ANISOTROPIC member of the D3DTEXTUREMINFILTER enumerated type. 
		//	D3DPTFILTERCAPS_MINFLINEAR					The device supports per-stage bilinear interpolation filtering for minifying textures. The bilinear minification filter is
		//												represented by the D3DTFN_LINEAR member of the D3DTEXTUREMINFILTER enumerated type. 
		//	D3DPTFILTERCAPS_MINFPOINT					The device supports per-stage point-sample filtering for minifying textures. The point-sample minification filter is
		//												represented by the D3DTFN_POINT member of the D3DTEXTUREMINFILTER enumerated type. 
		//	D3DPTFILTERCAPS_MIPFLINEAR					The device supports per-stage trilinear interpolation filtering for mipmaps. The trilinear interpolation mipmapping filter is
		//												represented by the D3DTFP_LINEAR member of the D3DTEXTUREMIPFILTER enumerated type. 
		//	D3DPTFILTERCAPS_MIPFPOINT					The device supports per-stage point-sample filtering for mipmaps. The point-sample mipmapping filter is represented by the
		//												D3DTFP_POINT member of the D3DTEXTUREMIPFILTER enumerated type. 

					TexFilterCaps();
		// DX7
		bool		mLinear;
		bool		mLinearMipLinear;
		bool		mLinearMipNearest;
		bool		mMipLinear;
		bool		mMipNearest;
		bool		mNearest;

		// DX7-DX8
		bool		mMagfAFlatCubic;
		bool		mMagfAnisotropic;
		bool		mMagfGaussianCubic;
		bool		mMagfLinear;
		bool		mMagfPoint;
		bool		mMinfAnisotropic;
		bool		mMinfLinear;
		bool		mMinfPoint;
		bool		mMipfLinear;
		bool		mMipfPoint;
	};

	//! Texture-addressing capabilities.
	struct ICERENDERER_API TexAddressCaps
	{
					TexAddressCaps();
		// DX7-DX8
		bool		mBorder;						//!< Device supports setting coordinates outside the range [0.0, 1.0] to the border color, as specified by the D3DTSS_BORDERCOLOR texture-stage state. 
		bool		mClamp;							//!< Device can clamp textures to addresses.
		bool		mIndependentUV;					//!< Device can separate the texture-addressing modes of the u and v coordinates of the texture. This ability corresponds to the D3DTSS_ADDRESSU and D3DTSS_ADDRESSV render-state values.
		bool		mMirror;						//!< Device can mirror textures to addresses.
		bool		mWrap;							//!< Device can wrap textures to addresses.

		// DX8
		bool		mMirrorOnce;					//!< Device can take the absolute value of the texture coordinate (thus, mirroring around 0), and then clamp to the maximum value.
	};

	//! Capabilities for line-drawing primitives.
	struct ICERENDERER_API LineCaps
	{
					LineCaps();
		// DX8
		bool		mAlphaCmp;						//!< Supports alpha-test comparisons.
		bool		mBlend;							//!< Supports source-blending.
		bool		mFog;							//!< Supports fog.
		bool		mTexture;						//!< Supports texture-mapping.
		bool		mZTest;							//!< Supports z-buffer comparisons.
	};

	//! Stencil-buffer operations.
	//! Stencil operations are assumed to be valid for all three stencil-buffer operation render states.
	struct ICERENDERER_API StencilOp
	{
		// DX7-DX8
		bool		mDecr;							//!< The STENCILOP_DECR		operation is supported.
		bool		mDecrSat;						//!< The STENCILOP_DECRSAT	operation is supported.
		bool		mIncr;							//!< The STENCILOP_INCR		operation is supported.
		bool		mIncrSat;						//!< The STENCILOP_INCRSAT	operation is supported.
		bool		mInvert;						//!< The STENCILOP_INVERT	operation is supported.
		bool		mKeep;							//!< The STENCILOP_KEEP		operation is supported.
		bool		mReplace;						//!< The STENCILOP_REPLACE	operation is supported.
		bool		mZero;							//!< The STENCILOP_ZERO		operation is supported.
	};

	//! Flexible vertex format capabilities.
	struct ICERENDERER_API FVFCaps
	{
		//	D3DFVFCAPS_DONOTSTRIPELEMENTS				It is preferable that vertex elements not be stripped. That is, if the vertex format contains elements
		//												that are not used with the current render states, there is no need to regenerate the vertices. If this
		//												capability flag is not present, stripping extraneous elements from the vertex format provides better
		//												performance.
		//	D3DFVFCAPS_TEXCOORDCOUNTMASK				Masks the low WORD of dwFVFCaps. These bits, cast to the WORD data type, describe the total number of
		//												texture coordinate sets that the device can simultaneously use for multiple texture blending. (You can
		//												use up to eight texture coordinate sets for any vertex, but the device can only blend using the specified
		//												number of texture coordinate sets.) 
					FVFCaps();
		// DX7-DX8
		bool		mDoNotStripleElements;
		udword		mTexCoordCountMask;

		// DX8
		udword		mPSize;
//The absence of D3DFVFCAPS_PSIZE indicates that the device does not support D3DFVF_PSIZE for pre-transformed vertices. In this case the base point size always comes fromD3DRS_POINTSIZE. This capability applies to fixed-function vertex processing in software only. D3DFVF_PSIZE is always supported when doing software vertex processing. 
//The output point size written by a vertex shader is always supported, and for vertex shaders any input can contribute to the output point size. 
//D3DFVF_PSIZE is always supported for post-transformed vertices. 
	};

	//! Texture operations.
	struct ICERENDERER_API TexOp
	{
					TexOp();
		// DX7-DX8
		bool		mAdd;							//!< The STAGEOP_ADD						texture-blending operation is supported. 
		bool		mAddSigned;						//!< The STAGEOP_ADDSIGNED					texture-blending operation is supported. 
		bool		mAddSigned2X;					//!< The STAGEOP_ADDSIGNED2X				texture-blending operation is supported. 
		bool		mAddSmooth;						//!< The STAGEOP_ADDSMOOTH					texture-blending operation is supported. 
		bool		mBlendCurrentAlpha;				//!< The STAGEOP_BLENDCURRENTALPHA			texture-blending operation is supported. 
		bool		mBlendDiffuseAlpha;				//!< The STAGEOP_BLENDDIFFUSEALPHA			texture-blending operation is supported. 
		bool		mBlendFactorAlpha;				//!< The STAGEOP_BLENDFACTORALPHA			texture-blending operation is supported. 
		bool		mBlendTextureAlpha;				//!< The STAGEOP_BLENDTEXTUREALPHA			texture-blending operation is supported. 
		bool		mBlendTextureAlphaPM;			//!< The STAGEOP_BLENDTEXTUREALPHAPM		texture-blending operation is supported. 
		bool		mBumpEnvMap;					//!< The STAGEOP_BUMPENVMAP					texture-blending operation is supported. 
		bool		mBumpEnvMapLuminance;			//!< The STAGEOP_BUMPENVMAPLUMINANCE		texture-blending operation is supported. 
		bool		mDisable;						//!< The STAGEOP_DISABLE					texture-blending operation is supported. 
		bool		mDotProduct3;					//!< The STAGEOP_DOTPRODUCT3				texture-blending operation is supported. 
		bool		mModulate;						//!< The STAGEOP_MODULATE					texture-blending operation is supported. 
		bool		mModulate2X;					//!< The STAGEOP_MODULATE2X					texture-blending operation is supported. 
		bool		mModulate4X;					//!< The STAGEOP_MODULATE4X					texture-blending operation is supported. 
		bool		mModulateAlpha_AddColor;		//!< The STAGEOP_MODULATEALPHA_ADDCOLOR		texture-blending operation is supported. 
		bool		mModulateColor_AddAlpha;		//!< The STAGEOP_MODULATECOLOR_ADDALPHA		texture-blending operation is supported. 
		bool		mModulateInvAlpha_AddColor;		//!< The STAGEOP_MODULATEINVALPHA_ADDCOLOR	texture-blending operation is supported. 
		bool		mModulateInvColor_AddAlpha;		//!< The STAGEOP_MODULATEINVCOLOR_ADDALPHA	texture-blending operation is supported. 
		bool		mPremodulate;					//!< The STAGEOP_PREMODULATE				texture-blending operation is supported. 
		bool		mSelectArg1;					//!< The STAGEOP_SELECTARG1					texture-blending operation is supported. 
		bool		mSelectArg2;					//!< The STAGEOP_SELECTARG2					texture-blending operation is supported. 
		bool		mSubtract;						//!< The STAGEOP_SUBTRACT					texture-blending operation is supported. 

		// DX8
		bool		mLerp;							//!< The STAGEOP_LERP						texture-blending operation is supported.
		bool		mMultiplyAdd;					//!< The STAGEOP_MULTIPLYADD				texture-blending operation is supported. 
	};

	//! Vertex processing capabilities.
	struct ICERENDERER_API VtxProcCaps
	{
					VtxProcCaps();
		// DX7
		bool		mVertexFog;						//!< Device supports vertex fog.

		// DX7-DX8
		bool		mDirectionalLights;				//!< Device supports directional lights.
		bool		mLocalViewer;					//!< Device supports local viewer. (Device supports orthogonal specular highlights, enabled by setting the D3DRENDERSTATE_LOCALVIEWER render state to FALSE.)
		bool		mMaterialSource;				//!< Device supports selectable vertex color sources.
		bool		mPositionalLights;				//!< Device supports positional lights (including point lights and spotlights).
		bool		mTexGen;						//!< Device can generate texture coordinates.

		// DX8
		bool		mTweening;						//!< Device supports vertex tweening.
		bool		mNoVSDT_UBYTE4;					//!< Device does not support the D3DVSDT_UBYTE4 vertex declaration type.
	};

	//! Guard band
	//! The screen-space coordinates of the guard-band clipping region.
	//! Coordinates inside this rectangle but outside the viewport rectangle are automatically clipped. 
	struct ICERENDERER_API GuardBand
	{
		// DX7-DX8
		float		mLeft;
		float		mTop;
		float		mRight;
		float		mBottom;
	};

	//! Vertex shader capabilities
	struct ICERENDERER_API VertexShaderCaps
	{
					VertexShaderCaps();
		// DX8
		udword		mVertexShaderVersion;			//!< Vertex shader version, indicating the level of vertex shader supported by the device.
													//!< Only vertex shaders with version numbers equal to or less than this will succeed in calls to
													//!< IDirect3DDevice8::CreateVertexShader. The level of shader is specified to CreateVertexShader
													//!< as the first token in the vertex shader token stream.
													//!< DirectX 7.0 functionality is 0
													//!< DirectX 8.0 functionality is 01
													//!< The main version number is encoded in the second byte. The low byte contains a sub-version number.
		udword		mMaxVertexShaderConst;			//!< Number of vertex shader constant registers.
	};

	//! Pixel shader capabilities
	struct ICERENDERER_API PixelShaderCaps
	{
					PixelShaderCaps();
		// DX8
		udword		mPixelShaderVersion;			//!< Pixel shader version, indicating the level of pixel shader supported by the device.
													//!< Only pixel shaders with version numbers equal to or less than this will succeed in calls to
													//!< IDirect3DDevice8::CreatePixelShader. DirectX 8.0 functionality is 01. The main version number is
													//!< encoded in the second byte. The low byte contains a sub-version number.
		float		mMaxPixelShaderValue;			//!< Maximum value of pixel shader arithmetic component. This value indicates the internal range of
													//!< values supported for pixel color blending operations. Within the range that they report to,
													//!< implementations must allow data pass through pixel processing unmodified (unclamped). Normally,
													//!< the value of this member is an absolute value. For example, a 1.0 indicates that the range is -1.0
													//!< to 1, and an 8.0 indicates that the range is -8.0 to 8.0. Note that the value 0.0 indicates that
													//!< no signed range is supported; therefore, the range is 0 to 1.0 as in DirectX 6.0 and 7.0
	};





	//! Renderer generic caps
	struct ICERENDERER_API RenderCaps
	{
							RenderCaps();

		void				LogCaps();

		udword				GetNbTextureStages();

		DriverCaps			mDriverCaps;
		DeviceCaps			mDeviceCaps;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		MiscCaps			mMiscCaps;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		RasterCaps			mRasterCaps;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		CmpCaps				mZCmpCaps;
		CmpCaps				mAlphaCmpCaps;
		// Alpha-test comparison capabilities. This member can include the same capability flags defined for the dwZCmpCaps member. If this member contains only the
		// D3DPCMPCAPS_ALWAYS capability or only the D3DPCMPCAPS_NEVER capability, the driver does not support alpha tests. Otherwise, the flags identify the individual
		// comparisons that are supported for alpha testing.

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		BlendCaps			mSrcBlendCaps;		//!< Source-blending capabilities.
		BlendCaps			mDestBlendCaps;		//!< Destination-blending capabilities.

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		ShadeCaps			mShadeCaps;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		TextureCaps			mTextureCaps;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		TexFilterCaps		mTexmapFilterCaps;
		TexFilterCaps		mCubemapFilterCaps;
		TexFilterCaps		mVolumemapFilterCaps;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		TexAddressCaps		mTexmapAddressCaps;
		TexAddressCaps		mVolumemapAddressCaps;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		LineCaps			mLineCaps;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		StencilOp			mStencilOp;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		FVFCaps				mFVFCaps;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		TexOp				mTexOp;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		VtxProcCaps			mVtxProcCaps;

		GuardBand			mGuardBand;

		VertexShaderCaps	mVertexShaderCaps;
		PixelShaderCaps		mPixelShaderCaps;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// DX7-DX8
		udword				mMaxTextureWidth;						//!< Maximumm texture width for this device.
		udword				mMaxTextureHeight;						//!< Maximumm texture height for this device.
		udword				mMaxTextureAspectRatio;					//!< Maximum texture aspect ratio supported by the hardware; this is typically a power of 2.
		udword				mMaxAnisotropy;							//!< Maximum valid value for the D3DTSS_MAXANISOTROPY texture-stage state.

		// Full range of the integer bits of the post-normalized texture indices. If the D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE bit is set, the device defers
		// scaling by the texture size until after the texture address mode is applied. If not set, the device scales the texture indices by the texture size
		// (largest level of detail) prior to interpolation.
		udword				mMaxTextureRepeat;

		float				mExtentsAdjust;							//!< Number of pixels to adjust the extents rectangle outward to accommodate anti-aliasing kernels.

		udword				mMaxTextureBlendStages;				//!< Maximum number of texture-blending stages supported.
//Maximum number of texture-blending stages supported. This value is the number of blenders available. In the DirectX 8.0 programmable pixel pipeline, this should correspond to the number of instructions supported by pixel shaders on this particular implementation. 

		udword				mMaxSimultaneousTextures;			//!< Maximum number of textures that can be simultaneously bound to the texture blending stages.
//Maximum number of textures that can be simultaneously bound to the texture blending stages. This value is the number of textures that can be used in a single pass. In the DirectX 8.0 programmable pixel pipeline, this indicates the number of texture registers supported by pixel shaders on this particular piece of hardware, and the number of texture declaration instructions that can be present. 

		udword				mMaxActiveLights;					// Maximum number of lights that can be active simultaneously.
//Maximum number of lights that can be active simultaneously. For a given physical device, this capability might vary across Direct3D Device Objects depending on the parameters supplied to IDirect3D8::CreateDevice. 

		udword				mMaxUserClipPlanes;					// Maximum number of user-defined clipping planes supported. This member can range from 0 through D3DMAXUSERCLIPPLANES.
//Maximum number of user-defined clipping planes supported. This member can range from 0 through D3DMAXUSERCLIPPLANES. For a given physical device, this capability may vary across Direct3D Device Objects depending on the parameters supplied to IDirect3D8::CreateDevice. 

		udword				mMaxVertexBlendMatrices;				// Maximum number of matrices that this device can apply when performing multimatrix vertex blending.
//Maximum number of matrices that this device can apply when performing multimatrix vertex blending. For a given physical device, this capability may vary across Direct3D Device Objects depending on the parameters supplied to IDirect3D8::CreateDevice. 

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// DX7
		udword				mDeviceRenderBitDepth;					// Device's rendering bit depth. This can be one or more of the following DirectDraw bit-depth
															// constants: DDBD_8, DDBD_16, DDBD_24, or DDBD_32.
		udword				mDeviceZBufferBitDepth;					// Bit depth of the device's depth-buffer. This can be one of the following DirectDraw bit-depth
															// constants: DDBD_8, DDBD_16, DDBD_24, or DDBD_32.
		udword				mMinTextureWidth;						// Minimum texture width for this device.
		udword				mMinTextureHeight;						// Minimum texture height for this device.

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		float				mMaxVertexW;							// Maximum W-based depth value that the device supports.

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Maximum width and height of the supported stipple (up to 32×32). 
		udword				mStippleWidth;
		udword				mStippleHeight;

		// DX8

		// Maximum volume extent.
		udword				mMaxVolumeExtent;

		// DWORD value that specifies the maximum matrix index that can be indexed into using the per-vertex indices. The number of matrices is
		// MaxVertexBlendMatrixIndex + 1, which is the size of the matrix palette. If normals are present in the vertex data that needs to be blended
		// for lighting, then the number of matrices is half the number specified by this capability flag. If MaxVertexBlendMatrixIndex is set to zero,
		// the driver does not support indexed vertex blending. If this value is not zero then the valid range of indices is zero through
		// MaxVertexBlendIndexedMatrices.
		//
		// A zero value for MaxVertexBlendMatrixIndex indicates that the driver does not support indexed matrices.
		// When software vertex processing is used, 256 matrices could be used for indexed vertex blending, with or without normal blending.
		// For a given physical device, this capability may vary across Direct3D Device Objects depending on the parameters supplied to IDirect3D8::CreateDevice.
		udword				mMaxVertexBlendMatrixIndex;

		// Maximum size of a point primitive. If set to 1.0f then device does not support point size control. The range is greater than or equal to 1.0f.
		float				mMaxPointSize;

		// Maximum number of primitives for each DrawPrimitive call.
		udword				mMaxPrimitiveCount;

		// Maximum size of indices supported for hardware vertex processing. It is possible to create 32-bit index buffers by specifying D3DFMT_INDEX32;
		// however, you will not be able to render with the index buffer unless this value is greater than 0x0000FFFF.
		udword				mMaxVertexIndex;

		// Maximum number of concurrent data streams for IDirect3DDevice8::SetStreamSource. The valid range is 1 to 16. Note that if this value is 0,
		// then the driver is not a DirectX 8.0 driver.
		udword				mMaxStreams;

		// Maximum stride for IDirect3DDevice8::SetStreamSource.
		udword				mMaxStreamStride;

		// ICE enhanced caps
		udword				mTextureMatrixTranslationRow;
		BOOL				mProjectorInverseView;
		BOOL				mZSymmetry;
	};

#endif // ICERENDERCAPS_H
