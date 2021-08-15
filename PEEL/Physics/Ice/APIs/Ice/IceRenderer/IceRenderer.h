///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main file for IceRenderer.dll.
 *	\file		IceRenderer.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERENDERER_H
#define ICERENDERER_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compilation messages
#if defined(ICERENDERER_EXPORTS)
	#pragma message("----Compiling ICE Renderer")
#elif !defined(ICERENDERER_EXPORTS)
	#pragma message("----Using ICE Renderer")
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Automatic linking
	#ifndef BAN_ICERENDERER_AUTOLINK
		#ifdef _DEBUG
			#pragma comment(lib, "IceRenderer_D.lib")
		#else
			#pragma comment(lib, "IceRenderer.lib")
		#endif
	#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Preprocessor

#ifndef ICE_NO_DLL
	#ifdef ICERENDERER_EXPORTS
		#define ICERENDERER_API			__declspec(dllexport)
	#else
		#define ICERENDERER_API			__declspec(dllimport)
	#endif
#else
		#define ICERENDERER_API
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes

	#include "IceRendererSettings.h"

	namespace IceRenderer
	{
		#include "IceRendererErrors.h"
		// Console
		#include "IceConsole.h"
		#include "IceBasicCmds.h"

		#include "IceDetectCard.h"

		#include "IcePixelFormat.h"
		#include "IceVertexFormat.h"

		#include "IceTextureBasis.h"

		// Programmable shaders
		#include "IceVertexShader.h"
		#include "IcePixelShader.h"
		#include "IceVertexDeclaration.h"

		#include "IceRenderBase.h"
		#include "IceRenderMatrices.h"
		#include "IceLight.h"
		#include "IceSurface.h"
		#include "IceTexture.h"
		#include "IceRenderCaps.h"
		#include "IcePrimitive.h"
		#include "IceVertexBuffer.h"
		#include "IceVertexBufferFactory.h"
		#include "IceVBMultiplexer.h"
		#include "IceIndexBuffer.h"
		#include "IceRenderStates.h"
		#include "IceMaterial.h"
		#include "IceDynamicStateManager.h"
		#include "IceRenderStateManager.h"
		#include "IceStateBlock.h"
		#include "IceControlInterface.h"
		#include "IceViewport.h"
		#include "IceMipmap.h"
		#include "IceRenderCore.h"
		#include "IceRenderHelpers.h"
		#include "IceRenderDebug.h"
		#include "IceCubeMap.h"
		#include "IceAnchor.h"
		#include "IceFog.h"
		#include "IceProjector.h"
		#include "IceClipping.h"
		#include "IceVirtualTrackball.h"
		#include "IceScreenQuad.h"
		#include "IceTextRenderer.h"

		// High-level render
		#include "IcePipeline.h"
		#include "IceConsolidation.h"
		#include "IceConsolidatedSurface.h"
		#include "IceRenderableSurface.h"
		#include "IceTriPusher.h"
		#include "IceDecalManager.h"
		#include "IceBillboardManager.h"
		#include "IceShader.h"
		#include "IceShaderCompiler.h"

		// Software rendering & Scanline
		#include "IceXForm.h"
		#include "IceScanline.h"
		#include "IceFlat.h"
		#include "IceIllumination.h"
		#include "IceTextureAddress.h"

		// Void rendering
		#include "IceVoidRenderer.h"

		#include "IceRendererAPI.h"
	}

#endif // ICERENDERER_H


