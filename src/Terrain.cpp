//
// very simple utility class for working with heightmaps
//

#include "Terrain.h"
#include <iostream>




Terrain::Terrain( base::Path filename )
{
	base::ImagePtr heightMap = base::Image::load( filename );
	m_width = heightMap->m_width;
	m_height = heightMap->m_height;

	float minHeight, maxHeight;
	minHeight = FLT_MAX;
	maxHeight = -FLT_MAX;

	m_heightMap.resize( m_width*m_height, 0.0f );
	for( int j=0;j<heightMap->m_height;++j )
		for( int i=0;i<heightMap->m_width;++i )
		{
			float h = heightMap->lookup(i, j).r;
			m_heightMap[ j*heightMap->m_width  + i ] = h;
			minHeight = std::min( minHeight, h );
			maxHeight = std::max( maxHeight, h );
		}
	std::cout << minHeight << std::endl;
	std::cout << maxHeight << std::endl;


	m_normalMap.resize( m_width*m_height, 0.0f );
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

			math::Vec3f right = math::Vec3f( 1.0f/(float)m_width, (a21-a01)/2.0f, 0.0f ).normalized();
			math::Vec3f forward = math::Vec3f( 0.0f, (a12-a10)/2.0f, 1.0f/(float)m_height ).normalized();
			math::Vec3f n = math::crossProduct( forward, right ).normalized();
			m_normalMap[ y*heightMap->m_width  + x ] = n;
		}
}


// path to image file
TerrainPtr Terrain::create( base::Path filename )
{
	return TerrainPtr( new Terrain(filename) );
}




float Terrain::getHeight( int x, int y )
{
	if( x < 0 )
		x = 0;
	if( x >= m_width )
		x = m_width-1;
	if( y < 0 )
		y = 0;
	if( y >= m_height )
		y = m_height-1;

	float height = m_heightMap[y*m_width + x];

	return height;
}


math::Vec3f Terrain::getNormal( int x, int y )
{
	if( x < 0 )
		return math::Vec3f( 0.0f, 1.0f, 0.0f );
	if( x >= m_width )
		return math::Vec3f( 0.0f, 1.0f, 0.0f );
	if( y < 0 )
		return math::Vec3f( 0.0f, 1.0f, 0.0f );
	if( y >= m_height )
		return math::Vec3f( 0.0f, 1.0f, 0.0f );

	math::Vec3f n = m_normalMap[ y*m_width  + x ];

	return n;
}

math::Vec3f Terrain::getNormal( float u, float v )
{
	// 
	float _x = u*m_width - 0.5f;
	float _y = v*m_height - 0.5f;

	float tx = _x - floor(_x);
	float ty = _y - floor(_y);
	int x = (int)floor(_x);
	int y = (int)floor(_y);

	math::Vec3f a, b, c, d;
	a = getNormal( x, y );
	b = getNormal( x+1, y );
	c = getNormal( x, y+1 );
	d = getNormal( x+1, y+1 );

	math::Vec3f n = math::slerp( math::slerp( a, b, tx ), math::slerp( b, c, tx ), ty );

	return a;
}


float Terrain::getHeight( float u, float v )
{
	// bilinear interpolation
	float _x = u*m_width - 0.5f;
	float _y = v*m_height - 0.5f;

	float tx = _x - floor(_x);
	float ty = _y - floor(_y);
	int x = (int)floor(_x);
	int y = (int)floor(_y);

	float a, b, c, d;
	a = getHeight( x, y );
	b = getHeight( x+1, y );
	c = getHeight( x, y+1 );
	d = getHeight( x+1, y+1 );

	float height = math::lerp( math::lerp( a, b, tx ), math::lerp( c, d, tx ), ty );

	return height;
}

// creates a geometry from sampling given heightmap
base::GeometryPtr Terrain::createGeometry( base::Path filename, int xres, int yres )
{
	TerrainPtr terrain = create( filename );
	// get a grid...
	base::GeometryPtr geo = base::geo_grid( xres, yres );
	// get points
	base::AttributePtr pAttr = geo->getAttr( "P" );
	int numElements = pAttr->numElements();
	// update points heights from heightmap
	for( int i=0;i<numElements;++i )
		{
			math::Vec3f p = pAttr->get<math::Vec3f>(i);
			p.y = terrain->getHeight( p.x+0.5f, p.z+0.5f );
			pAttr->set<math::Vec3f>(i, p);
		}
	base::apply_normals( geo );

	return geo;
}


