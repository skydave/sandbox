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

base::GLViewer    *glviewer;
base::ContextPtr    context;

base::GeometryPtr      grid;
base::ShaderPtr  greyShader;




// SPH stuff ======================

struct Particle
{
	/*
	struct Neighbour
	{
		math::Vec3f 
		float        distance;
		Particle   *neighbour;
	};
	*/

	math::Vec3f                                          position;
	math::Vec3f                                      positionPrev;
	math::Vec3f                                          velocity;
	math::Vec3f                                      acceleration;
	float                                                    mass; // in kg
	float                                             massDensity; // in kg/m³
	float                                                pressure; // in ? pascal ?

	typedef std::vector< std::pair<float, Particle*> > Neighbours;
	Neighbours                                         neighbours; // list of particles within support radius and their distances; computed once per timestep

	// used for debugging:
	int                                                        id;
	math::Vec3f                                             color;
};

typedef std::vector<Particle> ParticleContainer;
ParticleContainer                   m_particles;


// material properties
float                        m_idealGasConstant; // nRT - expresses amount of substance per mol plus temperature
float                             m_restDensity;



float                    m_supportRadius = 1.0f;
int                          m_numParticles = 0;
float                       m_timeStep = 0.001f;

// rendering
base::GeometryPtr             m_renderParticles;
base::ShaderPtr                m_particleShader;
std::vector<int>                    m_renderIds; // indicees of particles to render

// internal
float                    m_supportRadiusSquared;


// debug
float                             m_maxPressure;
float                          m_maxMassDensity;


struct HalfPlaneCollider
{
	math::Vec3f m_normal;
	float     m_distance;

	HalfPlaneCollider()
	{
		m_normal = math::Vec3f( 0.0f, 1.0f, 0.0f );
		m_distance = 0.0f;
	}

	void operator()( ParticleContainer &container )
	{
		for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
		{
			Particle &p = *it;

			float d = math::dotProduct( m_normal, p.position ) + m_distance;

			if( d < 0.0f )
			{
				p.position = p.position - d*m_normal;
				p.velocity = p.velocity - 2.0f*( math::dotProduct(p.velocity, m_normal) )*m_normal;
			}
		}
	}
};

struct HeightMapCollider
{
	std::vector<float>                m_heightMap;
	std::vector<math::Vec3f>   m_heightMapNormals;
	int                          m_heightMapWidth;
	int                         m_heightMapHeight;

	float                               m_extendX; // spatial extend in m
	math::Vec3f                          m_offset;
	float                         m_verticalScale;

	bool                                 m_make2d;

	float                                 m_delta; // used for numerical computation of the gradient

	// used for preview
	base::GeometryPtr           m_previewGeometry;
	base::ShaderPtr                      m_shader;
	base::GeometryPtr                   m_terrain;
	base::ShaderPtr              m_geometryShader;

