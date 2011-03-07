#include "FBO.h"
#include "../sys/msys.h"
#include "Texture.h"

FBO::FBO( int _width, int _height ) : width(_width), height(_height), numAttachments(0)
{
	oglGenFramebuffersEXT(1, &fboId);
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);


	unsigned int depthbufferId;
	oglGenRenderbuffersEXT(1, &depthbufferId);
	oglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbufferId);

	//
	// depth
	//
	oglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);

	// attach it to fbo
	oglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbufferId);
}

void FBO::attach( Texture *texture )
{
	oglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT+numAttachments, GL_TEXTURE_2D, texture->m_id, 0);
	++numAttachments;
}

void FBO::finalize()
{
	//check
	GLenum status = oglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);


	if( status == GL_FRAMEBUFFER_COMPLETE_EXT  )
		printf("FBO successfull finalized\n");
	else
		printf("FBO finalization error %i\n", status);

	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void FBO::begin()
{
	//
	// draw into fbo
	//
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,width, height);

	// Set the render target
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT };
	oglDrawBuffers(numAttachments, buffers);
}

void FBO::end()
{
	glPopAttrib();
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}
