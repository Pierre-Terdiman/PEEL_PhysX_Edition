///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains lists of render states. Supposed to be the same IDs for DX7, DX8, GL, etc
 *	\file		IceRenderStates.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERENDERSTATES_H
#define ICERENDERSTATES_H

	//! Boolean-controlled render states
	enum BooleanRenderState
	{
		// Standard set
		BRS_TEXTUREPERSPECTIVE					= (1<<0),	BRS_ZWRITE							= (1<<1),
		BRS_ALPHATEST							= (1<<2),	BRS_DITHERING						= (1<<3),
		BRS_ALPHABLENDING						= (1<<4),	BRS_FOG								= (1<<5),
		BRS_SPECULAR							= (1<<6),	BRS_STENCILTEST						= (1<<7),
		BRS_LIGHTING							= (1<<8),	BRS_TEXTURING						= (1<<9),
		BRS_LASTPIXEL							= (1<<10),	BRS_STIPPLEDALPHA					= (1<<11),
		BRS_EDGEANTIALIAS						= (1<<12),	BRS_COLORKEYMODE					= (1<<13),
		BRS_COLORKEYBLENDMODE					= (1<<14),	BRS_RANGEBASEDFOG					= (1<<15),
		BRS_CLIPPING							= (1<<16),	BRS_SCREENEXTENTS					= (1<<17),
		BRS_COLORVERTEX							= (1<<18),	BRS_PERSPSPECHIGH					= (1<<19),
		BRS_NORMALIZE							= (1<<20),
		// New set
		BRS_SOFTWAREVERTEXPROCESSING			= (1<<21),	BRS_POINTSPRITE						= (1<<22),
		BRS_POINTSCALE							= (1<<23),	BRS_MULTISAMPLEANTIALIAS			= (1<<24),
		BRS_INDEXEDVERTEXBLEND					= (1<<25),

		BRS_FORCE_DWORD							= 0x7fffffff
	};

	inline_ udword GetBRSIndex(BooleanRenderState brs)
	{
		return CodeSize2(udword(brs)) - 1;
	}

	//! Dword-controlled render states
	enum DwordRenderState
	{
		// Standard set
		DRS_AMBIENTCOLOR						= 0,		DRS_ANTIALIASMODE					= 1,
		DRS_ZBUFFERMODE							= 2,		DRS_ZBUFFERCMPMODE					= 3,
		DRS_ZBIAS								= 4,		DRS_LINEPATTERN						= 5,
		DRS_ALPHAREF							= 6,		DRS_ALPHAFUNC						= 7,
		DRS_SRCBLEND							= 8,		DRS_DESTBLEND						= 9,
		DRS_DIFFUSEMATSRC						= 10,		DRS_SPECULARMATSRC					= 11,
		DRS_AMBIENTMATSRC						= 12,		DRS_EMISSIVEMATSRC					= 13,
		DRS_VERTEXBLEND							= 14,		DRS_TEXTUREFACTOR					= 15,
		DRS_FILLMODE							= 16,		DRS_SHADEMODE						= 17,
		DRS_CULLMODE							= 18,		DRS_STENCILREF						= 19,
		DRS_STENCILCMPMODE						= 20,		DRS_STENCILCMPMASK					= 21,
		DRS_STENCILWRITEMASK					= 22,		DRS_STENCILFAILOP					= 23,
		DRS_STENCILZFAILOP						= 24,		DRS_STENCILPASSOP					= 25,
		DRS_FOGVERTEXMODE						= 26,		DRS_FOGMODE							= 27,
		DRS_FOGDENSITY							= 28,		DRS_FOGCOLOR						= 29,
		DRS_FOGSTART							= 30,		DRS_FOGEND							= 31,
		DRS_MATERIAL							= 32,
		// New set
		DRS_POINTSIZE							= 33,		DRS_POINTSIZEMIN					= 34,
		DRS_POINTSCALEA							= 35,		DRS_POINTSCALEB						= 36,
		DRS_POINTSCALEC							= 37,		DRS_MULTISAMPLEMASK					= 38,
		DRS_PATCHEDGESTYLE						= 39,		DRS_PATCHSEGMENTS					= 40,
		//debug monitor token					41
		DRS_POINTSIZEMAX						= 42,		DRS_COLORWRITEENABLE				= 43,
		DRS_TWEENFACTOR							= 44,		DRS_BLENDOP							= 45,
		//
		DRS_VERTEXSHADER						= 46,		DRS_PIXELSHADER						= 47,
		DRS_VERTEXDECLARATION					= 48,

		// Total size
		DRS_SIZE								= 49,		// Last one + 1

		DRS_FORCE_DWORD							= 0x7fffffff
	};

	//! Render state identifiers
	enum RenderStateID
	{
		// Standard set
		RS_TEXTUREPERSPECTIVE					= 0,		RS_ZWRITE							= 1,
		RS_ALPHATEST							= 2,		RS_DITHERING						= 3,
		RS_ALPHABLENDING						= 4,		RS_FOG								= 5,
		RS_SPECULAR								= 6,		RS_STENCILTEST						= 7,
		RS_LIGHTING								= 8,		RS_TEXTURING						= 9,
		RS_LASTPIXEL							= 10,		RS_STIPPLEDALPHA					= 11,
		RS_EDGEANTIALIAS						= 12,		RS_COLORKEYMODE						= 13,
		RS_COLORKEYBLENDMODE					= 14,		RS_RANGEBASEDFOG					= 15,
		RS_CLIPPING								= 16,		RS_SCREENEXTENTS					= 17,
		RS_COLORVERTEX							= 18,		RS_PERSPSPECHIGH					= 19,
		RS_NORMALIZE							= 20,		RS_AMBIENTCOLOR						= 21,
		RS_ANTIALIASMODE						= 22,		RS_ZBUFFERMODE						= 23,
		RS_ZBUFFERCMPMODE						= 24,		RS_ZBIAS							= 25,
		RS_LINEPATTERN							= 26,		RS_ALPHAREF							= 27,
		RS_ALPHAFUNC							= 28,		RS_SRCBLEND							= 29,
		RS_DESTBLEND							= 30,		RS_DIFFUSEMATSRC					= 31,
		RS_SPECULARMATSRC						= 32,		RS_AMBIENTMATSRC					= 33,
		RS_EMISSIVEMATSRC						= 34,		RS_VERTEXBLEND						= 35,
		RS_TEXTUREFACTOR						= 36,		RS_FILLMODE							= 37,
		RS_SHADEMODE							= 38,		RS_CULLMODE							= 39,
		RS_STENCILREF							= 40,		RS_STENCILCMPMODE					= 41,
		RS_STENCILCMPMASK						= 42,		RS_STENCILWRITEMASK					= 43,
		RS_STENCILFAILOP						= 44,		RS_STENCILZFAILOP					= 45,
		RS_STENCILPASSOP						= 46,		RS_FOGVERTEXMODE					= 47,
		RS_FOGMODE								= 48,		RS_FOGDENSITY						= 49,
		RS_FOGCOLOR								= 50,		RS_FOGSTART							= 51,
		RS_FOGEND								= 52,		RS_MATERIAL							= 53,
		// New set
		RS_SOFTWAREVERTEXPROCESSING				= 54,		RS_POINTSPRITE						= 55,
		RS_POINTSCALE							= 56,		RS_MULTISAMPLEANTIALIAS				= 57,
		RS_INDEXEDVERTEXBLEND					= 58,		RS_POINTSIZE						= 59,
		RS_POINTSIZEMIN							= 60,		RS_POINTSCALEA						= 61,
		RS_POINTSCALEB							= 62,		RS_POINTSCALEC						= 63,
		RS_MULTISAMPLEMASK						= 64,		RS_PATCHEDGESTYLE					= 65,
		RS_PATCHSEGMENTS						= 66,		//debug monitor token				= 67,
		RS_POINTSIZEMAX							= 68,		RS_COLORWRITEENABLE					= 69,
		RS_TWEENFACTOR							= 70,		RS_BLENDOP							= 71,
		//
		RS_VERTEXSHADER							= 72,		RS_PIXELSHADER						= 73,
		RS_VERTEXDECLARATION					= 74,

		RS_SIZE									= 76,	// Increment to the next 4-bytes boundary! (and warning!..starts with 0)

		RS_FORCE_DWORD							= 0x7fffffff
	};

	//! Texture address enum
	enum TEXTUREADDRESS
	{
		TADDRESS_WRAP							= 1,			//!< Tile the texture at every integer junction.
		TADDRESS_MIRROR							= 2,			//!< Similar to TADDRESS_WRAP, except that the texture is flipped at every integer junction.
		TADDRESS_CLAMP							= 3,			//!< Texture coordinates outside the range [0.0, 1.0] are set to the texture color at 0.0 or 1.0, respectively.
		TADDRESS_BORDER							= 4,			//!< Texture coordinates outside the range [0.0, 1.0] are set to the border color, which is a new render state.
		TADDRESS_MIRRORONCE						= 5,			//!< Similar to TADDRESS_MIRROR and TADDRESS_CLAMP. Takes the absolute value of the texture coordinate (thus,
																//!< mirroring around 0), and then clamps to the maximum value. The most common usage is for volume textures,
																//!< where support for the full TADDRESS_MIRRORONCE texture-addressing mode is not necessary, but the data is
																//!< symmetric around the one axis.
		TADDRESS_FORCE_DWORD					= 0x7FFFFFFF
	}; 

	//! Texture min filters
	enum TEXTUREMINFILTER
	{
		TFN_POINT								= 1,			//!< Point filtering. The texel with coordinates nearest to the desired pixel value is used.
		TFN_LINEAR								= 2,			//!< Bilinear interpolation filtering. A weighted average of a 2×2 area of texels surrounding the desired pixel is used.
		TFN_ANISOTROPIC							= 3,			//!< Anisotropic texture filtering. Compensates for distortion caused by the difference in angle between the texture polygon and the plane of the screen.

		TFN_FORCE_DWORD							= 0x7FFFFFFF
	};

	//! Texture mag filters
	enum TEXTUREMAGFILTER
	{
		TFG_POINT								= 1,			//!< Point filtering. The texel with coordinates nearest to the desired pixel value is used.
		TFG_LINEAR								= 2,			//!< Bilinear interpolation filtering. A weighted average of a 2×2 area of texels surrounding the desired pixel is used.
		TFG_FLATCUBIC							= 3,			//!< Not currently supported; do not use.
		TFG_GAUSSIANCUBIC						= 4,			//!< Not currently supported; do not use.
		TFG_ANISOTROPIC							= 5,			//!< Anisotropic texture filtering. Compensates for distortion caused by the difference in angle between the texture polygon and the plane of the screen.

		TFG_FORCE_DWORD							= 0x7FFFFFFF
	};

	//! Texture mip filters
	enum TEXTUREMIPFILTER
	{
		TFP_NONE								= 1,			//!< Mipmapping disabled. The rasterizer should use the magnification filter instead.
		TFP_POINT								= 2,			//!< Nearest-point mipmap filtering. The rasterizer uses the color from the texel of the nearest mipmap texture.
		TFP_LINEAR								= 3,			//!< Trilinear mipmap interpolation. The rasterizer linearly interpolates pixel color, using the texels of the two nearest mipmap textures.

		TFP_FORCE_DWORD							= 0x7FFFFFFF
	};

	//! Antialiasing modes
	enum ANTIALIASMODE
	{
		ANTIALIAS_NONE							= 0,			//!< No antialiasing is performed.
		ANTIALIAS_SORTDEPENDENT					= 1,			//!< Antialiasing is dependent on the sort order of the polygons (back to front or front to back).
																//!< The application must draw polygons in the right order for antialiasing to occur.
		ANTIALIAS_SORTINDEPENDENT				= 2,			//!< Antialiasing is not dependent on the sort order of the polygons.

		ANTIALIAS_FORCE_DWORD					= 0x7FFFFFFF
	};

	//! Fill modes
	enum FILLMODE
	{
		FILL_POINT								= 1,			//!< Fill points.
		FILL_WIREFRAME							= 2,			//!< Fill wireframes.
		FILL_SOLID								= 3,			//!< Fill solids.

		FILL_FORCE_DWORD						= 0x7FFFFFFF
	};

	//! Shade modes
	enum SHADEMODE
	{
		SHADE_FLAT								= 1,			//!< Flat shading mode. The color and specular component of the first vertex in the triangle are used to determine
																//!< the color and specular component of the face. These colors remain constant across the triangle; that is, they
																//!< are not interpolated. 
		SHADE_GOURAUD							= 2,			//!< Gouraud shading mode. The color and specular components of the face are determined by a linear interpolation
																//!< between all three of the triangle's vertices. 
		SHADE_PHONG								= 3,			//!< Not currently supported.

		SHADE_FORCE_DWORD						= 0x7FFFFFFF
	};

	//! Cull modes
	enum CULLMODE
	{
		CULL_NONE								= 1,			//!< Do not cull back faces.
		CULL_CW									= 2,			//!< Cull back faces with clockwise vertices.
		CULL_CCW								= 3,			//!< Cull back faces with counterclockwise vertices.

		CULL_FORCE_DWORD						= 0x7FFFFFFF
	};

	//! Front faces
	enum FRONTFACE
	{
		FRONTFACE_CW							= 0,			//!< Front faces are CW faces
		FRONTFACE_CCW							= 1,			//!< Front faces are CCW faces

		FRONTFACE_FORCE_DWORD					= 0x7FFFFFFF
	};

	//! Z-Buffer modes
	enum ZBUFFERMODE
	{
		ZB_FALSE								= 0,			//!< Disable depth buffering.
		ZB_TRUE									= 1,			//!< Enable z-buffering.
		ZB_USEW									= 2,			//!< Enable w-buffering.

		ZB_FORCE_DWORD							= 0x7FFFFFFF
	};

	//! Comparison functions
	enum CMPFUNC
	{
		CMP_NEVER								= 1,			//!< Always fail the test.
		CMP_LESS								= 2,			//!< Accept the new pixel if its value is less than the value of the current pixel.
		CMP_EQUAL								= 3,			//!< Accept the new pixel if its value equals the value of the current pixel.
		CMP_LESSEQUAL							= 4,			//!< Accept the new pixel if its value is less than or equal to the value of the current pixel.
		CMP_GREATER								= 5,			//!< Accept the new pixel if its value is greater than the value of the current pixel.
		CMP_NOTEQUAL							= 6,			//!< Accept the new pixel if its value does not equal the value of the current pixel.
		CMP_GREATEREQUAL						= 7,			//!< Accept the new pixel if its value is greater than or equal to the value of the current pixel.
		CMP_ALWAYS								= 8,			//!< Always pass the test.

		CMP_FORCE_DWORD							= 0x7FFFFFFF
	};

	//! Stencil operations
	enum STENCILOP
	{
		STENCILOP_KEEP							= 1,			//!< Do not update the entry in the stencil buffer.
		STENCILOP_ZERO							= 2,			//!< Set the stencil-buffer entry to 0.
		STENCILOP_REPLACE						= 3,			//!< Replace the stencil-buffer entry with reference value.
		STENCILOP_INCRSAT						= 4,			//!< Increment the stencil-buffer entry, clamping to the maximum value.
		STENCILOP_DECRSAT						= 5,			//!< Decrement the stencil-buffer entry, clamping to 0.
		STENCILOP_INVERT						= 6,			//!< Invert the bits in the stencil-buffer entry.
		STENCILOP_INCR							= 7,			//!< Increment the stencil-buffer entry, wrapping to 0 if the new value exceeds the maximum value.
		STENCILOP_DECR							= 8,			//!< Decrement the stencil-buffer entry, wrapping to the maximum value if the new value is less than 0.

		STENCILOP_FORCE_DWORD					= 0x7FFFFFFF
	};

	//! Alpha-blending factors
	enum ALPHABLEND
	{
		ABLEND_ZERO								= 1,			//!< Blend factor is (0, 0, 0, 0).
		ABLEND_ONE								= 2,			//!< Blend factor is (1, 1, 1, 1).
		ABLEND_SRCCOLOR							= 3,			//!< Blend factor is (Rs, Gs, Bs, As).
		ABLEND_INVSRCCOLOR						= 4,			//!< Blend factor is (1-Rs, 1-Gs, 1-Bs, 1-As).
		ABLEND_SRCALPHA							= 5,			//!< Blend factor is (As, As, As, As).
		ABLEND_INVSRCALPHA						= 6,			//!< Blend factor is (1-As, 1-As, 1-As, 1-As).
		ABLEND_DESTALPHA						= 7,			//!< Blend factor is (Ad, Ad, Ad, Ad).
		ABLEND_INVDESTALPHA						= 8,			//!< Blend factor is (1-Ad, 1-Ad, 1-Ad, 1-Ad).
		ABLEND_DESTCOLOR						= 9,			//!< Blend factor is (Rd, Gd, Bd, Ad).
		ABLEND_INVDESTCOLOR						= 10,			//!< Blend factor is (1-Rd, 1-Gd, 1-Bd, 1-Ad).
		ABLEND_SRCALPHASAT						= 11,			//!< Blend factor is (f, f, f, 1); f = min(As, 1-Ad).
		ABLEND_BOTHSRCALPHA						= 12,			//!< Obsolete! Use ABLEND_SRCALPHA and ABLEND_INVSRCALPHA in separate calls.
		ABLEND_BOTHINVSRCALPHA					= 13,			//!< Source blend factor is (1-As, 1-As, 1-As, 1-As), and destination blend factor is (As, As, As, As);
																//!< the destination blend selection is overridden.
		ABLEND_FORCE_DWORD						= 0x7FFFFFFF
	};

	//! Texture stage operations
	enum STAGEOP
	{
		// Control members
		STAGEOP_DISABLE							= 1,			//!< Disables output from this texture stage and all stages with a higher index. To disable texture mapping,
																//!< set this as the color operation for the first texture stage (stage 0). Alpha operations cannot be disabled when
																//!< color operations are enabled. Setting the alpha operation to D3DTOP_DISABLE when color blending is enabled
																//!< causes undefined behavior. 
		STAGEOP_SELECTARG1						= 2,			//!< s = Arg1
		STAGEOP_SELECTARG2						= 3,			//!< s = Arg2
		// Modulation members
		STAGEOP_MODULATE						= 4,			//!< s = Arg1 * Arg2
		STAGEOP_MODULATE2X						= 5,			//!< s = Arg1 * Arg2 * 2
		STAGEOP_MODULATE4X						= 6,			//!< s = Arg1 * Arg2 * 4
		// Addition and subtraction members
		STAGEOP_ADD								= 7,			//!< s = Arg1 + Arg2
		STAGEOP_ADDSIGNED						= 8,			//!< s = Arg1 + Arg2 - 0.5
		STAGEOP_ADDSIGNED2X						= 9,			//!< s = (Arg1 + Arg2 - 0.5) * 2
		STAGEOP_SUBTRACT						= 10,			//!< s = Arg1 - Arg2
		STAGEOP_ADDSMOOTH						= 11,			//!< s = Arg1 + Arg2 - Arg1 * Arg2 = Arg1 + Arg2(1 - Arg1)
		// Linear alpha blending members
		STAGEOP_BLENDDIFFUSEALPHA				= 12,			//!< s = Arg1 * Alpha + Arg2 * (1 - Alpha)
		STAGEOP_BLENDTEXTUREALPHA				= 13,			//!< s = Arg1 * Alpha + Arg2 * (1 - Alpha)
		STAGEOP_BLENDFACTORALPHA				= 14,			//!< s = Arg1 * Alpha + Arg2 * (1 - Alpha)
		STAGEOP_BLENDTEXTUREALPHAPM				= 15,			//!< s = Arg1 + Arg2 * (1 - Alpha)
		STAGEOP_BLENDCURRENTALPHA				= 16,			//!< s = Arg1 * Alpha + Arg2 * (1 - Alpha)
		// Specular mapping members
		STAGEOP_PREMODULATE						= 17,			//!< Modulate this texture stage with the next texture stage.
		STAGEOP_MODULATEALPHA_ADDCOLOR			= 18,			//!< s = Arg1RGB + Arg1A * Arg2RGB
		STAGEOP_MODULATECOLOR_ADDALPHA			= 19,			//!< s = Arg1RGB * Arg2RGB + Arg1A
		STAGEOP_MODULATEINVALPHA_ADDCOLOR		= 20,			//!< s = (1 - Arg1A)*Arg2RGB + Arg1RGB
		STAGEOP_MODULATEINVCOLOR_ADDALPHA		= 21,			//!< s = (1 - Arg1RGB)*Arg2RGB + Arg1A
		// Bump-mapping members
		STAGEOP_BUMPENVMAP						= 22,			//!< Perform per-pixel bump mapping, using the environment map in the next texture stage (without luminance).
		STAGEOP_BUMPENVMAPLUMINANCE				= 23,			//!< Perform per-pixel bump mapping, using the environment map in the next texture stage (with luminance).
		STAGEOP_DOTPRODUCT3						= 24,			//!< s = Arg1R*Arg2R + Arg1G*Arg2G + Arg1B*Arg2B
		// Triadic texture blending members
		STAGEOP_MULTIPLYADD						= 25,			//!< s = Arg1 + Arg2 * Arg3
		STAGEOP_LERP							= 26,			//!< s = (Arg1) * Arg2 + (1-Arg1) * Arg3

		STAGEOP_FORCE_DWORD						= 0x7FFFFFFF
	};

	//! Texture stage arguments
	enum STAGEARG
	{
		// Argument flags
		STAGEARG_DIFFUSE						= 0,			//!< The texture argument is the diffuse color interpolated from vertex components during Gouraud shading.
																//!< If the vertex does not contain a diffuse color, the default color is 0xFFFFFFFF. 
		STAGEARG_CURRENT						= 1,			//!< The texture argument is the result of the previous blending stage. In the first texture stage (stage 0),
																//!< this argument is equivalent to STAGEARG_DIFFUSE. If the previous blending stage uses a bump-map texture
																//!< (the STAGEOP_BUMPENVMAP operation), the system chooses the texture from the stage before the bump-map texture.
		STAGEARG_TEXTURE						= 2,			//!< The texture argument is the texture color for this texture stage. This is valid only for the first color
																//!< and alpha arguments in a stage. (ARG1)
		STAGEARG_TFACTOR						= 3,			//!< The texture argument is the texture factor set in a previous call to SetTextureFactor().
		STAGEARG_SPECULAR						= 4,			//!< The texture argument is the specular color interpolated from vertex components during Gouraud shading.
																//!< If the vertex does not contain a diffuse color, the default color is 0xFFFFFFFF.
		STAGEARG_TEMP							= 5,			//!< The texture argument is a temporary register color for read or write. D3DTA_TEMP is supported if the
																//!< D3DPMISCCAPS_TSSARGTEMP device capability is present. The default value for the register is (0.0, 0.0, 0.0, 0.0).
																//!< Permissions are read/write. 
		STAGEARG_SELECTMASK						= 0xf,			//!< Mask value for all arguments; not used when setting texture arguments.
		// Modifier flags
		STAGEARG_COMPLEMENT						= 0x10,			//!< Invert the argument so that, if the result of the argument were referred to by the variable x, the value would be 1.0 - x. 
		STAGEARG_ALPHAREPLICATE					= 0x20,			//!< Replicate the alpha information to all color channels before the operation completes.

		STAGEARG_DIFFUSE_COMPLEMENT				= STAGEARG_DIFFUSE|STAGEARG_COMPLEMENT,
		STAGEARG_CURRENT_COMPLEMENT				= STAGEARG_CURRENT|STAGEARG_COMPLEMENT,
		STAGEARG_TEXTURE_COMPLEMENT				= STAGEARG_TEXTURE|STAGEARG_COMPLEMENT,
		STAGEARG_TFACTOR_COMPLEMENT				= STAGEARG_TFACTOR|STAGEARG_COMPLEMENT,
		STAGEARG_SPECULAR_COMPLEMENT			= STAGEARG_SPECULAR|STAGEARG_COMPLEMENT,
		STAGEARG_TEMP_COMPLEMENT				= STAGEARG_TEMP|STAGEARG_COMPLEMENT,

		STAGEARG_DIFFUSE_ALPHAREPLICATE			= STAGEARG_DIFFUSE|STAGEARG_ALPHAREPLICATE,
		STAGEARG_CURRENT_ALPHAREPLICATE			= STAGEARG_CURRENT|STAGEARG_ALPHAREPLICATE,
		STAGEARG_TEXTURE_ALPHAREPLICATE			= STAGEARG_TEXTURE|STAGEARG_ALPHAREPLICATE,
		STAGEARG_TFACTOR_ALPHAREPLICATE			= STAGEARG_TFACTOR|STAGEARG_ALPHAREPLICATE,
		STAGEARG_SPECULAR_ALPHAREPLICATE		= STAGEARG_SPECULAR|STAGEARG_ALPHAREPLICATE,
		STAGEARG_TEMP_ALPHAREPLICATE			= STAGEARG_TEMP|STAGEARG_ALPHAREPLICATE,

		STAGEARG_FORCE_DWORD					= 0x7FFFFFFF
	};

	//! TEXGEN modes
	enum STAGETEXCOORDINDEX
	{
		STAGETCI_PASSTHRU 						= 0x00000000,	//!< Use the specified texture coordinates contained within the vertex format.
		STAGETCI_CAMERASPACENORMAL				= 0x00010000,	//!< Use the vertex normal, transformed to camera space, as the input texture coordinates for this stage's
																//!< texture transformation.
		STAGETCI_CAMERASPACEPOSITION			= 0x00020000,	//!< Use the vertex position, transformed to camera space, as the input texture coordinates for this stage's
																//!< texture transformation.
		STAGETCI_CAMERASPACEREFLECTIONVECTOR	= 0x00030000,	//!< Use the reflection vector, transformed to camera space, as the input texture coordinate for this stage's
																//!< texture transformation. The reflection vector is computed from the input vertex position and normal vector.
		STAGETCI_FORCE_DWORD					= 0x7FFFFFFF
		// Note: you can OR those flags with an extra index used to determine the texture wrapping mode.
	};

	//! Fog modes
	enum FOGMODE
	{
		FOGMODE_NONE							= 0,			//!< No fog effect.
		FOGMODE_EXP								= 1,			//!< The fog effect intensifies exponentially.
		FOGMODE_EXP2							= 2,			//!< The fog effect intensifies exponentially with the square of the distance.
		FOGMODE_LINEAR							= 3,			//!< The fog effect intensifies linearly between the start and end points.

		FOGMODE_FORCE_DWORD						= 0x7FFFFFFF
	};

	//! Vertex colors sources
	enum MCS
	{
		MCS_MATERIAL							= 0,			//!< Use the color from the current material.
		MCS_COLOR1								= 1,			//!< Use the diffuse vertex color.
		MCS_COLOR2								= 2,			//!< Use the specular vertex color.

		MCS_FORCE_DWORD							= 0x7FFFFFFF
	};

	//! Vertex blending modes
	enum VBLEND
	{
		VBLEND_DISABLE							= 0,			//!< Disable vertex blending; only apply the standard world matrix.
		VBLEND_1WEIGHT							= 1,			//!< Enable vertex blending between the two first world matrices.
		VBLEND_2WEIGHTS							= 2,			//!< Enable vertex blending between the three first world matrices.
		VBLEND_3WEIGHTS							= 3,			//!< Enable vertex blending between the four world matrices.
		VBLEND_TWEENING							= 255,			//!< Enable vertex blending using RS_TWEENFACTOR
		VBLEND_0WEIGHTS							= 256,			//!< One matrix is used with weight 1.0

		VBLEND_FORCE_DWORD						= 0x7FFFFFFF
	};

	//! State blocks types
	enum STATEBLOCKTYPE
	{
		SBT_ALL									= 1,			//!< Capture all device states.
		SBT_PIXELSTATE							= 2,			//!< Capture only pixel-related device states.
		SBT_VERTEXSTATE							= 3,			//!< Capture only vertex-related device states.

		SBT_FORCE_DWORD							= 0x7fffffff
	};

	//! Texture transformation flags
	enum TEXTURETRANSFLAGS
	{
		TTFF_DISABLE							= 0,			//!< Texture coordinates are passed directly to the rasterizer.
		TTFF_COUNT1								= 1,			//!< The rasterizer should expect 1-D texture coordinates.
		TTFF_COUNT2								= 2,			//!< The rasterizer should expect 2-D texture coordinates.
		TTFF_COUNT3								= 3,			//!< The rasterizer should expect 3-D texture coordinates.
		TTFF_COUNT4								= 4,			//!< The rasterizer should expect 4-D texture coordinates.
		TTFF_PROJECTED							= 256,			//!< The texture coordinates are all divided by the last element before being passed to the rasterizer.

		TTFF_FORCE_DWORD						= 0x7fffffff
	};

	//! Patch edge styles
	enum PATCHEDGESTYLE
	{
		PES_DISCRETE							= 0,			//!< Discrete edge style. In discrete mode, you can specify float tessellation but it will be truncated to integers.
		PES_CONTINUOUS							= 1,			//!< Continuous edge style. In continuous mode, tessellation is specified as float values which can be smoothly varied to reduce "popping" artifacts.

		PES_FORCE_DWORD							= 0x7fffffff
	};

	 enum BLENDOP
	 {
		BLENDOP_ADD								= 1,			//!< Result = Source + Destination
		BLENDOP_SUBTRACT						= 2,			//!< Result = Source - Destination
		BLENDOP_REVSUBTRACT						= 3,			//!< Result = Destination - Source
		BLENDOP_MIN								= 4,			//!< Result = MIN(Source, Destination)
		BLENDOP_MAX								= 5,			//!< Result = MAX(Source, Destination)

		BLENDOP_FORCE_DWORD						= 0x7fffffff
	};

	 enum COLORWRITEENABLE
	 {
		 CWE_RED								= (1<<0),
		 CWE_GREEN								= (1<<1),
		 CWE_BLUE								= (1<<2),
		 CWE_ALPHA								= (1<<3),
		 CWE_ALL								= CWE_RED|CWE_GREEN|CWE_BLUE|CWE_ALPHA,

		 CWE_FORCE_DWORD						= 0x7fffffff
	 };

	typedef uword	SBID;				//!< State block identifier