	HeightMapCollider()
	{
		m_make2d = true;

		m_delta = 0.05f;

		m_extendX = 1.0f;
		m_offset.x = -0.5f;
		m_offset.y = .0f;
		m_offset.z = -0.5f;
		m_verticalScale = .6f;

		//m_geometryShader = base::Shader::load( base::Path( SRC_PATH ) + "src/SPH.geometry" );
		m_geometryShader = base::Shader::createSimpleLambertShader();


		// get height data from image
		{
			base::ImagePtr heightMap = base::Image::load( base::Path( SRC_PATH ) + "data/terrain_1.tga" );
			m_heightMapWidth = heightMap->m_width;
			m_heightMapHeight = heightMap->m_height;
			m_heightMap.resize( heightMap->m_width*heightMap->m_height, 0.0f );
			for( int j=0;j<heightMap->m_height;++j )
				for( int i=0;i<heightMap->m_width;++i )
				{
					m_heightMap[ j*heightMap->m_width  + i ] = heightMap->lookup(i, j).r;
				}

			m_heightMapNormals.resize( heightMap->m_width*heightMap->m_height, 0.0f );
			for( int j=0;j<heightMap->m_height;++j )
				for( int i=0;i<heightMap->m_width;++i )
				{
					int x = i;
					int y = j;

					float a11 = getHeight( x, y );
					float a01 = getHeight( x-1, y );
					float a21 = getHeight( x+1, y );
					float a10 = getHeight( x, y-1 );
					float a12 = getHeight( x, y+1 );

					math::Vec3f right = math::Vec3f( 1.0f/(float)m_heightMapWidth, (a21-a01)/2.0f, 0.0f ).normalized();
					math::Vec3f forward = math::Vec3f( 0.0f, (a12-a10)/2.0f, 1.0f/(float)m_heightMapHeight ).normalized();
					math::Vec3f n = math::crossProduct( forward, right ).normalized();
					n.y *= m_verticalScale;
					m_heightMapNormals[ y*heightMap->m_width  + x ] = n;
				}
		}

		// setup preview rendering stuff
		m_shader = base::Shader::createSimpleConstantShader( 1.0f, 1.0f, 1.0f );
		m_previewGeometry = base::Geometry::createLineGeometry();
		base::AttributePtr pAttr = m_previewGeometry->getAttr("P");
		int numSegments = 100;
		for( int i=0;i<numSegments;++i )
		{
			float u = (float)i/(float)numSegments;
			float height = getHeight( u, 0.5f );

			math::Vec3f p = math::Vec3f(u*m_extendX+m_offset.x, height, 0.0f);
			pAttr->appendElement(p);

			if( i > 0 )
				m_previewGeometry->addLine( i-1, i );
		}

		{
			for( int j=0;j<m_heightMapHeight;++j )
				for( int i=0;i<m_heightMapWidth;++i )
				{
					math::Vec3f p = math::Vec3f( (float)i/(float)m_heightMapWidth+m_offset.x, getHeight( i, j ), (float)j/(float)m_heightMapHeight+m_offset.z );
					math::Vec3f n = getNormal( i, j );
					int i0 = pAttr->appendElement(p);
					int i1 = pAttr->appendElement(p+n*0.005f);
					//m_previewGeometry->addLine( i0, i1 );
				}
		}

		{
			// get a grid...
			m_terrain = base::geo_grid( 130, 130 );
			// move it into right postion
			base::apply_transform( m_terrain, math::Matrix44f::TranslationMatrix( 0.5f+m_offset.x, 0.0f, 0.5f+m_offset.z ) );
			// get points
			base::AttributePtr pAttr = m_terrain->getAttr( "P" );
			int numElements = pAttr->numElements();
			// update points heights from heightmap
			for( int i=0;i<numElements;++i )
				{
					math::Vec3f p = pAttr->get<math::Vec3f>(i);

					float h = this->getHeight_interp( p.x-m_offset.x, p.z-m_offset.z );
					p.y = h;

					pAttr->set<math::Vec3f>(i, p);

				}
			base::apply_normals( m_terrain );
		}


	}



	float getHeight( int x, int y )
	{
		if( x < 0 )
			x = 0;
		if( x >= m_heightMapWidth )
			x = m_heightMapWidth-1;
		if( y < 0 )
			y = 0;
		if( y >= m_heightMapHeight )
			y = m_heightMapHeight-1;

		float height = m_heightMap[y*m_heightMapWidth + x];

		return height*m_verticalScale + m_offset.y;
	}


	math::Vec3f getNormal( int x, int y )
	{
		if( x < 0 )
			return math::Vec3f( 0.0f, 1.0f, 0.0f );
		if( x >= m_heightMapWidth )
			return math::Vec3f( 0.0f, 1.0f, 0.0f );
		if( y < 0 )
			return math::Vec3f( 0.0f, 1.0f, 0.0f );
		if( y >= m_heightMapHeight )
			return math::Vec3f( 0.0f, 1.0f, 0.0f );

		return m_heightMapNormals[ y*this->m_heightMapWidth  + x ];
	}

	float getHeight( float u, float v )
	{
		// nearest neighbour
		int x = int(u*m_heightMapWidth);
		int y = int(v*m_heightMapHeight);
		return getHeight( x, y );
	}

