#ifndef RENDER_TARGET
#define RENDER_TARGET

#include "FrameBufferObject.h"

class SystemTexture;

namespace PEEL	// To avoid conflicts with the ICE RenderTarget class
{
	class RenderTarget 
	{
		public:
									RenderTarget(udword width, udword height);
									~RenderTarget();

				void				Resize(udword width, udword height);

				void				BeginCapture();
				void				EndCapture();

		inline_	GLuint				GetColorTexId()	const	{ return mColorTexId;	}
		inline_	GLuint				GetDepthTexId()	const	{ return mDepthTexId;	}
		inline_	udword				GetWidth()		const	{ return mWidth;		}
		inline_	udword				GetHeight()		const	{ return mHeight;		}

		private:
				void				Clear();

				FrameBufferObject*	mFBO;
				GLuint				mColorTexId;
				GLuint				mDepthTexId;
				udword				mWidth;
				udword				mHeight;

				SystemTexture*		mColorTexture;
				SystemTexture*		mDepthTexture;
	};
}

#endif