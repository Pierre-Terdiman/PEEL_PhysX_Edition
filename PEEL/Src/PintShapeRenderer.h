///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_SHAPE_RENDERER_H
#define PINT_SHAPE_RENDERER_H

//	struct ManagedTexture;
	struct PintSurfaceInterface;
	class ManagedTexture;
	class RenderDataChunk;

/*	enum ShapeRendererType
	{
		SHAPE_RENDERER_UNDEFINED,
		SHAPE_RENDERER_SPHERE,
		SHAPE_RENDERER_CAPSULE,
		SHAPE_RENDERER_CYLINDER,
		SHAPE_RENDERER_BOX,
		SHAPE_RENDERER_CONVEX,
		SHAPE_RENDERER_MESH,

		SHAPE_RENDERER_COLOR,
		SHAPE_RENDERER_COLLECTION,

		SHAPE_RENDERER_FORCE_DWORD	= 0x7fffffff
	};*/

	enum ShapeRendererFlags
	{
		SHAPE_RENDERER_CAN_BE_SHARED				= (1<<0),
		SHAPE_RENDERER_IS_REGISTERED				= (1<<1),
		SHAPE_RENDERER_TRANSPARENT					= (1<<2),	// ### maybe we'll need a pipeline ID instead, like in ICE
		SHAPE_RENDERER_USES_EXPLICIT_VERTEX_NORMALS	= (1<<3),
//		SHAPE_RENDERER_USES_BATCHING				= (1<<4),	// Skips per-shape SetupMaterial / SetupGeometry calls
	};

	class PintShapeRenderer	: public Allocateable
	{
		protected:
										PintShapeRenderer(/*const ShapeRendererType type,*/ udword flags=SHAPE_RENDERER_CAN_BE_SHARED) :
											mRenderSource	(null),
											mBaseFlags		(flags),
											mSortKey		(0),
											mRefCount		(1)
//											mID				(INVALID_ID),
//											mExportable		(false)
											//mType			(type)
															{ /*mColor = Point(1.0f, 1.0f, 1.0f);*/	}
		virtual							~PintShapeRenderer()	{}
		public:


		virtual	PtrContainer*			GetOwnerContainer()			const	{ return null;	}
		virtual	bool					Release();

		// The design here is a WIP battlefield.
		// Consider merging "CanBeShared" / mExportable / UseDirectData / etc into
		// simple public udword flags.

		virtual	const char*				GetClassName()				const	{ return null;	}
		virtual	udword					GetNbRenderers()			const	{ return 1;		}
		// ### just to get things working
		struct RenderData
		{
			PintShapeRenderer*	mRenderer;
			PR					mPose;
		};
		virtual	const RenderData*		GetRenderers()				const	{ return null;	}
		virtual	void					_Render(const PR& pose)		const	= 0;
		// Experimental
		virtual	const RGBAColor*		GetColor()					const	{ return null;	}
		virtual	const ManagedTexture*	GetTexture()				const	{ return null;	}
//		virtual	PintShapeRenderer*		GetSource()							{ return this;	}
		virtual	bool					UpdateVerts(const Point* verts, const Point* normals)	{ return false;	}

		// TODO: consider making the color a first-class citizen here, could simplify things as well.
//				Point					mColor;

		// This is for export purpose only. A less intrusive design would use a hashmap here but
		// for just one small ID it's much easier this way.
//				udword					mID;

		// TODO: flags or something
		// By default renderers aren't exportable, i.e. the renderer is fully implicit and defined by the shape.
		// We don't need to export them in that case, to make files smaller.
//				bool					mExportable;

		inline_	udword					_CanBeShared()				const	{ return mBaseFlags & SHAPE_RENDERER_CAN_BE_SHARED;					}
		inline_	udword					_UseExplicitVertexNormals()	const	{ return mBaseFlags & SHAPE_RENDERER_USES_EXPLICIT_VERTEX_NORMALS;	}

				RenderDataChunk*		mRenderSource;	// Experimental.
				udword					mBaseFlags;
				udword					mSortKey;
				udword					mRefCount;
				//const ShapeRendererType	mType;			// Experimental. Used to share renderers automatically.
	};

	udword				GetNbShapeRenderers();
	PintShapeRenderer*	GetShapeRenderer(udword i);
	bool				ReleaseShapeRenderer(PintShapeRenderer*);

	udword				GetNbRenderSources();
	RenderDataChunk*	GetRenderSource(udword i);

	void				ReleaseAllShapeRenderers();

	// All renderer creation goes through these functions
	PintShapeRenderer*	CreateNullRenderer();
	PintShapeRenderer*	CreateSphereRenderer(float radius, bool geo_sphere=false);
	PintShapeRenderer*	CreateCapsuleRenderer(float radius, float height);
	PintShapeRenderer*	CreateCylinderRenderer(float radius, float height);
	PintShapeRenderer*	CreateBoxRenderer(const Point& extents);
	PintShapeRenderer*	CreateConvexRenderer(udword nb_verts, const Point* verts);
	PintShapeRenderer*	CreateMeshRenderer(const PintSurfaceInterface& surface, const Point* normals=null, bool active_edges=false, bool direct_data=false);
	PintShapeRenderer*	CreateMeshRenderer(const MultiSurface& multi_surface, bool active_edges=false, bool direct_data=false);

	// Purely experimental
	PintShapeRenderer*	CreateColorShapeRenderer(PintShapeRenderer* renderer, const RGBAColor& color, const ManagedTexture* texture=null);

//	PintShapeRenderer*	CreateCustomRenderer(PintShapeRenderer*);


	// Even more experimental
	class PintRendererCollection : public PintShapeRenderer
	{
		public:
									PintRendererCollection();
		virtual						~PintRendererCollection();

		// PintShapeRenderer
		virtual	const char*			GetClassName()			const	override	{ return "PintRendererCollection";												}
		virtual	udword				GetNbRenderers()		const	override	{ return mRenderData.GetNbEntries()/(sizeof(RenderData)/sizeof(udword));		}
		virtual	const RenderData*	GetRenderers()			const	override	{ return (const PintRendererCollection::RenderData*)mRenderData.GetEntries();	}
		virtual	void				_Render(const PR& pose)	const	override;
		//~PintShapeRenderer

				void				AddRenderer(PintShapeRenderer* renderer, const PR& pose);
		private:
				Container			mRenderData;
	};

	PintShapeRenderer*	CreateRendererCollection();

#endif