	float getHeight_interp( float u, float v )
	{
		// bilinear interpolation
		float _x = u*m_heightMapWidth - 0.5f;
		float _y = v*m_heightMapHeight - 0.5f;

		float tx = _x - floor(_x);
		float ty = _y - floor(_y);
		int x = (int)floor(_x);
		int y = (int)floor(_y);

		float a, b, c, d;
		a = getHeight( x, y );
		b = getHeight( x+1, y );
		c = getHeight( x, y+1 );
		d = getHeight( x+1, y+1 );

		float height = math::lerp( math::lerp( a, b, tx ), math::lerp( b, c, tx ), ty );

		return height;
	}


	void operator()( ParticleContainer &container )
	{
		for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
		{
			Particle &p = *it;

			if( (p.position.x > m_offset.x)&&(p.position.x+m_offset.x < m_extendX) ) 
			{
				// query heightmap
				float height = getHeight((p.position.x-m_offset.x) / m_extendX , 0.5f);
	
				// compute gradient
				//math::Vec3f n = getNormal((p.position.x-m_offsetX) / m_extendX, 0.5f );


				
				if( p.position.y < height )
				{
					float d = height -p.position.y;
					p.position = math::Vec3f( p.position.x, height, 0.0f );
					//p.position += math::dotProduct(n, math::Vec3f(1.0f, 0.0f, 0.0f))*0.001f*math::Vec3f(1.0f, 0.0f, 0.0f);
					//p.velocity = p.velocity - 2.0f*( math::dotProduct(p.velocity, n) )*n;
				}
			}
		}
	}
};

HeightMapCollider *heightMap;

HalfPlaneCollider bottom, bottom2;

void timeIntegration()
{
	/*
	// semi-implicit euler
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;
		p.velocity = p.velocity + p.acceleration*m_timeStep;
		p.position = p.position + p.velocity*m_timeStep;
	}
	*/
	// verlet
	float Damping = 0.01f;
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		// Position = Position + (1.0f - Damping) * (Position - PositionOld) + dt * dt * a;
		math::Vec3f oldPos = p.position;
		p.position = p.position + (1.0f - Damping) * (p.position - p.positionPrev) + m_timeStep*m_timeStep*p.acceleration;
		p.positionPrev = oldPos;

		// Velocity = (Position - PositionOld) / dt;
		p.velocity = (p.position - p.positionPrev) / m_timeStep;


	}

}

void handleCollisions()
{
	/*
	bottom.m_normal = math::Vec3f( -1.0f, 1.0f, 0.0f ).normalized();
	bottom.m_distance = 2.0f;
	bottom( m_particles );


	bottom2.m_normal = math::Vec3f( 1.0f, 1.0f, 0.0f ).normalized();
	bottom2( m_particles );
	*/
	(*heightMap)( m_particles );
}