//	#define	RS_PRIORITYLEVELS			//!< Include support for priority levels

#ifdef RS_PRIORITYLEVELS
	typedef ubyte	PriorityLevel;		//!< Prevent confusion between a priority level and a render state.
#endif

	//! Texture stage
	class ICERENDERER_API TextureStage : public Allocateable
	{
		public:
								TextureStage();
								~TextureStage();

		void*					mBitmap;				//!< Texture bitmap
		STAGEOP					mColorOp;				//!< Color blending operation
		STAGEARG				mColorArg1;				//!< First color argument
		STAGEARG				mColorArg2;				//!< Second color argument
		STAGEOP					mAlphaOp;				//!< Alpha blending operation
		STAGEARG				mAlphaArg1;				//!< First alpha argument
		STAGEARG				mAlphaArg2;				//!< Second alpha argument
		STAGETEXCOORDINDEX		mCoordIndex;			//!< Index of the texture coordinates set to use with this stage
		TEXTUREADDRESS			mAddressU;				//!< Texture Address U
		TEXTUREADDRESS			mAddressV;				//!< Texture Address V
		udword					mBorder;				//!< Border Color
		TEXTUREMAGFILTER		mMagFilter;				//!< Mag Filter
		TEXTUREMINFILTER		mMinFilter;				//!< Min Filter
		TEXTUREMIPFILTER		mMipFilter;				//!< Mip Filter
		float					mMipMapLODBias;			//!< Level of detail bias for mipmaps
		udword					mMaxMipMapLevel;		//!< Maximum mipmap level of detail
		udword					mMaxAnisotropy;			//!< Maximum level of anisotropy
		TEXTURETRANSFLAGS		mTransFlags;			//!< UVs Transform flags
		BOOL					mWrapU;					//!< Textures Wrap for U
		BOOL					mWrapV;					//!< Textures Wrap for V
	};

	//! Texture state identifiers
	enum TextureRenderState
	{
		TS_BITMAP	= 0,		TS_COLOROP,
		TS_COLORARG1,			TS_COLORARG2,
		TS_ALPHAOP,				TS_ALPHAARG1,
		TS_ALPHAARG2,			TS_COORDINDEX,
		TS_ADDRESSU,			TS_ADDRESSV,
		TS_BORDER,				TS_MAGFILTER,
		TS_MINFILTER,			TS_MIPFILTER,
		TS_MIPMAPLODBIAS,		TS_MAXMIPMAPLEVEL,
		TS_MAXANISOTROPY,		TS_TRANSFLAGS,
		TS_WRAPU,				TS_WRAPV,

		TS_SIZE,		// Last one + 1

		TS_FORCE_DWORD	= 0x7fffffff
	};

