//============================================================================
//
// This example demonstrates stencil routed k-buffers. a textured cube is
// rendered into a multisampled fbo. the multiple samples are used to store
// the different fragments per pixel. the stencil buffer is used to route
// the fragments to their final sample.
// 
//============================================================================



#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include <ui/GLViewer.h>

#include <gltools/gl.h>
#include <gltools/misc.h>

#include <util/StringManip.h>
#include <util/Path.h>

#include <gfx/Geometry.h>
#include <gfx/ObjIO.h>
#include <gfx/Shader.h>
#include <gfx/Texture.h>
#include <gfx/Image.h>
#include <gfx/Context.h>
#include <gfx/FCurve.h>
#include <gfx/glsl/common.h>
#include <gfx/FBO.h>

base::GLViewer                      *glviewer;
base::ContextPtr                      context;
base::GeometryPtr                    geometry;
base::ShaderPtr                        shader;
base::Texture2dPtr                    texture;
base::ShaderPtr       initializeStencilShader;

base::Texture2dPtr         multisampleTexture;
base::FBOPtr                   multisampleFBO;
base::ShaderPtr         multisampleTestShader;


void render( base::CameraPtr cam )
{
	// render to texture
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClearStencil( 0 );
	multisampleFBO->begin( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	glDisable( GL_DEPTH_TEST );

	// initialize stencil buffer ---
	glEnable(GL_STENCIL_TEST);
	glEnable( GL_MULTISAMPLE );
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
	glEnable(GL_SAMPLE_MASK);
	for(int i = 0; i < 4; ++i)
	{
		glStencilFunc(GL_ALWAYS, i + 1, 0xff);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		glSampleMaski(0, 0x1 << i);
		context->renderScreen(initializeStencilShader);
	}
	glSampleMaski(0, 0xFFFFFF);
	glDisable(GL_SAMPLE_MASK);
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glDisable( GL_MULTISAMPLE );


	// now render geometry using stencil routing ---
	glStencilFunc(GL_EQUAL, 1, 0xff);
	glStencilOp(GL_DECR, GL_DECR, GL_DECR );

	context->setCamera( cam );
	context->render( geometry, shader );

	glDisable(GL_STENCIL_TEST);

	multisampleFBO->end();





	//  final render the shader
	// NB: the multisampleTestShader in its pixel section fetches a single sample for each fragment. change the sample index in the
	// fetchTexture function to check the values for the different samples
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClearStencil( 0 );

	//we also clear the stencil buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	context->renderScreen( multisampleTestShader );



}






void init()
{
	std::cout << "init!\n";


	GLenum glewResult = glewInit();
	if (GLEW_OK != glewResult)
	{
		std::cout << "glew init failed\n";
	}

	context = base::ContextPtr( new base::Context() );
	base::Context::setCurrentContext(context);

	// put your initialization stuff here
	int width = 512;
	int height = 512;

	// init resources
	geometry = base::geo_cube();
	shader = base::Shader::createSimpleTextureShader();
	texture = base::Texture2d::createUVRefTexture();
	shader->setUniform("texture",texture->getUniform());
	

	initializeStencilShader = base::Shader::create().attachPS(base::Path( SRC_PATH ) + "/src/initializeStencil.ps.glsl").attachVS(base::Path( SRC_PATH ) + "/src/initializeStencil.vs.glsl");

	
	multisampleTexture = base::Texture2d::createRGBAFloat32( 512, 512, true, 4 );

	multisampleFBO = base::FBO::create().width(512).height(512).multisample(true).numSamples(4).stencilBuffer(true).attach(multisampleTexture);
	multisampleTestShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/multisampleTest" );
	multisampleTestShader->setUniform( "texture", multisampleTexture->getUniform() );


}

void shutdown()
{
	// put your deinitialization stuff here
}





int main(int argc, char ** argv)
{
	base::Application app;
	glviewer = new base::GLViewer();
	glviewer->setSize( 800, 600 );
	glviewer->setCaption( "app" );
	glviewer->setInitCallback( init );
	glviewer->setRenderCallback( render );
	glviewer->show();
	return app.exec();
}
