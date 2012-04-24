//===============================================================================
//
// TestEffect renders plane and a mesh with phongshading and shadows
//
//===============================================================================

#include <QtGui>
#include <QApplication>

#include <gltools/gl.h>
#include <gltools/misc.h>

#include <gfx/Context.h>

#include "composer/widgets/GLViewer/GLViewer.h"

#include "ColorGrading.h"
#include "ColorGrading.ui.h"


composer::widgets::GLViewer *glviewer; // derived from qglwidget
base::ContextPtr              context; // this mainly contains transform matrices and convinience functions

ColorGradingPtr          colorgrading; // color grading

base::FBOPtr                      fbo;
base::Texture2dPtr          fboOutput;
base::Texture2dPtr         refTexture;




// render function (called by glviewer)
void render( base::CameraPtr cam )
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render scene to texture
	fbo->begin();
	context->renderScreen( refTexture );
	fbo->end();

	// apply color grading to texture
	colorgrading->render(cam);


}


// init function (called by glviewer)
void init()
{
	// init glew =====================
	GLenum glewResult = glewInit();
	if (GLEW_OK != glewResult)
	{
		std::cout << "glew init failed\n";
	}

	// setup context =====================
	context = base::ContextPtr( new base::Context() );
	base::Context::setCurrentContext(context);

	// initialize everything ==============
	colorgrading = ColorGradingPtr( new ColorGrading() );
	colorgrading->init();

	refTexture = base::Texture2d::createUVRefTexture();
	fboOutput = base::Texture2d::createRGBA8();
	fbo = base::FBO::create().width(512).height(512).attach(fboOutput);

	colorgrading->setInput(fboOutput);

	// bring up colorgrading editor ==================
	ColorGradingUI *widget = new ColorGradingUI(colorgrading);
	widget->move( glviewer->mapToGlobal( QPoint( glviewer->width()+10, -30 ) ) );
	widget->show();
	glviewer->connect( widget, SIGNAL(makeDirty(void)), SLOT(update(void)) );



	// setup operator graph:
	// clear
	// fbobegin
	// renderscreen ref texture
	// fboend
	// colograding
	// renderscreen colograding output

	// fbo output0 connected to colograding

	/*
	opgraphbuilder o;

	o.op( "clear" );
	o.op( "renderfbo" ).edit();
		o.op( "clear" );
		o.op( "renderScreen" ).plug( texture );
		o.done();
	o.op( "colorgrading" ).plug(  )
	o.done();


	*/



}





int main(int argc, char ** argv)
{
	QApplication app(argc, argv);
	app.setOrganizationName("app");
	app.setApplicationName("app");

	QMainWindow mainWin;
	mainWin.resize(800, 600);
	glviewer = new composer::widgets::GLViewer(init, render);

	// setup initial view ===========
	glviewer->getCamera()->m_znear = .1f;
	glviewer->getCamera()->m_zfar = 1000.0f;
	glviewer->getOrbitNavigator().m_distance = 11.0f;
	glviewer->getOrbitNavigator().m_azimuth = 45.0f;
	glviewer->getOrbitNavigator().m_elevation = 45.0f;
	glviewer->getOrbitNavigator().update();

	mainWin.setCentralWidget( glviewer );
	mainWin.show();

	return app.exec();
}
