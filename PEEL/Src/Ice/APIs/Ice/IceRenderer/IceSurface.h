///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains surface-related code.
 *	\file		IceSurface.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESURFACE_H
#define ICESURFACE_H

	class ICERENDERER_API RenderTarget : public RenderBase
	{
		public:
		//! Constructor
						RenderTarget() 		{}
		//! Destructor
		virtual			~RenderTarget()		{}

		// Render target interface
		virtual	bool	SetSurface(void* surface)	= 0;
		virtual	void*	GetSurface()				= 0;
		virtual	udword	ReleaseTarget()				= 0;
		virtual	void	RestoreTarget()				= 0;
	};

	// Surface lock flags (i.e. Vertex Buffers, Textures...)
	enum LockFlags
	{
		LOCK_WAIT				= 0x00000001,	//!< If a lock cannot be obtained immediately, the method retries until a lock is obtained or another error occurs.
		LOCK_READONLY			= 0x00000010,	//!< Indicates that the memory being locked is only to be read from.
		LOCK_WRITEONLY			= 0x00000020,	//!< Indicates that the memory being locked is only to be written to.
		LOCK_NOSYSLOCK			= 0x00000800,	//!< If possible, do not take the Win16Mutex (also known as Win16Lock)
		LOCK_NOOVERWRITE		= 0x00001000,	//!< Used only with vertex-buffer locks. Indicates that no vertices that were referred to in vertex-buffer DrawPrimitive
												//!< calls since the start of the frame (or the last lock without this flag) are modified during the lock. This can be
												//!< useful when you want only to append data to the vertex buffer.
		LOCK_DISCARDCONTENTS	= 0x00002000,	//!< Indicates that no assumptions are made about the contents of the vertex buffer during this lock. This enables Direct3D
												//!< or the driver to provide an alternative memory area as the vertex buffer. This is useful when you plan to clear the
												//!< contents of the vertex buffer and fill in new data. 
		LOCK_DONOTWAIT			= 0x00004000,	//!< Not for VBs.

		LOCK_FLUSH				= LOCK_NOSYSLOCK | LOCK_WRITEONLY | LOCK_DISCARDCONTENTS,
		LOCK_APPEND				= LOCK_NOSYSLOCK | LOCK_WRITEONLY | LOCK_NOOVERWRITE,

		LOCK_FORCE_DWORD		= 0x7fffffff,
	};

	//
	class ICERENDERER_API Hardwired : public RenderBase
	{
		public:
		//! Constructor
						Hardwired() : mLock(null)	{}
		//! Destructor
		virtual			~Hardwired()				{}

		// Lock/Unlock
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Locks the hardwired surface and get a pointer to it.
		 *	\param		flags	[in] LockFlags combination
		 *	\param		offset	[in] Offset into the vertex data to lock, in bytes
		 *	\param		size	[in] Size of the vertex data to lock, in bytes
		 *	\return		pointer to locked memory, or null
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	void*	Lock(udword flags = 0, udword offset = 0, udword size = 0)	= 0;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Unlocks a previously locked surface.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	void	Unlock()								= 0;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets the cached locked surface pointer.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	void*	GetLockedSurface()	const	{ return mLock;		}

		protected:
		// Lock
				void*	mLock;			//!< Locked surface (cached address)
	};


	// Remarks for vertex buffers:
	//	LOCK_DISCARDCONTENTS	The application overwrites, with a write-only operation, the entire index buffer. This enables Direct3D to return a
	//							pointer to a new memory area so that the dynamic memory access (DMA) and rendering from the old area do not stall.
	//	LOCK_NOOVERWRITE		Indicates that no vertices that were referred to in drawing calls since the start of the frame or the last lock without
	//							this flag will be modified during the lock. This can enable optimizations when the application is appending data only to
	//							the vertex buffer.
	//	LOCK_NOSYSLOCK			The default behavior of a video memory lock is to reserve a system-wide critical section, guaranteeing that no display
	//							mode changes will occur for the duration of the lock. This flag causes the system-wide critical section not to be held
	//							for the duration of the lock. The lock operation is slightly more expensive, but can enable the system to perform other
	//							duties, such as moving the mouse cursor. This flag is useful for long-duration locks, such as the lock of the back buffer
	//							for software rendering that would otherwise adversely affect system responsiveness.
	//	LOCK_READONLY			The application will not write to the buffer. This enables some optimizations.LOCK_READONLY cannot be specified with
	//							LOCK_DISCARD, nor can it be specified on a vertex buffer created with USAGE_WRITEONLY.


	// Remarks
	// When working with vertex buffers, you are allowed to make multiple lock calls; however, you must ensure that the number of lock calls match
	// the number of unlock calls. DrawPrimitive calls will not succeed with any outstanding lock count on any currently set vertex buffer.
	// The LOCK_DISCARD and LOCK_NOOVERWRITE flags are valid only on buffers created with D3DUSAGE_DYNAMIC.
	// See Using Dynamic Vertex and Index Buffers for information on using D3DLOCK_DISCARD or D3DLOCK_NOOVERWRITE for the Flags parameter of the Lock method.


