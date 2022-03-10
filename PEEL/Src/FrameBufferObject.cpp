#include "stdafx.h"

#include "FrameBufferObject.h"
#include <iostream>
using namespace std;

#define LOG_FBO_CALLS 0

static GLuint _GenerateFboId()
{
	GLuint id = 0;
	glGenFramebuffersEXT(1, &id);
	//glGenFramebuffers(1, &id);

#if LOG_FBO_CALLS
	printf("created FBO %d\n", id);
#endif

	return id;
}

FrameBufferObject::FrameBufferObject() : m_fboId(_GenerateFboId()), m_savedFboId(0)
{
	// Bind this FBO so that it actually gets created now
	_GuardedBind();
	_GuardedUnbind();
}

FrameBufferObject::~FrameBufferObject() 
{
#if LOG_FBO_CALLS
	printf("deleting FBO %d\n", m_fboId);
#endif
	glDeleteFramebuffersEXT(1, &m_fboId);
}

void FrameBufferObject::Bind() 
{
#if LOG_FBO_CALLS
	printf("binding framebuffer %d\n", m_fboId);
#endif
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboId);
}

void FrameBufferObject::Disable() 
{
#if LOG_FBO_CALLS
	printf("binding window framebuffer \n");
#endif
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void FrameBufferObject::AttachTexture(GLenum texTarget, GLuint texId, GLenum attachment, int mipLevel, int zSlice)
{
	_GuardedBind();

  /*
#ifndef NDEBUG
  if( GetAttachedId(attachment) != texId ) {
#endif
  */

	{
#if LOG_FBO_CALLS
		printf("fbo %d: attaching texture %d to attachment point %d\n", m_fboId, texId, attachment);
#endif

		if(texTarget == GL_TEXTURE_1D)
			glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_1D, texId, mipLevel);
		else if(texTarget == GL_TEXTURE_3D)
			glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_3D, texId, mipLevel, zSlice);
		else	// Default is GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_ARB, or cube faces
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, texTarget, texId, mipLevel);
	}

/*
#ifndef NDEBUG
  }
  else {
    cerr << "FrameBufferObject::AttachTexture PERFORMANCE WARNING:\n"
      << "\tRedundant bind of texture (id = " << texId << ").\n"
      << "\tHINT : Compile with -DNDEBUG to remove this warning.\n";
  }
#endif
*/

	_GuardedUnbind();
}

/*void FrameBufferObject::AttachTextures( int numTextures, GLenum texTarget[], GLuint texId[], GLenum attachment[], int mipLevel[], int zSlice[] )
{
	for(int i = 0; i < numTextures; ++i)
	{
		AttachTexture( texTarget[i], texId[i], attachment ? attachment[i] : (GL_COLOR_ATTACHMENT0_EXT + i), mipLevel ? mipLevel[i] : 0, zSlice ? zSlice[i] : 0 );
	}
}*/

/*void FrameBufferObject::AttachRenderBuffer( GLuint buffId, GLenum attachment )
{
	_GuardedBind();

#ifndef NDEBUG
	if( GetAttachedId(attachment) != buffId )
	{
#endif

#if LOG_FBO_CALLS
	printf("fbo %d: attaching renderbuffer to attachment point %d\n",m_fboId,attachment);
#endif

	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, attachment, GL_RENDERBUFFER_EXT, buffId);

#ifndef NDEBUG
	}
	else
	{
		cerr << "FrameBufferObject::AttachRenderBuffer PERFORMANCE WARNING:\n"
		<< "\tRedundant bind of Renderbuffer (id = " << buffId << ")\n"
		<< "\tHINT : Compile with -DNDEBUG to remove this warning.\n";
	}
#endif

	_GuardedUnbind();
}

void FrameBufferObject::AttachRenderBuffers( int numBuffers, GLuint buffId[], GLenum attachment[] )
{
	for(int i = 0; i < numBuffers; ++i)
	{
		AttachRenderBuffer( buffId[i], attachment ? attachment[i] : (GL_COLOR_ATTACHMENT0_EXT + i) );
	}
}

void FrameBufferObject::Unattach( GLenum attachment )
{
	_GuardedBind();
	GLenum type = GetAttachedType(attachment);

#if LOG_FBO_CALLS
	 printf("fbo %d: detatching from attachment point %d\n",m_fboId,attachment);
#endif

	switch(type)
	{
		case GL_NONE:
			break;
		case GL_RENDERBUFFER_EXT:
			AttachRenderBuffer( 0, attachment );
			break;
		case GL_TEXTURE:
			AttachTexture( GL_TEXTURE_2D, 0, attachment );
			break;
		default:
			cerr << "FrameBufferObject::unbind_attachment ERROR: Unknown attached resource type\n";
	}
	_GuardedUnbind();
}

void FrameBufferObject::UnattachAll()
{
	int numAttachments = GetMaxColorAttachments();
	for(int i = 0; i < numAttachments; ++i)
	{
		Unattach( GL_COLOR_ATTACHMENT0_EXT + i );
	}
}*/

