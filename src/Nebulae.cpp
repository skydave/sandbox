
#include "Nebulae.h"

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



NebulaePtr Nebulae::create()
{
	return NebulaePtr( new Nebulae() );
}


Nebulae::Nebulae()
{
	m_particleDataRes = 1024;
	m_maxNumParticles = m_particleDataRes*m_particleDataRes;

	m_particleShader = base::Shader::load( base::Path( SRC_PATH ) + "src/particles.vs.glsl", base::Path( SRC_PATH ) + "src/particles.ps.glsl" );
	m_particleShader->setUniform( "scale", 10.0f );
	m_particleShader->setUniform( "alpha", 0.1f );

	{
		base::ImagePtr img = base::Image::load( base::Path( SRC_PATH ) + "data/circlealpha.bmp" );
		m_particleTex = base::Texture2d::createRGBA8();
		m_particleTex->upload( img );
	}
	m_particleShader->setUniform( "tex", m_particleTex->getUniform() );

	m_particlePositionsTex = base::Texture2d::createRGBAFloat32( m_particleDataRes, m_particleDataRes );
	m_particleShader->setUniform( "pos", m_particlePositionsTex->getUniform() );


	m_billboards = BillboardsPtr( new Billboards() );
	m_billboardShader = base::Shader::load( base::Path( SRC_PATH ) + "src/billboard.vs.glsl", base::Path( SRC_PATH ) + "src/billboard.ps.glsl" );
	{
		base::ImagePtr img = base::Image::load( base::Path( SRC_PATH ) + "data/billboard_nebulae1.jpg" );
		m_billboardTex = base::Texture2d::createRGBA8();
		m_billboardTex->upload( img );
	}
	m_billboardShader->setUniform( "tex", m_billboardTex->getUniform() );


	m_particles = base::Geometry::createPointGeometry();
	m_particlePositions = (float *)malloc( m_particleDataRes*m_particleDataRes*sizeof(float)*4 );
}






void Nebulae::generate()
{
	m_attractor.reset();
	m_particles->clear();
	base::AttributePtr positions = m_particles->getAttr("P");



	math::PerlinNoise pn;
	pn.setFrequency( .5f );
	pn.setDepth(3);

	float voxelSize = .005f;


	// go through the equations many times, drawing a point for each iteration
	int skipped = 0;

	Grid grid;


	while( grid.size() < m_maxNumParticles )
	{
		math::Vec3f p = m_attractor.next();

		/*
		// apply perlin noise
		float t1 = pn.perlinNoise_3D( p.x, p.y, p.z )*14.0f;
		float t2 = pn.perlinNoise_3D( p.x+100.0f, p.y+100.0f, p.z+100.0f )*14.0f;
		float t3 = pn.perlinNoise_3D( p.x+200.0f, p.y+200.0f, p.z+200.0f )*14.0f;
		p += math::Vec3f(t1,t2,t3);
		*/


		V3i key;
		key.i = (int)std::floor(p.x / voxelSize);
		key.j = (int)std::floor(p.y / voxelSize);
		key.k = (int)std::floor(p.z / voxelSize);
		// if particle falls into already existing bucket
		if( grid.find( key ) != grid.end() )
		{
			++skipped;
			if(skipped>m_maxNumParticles)
				break;
			// drop it
			continue;
		}

		// else: create bucket and put in the particleA
		grid[key] = p;
	}

	std::cout << "skipped " << skipped << " particles during generation...\n";
	std::cout << "number of particles " << grid.size() << std::endl;



	/*
	// TODO: randomly remove buckets and spawn bigger billboard particles
	float bratio = .0001f;// percent of particles which will be turned into billboards
	int numBillboards =  (int)(bratio * (float)grid.size());
	std::cout << "number of billboards " << numBillboards << std::endl;
	std::cerr << "adding billboards\n";
	for( int i=0; i<numBillboards; ++i )
	{
		// randomly select a particle
		int index = (int)(math::g_randomNumber()*grid.size());
		Grid::iterator it = grid.begin();
		std::advance( it, index );
		// create billboard
		billboards->add( it->second );
		// remove particle
		grid.erase( it );
	}
	std::cerr << "done\n";
	*/


	// the grid now contains all particles which we want to render
	// we transfer the particle positions into pos array and create the points geometry
	int count = 0;
	int i = 0;
	int j = 0;

	for( Grid::iterator it = grid.begin(); it != grid.end(); ++it, ++count )
	{
		const V3i &key = it->first;
		math::Vec3f &value = it->second;

		if( count < m_maxNumParticles )
		{
			// apply perlin noise
			math::Vec3f p = value;

			float t1 = pn.perlinNoise_3D( p.x, p.y, p.z )*14.0f;
			float t2 = pn.perlinNoise_3D( p.x+100.0f, p.y+100.0f, p.z+100.0f )*14.0f;
			float t3 = pn.perlinNoise_3D( p.x+200.0f, p.y+200.0f, p.z+200.0f )*14.0f;
			p += math::Vec3f(t1,t2,t3);


			m_particlePositions[count*4 + 0] = p.x;
			m_particlePositions[count*4 + 1] = p.y;
			m_particlePositions[count*4 + 2] = p.z;
			m_particlePositions[count*4 + 3] = 1.0f;


			float u = ((float)i+0.5f)/(float)(m_particleDataRes);
			float v = ((float)j+0.5f)/(float)(m_particleDataRes);

			m_particles->addPoint(positions->appendElement(math::Vec3f(u,v,0.0f)));
		}


		if( ++i >= m_particleDataRes )
		{
			i = 0;
			j +=1;
		}
	}

	// TODO: randomly remove buckets and spawn bigger billboard particles
	m_billboards->geo->clear();
	float bratio = .0001f;// percent of particles which will be turned into billboards
	int numBillboards =  (int)(bratio * (float)grid.size());
	std::cout << "number of billboards " << numBillboards << std::endl;
	std::cerr << "adding billboards\n";
	for( int i=0; i<numBillboards; ++i )
	{
		// randomly select a particle
		int index = (int)(math::g_randomNumber()*grid.size());
		math::Vec3f p( m_particlePositions[index*4 + 0], m_particlePositions[index*4 + 1], m_particlePositions[index*4 + 2] );
		// create billboard
		m_billboards->add( p );
	}
	std::cerr << "done\n";


	// upload initial particle positions
	m_particlePositionsTex->uploadRGBAFloat32( m_particleDataRes, m_particleDataRes, m_particlePositions );
}



