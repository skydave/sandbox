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

#include "composer/widgets/CurveEditor/CurveEditor.h"
#include "composer/widgets/Trackball/Trackball.h"
#include "composer/widgets/GLViewer/GLViewer.h"

#include "ColorGrading.ui.h"

composer::widgets::GLViewer *glviewer;
base::ContextPtr context;

base::GeometryPtr     locator;
base::GeometryPtr        axes;
base::GeometryPtr    geometry;
base::GeometryPtr       plane;
base::ShaderPtr        shader;
base::ShaderPtr defaultShader;
base::Texture2dPtr refTexture;

ColorGradingPtr colorGrading;

BASE_DECL_SMARTPTR_STRUCT(PointLight);
struct PointLight
{
	PointLight();
	void             setPos( const math::Vec3f &pos );
	void       setLookAt( const math::Vec3f &lookAt );
	void                                     update();

	base::FBOPtr                       m_shadowFBO;
	base::Texture2dPtr                 m_shadowMap;
	base::CameraPtr                          m_cam;
	math::Vec3f                              m_pos;
	math::Vec3f                           m_lookAt;
};

PointLight::PointLight()
{
	m_cam = base::CameraPtr( new base::Camera() );
	m_cam->m_znear = 1.0f;
	m_cam->m_zfar  = 10.0f;
	//m_cam->m_focalLength = 10.0; // ? this should be derived from m_fov
	//m_cam->m_fov   = 30.0f;
	m_cam->m_aspectRatio = 1.0f;
	m_shadowMap = base::Texture2d::createRGBAFloat32();
	m_shadowFBO = base::FBO::create().width(512).height(512).attach(m_shadowMap);
}

void PointLight::setPos( const math::Vec3f &pos )
{
	m_pos = pos;
	update();
}

void PointLight::setLookAt( const math::Vec3f &lookAt )
{
	m_lookAt = lookAt;
	update();
}

void PointLight::update()
{
	//m_cam->m_transform = math::createLookAtMatrix( m_pos, m_lookAt, math::Vec3f(0.0f,1.0f,0.0f), false );
	m_cam->m_transform = math::createLookAtMatrix( m_pos, m_lookAt, math::Vec3f(0.0f,1.0f,0.0f), false );
	//m_cam->m_transform = math::Matrix44f::TranslationMatrix( 0.0f, 0.0f, 3.5f );
	//m_cam->m_transform = math::Matrix44f::Identity();
	m_cam->update();
}


base::ShaderPtr    depthMapShader;

PointLightPtr              light0;



void render( base::CameraPtr cam )
{
	// put rendering code here
	glEnable( GL_DEPTH_TEST );

	// render shadowmap pass ==========
	light0->m_shadowFBO->begin();
	//glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	context->setCamera( light0->m_cam );
	//context->setCamera( cam );

	//glEnable( GL_CULL_FACE );
	//glCullFace( GL_FRONT );

	//context->setModelMatrix( math::Matrix44f::TranslationMatrix( 0.0f, 0.0f, -1.5f ) );
	context->render( geometry, depthMapShader );
	//context->setModelMatrix( math::Matrix44f::Identity() );
	context->render( plane, depthMapShader );

	//context->setModelMatrix( math::Matrix44f::RotationMatrixY( math::degToRad(180.0f) ) );
	//context->render( axes, context->m_simpleTextureShader );
	//context->setModelMatrix( math::Matrix44f::Identity() );

	//glCullFace( GL_BACK );
	//glDisable( GL_CULL_FACE );

	light0->m_shadowFBO->end();



	///*
	// render primary pass ============
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	context->setCamera( cam );

	math::Matrix44f viewToLight = context->getModelViewInverse() * light0->m_cam->m_viewMatrix * light0->m_cam->m_projectionMatrix;
	colorGrading->shader->setUniform( "viewToLight0", viewToLight );
	colorGrading->shader->setUniform( "lightPos0", context->worldToView(light0->m_cam->m_transform.getTranslation()) );

	//context->setModelMatrix( math::Matrix44f::TranslationMatrix( 0.0f, 0.0f, -1.5f ) );
	context->render( geometry, colorGrading->shader );
	//context->setModelMatrix( math::Matrix44f::Identity() );
	context->render( plane, colorGrading->shader );

	context->render( axes, context->m_constantShader );

	context->setModelMatrix( math::Matrix44f::TranslationMatrix( light0->m_cam->m_transform.getTranslation() ) );
	context->m_constantShader->setUniform( "color", math::Vec3f(0.0f,0.0f,1.0f) );
	context->render( locator, context->m_constantShader );
	context->setModelMatrix( math::Matrix44f::Identity() );
	//*/

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

	locator = base::geo_sphere(10,10,0.1f);
	axes = base::importObj( base::Path( BASE_PATH ) + "/data/axes.obj" );


	// setup light
	light0 = PointLightPtr( new PointLight() );
	light0->setPos(math::Vec3f( 0.0f, 1.0f, 3.5f ));
	//light0->setPos(math::Vec3f( 4.0f, 2.0f, 0.0f ));
	light0->setLookAt(math::Vec3f( 0.0f, 0.0f, 0.0f ));
	light0->update();

	depthMapShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/simpleDepth" );


	plane = base::geo_grid(30,30);
	//base::apply_transform( plane, math::Matrix44f::ScaleMatrix(10.0f) * math::Matrix44f::TranslationMatrix(0.0f, -1.0f, 0.0f) );
	base::apply_transform( plane, math::Matrix44f::ScaleMatrix(10.0f) );
	base::apply_transform( plane, math::Matrix44f::TranslationMatrix(0.0f, -1.0f, 0.0f) );
	base::apply_normals(plane);
	//geometry = base::Geometry::createReferenceMesh();

	geometry = base::geo_sphere(30,30, 1.0f);
	//base::apply_transform(geometry, math::Matrix44f::TranslationMatrix(  0.0f, 2.0f, 0.0f));
	base::apply_normals(geometry);

	colorGrading = ColorGradingPtr(new ColorGrading());
	colorGrading->lightMapShader = depthMapShader;
	colorGrading->shader = base::Shader::load( base::Path( SRC_PATH ) + "/src/simpleGeometry" );
	colorGrading->shader->setUniform( "l", math::Vec3f( 1.0f, 1.0f, 1.0f ).normalized() );
	refTexture = base::Texture2d::createUVRefTexture();
	colorGrading->shader->setUniform( "texture", refTexture->getUniform() );
	colorGrading->shader->setUniform( "depthMap0", light0->m_shadowMap->getUniform() );

	ColorGradingUI *widget = new ColorGradingUI(colorGrading);
	widget->move( glviewer->mapToGlobal( QPoint( glviewer->width()+10, -30 ) ) );
	widget->show();
	glviewer->connect( widget, SIGNAL(makeDirty(void)), SLOT(update(void)) );
}

void shutdown()
{
	// put your deinitialization stuff here
}





int main(int argc, char ** argv)
{
	//Q_INIT_RESOURCE(application);
	QApplication app(argc, argv);
	app.setOrganizationName("app");
	app.setApplicationName("app");

	QMainWindow mainWin;
	mainWin.resize(800, 600);
	glviewer = new composer::widgets::GLViewer(init, render);
	glviewer->getCamera()->m_znear = .1f;
	glviewer->getCamera()->m_zfar = 1000.0f;
	glviewer->getOrbitNavigator().m_distance = 1.0f;
	glviewer->getOrbitNavigator().m_elevation = 45.0f;
	glviewer->getOrbitNavigator().update();
	mainWin.setCentralWidget( glviewer );
	mainWin.show();

	return app.exec();
}
