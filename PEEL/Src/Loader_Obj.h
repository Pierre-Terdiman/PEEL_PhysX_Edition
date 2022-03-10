///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef LOADER_OBJ_H
#define LOADER_OBJ_H

	class WavefrontMesh : public Allocateable
	{
		public:
									WavefrontMesh(const char* string_id, udword id);
									~WavefrontMesh();

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

	class WavefrontDatabase : public Allocateable
	{
		public:
							~WavefrontDatabase();

					void	Release();

		PtrContainer		mMeshes;
	};

	struct WavefrontLoaderParams
	{
		WavefrontLoaderParams() : mScale(1.0f), mTransform(Idt), mMergeMeshes(false)
		{
		}

		Matrix4x4	mTransform;
		float		mScale;
		bool		mMergeMeshes;
	};

	bool	LoadObj(const char* filename, const WavefrontLoaderParams& params, WavefrontDatabase& database);

#endif