GLint FrameBufferObject::GetMaxColorAttachments()
{
	GLint maxAttach = 0;
	glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS_EXT, &maxAttach );
	return maxAttach;
}

void FrameBufferObject::_GuardedBind() 
{
	// Only binds if m_fboId is different than the currently bound FBO
	glGetIntegerv( GL_FRAMEBUFFER_BINDING_EXT, &m_savedFboId );
	if (m_fboId != (GLuint)m_savedFboId)
	{
#if LOG_FBO_CALLS
		printf("binding framebuffer %d\n",m_fboId);
#endif
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboId);
	}
}

void FrameBufferObject::_GuardedUnbind() 
{
	// Returns FBO binding to the previously enabled FBO
	if (m_fboId != (GLuint)m_savedFboId)
	{
#if LOG_FBO_CALLS
		printf("binding framebuffer %d\n",m_savedFboId);
#endif
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint)m_savedFboId);
	}
}

#ifndef NDEBUG
bool FrameBufferObject::IsValid( ostream& ostr )
{
	_GuardedBind();

#if LOG_FBO_CALLS
	printf("fbo %d: checking for FBO errors\n",m_fboId);
#endif

	bool isOK = false;

	GLenum status;                                            
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	switch(status)
	{                                          
		case GL_FRAMEBUFFER_COMPLETE_EXT: // Everything's OK
			isOK = true;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
			<< "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT\n";
			isOK = false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
			<< "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT\n";
			isOK = false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
			<< "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT\n";
			isOK = false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
			<< "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT\n";
			isOK = false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
			<< "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n";
			isOK = false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
			<< "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT\n";
			isOK = false;
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
			<< "GL_FRAMEBUFFER_UNSUPPORTED_EXT\n";
			isOK = false;
			break;
		default:
			ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
			<< "Unknown ERROR\n";
			isOK = false;
	}

	_GuardedUnbind();
	return isOK;
}
#endif // NDEBUG

/// Accessors
GLenum FrameBufferObject::GetAttachedType( GLenum attachment )
{
#if LOG_FBO_CALLS
	printf("querying type of attachment at %d\n",attachment);
#endif
	// Returns GL_RENDERBUFFER_EXT or GL_TEXTURE
	_GuardedBind();
	GLint type = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT, attachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT, &type);
	_GuardedUnbind();
	return GLenum(type);
}

GLuint FrameBufferObject::GetAttachedId( GLenum attachment )
{
#if LOG_FBO_CALLS
	printf("getting id of attachment at %d\n",attachment);
#endif
	_GuardedBind();
	GLint id = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT, attachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT, &id);
	_GuardedUnbind();
	return GLuint(id);
}

GLint FrameBufferObject::GetAttachedMipLevel( GLenum attachment )
{
#if LOG_FBO_CALLS
	 printf("getting mip level of attachment at %d\n",attachment);
#endif
	_GuardedBind();
	GLint level = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT, attachment, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT, &level);
	_GuardedUnbind();
	return level;
}

GLint FrameBufferObject::GetAttachedCubeFace( GLenum attachment )
{
#if LOG_FBO_CALLS
	printf("getting cube face id of attachment at %d\n",attachment);
#endif
	_GuardedBind();
	GLint level = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT, attachment, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT, &level);
	_GuardedUnbind();
	return level;
}

GLint FrameBufferObject::GetAttachedZSlice( GLenum attachment )
{
#if LOG_FBO_CALLS
	printf("querying z slice of attachment at %d\n",attachment);
#endif
	_GuardedBind();
	GLint slice = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT, attachment, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT, &slice);
	_GuardedUnbind();
	return slice;
}
