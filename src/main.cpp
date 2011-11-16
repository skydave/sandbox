//============================================================================
//
//
//
//============================================================================


#include <fbxsdk.h>

// need this to make fbxsdk happy
#ifdef _WINDOWS
#pragma message("     _Adding library: wininet.lib" ) // apparently not linking this library causes unresolved symbols when statically linking fbxsdk - it starts getting messy :(
#pragma comment (lib, "wininet.lib")
#endif 

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

#include "ops/TimeOp.h"
#include "ops/CameraOp.h"
#include "ops/DemoOp.h"

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


void renderGeo()
{
	context->render( geo, baseShader );
}

void render( base::CameraPtr cam )
{
	orbitTransform->m_variant = cam->m_transform;
	opRoot->execute();
}



// Initialize the sdk manager. This object handles all our memory management.
KFbxSdkManager* lSdkManager = NULL;
KFbxIOSettings *ios = NULL;

base::ops::OpPtr buildFromFBX( std::string path )
{
	// Create an importer using our sdk manager.
	KFbxImporter* lImporter = KFbxImporter::Create(lSdkManager,"");

	// Use the first argument as the filename for the importer.
	if(!lImporter->Initialize(path.c_str(), -1, lSdkManager->GetIOSettings()))
	{
		printf("Call to KFbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetLastErrorString());
		exit(-1);
	}


	// Create a new scene so it can be populated by the imported file.
	KFbxScene* lScene = KFbxScene::Create(lSdkManager,"myScene");

	// Import the contents of the file into the scene.
	lImporter->Import(lScene);

	// The file has been imported; we can get rid of the importer.
	lImporter->Destroy();

	base::ops::OpPtr();
}



void init()
{
	std::cout << "init!\n";


	GLenum glewResult = glewInit();
	if (GLEW_OK != glewResult)
	{
		std::cout << "glew init failed\n";
	}

	std::string fbxTest = "asfasf";

	// Initialize the sdk manager. This object handles all our memory management.
	lSdkManager = KFbxSdkManager::Create();
	// Create the io settings object.
	ios = KFbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);



	context = base::ContextPtr( new base::Context() );


	// op testing =============
	base::ops::SphereOpPtr s = base::ops::SphereOp::create(1.0f);
	base::MeshPtr m = s->getMesh(0);
	geo = m->getGeometry();


	// demo =============
	//demoOp = DemoOp::create( "/usr/people/david-k/dev/testprojects/sandbox/temp/sketch039.ogg" );
	demoOp = DemoOp::create( "c:\\projects\\sandbox\\temp\\code\\sketch039.ogg" );
	TimeOpPtr time = TimeOp::create();
	CameraOpPtr cam = CameraOp::create();
	base::ops::ClearOpPtr clear = base::ops::ClearOp::create();
	orbitTransform = base::ops::ConstantOp::create();
	base::ops::FuncOpPtr renderFunc = base::ops::FuncOp::create( renderGeo );

	clear->plug( cam );
	renderFunc->plug( cam );

	cam->plug( demoOp );

	orbitTransform->plug( cam, "transformMatrix" );

	opRoot = demoOp;

	// fbx import test =============
	//buildFromFBX(fbxTest);


	//
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
