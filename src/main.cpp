//============================================================================
//
//
//
//============================================================================


//#include <fbxsdk.h>

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
#include "ops/RenderGeoOp.h"
#include "ops/DemoOp.h"
#include "ops/TransformOp.h"
#include "ops/FBXTransformOp.h"

#include "composer/widgets/CurveEditor/CurveEditor.h"
#include "composer/widgets/Trackball/Trackball.h"
#include "composer/widgets/GLViewer/GLViewer.h"

#include "ParticleCloud.h"
#include "ParticleCloud.ui.h"


float g_testValue = 0.0f;


composer::widgets::GLViewer *glviewer;


base::ContextPtr context;

base::GeometryPtr geo;

base::ShaderPtr baseShader;
base::ShaderPtr defaultGeometryShader;
base::Texture2dPtr baseTexture;

base::GeometryPtr baseGeo;

base::ops::OpPtr opRoot;
DemoOpPtr demoOp;
base::ops::ConstantOpPtr orbitTransform;


ParticleCloudPtr particleCloud;

struct TempTest
{
	float dist2;
	math::Vec3f center;
	int indices[3];
	int indices2[3];
	// Für sortierung
	bool operator < (const TempTest &s) const
	{
		// back to front drawing -> the closer the sprite, the later it is drawed
		return this->dist2 > s.dist2;
	}
};

std::vector<TempTest> sprites;

bool               g_doSort = true;


void renderGeo()
{
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	context->render( particleCloud->m_spriteGeo, particleCloud->m_cloudSpriteShader );


	glDisable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
}

void render( base::CameraPtr cam )
{
	glEnable( GL_DEPTH_TEST );
	orbitTransform->m_variant = cam->m_transform;


	if(g_doSort)
	{
		particleCloud->sort(cam);
	}
/*
	if(g_doSort)
	{
		// compute distance to camera
		math::Vec3f camPos = cam->m_transform.getTranslation();
		math::Vec3f dir = cam->m_transform.getDir();
		for( std::vector<TempTest>::iterator it = sprites.begin(); it != sprites.end(); ++it )
		{
			TempTest &tt = *it;
			//tt.dist2 = (tt.center - camPos).getSquaredLength();
			tt.dist2 = -math::dotProduct( tt.center - camPos, dir );
		}
		// sort
		std::sort(sprites.begin(), sprites.end());
		// update indexbuffer
		int c = 0;
		for( std::vector<TempTest>::iterator it = sprites.begin(); it != sprites.end(); ++it )
		{
			TempTest &tt = *it;
			spriteGeo->m_indexBuffer[c++] = tt.indices[0];
			spriteGeo->m_indexBuffer[c++] = tt.indices[1];
			spriteGeo->m_indexBuffer[c++] = tt.indices[2];

			spriteGeo->m_indexBuffer[c++] = tt.indices2[0];
			spriteGeo->m_indexBuffer[c++] = tt.indices2[1];
			spriteGeo->m_indexBuffer[c++] = tt.indices2[2];
		}
		spriteGeo->m_indexBufferIsDirty = true;

		g_doSort = false;
	}
*/
	opRoot->execute();
}
void mouseMove( base::MouseState ms )
{
	//if(ms.buttons)
	float t = (float)ms.x / (float)glviewer->width();
	base::ops::Manager::context()->setTime(t);
}

void keyPress( base::KeyboardState &ks )
{
	if( ks.press[KEY_S] )
	{
		// SORT!
		g_doSort = true;
	}
}

/*
// Initialize the sdk manager. This object handles all our memory management.
KFbxSdkManager* lSdkManager = NULL;
KFbxIOSettings *ios = NULL;
KFbxGeometryConverter  *geoConverter = NULL;
KFbxAnimEvaluator *animEvaluator = NULL;
*/