void advance()
{
	// debug ===============
	m_maxPressure = 0.0f;
	m_maxMassDensity = 0.0f;
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;
		p.color = math::Vec3f( 0.54f, 0.85f, 1.0f );
	}

	// update particle properties =========================================
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;


		// update neighbour information - we look them up once and reuse them throughout the timestep
		p.neighbours.clear();
		// bruteforce for now - use efficient lookup structure later
		for( ParticleContainer::iterator it2 = m_particles.begin(); it2 != m_particles.end();++it2 )
		{
			if( it == it2 )continue;
			Particle &p2 = *it2;
			float distanceSquared = (p.position - p2.position).getSquaredLength();
			if( distanceSquared < m_supportRadiusSquared )
				p.neighbours.push_back( std::make_pair(sqrt(distanceSquared), &p2) );
		}

		// compute mass-densities of particles
		p.massDensity = p.mass*W_poly6(0.0f);
		for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
			p.massDensity += it2->second->mass*W_poly6(it2->first);

		// compute pressure at particle
		p.pressure = m_idealGasConstant*(p.massDensity-m_restDensity);


		// particle colors for debug
		if( p.id == 0 )
		{
			p.color = math::Vec3f( 1.0f, 0.0f, 0.0f );
			for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
				it2->second->color = math::Vec3f( 0.0f, 1.0f, 0.0f );
		}
		m_maxPressure = std::max( fabsf(p.pressure), m_maxPressure );
		m_maxMassDensity = std::max( fabsf(p.massDensity), m_maxMassDensity );
	}


	// compute forces =====================================================
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		// pressure force
		math::Vec3f f_pressure( 0.0f, 0.0f, 0.0f );
		for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
		{
			float distance = it2->first;
			Particle &n = *(it2->second);
			math::Vec3f gradW = gradW_spiky( distance, p.position - n.position );
			f_pressure +=  -(p.pressure + n.pressure)*0.5f*(n.mass/n.massDensity)*gradW;
		}


		//gravity
		math::Vec3f f_gravity( 0.0f, 0.0f, 0.0f );
		f_gravity = -math::Vec3f( 0.05f, 1.0f, 0.0f ).normalized()*0.98f;


		// compute acceleration
		// TODO:  check if we need to divide by mass or massDensity
		p.acceleration = (f_pressure+f_gravity)*(1.0f/p.mass);

	}



	// 
	timeIntegration();

	handleCollisions();


	// update renderparticles
	base::AttributePtr positions = m_renderParticles->getAttr( "P" );
	base::AttributePtr colors = m_renderParticles->getAttr( "Cd" );
	for( std::vector<int>::iterator it = m_renderIds.begin(); it != m_renderIds.end();++it )
	{
		int &id =*it;
		positions->set<math::Vec3f>( id, m_particles[id].position );
		colors->set<math::Vec3f>( id, m_particles[id].color );
	}

	// print some debug info
	std::cout << "max abs pressure: " << m_maxPressure << std::endl;
	std::cout << "max abs mass density: " << m_maxMassDensity << std::endl;
}


void updateSupportRadius( float newSupportRadius )
{
	m_supportRadius = newSupportRadius;
	m_supportRadiusSquared = m_supportRadius*m_supportRadius;

	// update weighting kernels
	W_poly6_precompute(m_supportRadius);
	W_spiky_precompute(m_supportRadius);

}

void initialize()
{
	updateSupportRadius( .190625f );

	m_idealGasConstant = 0.1f;
	//m_restDensity = 998.29;
	m_restDensity = 100.0f;

	m_numParticles = 0;

	m_timeStep = 0.01f;


	// initial fluid
	float spacing = 0.2f;
	if(1)
	{
		int n = 10;
		for( int i=0;i<n;++i )
			for( int j=0;j<n;++j, ++m_numParticles )
			{
				Particle p;
				p.id = m_numParticles;
				p.position = math::Vec3f( 0.5f + i * spacing, 0.8f + j * spacing, 0.0f );
				p.positionPrev = p.position;
				p.color = math::Vec3f( 0.54f, 0.85f, 1.0f );
				//p.mass = 0.02f; // water
				p.mass = 3.8125f; // ?
				m_particles.push_back(p);
			}
	}


	// setup rendering
	m_renderIds.clear();
	m_renderParticles = base::Geometry::createPointGeometry();
	base::AttributePtr positions = m_renderParticles->getAttr("P");
	m_renderParticles->setAttr( "Cd", base::Attribute::createVec3f((int)m_particles.size()) );
	int index = 0;
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it, ++index )
	{
		m_renderParticles->addPoint(positions->appendElement(math::Vec3f()));
		m_renderIds.push_back( index);
	}



	m_particleShader = base::Shader::load( base::Path( SRC_PATH ) + "src/SPH.particle" );
	m_particleShader->setUniform( "scale", 10.0f );
	m_particleShader->setUniform( "alpha", 1.0f );
}




void render( base::CameraPtr cam )
{
	// put rendering code here
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	context->setCamera( cam );

	glEnable( GL_DEPTH_TEST );
	context->render( grid, greyShader );

	//context->renderScreen( heightMap->m_heightMap );
	context->render( heightMap->m_previewGeometry, heightMap->m_shader );

	context->render( heightMap->m_terrain, heightMap->m_geometryShader );
	glDisable( GL_DEPTH_TEST );

	// advance sph system
	advance();

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


	// initialize sph system
	initialize();


	heightMap = new HeightMapCollider();
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
