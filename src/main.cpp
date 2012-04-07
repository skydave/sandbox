//============================================================================
//
//
// TODO: update to gl4.2 render grid/transform
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

base::GLViewer *glviewer;
base::ContextPtr context;




void render( base::CameraPtr cam )
{
	// put rendering code here
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	context->setCamera( cam );


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
}

void shutdown()
{
	// put your deinitialization stuff here
}





int main(int argc, char ** argv)
{
	QApplication app(argc, argv);
	app.setOrganizationName("test");
	app.setApplicationName("test");

	QMainWindow mainWin;
	mainWin.resize(800, 600);
	glviewer = new composer::widgets::GLViewer(init, render, shutdown);
	glviewer->getCamera()->m_znear = .1f;
	glviewer->getCamera()->m_zfar = 100000.0f;
	//glviewer->setRenderInSeperateThread(true);
	glviewer->setMouseMoveCallback( mouseMove );
	glviewer->setKeyPressCallback( keyPress );
	mainWin.setCentralWidget( glviewer );
	mainWin.show();

	return app.exec();
}
