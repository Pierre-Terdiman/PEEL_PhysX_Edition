///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains renderer base code.
 *	\file		IceRenderCore.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERENDERCORE_H
#define ICERENDERCORE_H

#ifdef OBSOLETE_RENDERER
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Returns the global renderer.
	 *	\fn			GetRenderer()
	 *	\return		the global renderer
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ICERENDERER_API	Renderer* GetRenderer();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Loads a renderer.
	 *	\fn			LoadRenderer(const char* dll_filename)
	 *	\param		dll_filename	[in] name of the renderer DLL
	 *	\return		the loaded renderer, or null
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ICERENDERER_API	Renderer* LoadRenderer(const char* dll_filename);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Unloads a renderer.
	 *	\fn			UnloadRenderer()
	 *	\return		true if success
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ICERENDERER_API	bool UnloadRenderer();
#endif

	ICERENDERER_API	BOOL	ChangeResolution(udword w, udword h, udword bitdepth);
	ICERENDERER_API	void	RestoreResolution();

	//! ZBuffer format caps
	enum ZBufferFlag
	{
		RENDERER_ZBUFFER		= 0x0001,	//!< Use Z Buffer HSR
		RENDERER_WBUFFER		= 0x0002,	//!< Use W Buffer HSR
		RENDERER_ZDEPTH8		= 0x0010,	//!< Z/W Buffer with a  8bits depth
		RENDERER_ZDEPTH16		= 0x0020,	//!< Z/W Buffer with a 16bits depth
		RENDERER_ZDEPTH24		= 0x0040,	//!< Z/W Buffer with a 24bits depth
		RENDERER_ZDEPTH32		= 0x0080,	//!< Z/W Buffer with a 32bits depth
		RENDERER_ZDEPTHMASK		= 0x00F0,
		RENDERER_STENCILBUFFER	= 0x0100,	//!< Use Stencil Buffer (depth size is automatic)
		RENDERER_SDEPTH8		= 0x1000,	//!< Stencil Buffer with a  8bits depth
		RENDERER_SDEPTH16		= 0x2000,	//!< Stencil Buffer with a 16bits depth
		RENDERER_SDEPTH24		= 0x4000,	//!< Stencil Buffer with a 24bits depth
		RENDERER_SDEPTH32		= 0x8000,	//!< Stencil Buffer with a 32bits depth
		RENDERER_SDEPTHMASK		= 0xF000,

		RENDERER_FORCE_DWORD	= 0x7fffffff
	};

	//! Rasterizer Clipping flags
	enum ClipFlag
	{
		CLIP_LEFT			= 0x0001,
		CLIP_RIGHT			= 0x0002,
		CLIP_TOP			= 0x0004,
		CLIP_BOTTOM			= 0x0008,
		CLIP_FRONT			= 0x0010,
		CLIP_BACK			= 0x0020,

		CLIP_FORCE_DWORD	= 0x7fffffff
	};

	//! Renderer creation structure
	struct ICERENDERER_API RENDERERCREATE
	{
										RENDERERCREATE();

				HWND					mHWnd;						//!< Window handle
				HINSTANCE				mhInstance;

				udword					mZBufferFormat;				//!< RENDERER_xxxx flags above
				bool					mFullscreen;				//!< Fullscreen or window mode
				bool					mAntialiasing;				//!< FSAA
				bool					mConsole;					//!< Create a console or not
				bool					mFrameBuffering;			//!< Enable/disable frame buffering
				bool					mStretchOnResize;			//!< Stretch window on resizes
				// Following parameters only for fullscreen modes
				bool					mStereo;					//!< Stereoscopic viewing
				bool					mShowCursorWhenFullscreen;	//!< Show cursor in fullscreen mode
				bool					mVSYNC;						//!< Use VSync or not
				udword					mWidth;						//!< Render width
				udword					mHeight;					//!< Render height
				udword					mDepth;						//!< Frame buffer depth (16, 24, 32 bits...)
	};

	//! Renderer Flags
	enum RendererFlag
	{
		RDF_RENDER_ALLOWED			= (1<<0),	//!< Rendering allowed (between BeginScene() and EndScene() calls)
		RDF_SOFTWARE_RENDERING		= (1<<1),	//!< Current renderer doesn't support hardware acceleration
		RDF_DISABLE_ESC_KEY			= (1<<2),	//!< Disable ESC key = exit

/*		RDF_FULLSCREEN				= (1<<0),
		RDF_FSAA					= (1<<1),
		RDF_STEREO					= (1<<2),
		RDF_ACTIVE					= (1<<3),
		RDF_USE_DEPTH_BUFFER		= (1<<4),
		RDF_USE_STENCIL_BUFFER		= (1<<5),
		RDF_SHOW_FULLSCREEN_CURSOR	= (1<<6),
		RDF_FRAME_BUFFERING			= (1<<7),
		RDF_VSYNC					= (1<<8),
*/
		RDF_READONLY				= RDF_RENDER_ALLOWED|RDF_SOFTWARE_RENDERING,

		RDF_FORCE_DWORD				= 0x7fffffff
	};

	//! Base renderer class
	class ICERENDERER_API Renderer : public IceInterface, public RenderBase
	{
#ifdef PATHACK
		public:
#else
		protected:
#endif
											Renderer();
		virtual								~Renderer();

											DECLARE_ICE_INTERFACE(Renderer, IceInterface)
											DECLARE_FLAGS(RendererFlag, mRDFlags, RDF_READONLY)
		public:

		virtual				void			EvictManagedTextures()	{}
		virtual				void			GlareTest(void* data)	{}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Initializes the renderer.
		 *	\param		create		[in] creation structure
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			bool				Initialize(const RENDERERCREATE& create);

		// Release the renderer
		virtual			bool				Release();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Presents the frame on the front buffer.
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			bool				ShowFrame()											= 0;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Move callback.
		 *	\param		x	[in] x position on screen
		 *	\param		y	[in] y position on screen
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			void				OnMove(udword x, udword y);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Renderer validation.
		 *	\return		true if the renderer is ready
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			bool				IsReady()											= 0;

		// Viewports

		inline_			Viewports&			GetViewports()	{ return mViewports;	}
						void				CreateDefaultViewport();
						bool				SetDefaultViewport();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Selects a viewport.
		 *	\param		vp		[in] viewport to select
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			bool				SetViewport(const Viewport& vp)						= 0;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Clears a viewport.
		 *	\param		vp		[in] viewport to clear
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			bool				ClearViewport(const Viewport& vp)					= 0;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Begins a scene. Use one BeginScene/EndScene for each viewport.
		 *	\see		EndScene()
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	inline_	bool				BeginScene()										= 0
											{
												mRDFlags |= RDF_RENDER_ALLOWED;
												return true;
											}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Ends a scene.
		 *	\see		BeginScene()
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	inline_	bool				EndScene()											= 0
											{
												mRDFlags &= ~RDF_RENDER_ALLOWED;
												return true;
											}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Resize callback.
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual inline_	bool				Resize()											= 0
											{
												IceCore::GetClientRect(mHWnd, &mScreenRect);
												mRenderWidth  = mScreenRect.right - mScreenRect.left;
												mRenderHeight = mScreenRect.bottom - mScreenRect.top;
												return true;
											}


		// Little helpers used to display used CPU time in an old fashioned way...

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Retrieves the scan-line that is currently being drawn on the monitor.
		 *	\return		the scanline number or -1 if failed
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			udword				GetScanline()										= 0;
						bool				StartTimeRecording();
						bool				EndTimeRecording();
						bool				DrawCPUTime(udword y=100);
		inline_			udword				GetLastRecordedTime()						const		{ return mLastScanline - mFirstScanline; }

		// Rendering helpers
						bool				FlushHelpers(float tweak=0.1f);
						VertexBuffer*		mHelpersVB;

						Renderer&			AddTriangle(const Point& p0, const Point& p1, const Point& p2, udword color=ARGB_WHITE);
		inline_			Renderer&			AddTriangle(const Triangle& tri, udword color=ARGB_WHITE)	{ AddTriangle(tri.mVerts[0], tri.mVerts[1], tri.mVerts[2], color);	return *this;	}
						Renderer&			AddLine(const Point& p0, const Point& p1, udword color=ARGB_WHITE);
						Renderer&			AddDashedLine(const Point& p0, const Point& p1, float step, udword color=ARGB_WHITE);
						Renderer&			AddPoint(const Point& p, udword color=ARGB_WHITE);

						float				ComputeConstantScale(const Point& pos, const ViewMatrix& view, const ProjMatrix& proj) const;
						float				ComputeConstantScale(const Point& pos, const ViewMatrix* view=null, const ProjMatrix* proj=null) const;
						void				ComputeScreenQuad(const ViewMatrix& inverse_view, const ViewMatrix& view, const ProjMatrix& proj, ubyte* verts, ubyte* uvs, udword stride, const Point& p0, const Point& p1, float size, bool constant_size) const;
						void				ComputeArrowPoints(const ViewMatrix& inverse_view, const ViewMatrix& view, const ProjMatrix& proj, const Point& p0, const Point& p1, float radius, bool constant_size, Point& a0, Point& a1) const;
						Point				ComputeWorldRay(sdword xs, sdword ys, float fov, const Point& pos, const ViewMatrix* view=null) const;
		// Primitive rendering
		virtual			bool				DrawPrimitive			(PrimType type, VertexFormat fvf, const void* verts, udword nb_verts, udword flags=0)												= 0;
		virtual			bool				DrawIndexedPrimitive	(PrimType type, VertexFormat fvf, const void* verts, udword nb_verts, const uword* indices, udword nb_indices, udword flags=0)		= 0;
		// Vertex buffer rendering
		virtual			bool				DrawPrimitiveVB			(PrimType type, VertexBuffer* vb, udword start_vertex, udword nb_verts, udword flags=0)												= 0;
		virtual			bool				DrawIndexedPrimitiveVB	(PrimType type, VertexBuffer* vb, udword start_vertex, udword nb_verts, const uword* indices, udword nb_indices, udword flags=0)	= 0;
		// ...
		virtual			bool				DrawIndexedPrimitive	(const VBDesc* desc, IndexBuffer* ib, udword subset)	= 0;

		// Render caps
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets the general renderer caps. Don't take the caps as granted! you're supposed to check them before using any of them!
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			bool				GetCaps()											= 0;

		// Vertex buffers
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Creates a vertex buffer.
		 *	\param		create	[in] vertex buffer creation structure
		 *	\return		the newly created vertex buffer, or null if failed
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			VertexBuffer*		CreateVertexBuffer(const VERTEXBUFFERCREATE& create)= 0;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Optimizes a vertex buffer.
		 *	\param		vb	[in] vertex buffer pointer
		 *	\return		Self-Reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			Renderer&			OptimizeVertexBuffer(VertexBuffer* vb)				= 0;

		// Vertex buffer factory
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets the vertex buffer factory. Actually a factory for a factory!
		 *	\return		vertex buffer factory, or null
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						VBFactory*			GetVertexBufferFactory();
						bool				FillVertexBuffer(VertexBuffer* vb, const void* data);

		// Index buffers
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Creates an index buffer.
		 *	\param		create	[in] index buffer creation structure
		 *	\return		the newly created index buffer, or null if failed
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			IndexBuffer*		CreateIndexBuffer(const INDEXBUFFERCREATE& create);

		// Textures
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Creates a texture.
		 *	\param		create	[in] texture creation structure
		 *	\return		the newly created texture, or null if failed
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			Texture*			CreateTexture(const TEXTURECREATE& create);
		virtual			Texture*			CreateCompressedTexture(const char* filename)
											{
												return null;
											}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Fits a texture to rendering capabilities:
		 *	- checks min & max limitations
		 *	- checks power-of-two limitations
		 *	- checks non-square limitations
		 *
		 *	\param		pic		[in] source picture for the forthcoming texture
		 *	\return		true if the picture has been resized
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool				FitToRenderer(Picture& pic);

		// Vertex shaders
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Creates a vertex shader.
		 *	\param		create	[in] vertex shader creation structure
		 *	\return		the newly created vertex shader, or null if failed
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			VertexShader*		CreateVertexShader(const VERTEXSHADERCREATE& create)
											{
												// Legacy renderers don't support vertex shaders
												return null;
											}

		// Pixel shaders
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Creates a pixel shader.
		 *	\param		create	[in] pixel shader creation structure
		 *	\return		the newly created pixel shader, or null if failed
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			PixelShader*		CreatePixelShader(const PIXELSHADERCREATE& create)
											{
												// Legacy renderers don't support pixel shaders
												return null;
											}

		// Vertex declarations
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Creates a vertex declaration.
		 *	\param		create	[in] vertex declaration creation structure
		 *	\return		the newly created vertex declaration, or null if failed
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			VertexDeclaration*	CreateVertexDeclaration(const VERTEXDECLARATIONCREATE& create)
											{
												// Legacy renderers don't support vertex declarations
												return null;
											}

		virtual			BOOL				SetVertexShaderConstantB(udword start_register, const BOOL* constant_data, udword bool_count)		{ return FALSE;	}
		virtual			BOOL				SetVertexShaderConstantF(udword start_register, const float* constant_data, udword vector_4f_count)	{ return FALSE;	}
		virtual			BOOL				SetVertexShaderConstantI(udword start_register, const udword* constant_data, udword vector_4i_count){ return FALSE;	}
		virtual			BOOL				SetPixelShaderConstantB(udword start_register, const BOOL* constant_data, udword bool_count)		{ return FALSE;	}
		virtual			BOOL				SetPixelShaderConstantF(udword start_register, const float* constant_data, udword vector_4f_count)	{ return FALSE;	}
		virtual			BOOL				SetPixelShaderConstantI(udword start_register, const udword* constant_data, udword vector_4i_count)	{ return FALSE;	}
		virtual			void				SetupTestPS()	{}

		// Render states
						void				SetRSCheckings(bool flags)				{ if(mRS)	mRS->SetCheckings(flags); }
						void				SetRSUpdateLog(bool flags)				{ if(mRS)	mRS->SetUpdateLog(flags); }

		// Render targets
		inline_			RenderTarget*		GetCurrentRenderTarget()		const	{ return mCurrentRenderTarget;	}
		inline_			RenderTarget*		GetRenderTarget()				const	{ return mRenderTarget;			}
		virtual			bool				SetRenderTarget(RenderTarget* target)				= 0;
		virtual			bool				CopyScreenToTarget()					{ return false;					}

		// Texture blur helper
		virtual			bool				IsBlurTextureSupported()	const				{ return false;			}
		virtual			bool				InitBlurTexture(udword width, udword height)	{ return false;			}
		virtual			void				ReleaseBlurTexture()							{						}
		virtual			bool				BlurTexture(Texture* source)					{ return false;			}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Translates a color to match hardware format.
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			void				TranslateColor(udword& color)			{}	// Default behaviour is to keep a RGBAPixel-compliant format

/*		// Lighting
		virtual			bool				SetLight(udword lightindex, Light* light)			= 0;
		virtual			bool				GetLight(udword lightindex, Light* light)			= 0;
		virtual			bool				GetLightEnable(udword lightindex, udword* enable)	= 0;
		virtual			bool				LightEnable(udword lightindex, bool enable)			= 0;*/
		// Lighting code

		inline_			XList*				GetLights()								{ return mLights;				}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Setups default lights in a scene. A default material is also applied.
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool				SetDefaultLights();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Registers a light.
		 *	\param		light		[in] the light
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool				RegisterLight(Light* light);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Unregisters a light.
		 *	\param		light		[in] the light
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool				UnregisterLight(Light* light);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Setups lighting properties for a given light.
		 *	\param		light		[in] the light
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			bool				SetupLightProps(Light* light)	= 0;

//		virtual			bool				GetLight(udword lightindex, Light* light);
//		virtual			bool				GetLightEnable(udword lightindex, udword* enable);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Switches a given light on or off.
		 *	\param		light		[in] the light
		 *	\param		enable		[in] light used or not
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	inline_	bool				SwitchLight(Light* light, bool enable)
											{
												if(!light)				return false;
												if(light->mVI==0xffff)	return false;	// Light has not been registered!
												return true;
											}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Switches all registered lights off.
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool				LightOff();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Switches all registered lights on.
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool				LightOn();

						bool				FadeScreen(float r, float g, float b, float coeff);

		virtual			bool				Print(udword x, udword y, const char* text, udword color=ARGB_YELLOW)	= 0;
		inline_			void				UpdateFPS()												{ mFPS.Update();				}
		inline_			float				GetFPS()										const	{ return mFPS.GetFPS();			}
		inline_			float				GetInstantFPS()									const	{ return mFPS.GetInstantFPS();	}
						bool				ShowFPS();

		inline_			float				GetGlobalLODHint()								const	{ return mGlobalLODHint;		}
		inline_			void				SetGlobalLODHint(float lod)								{ mGlobalLODHint = lod;			}

		virtual			bool				InvalidateCaches();

		// Event proc
		virtual			LRESULT				EventProc(sdword msg, sdword wparam, sdword lparam);

		inline_			const MouseInfo&	GetMouseInfo()									const	{ return mMouse;				}
		inline_			ControlInterface*	GetControlInterface()							const	{ return mCallbacks;			}
		inline_			Cell*				GetControlled()									const	{ return mControlled;			}
		inline_			void				SetCallbacks(ControlInterface* callbacks)				{ mCallbacks	= callbacks;	}
		inline_			void				SetControlled(Cell* controlled)							{ mControlled	= controlled;	}

#ifdef SUPPORT_CONSOLE
		// Console
						bool				CreateConsole();
						bool				DrawConsole();
						bool				UpdateConsole(udword msg, udword param);
		inline_			Console*			GetConsole()									const	{ return mConsole;				}
#endif
		// Data access
		inline_			bool				IsActive()										const	{ return mActive;				}
		inline_			udword				GetFullWidth()									const	{ return mFullWidth;			}
		inline_			udword				GetFullHeight()									const	{ return mFullHeight;			}
		inline_			udword				GetRenderWidth()								const	{ return mRenderWidth;			}
		inline_			udword				GetRenderHeight()								const	{ return mRenderHeight;			}
		inline_			HWND				GetWindowHandle()								const	{ return mHWnd;					}

		// Frame buffering
		inline_			void				SetFrameBuffering(bool flag)							{ mFrameBuffering = flag;		}
		inline_			bool				GetFrameBuffering()								const	{ return mFrameBuffering;		}
		inline_			void				FlipFrameBuffering()									{ mFrameBuffering = !mFrameBuffering;	}

		// VSYNC
		inline_			void				SetVSYNC(bool flag)										{ mVSYNC = flag;				}
		inline_			bool				GetVSYNC()										const	{ return mVSYNC;				}
		inline_			void				FlipVSYNC()												{ mVSYNC = !mVSYNC;				}

		// Mipmap generation
		inline_	const	MipmapGenerator*	GetMipmapGenerator()							const	{ return mMipmap;				}
		inline_			void				SetMipmapGenerator(const MipmapGenerator* mip)			{ mMipmap = mip;				}

		public:
		// Render states
						RenderStateManager*	mRS;
		// Render caps
						RenderCaps			mCaps;
#ifdef SUPPORT_CONSOLE
		// Console-related data
						Console*			mConsole;
#endif
		private:
		// Mouse info and mouse callbacks
						MouseInfo			mMouse;
						ControlInterface*	mCallbacks;
						Cell*				mControlled;
		// Current fps
						FPS					mFPS;
						float				mGlobalLODHint;
		// Triangle container
						Container			mTriangles;
		// Segment container
						Container			mLines;
		// Point container
						Container			mPoints;
		// Vertex buffer factory
						VBFactory*			mVBF;
		// Light-related data
						XList*				mLights;
		// Mipmap generator
		const			MipmapGenerator*	mMipmap;
		protected:
		// Window-related data
						HWND				mHWnd;				//!< Window handle
						HINSTANCE			mhInstance;
						RECT				mScreenRect;
		// Viewport-related data
						Viewports			mViewports;
		// Symbolic lists
						Container			mSLists;
		// Rendersurface-related data
						RenderTarget*		mCurrentRenderTarget;	//!< Shortcut to current render target
						RenderTarget*		mRenderTarget;			//!< Backbuffer object
						udword				mFullWidth;
						udword				mFullHeight;
						udword				mRenderWidth;
						udword				mRenderHeight;
						udword				mZBufferFormat;
						udword				mMinDepthBits;
						udword				mMinStencilBits;
		// CPU time helper
						udword				mFirstScanline;
						udword				mLastScanline;
		// Settings
						bool				mFullscreen;
						bool				mFSAA;
						bool				mStereo;
						bool				mActive;
						bool				mUseDepthBuffer;
						bool				mUseStencilBuffer;
						bool				mShowCursorWhenFullscreen;
						bool				mFrameBuffering;
						bool				mVSYNC;
						bool				mStretchOnResize;
	};

#endif // ICERENDERCORE_H
