///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

	class ManagedTexture : public Allocateable
	{
							ManagedTexture(udword width, udword height, const RGBAPixel* pixels, const char* filename, udword index);
							~ManagedTexture();
		public:
				String		mFilename;
				udword		mWidth;		// Copy of width param, for texture tool
				udword		mHeight;	// Copy of height param, for texture tool
				Picture*	mSource;	// CPU-side copy only available in editor mode
		mutable	GLuint		mGLID;		// OpenGL texture ID
				udword		mID;		// Index in internal texture array (for ReleaseManagedTexture)
		mutable	GLuint		mExportID;	// Temp ID used during export, could be stored in a local hashmap instead but oh well

		friend const ManagedTexture* CreateManagedTexture(udword width, udword height, const RGBAPixel* pixels, const char* filename);
		friend bool ReleaseManagedTexture(const ManagedTexture* texture);
		friend void ReleaseManagedTextures();
	};

	udword					GetNbManagedTextures();
	const ManagedTexture*	GetManagedTexture(udword index);
	const ManagedTexture*	CreateManagedTexture(udword width, udword height, const RGBAPixel* pixels, const char* filename);
	bool					ReleaseManagedTexture(const ManagedTexture* texture);
	void					ReleaseManagedTextures();
	udword					GetManagedTexturesTimestamp();

	// The system textures below are not necessary, this is only to be able to display them in the texture tool.
	class SystemTexture : public Allocateable
	{
		public:
		SystemTexture() : mWidth(0), mHeight(0), mGLID(INVALID_ID)	{}

		String	mName;
		udword	mWidth;
		udword	mHeight;
		GLuint	mGLID;
	};
	udword					GetNbSystemTextures();
	const SystemTexture*	GetSystemTexture(udword index);
	SystemTexture*			CreateSystemTexture(udword width, udword height, GLuint id, const char* name);
	void					ReleaseSystemTextures();
	udword					GetSystemTexturesTimestamp();

#endif
