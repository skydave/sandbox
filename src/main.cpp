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
#include <algorithm>

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

composer::widgets::GLViewer *glviewer;

base::ContextPtr context;
base::Texture2dPtr particlePositions;
base::Texture2dPtr particleTex;
base::Texture2dPtr envTex;
base::TextureCubePtr envCubeMap;
base::ImagePtr envImage;
base::Texture2dPtr starsTex;


base::GeometryPtr particles;
base::GeometryPtr sphere;
base::ShaderPtr sphereShader;
base::ShaderPtr envShader;




math::Vec2f cubemapLookup( math::Vec3f n )
{
	math::Vec2f uv;

	math::Vec3f absN( fabs(n.x), fabs(n.y), fabs(n.z) );

	// nrm is the world normal from a vertex
	// absNrm is the same normal, but with absolute values
	if (absN.x > absN.y && absN.x > absN.z)
	{
		if (n.x>0)
		{
			// right face
			math::Vec3f p_n = math::Vec3f(1.0,0.0,0.0);
			float d = math::dotProduct(p_n, p_n)/dotProduct(n,p_n);
			//uv = 1.0 - (d*n*0.5 + 0.5).zy;
			uv.x = 1.0 - (d*n*0.5 + 0.5).z;
			uv.y = 1.0 - (d*n*0.5 + 0.5).y;

			uv = uv*math::Vec2f(0.333f,0.25f) + math::Vec2f(0.666f,0.25f);
		}
		else
		{
			// left face
			math::Vec3f p_n = math::Vec3f(-1.0,0.0,0.0);
			float d = dotProduct(p_n, p_n)/dotProduct(n,p_n);
			uv.x = (d*n*0.5 + 0.5).z;
			uv.y = (d*n*0.5 + 0.5).y;

			uv.y = 1.0 - uv.y;

			uv = uv*math::Vec2f(0.333333f,0.25f) + math::Vec2f(0.0f,0.25f);
		}
	} else
	if (absN.y > absN.x && absN.y > absN.z)
	{
		if (n.y>0)
		{
			// top face
			math::Vec3f p_n = math::Vec3f(0.0,1.0,0.0);
			float d = dotProduct(p_n, p_n)/dotProduct(n,p_n);
			//uv = (d*n*0.5 + 0.5).xz;
			uv.x = (d*n*0.5 + 0.5).x;
			uv.y = (d*n*0.5 + 0.5).z;

			uv = uv*math::Vec2f(0.333f,0.25f) + math::Vec2f(0.333f,0.0f);
		}
		else
		{
			// bottom face
			math::Vec3f p_n = math::Vec3f(0.0,-1.0,0.0);
			float d = dotProduct(p_n, p_n)/dotProduct(n,p_n);
			//uv = (d*n*0.5 + 0.5).xz;
			uv.x = (d*n*0.5 + 0.5).x;
			uv.y = (d*n*0.5 + 0.5).z;
			uv.y = 1.0 - uv.y;

			uv = uv*math::Vec2f(0.333f,0.25f) + math::Vec2f(0.333f,0.5f);
		}
	}else
	{
		if (n.z>0)
		{
			// front face
			math::Vec3f p_n = math::Vec3f(0.0,0.0,1.0);
			float d = dotProduct(p_n, p_n)/dotProduct(n,p_n);
			//uv = (d*n*0.5 + 0.5).xy;
			uv.x = (d*n*0.5 + 0.5).x;
			uv.y = (d*n*0.5 + 0.5).y;
			uv.y = 1.0 - uv.y;

			uv = uv*math::Vec2f(0.333f,0.25f) + math::Vec2f(0.333f,0.25f);
		}
		else
		{
			// back face
			math::Vec3f p_n = math::Vec3f(0.0,0.0,-1.0);
			float d = dotProduct(p_n, p_n)/dotProduct(n,p_n);
			//uv = (d*n*0.5 + 0.5).xy;
			uv.x = (d*n*0.5 + 0.5).x;
			uv.y = (d*n*0.5 + 0.5).y;

			uv = uv*math::Vec2f(0.333333f,0.25f) + math::Vec2f(0.333333f,0.75f);
		}
	}


	return uv;
}



void render( base::CameraPtr cam )
{
	// render to screen
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render environment map
	context->setView( cam->m_viewMatrix.getOrientation(), cam->m_transform.getOrientation(), cam->m_projectionMatrix );
	glDisable( GL_DEPTH_TEST );
	glDepthMask(false);
	context->render( sphere, envShader );
	glDepthMask(true);
	glEnable( GL_DEPTH_TEST );

	context->setCamera( cam );
	context->render( sphere, sphereShader );
}


void init()
{
	std::cout << "init!\n";

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "glew init failed\n";
	}


	context = base::ContextPtr( new base::Context() );

	envImage = base::Image::load( base::Path( SRC_PATH ) + "data/grace_cross.jpg" );
	envTex = base::Texture2d::load( base::Path( SRC_PATH ) + "data/grace_cross.jpg" );
	envCubeMap = base::TextureCube::createRGBA8();
	envCubeMap->upload( envImage );

	starsTex = base::Texture2d::load( base::Path( SRC_PATH ) + "data/stars1.jpg" );
	//starsTex = base::Texture2d::load( base::Path( SRC_PATH ) + "data/stars2.jpg" ); // stars2: cassini?
	//starsTex = base::Texture2d::load( base::Path( SRC_PATH ) + "data/cassini.jpg" );



	envShader = base::Shader::load( base::Path( SRC_PATH ) + "src/env.vs.glsl", base::Path( SRC_PATH ) + "src/env.ps.glsl" );
	envShader->setUniform( "envTex", envCubeMap->getUniform() );

	sphereShader = base::Shader::load( base::Path( SRC_PATH ) + "src/sh.vs.glsl", base::Path( SRC_PATH ) + "src/sh.ps.glsl" );
	sphereShader->setUniform( "envTex", envTex->getUniform() );
	sphereShader->setUniform( "envTex2", envCubeMap->getUniform() );
	sphereShader->setUniform( "starsTex", starsTex->getUniform() );


	// sphere
	sphere = base::geo_sphere( 30, 30, 1.0f );
	base::apply_normals( sphere );
}


int main(int argc, char ** argv)
{
	//Q_INIT_RESOURCE(application);
	QApplication app(argc, argv);
	app.setOrganizationName("test");
	app.setApplicationName("test");

	QMainWindow mainWin;
	mainWin.resize(800, 600);
	glviewer = new composer::widgets::GLViewer(init, render);
	glviewer->getCamera()->m_znear = 0.1f;
	glviewer->getCamera()->m_zfar = 100000.0f;
	mainWin.setCentralWidget( glviewer );
	mainWin.show();


	return app.exec();
}
