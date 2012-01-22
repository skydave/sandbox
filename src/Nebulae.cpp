
#include "Nebulae.h"

#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <util/unordered_map.h>

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
#include <gfx/glsl/noise.h>



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
	m_billboardShader = base::Shader::load( base::Path( SRC_PATH ) + "src/Nebulae.billboardAtmo.vs.glsl", base::Path( SRC_PATH ) + "src/Nebulae.billboardAtmo.ps.glsl" );
	{
		base::ImagePtr img = base::Image::load( base::Path( SRC_PATH ) + "data/billboard_nebulae1.jpg" );
		m_billboardTex = base::Texture2d::createRGBA8();
		m_billboardTex->upload( img );
	}
	m_billboardShader->setUniform( "tex", m_billboardTex->getUniform() );


	m_particles = base::Geometry::createPointGeometry();
	m_particlePositions = (float *)malloc( m_particleDataRes*m_particleDataRes*sizeof(float)*4 );




	// perlin noise
	m_perlinNoiseFBO = base::FBOPtr( new base::FBO( m_particleDataRes, m_particleDataRes) );
	m_perlinNoiseFBOOutput = base::Texture2d::createRGBAFloat32( m_particleDataRes, m_particleDataRes );
	m_perlinNoiseFBO->setOutputs( m_perlinNoiseFBOOutput );

	m_perlinNoiseShader = base::Shader::load( base::Path( SRC_PATH ) + "src/Nebulae.perlinnoise.vs.glsl", base::Path( SRC_PATH ) + "src/Nebulae.perlinnoise.ps.glsl" ).attachPS( base::glsl::noiseSrc() );
	m_perlinNoiseShader->setUniform( "inputPositions", m_particlePositionsTex->getUniform() );

	m_particleShader->setUniform( "pos", m_perlinNoiseFBOOutput->getUniform() );
	m_billboardShader->setUniform( "pos", m_perlinNoiseFBOOutput->getUniform() );


	// color
	m_colorFBO = base::FBOPtr( new base::FBO( m_particleDataRes, m_particleDataRes) );
	//m_colorFBOOutput = base::Texture2d::createRGBA8( m_particleDataRes, m_particleDataRes );
	m_colorFBOOutput = base::Texture2d::createRGBAFloat32( m_particleDataRes, m_particleDataRes );
	m_colorFBO->setOutputs( m_colorFBOOutput );

	m_colorShader = base::Shader::load( base::Path( SRC_PATH ) + "src/Nebulae.color.vs.glsl", base::Path( SRC_PATH ) + "src/Nebulae.color.ps.glsl" );
	m_colorShader->setUniform( "inputPositions", m_perlinNoiseFBOOutput->getUniform() );

	m_particleShader->setUniform( "col", m_colorFBOOutput->getUniform() );
	m_billboardShader->setUniform( "col", m_colorFBOOutput->getUniform() );

	// connect lighting info
	{
		Light l;
		l.pos = math::Vec3f(-0.1f, 0.05f, 0.0f);
		l.col = math::Vec3f(.9f, 0.7f, 0.2f);
		l.rad = 0.2f;
		m_lights.push_back(l);
	}
	{
		Light l;
		l.pos = math::Vec3f(0.1f, -0.01f, 0.0f);
		l.col = math::Vec3f(.1f, 0.5f, 0.9f);
		l.rad = 0.2f;
		m_lights.push_back(l);
	}
	m_colorShader->setUniform( "light0Pos", m_lights[0].pos );
	m_colorShader->setUniform( "light0Col", m_lights[0].col );
	m_colorShader->setUniform( "light0Radius", m_lights[0].rad );
	m_colorShader->setUniform( "light1Pos", m_lights[1].pos );
	m_colorShader->setUniform( "light1Col", m_lights[1].col );
	m_colorShader->setUniform( "light1Radius", m_lights[1].rad );


	// star flare billboards ======================================
	m_billboardsFlares = BillboardsPtr( new Billboards() );
	m_billboardFlareShader = base::Shader::load( base::Path( SRC_PATH ) + "src/Nebulae.billboardFlare.vs.glsl", base::Path( SRC_PATH ) + "src/Nebulae.billboardFlare.ps.glsl" );
	{
		base::ImagePtr img = base::Image::load( base::Path( SRC_PATH ) + "data/flare2.png" );
		m_flareTex = base::Texture2d::createRGBA8();
		m_flareTex->upload( img );
		m_billboardFlareShader->setUniform( "tex", m_flareTex->getUniform() );
		m_billboardFlareShader->setUniform( "scale", 0.05f );
		m_billboardFlareShader->setUniform( "alpha", 1.0f );
	}


	// glow -------
	m_billboardsGlow = BillboardsPtr( new Billboards() );
	m_billboardGlowShader = base::Shader::load( base::Path( SRC_PATH ) + "src/Nebulae.billboardFlare.vs.glsl", base::Path( SRC_PATH ) + "src/Nebulae.billboardFlare.ps.glsl" );
	{
		base::ImagePtr img = base::Image::load( base::Path( SRC_PATH ) + "data/glow1.png" );
		m_glowTex = base::Texture2d::createRGBA8();
		m_glowTex->upload( img );
		m_billboardGlowShader->setUniform( "tex", m_glowTex->getUniform() );
		m_billboardGlowShader->setUniform( "scale", 0.1f );
		m_billboardGlowShader->setUniform( "alpha", 0.5f );
	}

	for( std::vector<Light>::iterator it = m_lights.begin(); it != m_lights.end(); ++it )
	{
		Light &l = *it;
		m_billboardsFlares->add( l.pos );
		m_billboardsGlow->add( l.pos );
	}


	// bok globule -------
	m_billboardsBokGlobules = BillboardsPtr( new Billboards() );
	m_billboardBokGlobuleShader = base::Shader::load( base::Path( SRC_PATH ) + "src/Nebulae.billboardBokGlobule.vs.glsl", base::Path( SRC_PATH ) + "src/Nebulae.billboardBokGlobule.ps.glsl" );
	{
		base::ImagePtr img = base::Image::load( base::Path( SRC_PATH ) + "data/cumulus01.png" );
		m_cloudsTex = base::Texture2d::createRGBA8();
		m_cloudsTex->upload( img );
		m_billboardBokGlobuleShader->setUniform( "tex", m_cloudsTex->getUniform() );
		//m_billboardBokGlobuleShader->setUniform( "scale", 0.01f );
		m_billboardBokGlobuleShader->setUniform( "alpha", 1.0f );
	}

	for( int i=0;i<10;++i )
	{
		float scale = math::g_randomNumber()*0.05;
		int index = (int)(math::g_randomNumber()*16.0);
		math::Vec3f p;
		p.x = math::g_randomNumber()*0.19f - 0.08f;
		p.y = math::g_randomNumber()*0.19f - 0.08f;
		p.z = math::g_randomNumber()*0.19f - 0.08f;
		m_billboardsBokGlobules->add( p, index, scale );
	}
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
			math::Vec3f p = value;

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

	/*
	// run perlin noise as postprocess
	for( int i=0;i<count;++i )
	{
		math::Vec3f p;
		p.x = m_particlePositions[i*4 + 0];
		p.y = m_particlePositions[i*4 + 1];
		p.z = m_particlePositions[i*4 + 2];

		// pn
		float t1 = pn.perlinNoise_3D( p.x, p.y, p.z )*14.0f;
		float t2 = pn.perlinNoise_3D( p.x+100.0f, p.y+100.0f, p.z+100.0f )*14.0f;
		float t3 = pn.perlinNoise_3D( p.x+200.0f, p.y+200.0f, p.z+200.0f )*14.0f;
		p += math::Vec3f(t1,t2,t3);

		m_particlePositions[i*4 + 0] = p.x;
		m_particlePositions[i*4 + 1] = p.y;
		m_particlePositions[i*4 + 2] = p.z;
	}
	*/

	// upload initial particle positions
	m_particlePositionsTex->uploadRGBAFloat32( m_particleDataRes, m_particleDataRes, m_particlePositions );


	//
	applyPerlinNoise();

	//
	applyColor();



	generateBillboards();
}

void Nebulae::generateBillboards()
{
	m_billboards->geo->clear();
	float bratio = .0001f;// percent of particles which will be turned into billboards
	int numBillboards =  (int)(bratio * (float)m_particles->numPrimitives());
	std::cout << "number of billboards " << numBillboards << std::endl;
	std::cerr << "adding billboards\n";
	for( int i=0; i<numBillboards; ++i )
	{
		// randomly select a particle
		int index = (int)(math::g_randomNumber()*m_particles->numPrimitives());
		//math::Vec3f p( m_particlePositions[index*4 + 0], m_particlePositions[index*4 + 1], m_particlePositions[index*4 + 2] );
		math::Vec3f p = m_particles->getAttr("P")->get<math::Vec3f>(index);
		// create billboard
		m_billboards->add( p );
	}
	std::cerr << "done\n";
}

void Nebulae::applyPerlinNoise()
{
	m_perlinNoiseFBO->begin();
	base::Context::current()->renderScreen( m_perlinNoiseShader );
	m_perlinNoiseFBO->end();

	applyColor();
}

void Nebulae::applyColor()
{
	m_colorFBO->begin();
	base::Context::current()->renderScreen( m_colorShader );
	m_colorFBO->end();
}



