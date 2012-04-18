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
#include <util/fs.h>
#include <util/StopWatch.h>
#include <gfx/Geometry.h>
#include <gfx/Shader.h>
#include <gfx/Texture.h>
#include <gfx/Image.h>
#include <gfx/Context.h>
#include <gfx/FCurve.h>
#include <gfx/glsl/common.h>
#include <gfx/FBO.h>



#include "composer/widgets/GLViewer/GLViewer.h"

#include "Volume.h"
#include "Volume.ui.h"

composer::widgets::GLViewer *glviewer;

base::ContextPtr context;


VolumePtr volume;

QMainWindow *mainWindow;


/*
#include <time.h>
 
void sleep(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}
*/


void render( base::CameraPtr cam )
{
	base::StopWatch sw;

	sw.start();

	// render to screen
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render volume geometry =================
	volume->render( cam );

	glFinish();
	float t = sw.elapsedSeconds()*1000.0f;
	mainWindow->setWindowTitle( QString::number((double)(t)) );
}

struct Ellipsoid
{
	Ellipsoid() : center(), axisRadii(1.0f,1.0f,1.0f), orientation(math::Matrix44f::Identity())
	{
	}

	math::Matrix44f sphereToEllipsoid() const
	{
		return math::Matrix44f::ScaleMatrix(axisRadii) * orientation * math::Matrix44f::TranslationMatrix(center);
	}
	math::Matrix44f ellipsoidToSphere() const
	{
		math::Matrix44f s2e = sphereToEllipsoid();
		return s2e.inverted();
	}


	math::Vec3f          center;
	math::Vec3f       axisRadii;
	math::Matrix44f orientation;

};

base::Texture3dPtr field;
base::Texture3dPtr noise;


