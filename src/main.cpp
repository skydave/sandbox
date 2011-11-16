//============================================================================
//
//
//
//============================================================================


#include <fbxsdk.h>

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

#include <ops/ops.h>

#include "TimeOp.h"
#include "CameraOp.h"
#include "DemoOp.h"

#include "composer/widgets/CurveEditor/CurveEditor.h"
#include "composer/widgets/Trackball/Trackball.h"
#include "composer/widgets/GLViewer/GLViewer.h"

#include "OSCDispatcher.h"



float g_testValue = 0.0f;


OSCDispatcher oscDispatcher;

composer::widgets::GLViewer *glviewer;


base::ContextPtr context;

base::GeometryPtr geo;

base::ShaderPtr baseShader;
base::Texture2dPtr baseTexture;
base::GeometryPtr baseGeo;

base::ops::OpPtr opRoot;
DemoOpPtr demoOp;
base::ops::ConstantOpPtr orbitTransform;


void render( base::CameraPtr cam )
{

	orbitTransform->m_variant = cam->m_transform;
	opRoot->execute();
}


void render2()
{
	context->render( geo, baseShader );
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


	// op testing
	base::ops::SphereOpPtr s = base::ops::SphereOp::create(1.0f);
	base::MeshPtr m = s->getMesh(0);
	geo = m->getGeometry();


	// demo
	demoOp = DemoOp::create( "/usr/people/david-k/dev/testprojects/sandbox/temp/sketch039.ogg" );
	TimeOpPtr time = TimeOp::create();
	CameraOpPtr cam = CameraOp::create();
	base::ops::ClearOpPtr clear = base::ops::ClearOp::create();
	orbitTransform = base::ops::ConstantOp::create();
	base::ops::FuncOpPtr renderFunc = base::ops::FuncOp::create( render2 );

	clear->plug( cam );
	renderFunc->plug( cam );

	cam->plug( demoOp );

	orbitTransform->plug( cam, "transformMatrix" );

	opRoot = demoOp;


	// rendering
	/*

	RenderOpPtr rop = base::ops::RenderOp::create();
	rop->append( base::ops::ClearOp::create() );
	rop->append( base::ops::SkyOp::create() );
	rop->append( base::ops::RenderMeshOp::create() );

	rop->execute();

	*/



	baseShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_vs.glsl", base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_ps.glsl" );
	baseGeo = base::importObj( base::Path( SRC_PATH ) + "/data/test.1.obj" );
	//base::apply_transform( baseGeo, math::Matrix44f::ScaleMatrix( 30000.0f ) );
	base::apply_normals( baseGeo );

	baseTexture = base::Texture2d::load( base::Path( SRC_PATH ) + "/src/base/data/uvref2.png" );
	baseShader->setUniform( "input", baseTexture->getUniform() );


	demoOp->startAudio();
}

void shutdown()
{
	demoOp->stopAudio();
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
	glviewer->setRenderInSeperateThread(true);
	mainWin.setCentralWidget( glviewer );
	mainWin.show();



	oscDispatcher.start();

	return app.exec();
}
