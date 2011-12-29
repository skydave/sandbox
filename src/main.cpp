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
	/*
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


	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
*/
}

void render2( base::CameraPtr cam )
{
	glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );


	context->setCamera( cam );


	// render to screen
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );


	glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );
	glTexEnvf( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );

	glEnable( GL_POINT_SPRITE );

	context->render( particles, particleShader );

	glDisable( GL_POINT_SPRITE );

	glDisable( GL_BLEND );

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

	//particles = base::geo_blank(base::Geometry::POINT);
	//particles = base::geo_grid(14,14,base::Geometry::POINT);
	particles = base::Geometry::createPointGeometry();
	//particles = base::geo_sphere(14, 14, 1.0f, math::Vec3f(0.0f,0.0f,0.0f), base::Geometry::POINT);
	//particles = base::geo_sphere(28, 28, 1.0f, math::Vec3f(0.0f,0.0f,0.0f) );
	

	// todo: use texture for storing particle positions, position is retrieved by looking up texture in vertex shader, initial position of particle is the uv
	// coordinate into position texture
	// vertex shader: position lookup using uv
	// init: store uv position with particle, store position in texture

	int particlesDataRes = 1024;
	int maxNumParticles = particlesDataRes*particlesDataRes;
	particlePositions = base::Texture2d::createRGBAFloat32( particlesDataRes, particlesDataRes );
	float *posArray = (float *)malloc( particlesDataRes*particlesDataRes*sizeof(float)*4 );

	base::ImagePtr img = base::Image::load( base::Path( SRC_PATH ) + "data/circlealpha.bmp" );
	particleTex = base::Texture2d::createRGBA8();
	particleTex->upload( img );


	particleShader = base::Shader::load( base::Path( SRC_PATH ) + "src/particles.vs.glsl", base::Path( SRC_PATH ) + "src/particles.ps.glsl" );
	particleShader->setUniform( "tex", particleTex->getUniform() );


	base::AttributePtr positions = particles->getAttr("P");
	std::vector<math::Vec3f> posVec;


	math::Vec3f p(0.1f, 0.1f, 0.1f);
	//float a = -0.966918;                  // coefficients for "The King's Dream"
	//float b = 2.879879;
	//float c = 0.765145;
	//float d = 0.744728;
	float a = -2.643f;                  // coefficients for "The King's Dream"
	float b = 1.155f;
	float c = 2.896f;
	float d = 1.986f;
	int initialIterations = 100;        // initial number of iterations to allow the attractor to settle
	int iterations = 1000000;            // number of times to iterate through the functions and draw a point

	// compute some initial iterations to settle into the orbit of the attractor
	for (int i = 0; i <initialIterations; ++i)
	{
		// compute a new point using the strange attractor equations
		float xnew = sin(a * p.y) - p.z * cos(b * p.x);
		float ynew = p.z * sin(c * p.x) - cos(d * p.y);
		float znew = sin(p.x);

		// save the new point
		p.x = xnew;
		p.y = ynew;
		p.z = znew;
	}


	// go through the equations many times, drawing a point for each iteration
	for (int i = 0; i<iterations; ++i)
	{
		// compute a new point using the strange attractor equations
		float xnew = sin(a * p.y) - p.z * cos(b * p.x);
		float ynew = p.z * sin(c * p.x) - cos(d * p.y);
		float znew = sin(p.x);

		// save the new point
		p.x = xnew;
		p.y = ynew;
		p.z = znew;

		// draw the new point
		posVec.push_back( p );
	}


	//apply perlin noise
	math::PerlinNoise pn;
	pn.setFrequency( .5f );
	pn.setDepth(3);
	int numElements = posVec.size();
	for( int i=0;i<numElements;++i )
	{
		math::Vec3f &p = posVec[i];
		float t1 = pn.perlinNoise_3D( p.x, p.y, p.z )*14.0f;
		float t2 = pn.perlinNoise_3D( p.x+100.0f, p.y+100.0f, p.z+100.0f )*14.0f;
		float t3 = pn.perlinNoise_3D( p.x+200.0f, p.y+200.0f, p.z+200.0f )*14.0f;
		p += math::Vec3f(t1,t2,t3);

	}

	// tmp
	int count2 = 0;
	for( std::vector<math::Vec3f>::iterator it = posVec.begin(); it != posVec.end(); ++it, ++count2 )
	{
		math::Vec3f &v = *it;
		//particles->addPoint(positions->appendElement(v));
		posArray[count2*4 + 0] = v.x;
		posArray[count2*4 + 1] = v.y;
		posArray[count2*4 + 2] = v.z;
		posArray[count2*4 + 3] = 1.0f;
	}

	float count = 0;
	for( int j=0; j<particlesDataRes; ++j )
		for( int i=0; i<particlesDataRes; ++i, ++count )
		{
			if( count < maxNumParticles )
			{
				float u = ((float)i+0.5f)/(float)(particlesDataRes);
				float v = ((float)j+0.5f)/(float)(particlesDataRes);
				particles->addPoint(positions->appendElement(math::Vec3f(u,v,0.0f)));
			}
		}

	//base::apply_normals(particles);

	// upload initial particle positions
	particlePositions->uploadRGBAFloat32( 1024, 1024, posArray );
	particleShader->setUniform( "pos", particlePositions->getUniform() );
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
