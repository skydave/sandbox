
#include "Nebulae.h"

#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <util/unordered_map.h>

// required for prt export
#include <memory.h>
#include <zlib/zlib.h>
#include <cstdio>
#include <util/types.h>
#include <assert.h>
#include "Half\half.h"



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

	m_particleShader = base::Shader::load( base::Path( SRC_PATH ) + "src/Nebulae.particles.vs.glsl", base::Path( SRC_PATH ) + "src/Nebulae.particles.ps.glsl" );
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

	// here we point the particleShader to the output of the perlinnoise fbo so that it picks up the deformed positions
	m_particleShader->setUniform( "pos", m_perlinNoiseFBOOutput->getUniform() );
	// same for billboards
	m_billboardShader->setUniform( "pos", m_perlinNoiseFBOOutput->getUniform() );


	// color
	m_colorFBO = base::FBOPtr( new base::FBO( m_particleDataRes, m_particleDataRes) );
	m_colorFBOOutput = base::Texture2d::createRGBAFloat32( m_particleDataRes, m_particleDataRes );
	m_colorFBO->setOutputs( m_colorFBOOutput );

	m_colorShader = base::Shader::load( base::Path( SRC_PATH ) + "src/Nebulae.color.vs.glsl", base::Path( SRC_PATH ) + "src/Nebulae.color.ps.glsl" );
	m_colorShader->setUniform( "inputPositions", m_perlinNoiseFBOOutput->getUniform() );

	// here we point the particleShader to the output of the color fbo so that it picks up the colored particles
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
		m_billboardBokGlobuleShader->setUniform( "alpha", 1.0f );
	}

	for( int i=0;i<10;++i )
	{
		float scale = math::g_randomNumber()*0.05f;
		int index = (int)(math::g_randomNumber()*16.0f);
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

	// upload initial particle positions
	m_particlePositionsTex->uploadRGBAFloat32( m_particleDataRes, m_particleDataRes, m_particlePositions );


	//
	applyPerlinNoise();

	generateBillboards();

	// setup defaults
	setParticleScale( 0.26f );
	setParticleAlpha( 0.09f );
	setBillboardScale( 0.2f );
	setBillboardAlpha( 0.026f );
	setFrequency( 1.0f );
	setOctaves( 8 );
	setLacunarity( 1.82f );
	setGain( 0.482f );
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

void Nebulae::setGenerator_kingsdream( float a, float b, float c, float d )
{
	m_attractor.a = a;
	m_attractor.b = b;
	m_attractor.c = c;
	m_attractor.d = d;
	generate();
}

void Nebulae::setFrequency( float frequency )
{
	m_frequency = frequency;
	m_perlinNoiseShader->setUniform( "frequency", m_frequency );
	applyPerlinNoise();
}

void Nebulae::setParticleScale( float scale )
{
	m_particleScale = scale;
	m_particleShader->setUniform( "scale", scale );
}


void Nebulae::setParticleAlpha( float alpha )
{
	m_particleAlpha = alpha;
	m_particleShader->setUniform( "alpha", alpha );
}

void Nebulae::setBillboardScale( float scale )
{
	m_billboardScale = scale;
	m_billboardShader->setUniform( "scale", scale );
}

void Nebulae::setBillboardAlpha( float alpha )
{
	m_billboardAlpha = alpha;
	m_billboardShader->setUniform( "alpha", alpha );
}


void Nebulae::setOctaves( int octaves )
{
	m_octaves = octaves;
	m_perlinNoiseShader->setUniform( "octaves", octaves );
	applyPerlinNoise();
}

void Nebulae::setLacunarity( float lacunarity )
{
	m_lacunarity = lacunarity;
	m_perlinNoiseShader->setUniform( "lacunarity", lacunarity );
	applyPerlinNoise();
}

void Nebulae::setGain( float gain )
{
	m_gain = gain;
	m_perlinNoiseShader->setUniform( "gain", gain );
	applyPerlinNoise();
}


float Nebulae::getParticleScale()
{
	return m_particleScale;
}

float Nebulae::getParticleAlpha()
{
	return m_particleAlpha;
}

float Nebulae::getBillboardScale()
{
	return m_billboardScale;
}

float Nebulae::getBillboardAlpha()
{
	return m_billboardAlpha;
}

float Nebulae::getFrequency()
{
	return m_frequency;
}

int Nebulae::getOctaves()
{
	return m_octaves;
}

float Nebulae::getLacunarity()
{
	return m_lacunarity;
}

float Nebulae::getGain()
{
	return m_gain;
}


// PRT stuff =================================================================
struct Header
{
    char MagicNumber[8];
    int Len;
    char Signature[32];
    int Version;
    sint64 Count;
};

struct Channel
{
    char Name[32];
    int DataType;
    int Arity;
    int Offset;
};


enum DataType
{
	kUNKNOWN,
	kINT32 = 1,
	kINT64 = 2,
	kFLOAT16 = 3,
	kFLOAT32 = 4,
	kFLOAT64 = 5,
	kUINT32 = 7,
	kUINT64 = 8
};





void Nebulae::writePRT( const std::string filename, std::map< std::string, base::AttributePtr > attributes, int numParticles )
{
	sint64 count = numParticles;
	int numChannels = 0;

	//Create file to write
	std::FILE *file = std::fopen(filename.c_str(), "wb+");

	//Initialize the header
	Header H;
	memset(&H,0,sizeof(Header));
	H.MagicNumber[0] = (char)192;
	H.MagicNumber[1] = 'P';
	H.MagicNumber[2] = 'R';
	H.MagicNumber[3] = 'T';
	H.MagicNumber[4] = '\r';
	H.MagicNumber[5] = '\n';
	H.MagicNumber[6] = 26;
	H.MagicNumber[7] = '\n';
	H.Len = 56;
	sprintf(H.Signature,"Extensible Particle Format");
	H.Version = 1;
	H.Count = count;

	if(std::fwrite(&H, sizeof(Header), 1, file) != 1)
		throw std::runtime_error("file write failure");

	int RESERVED = 4;
	if(std::fwrite(&RESERVED, sizeof(int), 1, file) != 1)
		throw std::runtime_error("file write failure");

	if(0 != std::fflush(file))
		throw std::runtime_error("file flush failure");


	// prepare channels =========================

	// evaluate size of memory
	size_t sizeAllParticles = 0;
	size_t sizePerParticle = 0; // the size in bytes of all channels for a single data record
	for( std::map<std::string, base::AttributePtr>::iterator it = attributes.begin(); it != attributes.end(); ++it)
	{
		base::AttributePtr a = it->second;
		sizePerParticle += a->m_componentSize * a->m_numComponents;
		++numChannels;
	}

	sizeAllParticles = sizePerParticle*H.Count;


	// iterate through the channels, copying the data for each to the compression buffer
	char *cacheParticle = (char*)malloc( sizePerParticle );
	//char *cacheUncompressed = (char*)malloc( sizeAllParticles );

	/*
	// for each particle
	for( sint64 i=0;i<numParticles;++i )
	{
		size_t offset = 0;

		// for each channel
		for( std::map<std::string, base::AttributePtr>::iterator it = attributes.begin(); it != attributes.end(); ++it)
		{
			std::string n = it->first;
			base::AttributePtr a = it->second;

			size_t len = a->m_componentSize * a->m_numComponents;

			memcpy( cacheParticle + offset, a->getRawPointer((int)i), len );
			offset += len;
		}

		memcpy( cacheUncompressed + i*sizePerParticle, cacheParticle, sizePerParticle );
	}
	*/


	/*
	// compress the data
	uLongf ZSrcBufLen = (uLongf)sizeAllParticles;
	Bytef* ZSrcBuf = (Bytef*)cacheUncompressed;

	uLongf ZDstBufLen = compressBound(ZSrcBufLen);
	Bytef* ZDstBuf = (Bytef*)malloc(ZDstBufLen);

	int ZR = compress2(ZDstBuf,&ZDstBufLen,ZSrcBuf,ZSrcBufLen, -1);
	if( ZR != Z_OK )
		throw std::runtime_error("compress2 failure");
	*/






























	// write channel metadata ==========================
	if(std::fwrite(&numChannels, sizeof(int), 1, file) != 1)
		throw std::runtime_error("file write failure");
	int ChanLen = 44;
	if(std::fwrite(&ChanLen, sizeof(int), 1, file) != 1)
		throw std::runtime_error("file write failure");

	if(0 != std::fflush(file))
		throw std::runtime_error("file flush failure");

	// per channel metadata...
	int offset = 0;
	for( std::map<std::string, base::AttributePtr>::iterator it = attributes.begin(); it != attributes.end(); ++it)
	{
		std::string n = it->first;
		base::AttributePtr a = it->second;

		// adjust some default names (e.g. we call position P but krakatoa calls it Position)
		if( n == "P" )
			n = "Position";

		Channel ch;
		memset(&ch,0,sizeof(Channel));

		switch( a->elementComponentType() )
		{
		case base::Attribute::INT:
			ch.DataType = (int)kINT32;break;
		case base::Attribute::FLOAT:
			ch.DataType = (int)kFLOAT32;break;
		default:
			break;
		};
		ch.Arity = a->m_numComponents;

		strncpy(ch.Name, n.c_str(), 31);
		ch.Offset = offset;

		if(std::fwrite(&ch, sizeof(Channel), 1, file) != 1)
			throw std::runtime_error("file write failure");

		offset += a->m_componentSize * a->m_numComponents;
	}

	// write the compressed data
	//if(std::fwrite(&ZDstBuf, ZDstBufLen, 1, file) != 1)
	//	throw std::runtime_error("file write failure");

	{		
		const ULONG CHUNKSIZE = 10000;

        //hardcoding PARTICLESIZE for now
        //const int PARTICLESIZE = 12;
        int level = -1; //-1 compression level is zlib default
        int ret, flush;
        unsigned have;
        //unsigned char in[PARTICLESIZE];
        //unsigned char out[PARTICLESIZE];
		unsigned char *out = (unsigned char *)malloc( sizePerParticle );
		unsigned char *in = (unsigned char *)cacheParticle;

        z_stream strm;
        //allocate deflate state
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        ret = deflateInit(&strm, level);
        if (ret != Z_OK)
			throw std::runtime_error("compress2 failure");

        // read the data in chunks of 1000 elements
        for( ULONG i=0; i < count; i += CHUNKSIZE )
		{         
            ULONG upperLimit = count - i < CHUNKSIZE ? count - i : CHUNKSIZE;

            for( ULONG j=0; j < upperLimit; j++ )
			{                   
				//memcpy((void *)in, (const void *)cacheUncompressed, PARTICLESIZE);

				size_t offset = 0;
				// for each channel
				for( std::map<std::string, base::AttributePtr>::iterator it = attributes.begin(); it != attributes.end(); ++it)
				{
					std::string n = it->first;
					base::AttributePtr a = it->second;

					size_t len = a->m_componentSize * a->m_numComponents;

					memcpy( cacheParticle + offset, a->getRawPointer((int)i+j), len );
					offset += len;
				}

				strm.avail_in = sizePerParticle;
                                                
                //to figure out what the state of deflate() should be this iteration
                flush = i + j == count - 1 ? Z_FINISH : Z_NO_FLUSH;
                strm.next_in = (Byte*)in;
                                                
                //operate until avail_out is actually empty, it seems deflate never gets all of avail_out
                //even in our case where we are working with a really small amount of data each deflate() request
                do
				{
                    strm.avail_out = sizePerParticle; //avail_out must be at least 0.1% larger than avail_in plus 12 bytes, according to zlib docs
                    strm.next_out = out;
                    ret = deflate(&strm, flush);
                    assert(ret != Z_STREAM_ERROR);
                    have = sizePerParticle - strm.avail_out;

                    //if(fwrite(out, 1, have, file) != have || ferror(file))
					if(std::fwrite(out, 1, have, file) != have)
					{
						(void)deflateEnd(&strm);
						throw std::runtime_error("compress2 failure");
                    }
                }while(strm.avail_out == 0); //make sure avail_out is actually empty
                assert(strm.avail_in == 0);            
            }
		}
		assert(ret == Z_STREAM_END); //make sure zlib returned Z_STREAM_END
                                
		// clean up and return
		(void)deflateEnd(&strm);

		free(out);
	}




	//free( ZDstBuf );
	//free( cacheUncompressed );
	free( cacheParticle );


	// done
	std::fclose(file);
}

void Nebulae::writeParticles( const std::string filename )
{
	// TODO: get particles from textures
	base::ImagePtr positions = m_perlinNoiseFBOOutput->download();
	base::ImagePtr colors = m_colorFBOOutput->download();


	int numElements = positions->m_width*positions->m_height;

	std::map<std::string, base::AttributePtr> attrs;
	base::AttributePtr pAttr = base::Attribute::createVec3f(numElements);
	base::AttributePtr cAttr = base::Attribute::createVec3f(numElements);

	
	int index = 0;
	float *p = (float*)positions->m_data;
	float *c = (float*)colors->m_data;
	for( int j=0;j<positions->m_height;++j )
		for( int j=0;j<positions->m_width;++j, ++index )
		{
			pAttr->set( index, p[0], p[1], p[2] );
			cAttr->set( index, c[0], c[1], c[2] );

			p += 4;
			c += 4;
		}


	attrs["Position"] = pAttr;
	attrs["Color"] = cAttr;

	writePRT( filename, attrs, numElements );



	/*
	// TEMP: use geometry
	base::GeometryPtr geo = base::geo_grid(20,20, base::Geometry::POINT);

	// add color
	int numElements = geo->getAttr("P")->numElements();
	base::AttributePtr colorAttr = base::Attribute::createVec3f(numElements);
	base::AttributePtr uvAttr = geo->getAttr("UV");
	for( int i=0;i<numElements;++i )
	{
		math::Vec2f uv = uvAttr->get<math::Vec2f>(i);
		//colorAttr->set( i, uv.x, uv.y, 0.5f );
		colorAttr->set( i, 1.0f, 0.0f, 0.0f );
	}
	geo->setAttr( "Color", colorAttr );

	std::map<std::string, base::AttributePtr> attrs;
	base::AttributePtr attr = base::Attribute::createVec3f(1);
	attr->set(0,0.0f, 0.0f, 0.0f);

	attrs["Position"] = attr;

	writePRT( filename, geo->m_attributes, numElements );
	//writePRT( filename, attrs, 1 );
	*/
}