#ifdef RS_PRIORITYLEVELS
	#pragma pack(1)

	struct TSPriorityLevel : public Allocateable
	{
						TSPriorityLevel()	{ FillMemory(this, SIZEOFOBJECT, 16);	}

		PriorityLevel	mBitmap;
		PriorityLevel	mColorOp;
		PriorityLevel	mColorArg1;
		PriorityLevel	mColorArg2;
		PriorityLevel	mAlphaOp;
		PriorityLevel	mAlphaArg1;
		PriorityLevel	mAlphaArg2;
		PriorityLevel	mCoordIndex;
		PriorityLevel	mAddressU;
		PriorityLevel	mAddressV;
		PriorityLevel	mBorder;
		PriorityLevel	mMagFilter;
		PriorityLevel	mMinFilter;
		PriorityLevel	mMipFilter;
		PriorityLevel	mMipMapLODBias;
		PriorityLevel	mMaxMipMapLevel;
		PriorityLevel	mMaxAnisotropy;
		PriorityLevel	mTransflags;
		PriorityLevel	mWrapU;
		PriorityLevel	mWrapV;
	};

	#pragma pack()
#endif

//	#define TextureStateID(member)		OFFSET_OF(TSPriorityLevel, member)

#endif // ICERENDERSTATES_H
