///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GLScreenshot.h"

bool SaveScreenshotToFile(const char* filename, udword windowWidth, udword windowHeight, bool backBuffer)
{
	const int numberOfPixels = windowWidth * windowHeight * 3;
	unsigned char* pixels = new unsigned char[numberOfPixels];

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//ASSERT(GL_NO_ERROR == glGetError());
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	//ASSERT(GL_NO_ERROR == glGetError());
	glReadBuffer(backBuffer ? GL_BACK : GL_FRONT);
	//ASSERT(GL_NO_ERROR == glGetError());
	glReadPixels(0, 0, windowWidth, windowHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
	//ASSERT(GL_NO_ERROR == glGetError());

	FILE* outputFile = fopen(filename, "wb");
	if(!outputFile)
		return false;

	const short header[] = {0, 2, 0, 0, 0, 0, short(windowWidth), short(windowHeight), 24};

	fwrite(&header, sizeof(header), 1, outputFile);
	fwrite(pixels, numberOfPixels, 1, outputFile);
	fclose(outputFile);

	//printf("Finish writing to file.\n");
	delete [] pixels;
	return true;
}
