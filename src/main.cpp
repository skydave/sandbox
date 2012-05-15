//============================================================================
//
//
//
//============================================================================

#include <QtGui>
#include <QApplication>

#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <util/unordered_map.h>

#include <ui/GLViewer.h>
#include <gltools/gl.h>
#include <gltools/misc.h>
#include <util/StringManip.h>
#include <util/Path.h>
#include <gfx/Geometry.h>
#include <gfx/Shader.h>
#include <gfx/Texture.h>
#include <gfx/Image.h>
#include <gfx/Context.h>
#include <gfx/FCurve.h>
#include <gfx/glsl/common.h>
#include <gfx/FBO.h>


#include "composer/widgets/GLViewer/GLViewer.h"
#include "Nebulae.h"
#include "Nebulae.ui.h"

#include "Sky.h"

composer::widgets::GLViewer *glviewer;

base::ContextPtr context;


NebulaePtr nebulae;
SkyPtr         sky;


void onPlayButtonPressed( bool checked )
{
	if( checked )
	{
		// continue timer
		// switch glviewer into threadrendering mode
		glviewer->setRenderInSeperateThread(true);
	}else
	{
		// switch glviewer into non-threadrendering mode
		glviewer->setRenderInSeperateThread(false);
		// pause timer
	}
}










base::FBOPtr                           colorFBO;
base::ShaderPtr                     colorShader;
base::Texture2dPtr               colorFBOOutput;






void render2( base::CameraPtr cam )
{
	glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );


	context->setCamera( cam );




	// render to fbo
	colorFBO->begin();
	//glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render sky backgdrop
	sky->render( cam );

	// render particles
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );


	glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );
	glTexEnvf( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );

	glEnable( GL_POINT_SPRITE );

	context->render( nebulae->m_particles, nebulae->m_particleShader );

	glDisable( GL_POINT_SPRITE );

	context->render( nebulae->m_billboards->geo, nebulae->m_billboardShader );
	context->render( nebulae->m_billboardsFlares->geo, nebulae->m_billboardFlareShader );
	context->render( nebulae->m_billboardsGlow->geo, nebulae->m_billboardGlowShader );

	glBlendEquation( GL_FUNC_REVERSE_SUBTRACT );
	context->render( nebulae->m_billboardsBokGlobules->geo, nebulae->m_billboardBokGlobuleShader );
	glBlendEquation( GL_FUNC_ADD );

	glDisable( GL_BLEND );
	colorFBO->end();


	// render to screen
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	context->renderScreen( colorShader );


}


void init()
{
	std::cout << "init!\n";

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "glew init failed\n";
	}

    int dedicatedVidMem = -1;
	glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &dedicatedVidMem);
	int totalAvailableMem = -1; 
	glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalAvailableMem);
	int currentAvailableVidMem = -1;
	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &currentAvailableVidMem);
	int evictionCount = -1;
	glGetIntegerv(GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &evictionCount);
	int evictedMemory = -1;
	glGetIntegerv(GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &evictedMemory);
	int maxTextureSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

	std::cout << "dedicated video memory (in kb)   : " << dedicatedVidMem << "\n";
	std::cout << "total available memory (in kb)   : " << totalAvailableMem << "\n";
	std::cout << "current available memory (in kb) : " << currentAvailableVidMem << "\n";
	std::cout << "eviction count                   : " << evictionCount << "\n";
	std::cout << "evicted memory (in kb)           : " << evictedMemory << "\n";
	std::cout << "maximum texture dimension        : " << maxTextureSize << "\n";
 
	context = base::ContextPtr( new base::Context() );
	base::Context::setCurrentContext(context);

	sky = Sky::create();
	nebulae = Nebulae::create();

	// generate
	nebulae->generate();


	colorFBO = base::FBOPtr( new base::FBO( 800, 600) );
	//colorFBOOutput = base::Texture2d::createRGBA8( 800, 600 );
	colorFBOOutput = base::Texture2d::createRGBAFloat32( 800, 600 );
	colorFBO->setOutputs( colorFBOOutput );
	colorShader = base::Shader::load( base::Path( SRC_PATH ) + "src/base/gfx/glsl/screen_vs.glsl", base::Path( SRC_PATH ) + "src/base/gfx/glsl/screen_ps.glsl" );
	colorShader->setUniform( "color", colorFBOOutput->getUniform() );



	NebulaeUI *widget = new NebulaeUI(nebulae);
	widget->show();
	glviewer->connect( widget, SIGNAL(makeDirty(void)), SLOT(update(void)) );
}


int main(int argc, char ** argv)
{
	//Q_INIT_RESOURCE(application);
	QApplication app(argc, argv);
	app.setOrganizationName("test");
	app.setApplicationName("test");

	QMainWindow mainWin;
	mainWin.resize(800, 600);
	glviewer = new composer::widgets::GLViewer(init, render2);
	glviewer->getCamera()->m_znear = 0.1f;
	glviewer->getCamera()->m_zfar = 100000.0f;
	mainWin.setCentralWidget( glviewer );
	mainWin.show();


	return app.exec();
}
