#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#include <gfx/Geometry.h>
#include <gfx/ObjIO.h>
#include <gfx/Shader.h>
#include <gfx/Texture.h>
#include <gfx/Image.h>
#include <gfx/Context.h>
#include <gfx/FCurve.h>
#include <gfx/glsl/common.h>
#include <gfx/FBO.h>


struct SortHelper
{
	float dist2;
	math::Vec3f center;
	int indices[4];

	// for sorting
	bool operator < (const SortHelper &s) const
	{
		// back to front drawing -> the closer the sprite, the later it is drawed
		return this->dist2 > s.dist2;
	}
};

struct BillboardType
{
	BillboardType() : probability(0.0f), probabilityNormalized(0.0f), scale(1.0f)
	{
	}
	float probability;
	float probabilityNormalized;
	float scale;
	int typeId;
};

BASE_DECL_SMARTPTR_STRUCT(ParticleCloud);
struct ParticleCloud
{

	ParticleCloud() : m_numParticlesScale(1.0f), m_globalAlpha(1.0f), m_globalScale(0.1f)
	{
		// setup billboard types
		m_billboardTypes.resize(16);
		int ti = 0;
		for( std::vector<BillboardType>::iterator it = m_billboardTypes.begin(); it != m_billboardTypes.end(); ++it, ++ti )
			(*it).typeId = ti;

		// create sprite geo
		m_spriteGeo = base::GeometryPtr(new base::Geometry(base::Geometry::QUAD));

		base::AttributePtr spriteGeo_positions = base::Attribute::createVec3f();
		base::AttributePtr spriteGeo_color = base::Attribute::createVec3f();
		base::AttributePtr spriteGeo_uvs = base::Attribute::createVec2f();

		m_spriteGeo->setAttr( "P", spriteGeo_positions);
		m_spriteGeo->setAttr( "UV", spriteGeo_uvs );
		m_spriteGeo->setAttr( "Cd", spriteGeo_color );


		// initialize clousSpriteShader
		m_cloudSpriteShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/ParticleCloud.sprite.vs.glsl", base::Path( SRC_PATH ) + "/src/ParticleCloud.sprite.ps.glsl" );
		//m_cloudSpriteTexture = base::Texture2d::load( base::Path( SRC_PATH ) + "/data/puff_14.jpg" );
		m_cloudSpriteTexture = base::Texture2d::load( base::Path( SRC_PATH ) + "/data/cumulus01.tga" );
		m_cloudSpriteShader->setUniform( "diffuseMap", m_cloudSpriteTexture->getUniform() );

		// set parameters
		setGlobalAlpha(1.0f);
	}


	void buildGeometry();
	void setParticleNumScale( float numParticlesScale )
	{
		m_numParticlesScale = std::max( 0.0f, std::min(1.0f, numParticlesScale));
	}

	void setGlobalAlpha( float globalAlpha )
	{
		m_globalAlpha = globalAlpha;
		m_cloudSpriteShader->setUniform( "globalAlpha", m_globalAlpha );
	}

	void setGlobalScale( float globalScale )
	{
		m_globalScale = globalScale;
		buildGeometry();
	}

	void addParticle( const math::Vec3f &position, const math::Vec3f &normal, const math::Vec3f &tangentu, int textureId = 0 )
	{
		m_positions.push_back( position );
		m_normals.push_back( normal );
		m_tangentu.push_back( tangentu );
		m_textureIds.push_back(textureId);
	}

	void setupBillboardTypeProbability( int typeId, float probability )
	{
		m_billboardTypes[typeId].probability = probability;

		// normalize probabilities
		float sum = 0.0f;
		for( std::vector<BillboardType>::iterator it = m_billboardTypes.begin(); it != m_billboardTypes.end(); ++it )
			sum += (*it).probability;
		if( sum > 0.0f )
		{
			int c = 0;
			for( std::vector<BillboardType>::iterator it = m_billboardTypes.begin(); it != m_billboardTypes.end(); ++it )
			{
				BillboardType &bt = *it;
				bt.probabilityNormalized = bt.probability/sum;

				int numPoints = (int)floor(bt.probabilityNormalized*m_positions.size());
				for( int i=0;i<numPoints;++i,++c )
					m_textureIds[c] = bt.typeId;
			}

			buildGeometry();
		}

	}

	void setupBillboardTypeScale( int typeId, float scale )
	{
		m_billboardTypes[typeId].scale = scale;
		buildGeometry();
	}

	void sort( base::CameraPtr cam );

	// members
	float m_numParticlesScale;
	float m_globalAlpha;
	float m_globalScale;
	base::GeometryPtr m_spriteGeo;
	base::ShaderPtr m_cloudSpriteShader;
	base::Texture2dPtr m_cloudSpriteTexture;
	std::vector<math::Vec3f> m_positions;
	std::vector<math::Vec3f> m_normals;
	std::vector<math::Vec3f> m_tangentu;
	std::vector<int> m_textureIds;
	std::vector<SortHelper> m_sortHelpers;
	std::vector<BillboardType> m_billboardTypes;

};