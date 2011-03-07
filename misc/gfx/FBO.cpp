#include "FBO.h"
#include "../sys/msys.h"
#include "Texture.h"

FBO::FBO( int _width, int _height ) : width(_width), height(_height), numAttachments(0)
{
	oglGenFramebuffersEXT(1, &fboId);
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);


	oglGenRenderbuffersEXT(1, &m_depthbufferId);
	oglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthbufferId);

	//
	// depth
	//
	oglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);

	// attach it to fbo
	oglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depthbufferId);
}



void FBO::setOutputs( Texture *out0, Texture *out1, Texture *out2, Texture *out3 )
{
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);

	Texture *out[4] = { out0, out1, out2, out3 };

	numAttachments = 0;
	for( int i=0; i<4;++i )
		if( out[i] )
		{
			oglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT+i, GL_TEXTURE_2D, out[i]->m_id, 0);
			++numAttachments;
		}

	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}



void FBO::setSize( int newXres, int newYres )
{
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);

	oglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthbufferId);
	oglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, newXres, newYres);


	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	width = newXres;
	height = newYres;
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

void FBO::begin( bool clear )
{
	//
	// draw into fbo
	//
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);

	//finalize();

	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,width, height);

	// Set the render target
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT };
	oglDrawBuffers(numAttachments, buffers);

	if( clear )
	{
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}//else
	//	glDisable( GL_DEPTH_TEST );
}

void FBO::end()
{
	glPopAttrib();
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}
