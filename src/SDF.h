#pragma once
#include <math/Math.h>
#include <vector>

#include <util/shared_ptr.h>
#include <gfx/Texture.h>






// DOMAIN ===================================================
struct Domain
{
	Domain();
	Domain( math::Vec3f min, math::Vec3f max, int _width, int _height, int _depth );

	math::BoundingBox3d bound;
	int  width, height, depth;
};



// SDF ============================================================
BASE_DECL_SMARTPTR_STRUCT(ScalarField);
struct ScalarField
{

	std::vector<float>        m_grid;
	int       m_xres, m_yres, m_zres; // grid resolution in all dimensions
	math::Matrix44f   m_worldToLocal; // defines how the sdf is transformed in space

	ScalarField()
	{
		// define the region which is covered by the sdf grid (this allows to move the sdf independent from where it was sampled)
		//float uniformScale = 4.0f;
		float uniformScale = 1.0f;
		math::Matrix44f localToWorld = math::Matrix44f::Identity();

		//localToWorld.scale( uniformScale );
		//localToWorld.translate( -math::Vec3f(2.0f, 2.0f, 0.0f) ); // TODO: 3d
		localToWorld.translate( -math::Vec3f(.5f, 0.0f, 0.5f) ); // TODO: 3d

		m_worldToLocal = localToWorld.inverted();


		// this requires rescaling of the sdf so that distances are in worldspace units
		for( std::vector<float>::iterator it = m_grid.begin(); it != m_grid.end(); ++it )
		{
			float &sd = *it;
			sd = sd*uniformScale;
		}

	}

	static ScalarFieldPtr                             create();

	// returns sdf from voxelcoords
	float value( int x, int y, int z )
	{
		int _x = x;
		int _y = y;
		int _z = z;

		if( x < 0 )
			_x = 0;
		if( x >= m_xres )
			_x = m_xres - 1;
		if( y < 0 )
			_y = 0;
		if( y >= m_yres )
			_y = m_yres - 1;
		if( z < 0 )
			_z = 0;
		if( z >= m_zres )
			_z = m_zres - 1;

		return m_grid[ _z*m_xres*m_yres + _y*m_zres + _x ];
	}

	// evaluates sdf using baked grid; p is in worldspace
	float value( const math::Vec3f &p )
	{
		math::Vec3f vs;

		// convert from world to localspace
		math::Vec3f _p;
		_p = math::transform( p, m_worldToLocal );

		// convert from local to voxelspace
		vs.x = _p.x * m_xres;
		vs.y = _p.y * m_yres;
		vs.z = _p.z * m_zres;

		float tx = vs.x - floor(vs.x);
		float ty = vs.y - floor(vs.y);
		float tz = vs.z - floor(vs.z);
		int x = (int)floor(vs.x);
		int y = (int)floor(vs.y);
		int z = (int)floor(vs.z);

		float a, b, c, d, e, f, g, h;
		a = value( x, y, z );
		b = value( x+1, y, z );
		c = value( x, y+1, z );
		d = value( x+1, y+1, z );
		e = value( x, y, z+1 );
		f = value( x+1, y, z+1 );
		g = value( x, y+1, z+1 );
		h = value( x+1, y+1, z+1 );

		float sdf0 = math::lerp( math::lerp( a, b, tx ), math::lerp( c, d, tx ), ty );
		float sdf1 = math::lerp( math::lerp( e, f, tx ), math::lerp( g, h, tx ), ty );

		return math::lerp( sdf0, sdf1, tz );
	}

	math::Vec3f gradient( const math::Vec3f &p )
	{
		float d = 1.0f / m_xres;

		float s011 = value( p+math::Vec3f(-d, 0.0f, 0.0f) );
		float s211 = value( p+math::Vec3f(d, 0.0f, 0.0f) );
		float s101 = value( p+math::Vec3f(0.0f, -d, 0.0f) );
		float s121 = value( p+math::Vec3f(0.0f, d, 0.0f) );
		float s110 = value( p+math::Vec3f(0.0f, 0.0, -d) );
		float s112 = value( p+math::Vec3f(0.0f, 0.0, d) );

		// 2d // TODO: 3d
		math::Vec3f g;
		g.x = (s211 - s011)/(2.0f*d);
		g.y = (s121 - s101)/(2.0f*d);
		g.z = 0.0f;

		return g.normalized();
	}


	// samples slice along z=0 plane in localspace
	base::Texture2dPtr createSlice( const math::Vec2f &min, const math::Vec2f &max, float z, int width, int height )
	{
		math::Vec2f size = max - min;
		float *data = (float*)malloc( width*height*sizeof(float) );
		for( int j=0;j<height;++j )
			for( int i=0;i<width;++i )
			{
				float u=(float)i/(float)width;
				float v=(float)j/(float)height;

				// convert to worldspace
				math::Vec3f p;
				p.x = u*size.x + min.x; 
				p.y = v*size.y + min.y;
				p.z = z; // TODO: 3d

				float sdf = value(p); // worldspace

				//data[ (height-j-1)*width + i ] = fabs(sdf);
				data[ j*width + i ] = fabs(sdf);
			}


		base::Texture2dPtr tex = base::Texture2d::createFloat32(width, height);
		tex->uploadFloat32( width, height, data );
		free(data);
		return tex;
	}

};


// distanceTransform ===================================================

template<typename T>
ScalarFieldPtr distanceTransform( const Domain &domain, T test )
{
	ScalarFieldPtr sdfGrid = ScalarField::create();
	math::Vec3f size = domain.bound.size();

	sdfGrid->m_xres = domain.width;
	sdfGrid->m_yres = domain.height;
	sdfGrid->m_zres = domain.depth;

	sdfGrid->m_grid.clear();
	sdfGrid->m_grid.resize( domain.width*domain.height*domain.depth );
	for( int k=0;k<domain.depth;++k )
		for( int j=0;j<domain.height;++j )
			for( int i=0;i<domain.width;++i )
			{
				float u = (float)i/(float)domain.width;
				float v = (float)j/(float)domain.height;
				float w = (float)k/(float)domain.depth;

				// transform from grid localspace to sdf localspace
				math::Vec3f p;
				p.x = u*size.x + domain.bound.minPoint.x;
				p.y = v*size.y + domain.bound.minPoint.y;
				p.z = w*size.z + domain.bound.minPoint.z;

				sdfGrid->m_grid[ k*domain.width*domain.height + j*domain.width + i ] = test( p );
			}

	return sdfGrid;
}