/*
	dwCaps:

	DDSCAPS_3DDEVICE		This surface can be used for 3-D rendering. Applications can use this flag to ensure that a device that can render only
							to a certain heap has off-screen surfaces allocated from the correct heap. If this flag is set for a heap, the surface is
							not allocated from that heap.
	DDSCAPS_ALLOCONLOAD		Not used.
	DDSCAPS_ALPHA			This surface contains alpha-only information.
	DDSCAPS_BACKBUFFER		This surface is the back buffer of a surface flipping structure. Typically, this capability is set by the CreateSurface
							method when the DDSCAPS_FLIP flag is used. Only the surface that immediately precedes the DDSCAPS_FRONTBUFFER surface has
							this capability set. The other surfaces are identified as back buffers by the presence of the DDSCAPS_FLIP flag, their
							attachment order, and the absence of DDSCAPS_FRONTBUFFER and DDSCAPS_BACKBUFFER capabilities. If this capability is sent to
							the CreateSurface method, a stand-alone back buffer is being created. After this method is called, this surface could be
							attached to a front buffer, another back buffer, or both to form a flipping surface structure. For more information, see
							IDirectDrawSurface7::AddAttachedSurface. DirectDraw supports any number of surfaces in a flipping structure.
	DDSCAPS_COMPLEX			A complex surface is being described. A complex surface results in the creation of more than one surface. The additional
							surfaces are attached to the root surface. The complex surface can be destroyed only by destroying the root.
	DDSCAPS_FLIP			This surface is a part of a surface-flipping structure. When this capability is passed to the CreateSurface method, a front
							buffer and one or more back buffers are created. DirectDraw sets the DDSCAPS_FRONTBUFFER bit on the front-buffer surface and
							the DDSCAPS_BACKBUFFER bit on the surface adjacent to the front-buffer surface. The dwBackBufferCount member of the
							DDSURFACEDESC structure must be set to at least 1 for the method call to succeed. The DDSCAPS_COMPLEX capability must always
							be set when creating multiple surfaces by using the CreateSurface method.
	DDSCAPS_FRONTBUFFER		This surface is the front buffer of a surface-flipping structure. This flag is typically set by the CreateSurface method when
							the DDSCAPS_FLIP capability is set. If this capability is sent to the CreateSurface method, a stand-alone front buffer is
							created. This surface does not have the DDSCAPS_FLIP capability. It can be attached to back buffers to form a flipping
							structure by using IDirectDrawSurface7::AddAttachedSurface.
	DDSCAPS_HWCODEC			This surface can have a stream decompressed to it by the hardware.
	DDSCAPS_LIVEVIDEO		This surface can receive live video.
	DDSCAPS_LOCALVIDMEM		This surface exists in true, local video memory, rather than nonlocal video memory. If this flag is specified,
							DDSCAPS_VIDEOMEMORY must be specified, as well. This flag cannot be used with the DDSCAPS_NONLOCALVIDMEM flag.
	DDSCAPS_MIPMAP			This surface is one level of a mipmap. This surface is attached to other DDSCAPS_MIPMAP surfaces to form the mipmap.
							This can be done explicitly by creating a number of surfaces and attaching them by using the IDirectDrawSurface7::AddAttachedSurface
							method, or implicitly by the CreateSurface method. If this capability is set, DDSCAPS_TEXTURE must also be set.
	DDSCAPS_MODEX			This surface is a 320×200 or 320×240 Mode X surface.
	DDSCAPS_NONLOCALVIDMEM	This surface exists in nonlocal video memory, rather than true, local video memory. If this flag is specified,
							DDSCAPS_VIDEOMEMORY flag must be specified, as well. This cannot be used with the DDSCAPS_LOCALVIDMEM flag.
	DDSCAPS_OFFSCREENPLAIN	This surface is any off-screen surface that is not an overlay, texture, z-buffer, front-buffer, back-buffer, or alpha surface.
							It is used to identify plain surfaces.
	DDSCAPS_OPTIMIZED		Not currently implemented.
	DDSCAPS_OVERLAY			This surface is an overlay. It might or might not be directly visible, depending on whether it is currently being overlaid onto
							the primary surface. DDSCAPS_VISIBLE can be used to determine if it is currently being overlaid.
	DDSCAPS_OWNDC			This surface has a device context (DC) association for a long period of time.
	DDSCAPS_PALETTE			This device driver allows unique DirectDrawPalette objects to be created and attached to this surface.
	DDSCAPS_PRIMARYSURFACE	The surface is the primary surface. It represents what is currently visible.
	DDSCAPS_STANDARDVGAMODE	This surface is a standard VGA mode surface, and not a Mode X surface. This flag cannot be used in combination with the
							DDSCAPS_MODEX flag.
	DDSCAPS_SYSTEMMEMORY	This surface memory was allocated in system memory.
	DDSCAPS_TEXTURE			This surface can be used as a 3-D texture. It does not indicate whether the surface is currently being used for that purpose.
	DDSCAPS_VIDEOMEMORY		This surface exists in display memory.
	DDSCAPS_VIDEOPORT		This surface can receive data from a video port.
	DDSCAPS_VISIBLE			Changes made to this surface are immediately visible. It is always set for the primary surface, as well as for overlays
							while they are being overlaid and texture maps while they are being textured.
	DDSCAPS_WRITEONLY		Only write access is permitted to the surface. Read access from the surface can cause a general protection (GP) fault, and
							the read results from this surface would not be meaningful.
	DDSCAPS_ZBUFFER			This surface is the z-buffer. The z-buffer contains information that cannot be displayed. Instead, it contains bit-depth
							information that is used to determine which pixels are visible and which are obscured
*/

