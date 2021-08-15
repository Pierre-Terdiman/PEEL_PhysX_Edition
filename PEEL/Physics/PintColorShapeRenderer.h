///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COLOR_SHAPE_RENDERER_H
#define PINT_COLOR_SHAPE_RENDERER_H

#include "PintShapeRenderer.h"
#include "TextureManager.h"

	// TODO: consider splitting this class, the color isn't used in the shader when there's a texture. Or maybe we could modulate both.
	// Actually maybe this design is wrong anyway, we just need a material class somewhere
	class PintColorShapeRenderer : public PintShapeRenderer
	{
		friend PintColorShapeRenderer* CreatePintColorShapeRenderer(PintShapeRenderer* renderer, const RGBAColor& color, const ManagedTexture* texture);

										PREVENT_COPY(PintColorShapeRenderer)

										PintColorShapeRenderer(PintShapeRenderer* renderer, const RGBAColor& color, const ManagedTexture* texture=null);
		virtual							~PintColorShapeRenderer();

		public:

		// PintShapeRenderer
		virtual	PtrContainer*			GetOwnerContainer()			const	override;
		virtual	const char*				GetClassName()				const	override	{ return "PintColorShapeRenderer";	}
		virtual	void					_Render(const PR& pose)		const	override;
		virtual	const RGBAColor*		GetColor()					const	override	{ return &mData.mColor;				}
		virtual	const ManagedTexture*	GetTexture()				const	override	{ return mData.mTexture;			}
//		virtual	PintShapeRenderer*		GetSource()										{ return mRenderer;					}
		//~PintShapeRenderer

		struct Data
		{
			inline_	Data(PintShapeRenderer* source_renderer, const RGBAColor& color, const ManagedTexture* texture) : mRenderer(source_renderer), mColor(color), mTexture(texture){}

			PintShapeRenderer*		mRenderer;
			const ManagedTexture*	mTexture;
			const RGBAColor			mColor;
		};

		inline_ bool Equal(const PintColorShapeRenderer::Data& p)	const
		{
			return mData.mRenderer==p.mRenderer && mData.mColor==p.mColor && mData.mTexture==p.mTexture;
		};

				const Data				mData;
	};

	PintColorShapeRenderer* CreatePintColorShapeRenderer(PintShapeRenderer* renderer, const RGBAColor& color, const ManagedTexture* texture);

	void	RenderTransparent();
	void	ReleaseTransparent();

#endif