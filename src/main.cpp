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

#include <ui/GLViewer.h>
#include <gltools/gl.h>
#include <gltools/misc.h>
#include <util/StringManip.h>
#include <util/Path.h>
#include <gfx/Geometry.h>
#include <gfx/Shader.h>
#include <gfx/Texture.h>
#include <gfx/Context.h>
#include <gfx/FCurve.h>
#include <gfx/glsl/common.h>
#include <gfx/FBO.h>



#include "composer/widgets/GLViewer/GLViewer.h"

composer::widgets::GLViewer *glviewer;

base::ContextPtr context;
base::Texture2dPtr particlePositions;
base::GeometryPtr particles;
base::ShaderPtr particleShader;


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

void render( base::CameraPtr cam )
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadMatrixf( cam->m_projectionMatrix.ma );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadMatrixf( (GLfloat *)cam->m_viewMatrix.ma );

	// draw scene
	base::drawGrid(false);

	glEnable( GL_POINT_SPRITE_ARB );


	float quadratic[] =  { 0.0f, 0.0f, 0.01f };

	glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );

	float maxSize = 0.0f;

	glGetFloatv( GL_POINT_SIZE_MAX_ARB, &maxSize );

	glPointSize( maxSize );

	glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, maxSize );

	glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, 1.0f );

	glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );

	glEnable( GL_POINT_SPRITE_ARB );

	glBegin( GL_POINTS );
	{
		for(int i=0;i<1;i++)
		{
			//glColor4f(VecParticle[i].Color.x,VecParticle[i].Color.y,VecParticle[i].Color.z,1.0f);
			//glVertex3f(VecParticle[i].Position.x,VecParticle[i].Position.y,VecParticle[i].Position.z);
			glVertex3f(1.0f, 1.0f, 1.0f);
		}
	}

	glEnd();

	glDisable( GL_POINT_SPRITE_ARB );


/*
	//
	base::AttributePtr a = geo->getAttr("P");

	switch( geo->primitiveType() )
	{
		default:
		case base::Geometry::POINT:
		{
			for( unsigned int i=0; i<geo->numPrimitives();++i )
			{
				glBegin( GL_POINTS );
				glColor3f(1.0f, 0.0f, 0.0f);
				math::Vec3f &v=a->get<math::Vec3f>(i);
				glVertex3f( v.x, v.y, v.z );
				glEnd();
			}
		}break;
		case base::Geometry::TRIANGLE:
		{
			std::vector<unsigned int>::iterator it = geo->m_indexBuffer.begin();
			std::vector<unsigned int>::iterator end = geo->m_indexBuffer.end();
			while( it != end )
			{
				glBegin( GL_TRIANGLES );
				glColor3f(1.0f, 0.0f, 0.0f);
				math::Vec3f &v0=a->get<math::Vec3f>(*it++);
				math::Vec3f &v1=a->get<math::Vec3f>(*it++);
				math::Vec3f &v2=a->get<math::Vec3f>(*it++);
				glVertex3f( v0.x, v0.y, v0.z );
				glVertex3f( v1.x, v1.y, v1.z );
				glVertex3f( v2.x, v2.y, v2.z );
				glEnd();
			}
		}break;
		case base::Geometry::QUAD:
		{
			std::vector<unsigned int>::iterator it = geo->m_indexBuffer.begin();
			std::vector<unsigned int>::iterator end = geo->m_indexBuffer.end();
			while( it != end )
			{
				glBegin( GL_QUADS );
				glColor3f(1.0f, 0.0f, 0.0f);
				math::Vec3f &v0=a->get<math::Vec3f>(*it++);
				math::Vec3f &v1=a->get<math::Vec3f>(*it++);
				math::Vec3f &v2=a->get<math::Vec3f>(*it++);
				math::Vec3f &v3=a->get<math::Vec3f>(*it++);
				glVertex3f( v0.x, v0.y, v0.z );
				glVertex3f( v1.x, v1.y, v1.z );
				glVertex3f( v2.x, v2.y, v2.z );
				glVertex3f( v3.x, v3.y, v3.z );
				glEnd();
			}
		}break;
	};
*/

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}

void render2( base::CameraPtr cam )
{
	//fbo->begin(false);
	glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
	glDisable( GL_CULL_FACE );
	//glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );


	context->setView( cam->m_viewMatrix, cam->m_transform, cam->m_projectionMatrix );


	// render to screen
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	context->render( particles, particleShader );







	/*
	glEnable( GL_POINT_SPRITE_ARB );


	float quadratic[] =  { 0.0f, 0.0f, 0.01f };

	glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );

	float maxSize = 0.0f;

	glGetFloatv( GL_POINT_SIZE_MAX_ARB, &maxSize );

	glPointSize( maxSize );

	glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, maxSize );

	glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, 1.0f );

	glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );

	glEnable( GL_POINT_SPRITE_ARB );

	glBegin( GL_POINTS );
	{
		for(int i=0;i<1;i++)
		{
			//glColor4f(VecParticle[i].Color.x,VecParticle[i].Color.y,VecParticle[i].Color.z,1.0f);
			//glVertex3f(VecParticle[i].Position.x,VecParticle[i].Position.y,VecParticle[i].Position.z);
			glVertex3f(1.0f, 1.0f, 1.0f);
		}
	}

	glEnd();

	glDisable( GL_POINT_SPRITE_ARB );
	*/
	//context->renderScreen( shader_screen );
	//glEnable( GL_BLEND );
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//context->render( geo, cloudShader );

	/*
	// debug
	GLint viewport[4];
	//GLubyte pixel[3];
	float pixel[4];

	glGetIntegerv(GL_VIEWPORT,viewport);

	glReadPixels((int)(viewport[0] + viewport[2]*0.5f),(int)(viewport[3]-viewport[3]*0.5f),1,1, GL_RGBA,GL_FLOAT,(void *)pixel);

	std::cout << "pixel " << pixel[0] << " " << pixel[1] << " " << pixel[2] << " " << pixel[3] << std::endl;

	glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_TRUE);glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_TRUE);glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_TRUE);
	*/

	//fbo->end();
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

	particles = base::geo_grid(14, 14, base::Geometry::POINT);

	particleShader = base::Shader::load( base::Path( SRC_PATH ) + "src/particles.vs.glsl", base::Path( SRC_PATH ) + "src/particles.ps.glsl" );


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
	glviewer->getCamera()->m_znear = 1.0f;
	glviewer->getCamera()->m_zfar = 100000.0f;
	mainWin.setCentralWidget( glviewer );
	mainWin.show();


	return app.exec();
}