void init()
{
	std::cout << "init!\n";


	GLenum glewResult = glewInit();
	if (GLEW_OK != glewResult)
	{
		std::cout << "glew init failed\n";
	}


	context = base::ContextPtr( new base::Context() );
	base::ops::Manager::setContext(context);


	// op testing =============
	//base::ops::SphereOpPtr s = base::ops::SphereOp::create(1.0f);
	//base::MeshPtr m = s->getMesh(0);
	//geo = m->getGeometry();
	geo = base::geo_sphere(30,30,1.0);


	// demo =============
	//demoOp = DemoOp::create( "/usr/people/david-k/dev/testprojects/sandbox/temp/sketch039.ogg" );
	demoOp = DemoOp::create( "c:\\projects\\sandbox\\temp\\code\\sketch039.ogg" );
	TimeOpPtr time = TimeOp::create();
	CameraOpPtr cam = CameraOp::create();
	base::ops::ClearOpPtr clear = base::ops::ClearOp::create();
	orbitTransform = base::ops::ConstantOp::create();
	base::ops::FuncOpPtr renderFunc = base::ops::FuncOp::create( renderGeo );

	orbitTransform->plug( cam, "transformMatrix" );

	opRoot = demoOp;



	//
	baseShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_vs.glsl", base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_ps.glsl" );
	//baseGeo = base::importObj( base::Path( SRC_PATH ) + "/data/test.1.obj" );
	//baseGeo = base::geo_sphere( 30, 30, 1.0f );
	baseGeo = base::geo_grid( 30, 30 );
	//base::apply_transform( baseGeo, math::Matrix44f::ScaleMatrix( 30000.0f ) );
	base::apply_normals( baseGeo );

	baseTexture = base::Texture2d::load( base::Path( SRC_PATH ) + "/src/base/data/uvref2.png" );

	defaultGeometryShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/ParticleCloud.defaultgeometry.vs.glsl", base::Path( SRC_PATH ) + "/src/ParticleCloud.defaultgeometry.ps.glsl" );
	defaultGeometryShader->setUniform( "diffuseMap", baseTexture->getUniform() );

	baseShader->setUniform( "input", baseTexture->getUniform() );



	clear->plug( opRoot );
	cam->plug( opRoot );
	renderFunc->plug( cam );

	base::ops::Manager::context()->setTime( .5 );


	// get positions and normals
	//positions.push_back( math::Vec3f(0.0f, 0.0f, 0.0f) );
	//normals.push_back( math::Vec3f(0.0f, 1.0f, 0.0f) );
	//positions.push_back( math::Vec3f(1.0f, 1.0f, 0.0f) );
	//normals.push_back( math::Vec3f(1.0f, 1.0f, 0.1f).normalized() );

	particleCloud = ParticleCloudPtr(new ParticleCloud());


	{
		base::Path path = base::Path( SRC_PATH ) + "/data/positions.bin";
		if( !base::fs::exists( path ) )
			std::cerr << "path doesnt exist\n";
		base::fs::File *f = base::fs::open( path );
		if(!f)
			std::cerr << "failed to open file\n";

		int numPoints = 0;

		base::fs::read( f, &numPoints, sizeof(int), 1 );
		for( int i=0;i<numPoints;++i )
		{
			math::Vec3f p, n, tangentu;
			base::fs::read( f, &p.x, sizeof(float), 1 );
			base::fs::read( f, &p.y, sizeof(float), 1 );
			base::fs::read( f, &p.z, sizeof(float), 1 );
			base::fs::read( f, &n.x, sizeof(float), 1 );
			base::fs::read( f, &n.y, sizeof(float), 1 );
			base::fs::read( f, &n.z, sizeof(float), 1 );
			//base::fs::read( f, &tangentu.x, sizeof(float), 1 );
			//base::fs::read( f, &tangentu.y, sizeof(float), 1 );
			//base::fs::read( f, &tangentu.z, sizeof(float), 1 );

			// displace normal
			n.x += math::g_randomNumber()*2.0f-1.0f;
			n.y += math::g_randomNumber()*2.0f-1.0f;
			n.z += math::g_randomNumber()*2.0f-1.0f;
			n.normalize();

			particleCloud->addParticle( p, n, tangentu, 10 );
			//positions.push_back( p );
			//normals.push_back( n );
		}

		close(f);
	}

	particleCloud->buildGeometry();







	
	ParticleCloudUI *widget = new ParticleCloudUI(particleCloud);
	widget->show();
	glviewer->connect( widget, SIGNAL(makeDirty(void)), SLOT(update(void)) );

	//demoOp->startAudio();
}

void shutdown()
{
	//demoOp->stopAudio();
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
