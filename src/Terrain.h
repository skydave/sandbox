//
// very simple utility class for working with heightmaps
//
#pragma once
#include <util/shared_ptr.h>
#include <util/Path.h>
#include <gfx/Image.h>
#include <gfx/Geometry.h>



BASE_DECL_SMARTPTR_STRUCT(Terrain);
struct Terrain
{
	std::vector<float>                                                                  m_heightMap;
	std::vector<math::Vec3f>                                                            m_normalMap;
	int                                                                                     m_width;
	int                                                                                    m_height;


	Terrain( base::Path filename );                                                                  // constructor
	static TerrainPtr                                                 create( base::Path filename ); // path to image file
	static base::GeometryPtr              createGeometry( base::Path filename, int xres, int yres );







	

	float                                                                 getHeight( int x, int y );
	float                                                             getHeight( float u, float v ); // linear interpolated
	math::Vec3f                                                           getNormal( int x, int y );
	math::Vec3f                                                       getNormal( float u, float v ); // linear interpolated

};