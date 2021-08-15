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
		GLuint	createSingleColorTexture(udword width, udword height, const RGBAPixel& color, bool create_mipmaps);
		GLuint	createTexture(udword width, udword height, const RGBAPixel* pixels, bool create_mipmaps);

		void	releaseTexture(GLuint texId);
	}

#endif
