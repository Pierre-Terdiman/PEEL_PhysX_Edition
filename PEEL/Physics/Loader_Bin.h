///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef LOADER_BIN_H
#define LOADER_BIN_H

	enum TessellationScheme
	{
		TESS_BUTTERFLY,
		TESS_POLYHEDRAL,
	};

	struct BinLoaderSettings
	{
		BinLoaderSettings(float scale=1.0f) : mScale(scale, scale, scale), mTessellation(0), mTS(TESS_BUTTERFLY), mMergeMeshes(false), mResetPivot(false)
		{
		}

		Point				mScale;
		udword				mTessellation;
		TessellationScheme	mTS;
		bool				mMergeMeshes;
		bool				mResetPivot;
	};

	class SurfaceManager;
	void LoadMeshesFromFile_(SurfaceManager& test, const char* filename, const Point* scale=null, bool mergeMeshes=false, udword tessellation=0, TessellationScheme ts = TESS_BUTTERFLY);
	void LoadBinMeshesFromFile(SurfaceManager& test, const char* filename, const BinLoaderSettings& settings);

#endif
