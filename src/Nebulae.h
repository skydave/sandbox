#pragma once

#include <util/unordered_map.h>
#include <util/shared_ptr.h>
#include <gfx/Shader.h>
#include <gfx/Geometry.h>
#include <gfx/Texture.h>
#include <gfx/FBO.h>



//
// ============= strange attractor ==================
//

struct StrangeAttractor
{
	StrangeAttractor( math::Vec3f _initialP = math::Vec3f(0.1f, 0.1f, 0.1f), int _initialIterations = 100 )
	{
		// coefficients for "The King's Dream"
//		a = -2.643f;
//		b = 1.155f;
//		c = 2.896f;
//		d = 1.986f;

		// other coeffs
//		a = 2.0f;
//		b = 1.8f;
//		c = 0.71f;
//		d = 1.51f;

		// 1, 1.8, 1, 1.7
		a = 1.0f;
		b = 1.8f;
		c = 1.0f;
		d = 1.7f;


		initialP = _initialP;
		initialIterations = _initialIterations;

		reset();
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

	void reset()
	{
		p = initialP;
		// compute some initial iterations to settle into the orbit of the attractor
		for (int i = 0; i <initialIterations; ++i)
			next();
	}



	math::Vec3f              p; // last point which was being generated
	math::Vec3f       initialP; // initial point
	int      initialIterations;
	float           a, b, c, d; // coefficients for the update formula
};


//
// ============= grid ==================
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


//
// =============== billboards ==================
//
BASE_DECL_SMARTPTR_STRUCT(Billboards);
struct Billboards
{
	Billboards()
	{
		geo = base::Geometry::createQuadGeometry();
		pAttr = geo->getAttr( "P" );
		oAttr = base::Attribute::createVec3f();
		geo->setAttr( "offset", oAttr );

		iAttr.reset();
	}

	void add( const math::Vec3f &p )
	{
		int i0 = pAttr->appendElement( p );oAttr->appendElement(-0.5f, -0.5f, 0.0f);
		int i1 = pAttr->appendElement( p );oAttr->appendElement(-0.5f, 0.5f, 0.0f);
		int i2 = pAttr->appendElement( p );oAttr->appendElement(0.5f, 0.5f, 0.0f);
		int i3 = pAttr->appendElement( p );oAttr->appendElement(0.5f, -0.5f, 0.0f);
		geo->addQuad( i3, i2, i1, i0 );
	}

	void add( const math::Vec3f &p, int tileIndex, float scale )
	{
		if(!iAttr)
		{
			iAttr = base::Attribute::createFloat();
			geo->setAttr( "index", iAttr );

			sAttr = base::Attribute::createFloat();
			geo->setAttr( "scale", sAttr );
		}
		int i0 = pAttr->appendElement( p );oAttr->appendElement(-0.5f, -0.5f, 0.0f);iAttr->appendElement((float)tileIndex);sAttr->appendElement(scale);
		int i1 = pAttr->appendElement( p );oAttr->appendElement(-0.5f, 0.5f, 0.0f);iAttr->appendElement((float)tileIndex);sAttr->appendElement(scale);
		int i2 = pAttr->appendElement( p );oAttr->appendElement(0.5f, 0.5f, 0.0f);iAttr->appendElement((float)tileIndex);sAttr->appendElement(scale);
		int i3 = pAttr->appendElement( p );oAttr->appendElement(0.5f, -0.5f, 0.0f);iAttr->appendElement((float)tileIndex);sAttr->appendElement(scale);
		geo->addQuad( i3, i2, i1, i0 );
	}


	base::GeometryPtr geo;
	base::AttributePtr pAttr;
	base::AttributePtr oAttr; // billboard vertex offsets

	base::AttributePtr iAttr; // billboard texture tile indicees
	base::AttributePtr sAttr; // billboard scale
};

// lights =========================
struct Light
{
	math::Vec3f pos;
	math::Vec3f col;
	float       rad;
};


BASE_DECL_SMARTPTR_STRUCT(Nebulae);
struct Nebulae
{
	Nebulae();


	void                                                                                                generate();
	void                                                                                      generateBillboards();
	void                                                                                        applyPerlinNoise();
	void                                                                                              applyColor();

	static NebulaePtr                                                                                     create();

	void                                             setGenerator_kingsdream( float a, float b, float c, float d );
	void                                                                           setParticleScale( float scale );
	void                                                                           setParticleAlpha( float alpha );
	void                                                                          setBillboardScale( float scale );
	void                                                                          setBillboardAlpha( float alpha );
	void                                                                           setFrequency( float frequency );
	void                                                                                 setOctaves( int octaves );
	void                                                                         setLacunarity( float lacunarity );
	void                                                                                     setGain( float gain );

	float                                                                                       getParticleScale();
	float                                                                                       getParticleAlpha();
	float                                                                                      getBillboardScale();
	float                                                                                      getBillboardAlpha();
	float                                                                                           getFrequency();
	int                                                                                               getOctaves();
	float                                                                                          getLacunarity();
	float                                                                                                getGain();

	void                                                              writeParticles( const std::string filename );
	void writePRT(const std::string filename,std::map<std::string,base::AttributePtr> attributes,int numParticles);







	base::GeometryPtr                                                                                  m_particles; // particle positions are uv coords into position texture
	base::ShaderPtr                                                                               m_particleShader; // renders particle (uses particleTex in fragment shader and particlePositionsTex in vertex shader)
	base::Texture2dPtr                                                                               m_particleTex; // particle bilboard (blob) for rendering each particle

	BillboardsPtr                                                                                     m_billboards;
	base::ShaderPtr                                                                              m_billboardShader;
	base::Texture2dPtr                                                                              m_billboardTex;

	int                                                                                          m_particleDataRes;
	int                                                                                          m_maxNumParticles;
	float                                                                                     *m_particlePositions;
	base::Texture2dPtr                                                                      m_particlePositionsTex;

	StrangeAttractor                                                                                   m_attractor;

	base::FBOPtr                                                                                  m_perlinNoiseFBO;
	base::ShaderPtr                                                                            m_perlinNoiseShader;
	base::Texture2dPtr                                                                      m_perlinNoiseFBOOutput;

	base::FBOPtr                                                                                        m_colorFBO;
	base::ShaderPtr                                                                                  m_colorShader;
	base::Texture2dPtr                                                                            m_colorFBOOutput;

	BillboardsPtr                                                                               m_billboardsFlares;
	base::ShaderPtr		                                                                    m_billboardFlareShader;
	base::Texture2dPtr                                                                                  m_flareTex;
	BillboardsPtr                                                                                 m_billboardsGlow;
	base::ShaderPtr		                                                                     m_billboardGlowShader;
	base::Texture2dPtr                                                                                   m_glowTex;

	BillboardsPtr                                                                          m_billboardsBokGlobules;
	base::ShaderPtr		                                                               m_billboardBokGlobuleShader;
	base::Texture2dPtr                                                                                 m_cloudsTex;

	std::vector<Light>                                                                                    m_lights;

	float                                                                                          m_particleScale;
	float                                                                                          m_particleAlpha;
	float                                                                                         m_billboardScale;
	float                                                                                         m_billboardAlpha;

	float                                                                                              m_frequency;
	int                                                                                                  m_octaves;
	float                                                                                             m_lacunarity;
	float                                                                                                   m_gain;

	float                                                                                              m_voxelSize; // drives voxelgrid for pruning
};