/*
	dwCaps2:

	DDSCAPS2_CUBEMAP				New for DirectX 7.0. This surface is a cubic environment map. When using this flag, also specify which face or faces
									of the cubic environment map to create.
	DDSCAPS2_CUBEMAP_POSITIVEX		New for DirectX 7.0. This flag is used with the DDSCAPS2_CUBEMAP flag to create the positive X face of a cubic environment map.
	DDSCAPS2_CUBEMAP_NEGATIVEX		New for DirectX 7.0. This flag is used with the DDSCAPS2_CUBEMAP flag to create the negative X face of a cubic environment map.
	DDSCAPS2_CUBEMAP_POSITIVEY		New for DirectX 7.0. This flag is used with the DDSCAPS2_CUBEMAP flag to create the positive Y face of a cubic environment map.
	DDSCAPS2_CUBEMAP_NEGATIVEY		New for DirectX 7.0. This flag is used with the DDSCAPS2_CUBEMAP flag to create the negative Y face of a cubic environment map.
	DDSCAPS2_CUBEMAP_POSITIVEZ		New for DirectX 7.0. This flag is used with the DDSCAPS2_CUBEMAP flag to create the positive Z face of a cubic environment map.
	DDSCAPS2_CUBEMAP_NEGATIVEZ		New for DirectX 7.0. This flag is used with the DDSCAPS2_CUBEMAP flag to create the negative Z face of a cubic environment map.
	DDSCAPS2_CUBEMAP_ALLFACES		New for DirectX 7.0. This flag is used with the DDSCAPS2_CUBEMAP flag to create all six faces of a cubic environment map.
	DDSCAPS2_D3DTEXTUREMANAGE		New for DirectX 7.0. The texture is always managed by Direct3D.
	DDSCAPS2_DONOTPERSIST			New for DirectX 7.0. The managed surface can be safely lost.
	DDSCAPS2_HARDWAREDEINTERLACE	This surface receives data from a video port, using the de-interlacing hardware. This allows the driver to allocate memory for any extra buffers that might be required. The DDSCAPS_VIDEOPORT and DDSCAPS_OVERLAY flags must also be set. 
	DDSCAPS2_HINTANTIALIASING		The application intends to use antialiasing. Only valid if DDSCAPS_3DDEVICE is also set. For more information, see Antialiasing. 
	DDSCAPS2_HINTDYNAMIC			Indicates to the driver that this surface is locked very frequently (for procedural textures, dynamic light maps, and so on). This flag can only be used for texture surfaces (DDSCAPS_TEXTURE flag set in the dwCaps member). This flag cannot be used with the DDSCAPS2_HINTSTATIC or DDSCAPS2_OPAQUE flags. 
	DDSCAPS2_HINTSTATIC				Indicates to the driver that this surface can be reordered or retiled on load. This operation does not change the size of the texture. It is relatively fast and symmetrical, since the application can lock these bits (although it degrades performance when doing so). This flag can only be used for texture surfaces (DDSCAPS_TEXTURE flag set in the dwCaps member). This flag cannot be used with the DDSCAPS2_HINTDYNAMIC or DDSCAPS2_OPAQUE flags. 
	DDSCAPS2_MIPMAPSUBLEVEL			New for DirectX 7.0. It enables easier use of GetAttachedSurface, rather than EnumAttachedSurfaces, for surface constructs such as cube maps, in which there is more than one mipmap surface attached to the root surface. This should be set on all non-top-level surfaces in a mipmapped cube map so that a call to GetAttachedSurface can distinguish between top-level faces and attached mipmap levels. This capability bit is ignored by CreateSurface. 
	DDSCAPS2_OPAQUE					Indicates to the driver that this surface will never be locked again. The driver is free to optimize this surface by retiling and compression. Such a surface cannot be locked or used in blit operations; attempts to lock or blit a surface with this capability fail. This flag can only be used for texture surfaces (DDSCAPS_TEXTURE flag set in the dwCaps member). This flag cannot be used with the DDSCAPS2_HINTDYNAMIC or DDSCAPS2_HINTSTATIC flags. 
	DDSCAPS2_STEREOSURFACELEFT		New for DirectX 7.0. This surface is part of a stereo flipping chain. When this flag is set during a IDirectDraw7::CreateSurface call, a pair of stereo surfaces are created for each buffer in the primary flipping chain. You must create a complex flipping chain (with back buffers). You cannot create a single set of stereo surfaces. The IDirectDrawSurface7::Flip method requires back buffers, so at least 4 surfaces must be created. 
									In addition, when this flag is set in a DDSURFACEDESC structure as the result of an IDirectDraw7::EnumDisplayModes or IDirectDraw7::GetDisplayMode call, it indicates support for stereo in that mode. 
	DDSCAPS2_TEXTUREMANAGE			The client would like this texture surface to be managed by the driver if the driver is capable; otherwise, it is managed by Direct3D Immediate Mode (new in DirectX 7.0). This flag can be used only for texture surfaces (DDSCAPS_TEXTURE flag set in the dwCaps member). For more information, see Automatic Texture Management in the Direct3D Immediate Mode documentation. Do not use this flag if your application uses Direct3D Retained Mode. Instead, create textures in system memory, and allow Retained Mode to manage them. 
*/

#endif // ICESURFACE_H
