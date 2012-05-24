//============================================================================
//
//
// TODO: update to gl4.2 render grid/transform
//============================================================================



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

#include "SPH.h"
#include "SDF.h"
#include "Terrain.h"


base::GLViewer    *glviewer;
base::ContextPtr    context;

base::GeometryPtr      grid;
base::ShaderPtr  greyShader;




// SPH stuff ======================
SPHPtr sph;

// rendering
base::GeometryPtr             m_renderParticles;
base::ShaderPtr                m_particleShader;
std::vector<int>                    m_renderIds; // indicees of particles to render

// debug
base::GeometryPtr                        sdfGeo;
base::Texture2dPtr                        slice;
base::ShaderPtr                       sdfShader;





struct TerrainSDF
{
	TerrainPtr m_terrain;
	TerrainSDF()
	{
		m_terrain = Terrain::create( base::Path( SRC_PATH ) + "data/terrain_1_perlinnoise.tga" );
	}

	float operator()( const math::Vec3f &p )
	{
		math::Vec3f _p = p;


		// height map ==============================
		{
			float d = FLT_MAX;
			int tileResX = 20;
			int tileResY = 20;
			// iterate over all tiles and compute distance by approximating each tile with triangles
			// and using triangle distances
			float uInc = 1.0f/(float)tileResX;
			float vInc = 1.0f/(float)tileResY;
			for( int j=0;j<tileResY;++j )
				for( int i=0;i<tileResX;++i )
				{
					float u = (float)i/(float)tileResX;
					float v = (float)j/(float)tileResY;

					// get current triangles
					math::Vec3f p0, p1, p2, p3;
					p0 = math::Vec3f( u, m_terrain->getHeight( u, v ), v );
					p1 = math::Vec3f( u+uInc, m_terrain->getHeight( u+uInc, v ), v );
					p2 = math::Vec3f( u+uInc, m_terrain->getHeight( u+uInc, v+vInc ), v+vInc );
					p3 = math::Vec3f( u, m_terrain->getHeight( u, v+vInc ), v+vInc );
					d = std::min( d, math::distancePointTriangle( p, p0, p1, p2 ) );
					d = std::min( d, math::distancePointTriangle( p, p0, p2, p3 ) );
				}

			// determine sign by looking at the height of p
			float sign = 1.0f;
			if( p.y < m_terrain->getHeight( p.x, p.z ) )
				sign = -1.0f;

			//return d<0.01?1.0:0.0f;
			return sign*d;
		}
	}
};

base::GeometryPtr      terrainGeo;
base::ShaderPtr     terrainShader;






void render( base::CameraPtr cam )
{
	// put rendering code here
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	context->setCamera( cam );

	glEnable( GL_DEPTH_TEST );
	context->render( grid, greyShader );

	//context->renderScreen( heightMap->m_heightMap );
	//context->render( heightMap->m_previewGeometry, heightMap->m_shader );
	context->render( terrainGeo, terrainShader );
	if( sdfGeo )
		context->render( sdfGeo, sdfShader );
	glDisable( GL_DEPTH_TEST );




	// advance sph system
	sph->advance();

	// update renderparticles
	base::AttributePtr positions = m_renderParticles->getAttr( "P" );
	base::AttributePtr colors = m_renderParticles->getAttr( "Cd" );
	for( std::vector<int>::iterator it = m_renderIds.begin(); it != m_renderIds.end();++it )
	{
		int &id =*it;
		positions->set<math::Vec3f>( id, sph->m_particles[id].position );
		colors->set<math::Vec3f>( id, sph->m_particles[id].color );
	}

	// render sph
	glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );
	glTexEnvf( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );
	glEnable( GL_POINT_SPRITE );
	context->render( m_renderParticles, m_particleShader );
	glDisable( GL_POINT_SPRITE );



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
	grid = base::geo_grid( 5, 5, base::Geometry::LINE );
	//base::apply_transform( grid, math::Matrix44f::RotationMatrixX( math::degToRad(90.0f) ) );
	greyShader = base::Shader::createSimpleConstantShader( 0.6f, 0.6f, 0.6f );


	// initialize sph system =============================
	sph = SPH::create();
	sph->initialize();


	// add collisionobject (SDF)
	TerrainSDF terrainSDF;
	Domain m_sdfWindow; // defines the window in sdf's local space which we are sampling

	// define region in sdf local space and sampling rate
	m_sdfWindow.bound = math::BoundingBox3d( math::Vec3f( 0.0f, 0.0f, 0.0f ), math::Vec3f( 1.0f, 1.0f, 1.0f ) );
	m_sdfWindow.width = 30;
	m_sdfWindow.height = 30;
	m_sdfWindow.depth = 30;
	// distance values will now be in sdf local space
	ScalarFieldPtr sdf1 = distanceTransform( m_sdfWindow, terrainSDF );

	// TODO: sdf1->transform( ... );

	sph->addCollider( sdf1 );





	// setup rendering ======================
	// particles
	m_renderIds.clear();
	m_renderParticles = base::Geometry::createPointGeometry();
	base::AttributePtr positions = m_renderParticles->getAttr("P");
	m_renderParticles->setAttr( "Cd", base::Attribute::createVec3f((int)sph->m_particles.size()) );
	int index = 0;
	for( SPH::ParticleContainer::iterator it = sph->m_particles.begin(); it != sph->m_particles.end();++it, ++index )
	{
		m_renderParticles->addPoint(positions->appendElement(math::Vec3f()));
		m_renderIds.push_back( index);
	}

	m_particleShader = base::Shader::load( base::Path( SRC_PATH ) + "src/SPH.particle" );
	m_particleShader->setUniform( "scale", 10.0f );
	m_particleShader->setUniform( "alpha", 1.0f );

	// terrain (collider)
	terrainGeo = Terrain::createGeometry( (base::Path( SRC_PATH ) + "data/terrain_1_perlinnoise.tga").str(), 30, 30 );
	//base::apply_transform( terrainGeo, math::Matrix44f::TranslationMatrix(0.5f, 0.0f, 0.5f) );
	terrainShader = base::Shader::createSimpleLambertShader();


	/*
	// sdf test
	sdfGeo = base::geo_quad();
	//base::apply_transform( sdfGeo, math::Matrix44f::ScaleMatrix(2.0f)*math::Matrix44f::TranslationMatrix(0.0f, 0.0f, 0.0f) );
	//slice = sdf1.createSlice( math::Vec2f(-2.0f, -2.0f), math::Vec2f(2.0f, 2.0f), 512, 512 );
	base::apply_transform( sdfGeo, math::Matrix44f::ScaleMatrix(0.5f)*math::Matrix44f::TranslationMatrix(0.0f, 0.5f, 0.0f) );
	slice = sdf1->createSlice( math::Vec2f(-0.5f, 0.0f), math::Vec2f(.5f, 1.0f), 0.5f, 512, 512 );
	sdfShader = base::Shader::createSimpleTextureShader(slice);
	*/

}

void shutdown()
{
	// put your deinitialization stuff here
}





int main(int argc, char ** argv)
{
	base::Application app;
	glviewer = new base::GLViewer( 800, 600, "app", init, render );
	glviewer->show();
	return app.exec();
}
