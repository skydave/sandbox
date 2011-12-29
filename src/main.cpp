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
#include <tr1/unordered_map>

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



//
// ============= strange attractor ==================
//

struct StrangeAttractor
{
	StrangeAttractor( math::Vec3f initialP = math::Vec3f(0.1f, 0.1f, 0.1f), int initialIterations = 100 )
	{
		// coefficients for "The King's Dream"
		a = -2.643f;
		b = 1.155f;
		c = 2.896f;
		d = 1.986f;

		p = initialP;

		// compute some initial iterations to settle into the orbit of the attractor
		for (int i = 0; i <initialIterations; ++i)
			next();
	}

	math::Vec3f next()
	{
		// compute a new point using the strange attractor equations
		float xnew = sin(a * p.y) - p.z * cos(b * p.x);
		float ynew = p.z * sin(c * p.x) - cos(d * p.y);
		float znew = sin(p.x);

		// save the new point
		p.x = xnew;
		p.y = ynew;
		p.z = znew;

		return p;
	}



	math::Vec3f              p; // last point which was being generated
	float           a, b, c, d; // coefficients for the update formula



};



//
// ============= GRID ==================
//

struct V3i
{
	int i,j,k;
};


struct V3iHashFunction
{
	std::size_t operator ()(const V3i &key) const
	{
		// hash function proposed in [Worley 1996]
		return 541*key.i + 79*key.j + 31*key.k;
	}
};
struct V3iEqual
{
	bool operator ()(const V3i &a, const V3i &b) const
	{
		return (a.i == b.i)&&(a.j == b.j)&(a.k == b.k);
	}
};

typedef std::tr1::unordered_map<V3i, math::Vec3f, V3iHashFunction, V3iEqual> Grid;
Grid grid;

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


	StrangeAttractor sa;


	int iterations = maxNumParticles;            // number of times to iterate through the functions and draw a point


	math::PerlinNoise pn;
	pn.setFrequency( .5f );
	pn.setDepth(3);

	float voxelSize = .025f;


	// go through the equations many times, drawing a point for each iteration
	while( grid.size() < maxNumParticles )
	{
		math::Vec3f p = sa.next();
		float t1 = pn.perlinNoise_3D( p.x, p.y, p.z )*14.0f;
		float t2 = pn.perlinNoise_3D( p.x+100.0f, p.y+100.0f, p.z+100.0f )*14.0f;
		float t3 = pn.perlinNoise_3D( p.x+200.0f, p.y+200.0f, p.z+200.0f )*14.0f;
		p += math::Vec3f(t1,t2,t3);

		V3i key;
		key.i = (int)std::floor(p.x / voxelSize);
		key.j = (int)std::floor(p.y / voxelSize);
		key.k = (int)std::floor(p.z / voxelSize);
		// if particle falls into already existing bucket
		if( grid.find( key ) != grid.end() )
			// drop it
			continue;

		// else: create bucket and put in the particle
		grid[key] = p;
	}


	// TODO: randomly remove buckets and spawn bigger billboard particles

	// the grid now contains all particles which we want to render
	// we transfer the particle positions into pos array and create the points geometry
	int count = 0;
	int i = 0;
	int j = 0;

	for( Grid::iterator it = grid.begin(); it != grid.end(); ++it, ++count )
	{
		const V3i &key = it->first;
		math::Vec3f &value = it->second;

		if( count < maxNumParticles )
		{
			posArray[count*4 + 0] = value.x;
			posArray[count*4 + 1] = value.y;
			posArray[count*4 + 2] = value.z;
			posArray[count*4 + 3] = 1.0f;

			float u = ((float)i+0.5f)/(float)(particlesDataRes);
			float v = ((float)j+0.5f)/(float)(particlesDataRes);

			particles->addPoint(positions->appendElement(math::Vec3f(u,v,0.0f)));
		}


		if( ++i >= particlesDataRes )
		{
			i = 0;
			j +=1;
		}
	}


	/*
	// transfer particle positions to array (which will be used to initialize position texture)
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


	// add particles to geometry
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
	*/
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
