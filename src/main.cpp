//============================================================================
//
//
// TODO: update to gl4.2 render grid/transform
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

base::GLViewer       *glviewer;
base::ContextPtr       context;
base::GeometryPtr     geometry;
base::ShaderPtr       shader;




void render( base::CameraPtr cam )
{
	// put rendering code here
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	context->setCamera( cam );

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	context->render( geometry, shader );

	glEnable(GL_MULTISAMPLE_ARB);
 	context->render( geometry, shader );
 	glDisable(GL_MULTISAMPLE_ARB);

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

	// how much samples are supported?
	int maxSamples;
	int samples;
	int numSamples = 4;
	glGetIntegerv(GL_MAX_SAMPLES_EXT, &maxSamples);
	std::cout << "maxsamples: " << maxSamples << std::endl;

	glEnable( GL_MULTISAMPLE );
	glGetIntegerv(GL_SAMPLES, &samples);
	std::cout << "samples " << samples << std::endl;


	// multisampletexture
	unsigned int tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA32F, width, height, true);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);


	float sampleLoc[2];
	std::cout << "sample locations:" << std::endl;
	for( int i=0;i<maxSamples;++i )
	{
		glGetMultisamplefv( GL_SAMPLE_POSITION, i, sampleLoc );
		std::cout << i << sampleLoc[0] << " " << sampleLoc[1] << std::endl;
	}


	// geo
	geometry = base::geo_cube();
	shader = base::Shader::createSimpleTextureShader();


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
	glviewer->setSampleBuffers( true );
	glviewer->setSamples( 16 );
	glviewer->show();
	return app.exec();
}
