///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_ZCB_H
#define PINT_ZCB_H

#include "IceZCBFormat.h"
#include "IceZCBBreaker.h"
#include "SurfaceManager.h"

	class ZCBFactory : public ZCBBreaker, public SurfaceManager, public Allocateable
	{
		public:
						ZCBFactory();
		virtual			~ZCBFactory();

		// ZCBBreaker
		virtual	bool	NewScene		(const ZCBSceneInfo& scene);
		virtual	bool	NewCamera		(const ZCBCameraInfo& camera);
		virtual	bool	NewLight		(const ZCBLightInfo& light);
		virtual	bool	NewMaterial		(const ZCBMaterialInfo& material);
		virtual	bool	NewTexture		(const ZCBTextureInfo& texture);
		virtual	bool	NewMesh			(const ZCBMeshInfo& mesh);
		virtual	bool	NewShape		(const ZCBShapeInfo& shape);
		virtual	bool	NewHelper		(const ZCBHelperInfo& helper);
		virtual	bool	NewController	(const ZCBControllerInfo& controller);
		virtual	bool	NewMotion		(const ZCBMotionInfo& motion);
		virtual	bool	ZCBImportError	(const char* error_text, udword error_code);
		virtual	void	ZCBLog			(LPSTR fmt, ...);
		//~ZCBBreaker

//				PtrContainer	mMeshes;
	};

	class Pint;
	struct PintCaps;

	bool CreateZCBScene(Pint& pint, const PintCaps& caps, ZCBFactory& factory);

#endif
