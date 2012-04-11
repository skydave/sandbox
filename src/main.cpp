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

#include "TestEffect.h"
#include "TestEffect.ui.h"


composer::widgets::GLViewer *glviewer; // derived from qglwidget
base::ContextPtr              context; // this mainly contains transform matrices and convinience functions

TestEffectPtr                  effect; // our test effect...






// render function (called by glviewer)
void render( base::CameraPtr cam )
{
	effect->render(cam);
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
	effect = TestEffectPtr( new TestEffect() );
	effect->init();


	// bring up testeffect editor ==================
	TestEffectUI *widget = new TestEffectUI(effect);
	widget->move( glviewer->mapToGlobal( QPoint( glviewer->width()+10, -30 ) ) );
	widget->show();
	glviewer->connect( widget, SIGNAL(makeDirty(void)), SLOT(update(void)) );

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