float densityEllipsoid( const Ellipsoid &e, math::Vec3f vsPos )
{
	math::Vec3f vsPos_sphere = math::transform( vsPos, e.ellipsoidToSphere() );

	float b = 1.0; // maxdistance the field contributes

	// transform ellipsoid into canoncial sphere space
	float d = vsPos_sphere.getLength();
	//d = clamp( d, 0, b );
	if( d < b )
	{
		float r2 = d*d;
		float r4 = r2*r2;
		float r6 = r4*r2;
		float b2 = b*b;
		float b4 = b2*b2;
		float b6 = b4*b2;

		// wyvill blending
		float density = 1.0 - (4.0*r6)/(9.0*b6) + (17.0*r4)/(9.0*b4) - (22.0*r2)/(9.0*b2);
		//float density = 0.1;
		return density;
	}
	return 0.0;
	//return step( -b, -d )*density;
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
	base::Context::setCurrentContext( context );
	glEnable(GL_TEXTURE_3D);


	// setup stuff for volume rendering ===========================
	volume = Volume::create(base::Path( SRC_PATH ) + "src/Volume.clouds.glsl");
	//volume = Volume::create();

	// setup clouds
	Ellipsoid e0;
	e0.axisRadii = math::Vec3f( 0.21f, 0.12f, 0.37f );
	e0.center = math::Vec3f( 0.6f, 0.48f, 0.32f );
	Ellipsoid e1;
	e1.axisRadii = math::Vec3f( 0.2f, 0.2f, 0.2f );
	e1.center = math::Vec3f( 0.5f, 0.65f, 0.5f );

	base::AttributePtr ellipsoidsAttr = base::Attribute::createMat44();
	ellipsoidsAttr->appendElement<math::Matrix44f>(e0.ellipsoidToSphere());
	ellipsoidsAttr->appendElement<math::Matrix44f>(e1.ellipsoidToSphere());
	volume->setUniform( "ellipsoidToSphere", ellipsoidsAttr );
	base::AttributePtr numCloudsAttr = base::Attribute::createInt();
	numCloudsAttr->appendElement( ellipsoidsAttr->numElements() );
	volume->setUniform( "numClouds", numCloudsAttr );

	/*
	{
		// test compute lowres implicit fieldt texture
		field = base::Texture3d::createFloat16(64, 64, 64);

		float *fieldBuffer_f32 = (float*)malloc( field->m_xres*field->m_yres*field->m_zres*sizeof(float) );
		for( int k = 0; k<field->m_zres;++k )
			for( int j = 0; j<field->m_yres;++j )
				for( int i = 0; i<field->m_xres;++i )
				{
					math::Vec3f p( ((float)i/(float)field->m_xres), ((float)j/(float)field->m_yres), ((float)k/(float)field->m_zres) );
					float d  = densityEllipsoid(e0,p) + densityEllipsoid(e1,p);
					fieldBuffer_f32[k*field->m_xres*field->m_yres + j*field->m_xres + i] = d;
				}
		field->uploadFloat32( field->m_xres, field->m_yres, field->m_zres, fieldBuffer_f32 );
		free(fieldBuffer_f32);
		volume->setUniform( "implicitfield", field->getUniform() );
	}
	*/

	// load lowres implicit field data from file
	{
		field = base::Texture3d::createFloat16(32, 32, 32);

		float *fieldBuffer_f32 = (float*)malloc( field->m_xres*field->m_yres*field->m_zres*sizeof(float) );
		std::string path = base::Path( SRC_PATH ) + "data/cloud1.density";
		base::fs::File *f = base::fs::open( path );
		uint64 l = size(f);
		read( f, fieldBuffer_f32, sizeof(char), (unsigned int)l );
		close(f);
		field->uploadFloat32( field->m_xres, field->m_yres, field->m_zres, fieldBuffer_f32 );
		free(fieldBuffer_f32);
		volume->setUniform( "implicitfield", field->getUniform() );
	}

	/*
	//noise texture is used in clouds.glsl for computing fast_perlinnoise
	{
		// test compute noise texture
		noise = base::Texture3d::createFloat16(512, 512, 512);

		math::PerlinNoise pn;
		pn.setDepth(1);
		pn.setFrequency(1.0f);
		pn.setFrequencyRatio(2.0);
		pn.setAmplitudeRatio(0.5);
		//pn.

		float *noiseBuffer_f32 = (float*)malloc( noise->m_xres*noise->m_yres*noise->m_zres*sizeof(float) );
		for( int k = 0; k<noise->m_zres;++k )
			for( int j = 0; j<noise->m_yres;++j )
				for( int i = 0; i<noise->m_xres;++i )
				{
					math::Vec3f p( ((float)i/(float)noise->m_xres), ((float)j/(float)noise->m_yres), ((float)k/(float)noise->m_zres) );
					p = p*256.0f;
					//float d  = pn.perlinNoise_3D( p.x, p.y, p.z );
					float d  = 0.0f;
					noiseBuffer_f32[k*noise->m_xres*noise->m_yres + j*noise->m_xres + i] = d;
				}
		noise->uploadFloat32( noise->m_xres, noise->m_yres, noise->m_zres, noiseBuffer_f32 );
		free(noiseBuffer_f32);
		volume->setUniform( "noisetex", noise->getUniform() );
	}
	*/



	VolumeUI *widget = new VolumeUI(volume);
	widget->show();
	glviewer->connect( widget, SIGNAL(makeDirty(void)), SLOT(update(void)) );
}


int main(int argc, char ** argv)
{
	//Q_INIT_RESOURCE(application);
	QApplication app(argc, argv);
	app.setOrganizationName("test");
	app.setApplicationName("test");

	QMainWindow mainWin;
	mainWindow = &mainWin;
	mainWin.resize(800, 600);
	glviewer = new composer::widgets::GLViewer(init, render);
	glviewer->getCamera()->m_znear = 0.1f;
	glviewer->getCamera()->m_zfar = 100000.0f;
	mainWin.setCentralWidget( glviewer );
	mainWin.show();


	return app.exec();
}
