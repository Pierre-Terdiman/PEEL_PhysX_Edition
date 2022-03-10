///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a void renderer.
 *	\file		IceVoidRenderer.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "Stdafx.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEVOIDRENDERER_H
#define ICEVOIDRENDERER_H

#ifdef OBSOLETE_RENDERER
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Creates & returns the global void renderer.
	 *	\fn			CreateVoidRenderer()
	 *	\return		the global void renderer.
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION ICERENDERER_API Renderer* CreateVoidRenderer();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Releases the global void renderer.
	 *	\fn			ReleaseVoidRenderer()
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION ICERENDERER_API void ReleaseVoidRenderer();
#endif

	class ICERENDERER_API VoidRenderState : public RenderStateManager
	{
		public:
												VoidRenderState();
												~VoidRenderState();

		override(RenderStateManager)	bool	ValidateDevice(udword& nbpasses)						{ return true;	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Transformations
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(RenderStateManager)	BOOL	SetWorldMatrix(const Matrix4x4& world)					{ return TRUE;	}
		override(RenderStateManager)	void	GetWorldMatrix(Matrix4x4& world)						{}

		override(RenderStateManager)	BOOL	SetBoneMatrix(udword i, const Matrix4x4& world)			{ return TRUE;	}
		override(RenderStateManager)	void	GetBoneMatrix(udword i, Matrix4x4& world)				{}

		override(RenderStateManager)	BOOL	SetViewMatrix(const Matrix4x4& view)					{ return TRUE;	}
		override(RenderStateManager)	void	GetViewMatrix(Matrix4x4& view)							{}

		override(RenderStateManager)	BOOL	SetProjMatrix(const Matrix4x4& proj)					{ return TRUE;	}
		override(RenderStateManager)	void	GetProjMatrix(Matrix4x4& proj)							{}

		override(RenderStateManager)	BOOL	SetTextureMatrix(udword i, const Matrix4x4& texture)	{ return TRUE;	}
		override(RenderStateManager)	void	GetTextureMatrix(udword i, Matrix4x4& texture)			{}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Helper methods
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(RenderStateManager)	void	SetDepthRange				(float zmin, float zmax)	{}
		override(RenderStateManager)	void	SetColorUpdate				(BOOL flag)					{}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// User-defined clipping planes
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(RenderStateManager)	udword	MaxUserClippingPlanes()									{ return 0;		}
		override(RenderStateManager)	udword	MaxHardwareUserClippingPlanes()							{ return 0;		}
		override(RenderStateManager)	void	EnableClipPlane(udword mask)							{}
		override(RenderStateManager)	void	SetClipPlane(udword i, float* equation)					{}
		override(RenderStateManager)	void	GetClipPlane(udword i, float* equation)					{}
		override(RenderStateManager)	udword	AddClipPlane(const Plane& clipplane)					{ return 0;		}
		override(RenderStateManager)	bool	RemoveClipPlane(udword planeid)							{ return true;	}
		protected:
		override(RenderStateManager)	void	ValidateClipPlanes()									{}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// State block manager
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		private:

		override(RenderStateManager)	bool	BeginStateBlock()										{ return true; }
		override(RenderStateManager)	bool	EndStateBlock(udword& id)								{ return true; }
		override(RenderStateManager)	bool	ApplyStateBlock(udword blockid)							{ return true; }
		override(RenderStateManager)	bool	DeleteStateBlock(udword blockid)						{ return true; }

		public:
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Flag-controlled render states
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(RenderStateManager)	BOOL	SetTexturePerspective		(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetZWrite					(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetAlphaTest				(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetDithering				(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetAlphaBlending			(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetFog						(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetSpecular					(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetStencilTest				(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetLighting					(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetLastPixel				(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetStippledAlpha			(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetEdgeAntialias			(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetColorKeyMode				(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetColorKeyBlendMode		(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetRangeBasedFog			(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetClipping					(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetScreenExtents			(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetColorVertex				(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetPerspSpecHigh			(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetNormalize				(BOOL flag)					{ return TRUE; }

		override(RenderStateManager)	BOOL	SetSoftwareVertexProcessing	(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetPointSprite				(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetPointScale				(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetMultiSampleAntialias		(BOOL flag)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetIndexedVertexBlend		(BOOL flag)					{ return TRUE; }

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Dword-controlled render states
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(RenderStateManager)	BOOL	SetAmbientColor				(udword ambcolor)			{ return TRUE; }
		override(RenderStateManager)	BOOL	SetAntialiasMode			(ANTIALIASMODE am)			{ return TRUE; }
		override(RenderStateManager)	BOOL	SetZBufferMode				(ZBUFFERMODE zm)			{ return TRUE; }
		override(RenderStateManager)	BOOL	SetZBufferCmpMode			(CMPFUNC cf)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetZBias					(udword value)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetLinePattern				(udword value)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetAlphaRef					(udword ref)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetAlphaFunc				(CMPFUNC af)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetSrcBlend					(ALPHABLEND mode)			{ return TRUE; }
		override(RenderStateManager)	BOOL	SetDstBlend					(ALPHABLEND mode)			{ return TRUE; }
		override(RenderStateManager)	BOOL	SetDiffuseMaterialSource	(MCS type)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetSpecularMaterialSource	(MCS type)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetAmbientMaterialSource	(MCS type)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetEmissiveMaterialSource	(MCS type)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetVertexBlend				(VBLEND mode)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureFactor			(udword value)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetFillMode					(FILLMODE fm)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetShadeMode				(SHADEMODE sm)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetCullMode					(CULLMODE cm)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetStencilRef				(udword val)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetStencilCmpMode			(CMPFUNC cf)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetStencilCmpMask			(udword mask)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetStencilWriteMask			(udword mask)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetStencilFailOp			(STENCILOP sop)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetStencilZFailOp			(STENCILOP sop)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetStencilPassOp			(STENCILOP sop)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetFogVertexMode			(FOGMODE mode)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetFogMode					(FOGMODE fm)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetFogDensity				(float density)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetFogColor					(udword fogcolor)			{ return TRUE; }
		override(RenderStateManager)	BOOL	SetFogStart					(float fogstart)			{ return TRUE; }
		override(RenderStateManager)	BOOL	SetFogEnd					(float fogend)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetMaterialProps			(const MaterialProps* props){ return TRUE; }

		override(RenderStateManager)	BOOL	SetPointSize				(float size)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetPointSizeMin				(float minsize)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetPointScaleA				(float scalea)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetPointScaleB				(float scaleb)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetPointScaleC				(float scalec)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetMultiSampleMask			(udword mask)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetPatchEdgeStyle			(PATCHEDGESTYLE pes)		{ return TRUE; }
		override(RenderStateManager)	BOOL	SetPatchSegments			(float nb)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetPointSizeMax				(float maxsize)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetColorWriteEnable			(udword mask)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTweenFactor				(float factor)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetBlendOp					(BLENDOP op)				{ return TRUE; }

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Texture Render States
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(RenderStateManager)	BOOL	SetTextureBitmap			(udword i, void* bitmap)			{ return TRUE; }

		override(RenderStateManager)	BOOL	SetTextureColorOp			(udword i, STAGEOP op)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureColorArg1			(udword i, STAGEARG sa)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureColorArg2			(udword i, STAGEARG sa)				{ return TRUE; }

		override(RenderStateManager)	BOOL	SetTextureAlphaOp			(udword i, STAGEOP op)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureAlphaArg1			(udword i, STAGEARG sa)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureAlphaArg2			(udword i, STAGEARG sa)				{ return TRUE; }

		override(RenderStateManager)	BOOL	SetTextureMinFilter			(udword i, TEXTUREMINFILTER tmf)	{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureMagFilter			(udword i, TEXTUREMAGFILTER tmf)	{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureMipFilter			(udword i, TEXTUREMIPFILTER tmf)	{ return TRUE; }

		override(RenderStateManager)	BOOL	SetTextureCoordIndex		(udword i, STAGETEXCOORDINDEX ci)	{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureTransFlags		(udword i, TEXTURETRANSFLAGS ttff)	{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureMipMapLODBias		(udword i, float bias)				{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureMaxMipMapLevel	(udword i, udword maxlevel)			{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureMaxAnisotropy		(udword i, udword maxanis)			{ return TRUE; }

		override(RenderStateManager)	BOOL	SetTextureAddress			(udword i, TEXTUREADDRESS ta)		{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureAddressU			(udword i, TEXTUREADDRESS ta)		{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureAddressV			(udword i, TEXTUREADDRESS ta)		{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureBorder			(udword i, udword color)			{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureWrapU				(udword i, BOOL b)					{ return TRUE; }
		override(RenderStateManager)	BOOL	SetTextureWrapV				(udword i, BOOL b)					{ return TRUE; }
	};

	class ICERENDERER_API VoidRenderer : public Renderer
	{
		private:
											VoidRenderer();
		virtual								~VoidRenderer();

		public:
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Initializes the renderer.
		 *	\param		create		[in] creation structure
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Renderer)	bool			Initialize(const RENDERERCREATE& create);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Presents the frame on the front buffer.
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Renderer)	bool			ShowFrame();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Move callback.
		 *	\param		x	[in] x position on screen
		 *	\param		y	[in] y position on screen
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Renderer)	void			OnMove(udword x, udword y);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Renderer validation.
		 *	\return		true if the renderer is ready
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Renderer)	bool			IsReady();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Selects a viewport.
		 *	\param		vp		[in] viewport to select
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Renderer)	bool			SetViewport(const Viewport& vp);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Clears a viewport.
		 *	\param		vp		[in] viewport to clear
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Renderer)	bool			ClearViewport(const Viewport& vp);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Begins a scene. Use one BeginScene/EndScene for each viewport.
		 *	\see		EndScene()
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Renderer)	bool			BeginScene();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Ends a scene.
		 *	\see		BeginScene()
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Renderer)	bool			EndScene();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Resize callback.
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Renderer)	bool			Resize();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Retrieves the scan-line that is currently being drawn on the monitor.
		 *	\return		the scanline number or -1 if failed
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Renderer)	udword			GetScanline();

		// Primitive rendering
		override(Renderer)	bool			DrawPrimitive			(PrimType type, VertexFormat fvf, const void* verts, udword nbvertices, udword flags=0);
		override(Renderer)	bool			DrawIndexedPrimitive	(PrimType type, VertexFormat fvf, const void* verts, udword nbvertices, const uword* indices, udword nbindices, udword flags=0);
		// Vertex buffer rendering
		override(Renderer)	bool			DrawPrimitiveVB			(PrimType type, VertexBuffer* vb, udword startvertex, udword nbvertices, udword flags=0);
		override(Renderer)	bool			DrawIndexedPrimitiveVB	(PrimType type, VertexBuffer* vb, udword startvertex, udword nbvertices, const uword* indices, udword nbindices, udword flags=0);
		// ...
		override(Renderer)	bool			DrawIndexedPrimitive	(VertexBuffer* vb, const VBDesc& desc, IndexBuffer* ib, udword subset);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets the general renderer caps. Don't take the caps as granted! you're supposed to check them before using any of them!
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Renderer)	bool			GetCaps();

		// Vertex buffers
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Creates a vertex buffer.
		 *	\param		create	[in] vertex buffer creation structure
		 *	\return		the newly created vertex buffer, or null if failed
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Renderer)	VertexBuffer*	CreateVertexBuffer(const VERTEXBUFFERCREATE& create);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Optimizes a vertex buffer.
		 *	\param		vb	[in] vertex buffer pointer
		 *	\return		Self-Reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Renderer)	Renderer&		OptimizeVertexBuffer(VertexBuffer* vb);

		// Textures

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Creates a texture.
		 *	\param		create	[in] texture creation structure
		 *	\return		the newly created texture, or null if failed
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Renderer)	Texture*		CreateTexture(const TEXTURECREATE& create);

		// Lighting
		override(Renderer)	bool			SetupLightProps(Light* light);
		override(Renderer)	bool			SwitchLight(Light* light, bool enable);

		override(Renderer)	bool			Print(udword x, udword y, const char* text);

		// Render targets
		override(Renderer)	bool			SetRenderTarget(RenderTarget* target);

		private:

		friend ICERENDERER_API Renderer*	CreateVoidRenderer();
		friend ICERENDERER_API void			ReleaseVoidRenderer();
	};

#endif // ICEVOIDRENDERER_H
