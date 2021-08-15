///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GLTexture.h"

static GLuint createGLTexture(udword width, udword height, const GLubyte* buffer, bool create_mipmaps)
{
	GLuint texId;
	glGenTextures(1, &texId);

	glBindTexture(GL_TEXTURE_2D, texId);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	if(create_mipmaps)
	{
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	return texId;
}

GLuint GLTexture::createSingleColorTexture(udword width, udword height, const RGBAPixel& color, bool create_mipmaps)
{
	const udword NbPixels = width * height;
	GLubyte* buffer = new GLubyte[NbPixels * 4];
	udword NbToGo = NbPixels;
	GLubyte* dest = buffer;
	while(NbToGo--)
	{
		*dest++ = color.R;
		*dest++ = color.G;
		*dest++ = color.B;
		*dest++ = color.A;
	}

	const GLuint texId = createGLTexture(width, height, buffer, create_mipmaps);

	DELETEARRAY(buffer);
	return texId;
}

GLuint GLTexture::createTexture(udword width, udword height, const RGBAPixel* pixels, bool create_mipmaps)
{
	return createGLTexture(width, height, &pixels->R, create_mipmaps);
}

void GLTexture::releaseTexture(GLuint texId)
{
	glDeleteTextures(1, &texId);
}

/*
udword CreateTexture(udword width, udword height, const RGBAPixel* pixels)
{
	udword TextureID;
	glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_2D, TextureID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	sdword GLFormat = GL_RGBA8;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,	GL_TRUE);
//	glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);
//	glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_FASTEST);

	RGBAPixel* pixels0 = new RGBAPixel[width*height];
	for(udword j=0;j<height;j++)
	{
		for(udword i=0;i<width;i++)
		{
			if((i+j)&1)
			{
				pixels0[i+j*width].R = 255;
				pixels0[i+j*width].G = 255;
				pixels0[i+j*width].B = 255;
				pixels0[i+j*width].A = PIXEL_OPAQUE;
			}
			else
			{
				pixels0[i+j*width].R = 0;
				pixels0[i+j*width].G = 0;
				pixels0[i+j*width].B = 0;
				pixels0[i+j*width].A = PIXEL_OPAQUE;
			}
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GLFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels ? pixels : pixels0);
	DELETEARRAY(pixels0);
	return TextureID;
}
*/