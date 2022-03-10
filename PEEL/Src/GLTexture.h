///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GLTEXTURE_H
#define GLTEXTURE_H

	namespace GLTexture
	{
		GLuint	CreateSingleColorTexture(udword width, udword height, const RGBAPixel& color, bool create_mipmaps);
		GLuint	CreateTexture(udword width, udword height, const RGBAPixel* pixels, bool create_mipmaps);
		void	UpdateTexture(GLuint texId, udword width, udword height, const RGBAPixel* pixels, bool createMipmaps);
		void	ReleaseTexture(GLuint texId);
		void	BlitTextureToScreen(GLuint texId);
	}

#endif
