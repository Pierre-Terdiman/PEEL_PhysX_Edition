///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef LEGOLIB_H
#define LEGOLIB_H

#include "..\TextureManager.h"

	class LegoPartMesh : public Allocateable
	{
		public:
									LegoPartMesh(const char* string_id, udword id);
									~LegoPartMesh();

		inline_		udword			GetNbVerts()	const	{ return mVertices.GetNbVertices();	}
		inline_		udword			GetNbTVerts()	const	{ return mUVs.GetNbVertices();		}
		inline_		udword			GetNbTris()		const	{ return mIndices.GetNbEntries()/3;	}
		inline_		udword			GetNbTTris()	const	{ return mTIndices.GetNbEntries()/3;}
		inline_		const Point*	GetVerts()		const	{ return mVertices.GetVertices();	}
		inline_		const Point*	GetTVerts()		const	{ return mUVs.GetVertices();		}
		inline_		const udword*	GetIndices()	const	{ return mIndices.GetEntries();		}
		inline_		const udword*	GetTIndices()	const	{ return mTIndices.GetEntries();	}

					void			Recenter();

					udword			mID;
					Vertices		mVertices;
					Vertices		mUVs;
					Container		mIndices;
					Container		mTIndices;
					Point			mPos;
					String			mStringID;
					void*			mUserData;
	};

	class LegoImage : public Allocateable
	{
		public:
									LegoImage(const char* id);
									~LegoImage();

			String					mID;
			String					mFileName;
//			ManagedTexture			mTexture;
			const ManagedTexture*	mTexture;
	};

	class LegoEffect : public Allocateable
	{
		public:
							LegoEffect(const char* id);
							~LegoEffect();

			String			mID;
			String			mTexture;
			RGBAColor		mDiffuse;
			LegoImage*		mImage;
	};

	class LegoMaterial : public Allocateable
	{
		public:
							LegoMaterial(const char* id);
							~LegoMaterial();

			String			mID;
			LegoEffect*		mEffect;
	};

	class LegoActor : public Allocateable
	{
		public:
							LegoActor();
							~LegoActor();

			LegoPartMesh*	mMesh;
			LegoMaterial*	mMaterial;
			Matrix4x4		mPose;
			void*			mUserData;
	};

	bool				InitLegoLib(const char* filename, float scale_factor);
	void				CloseLegoLib();

	udword				GetNbLegoParts();
	const LegoPartMesh*	GetLegoPartByID(udword id);
	const LegoPartMesh*	GetLegoPartByIndex(udword i);

	udword				GetNbLegoActors();
	const LegoActor*	GetLegoActorByIndex(udword i);

#endif
