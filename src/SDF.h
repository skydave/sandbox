#pragma once
#include <math/Math.h>
#include <vector>







// DOMAIN ===================================================
struct Domain
{
	Domain();
	Domain( math::Vec3f min, math::Vec3f max, int _width, int _height, int _depth );

	math::BoundingBox3d bound;
	int  width, height, depth;
};

// distanceTransform ===================================================

typedef float (*SDF3D)( const math::Vec3f &);
template<typename T>
void distanceTransform( const Domain &domain, std::vector<float> &outGrid, T sdf )
{
	math::Vec3f size = domain.bound.size();

	outGrid.clear();
	outGrid.resize( domain.width*domain.height*domain.depth );
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

				outGrid[ k*domain.width*domain.height + j*domain.width + i ] = sdf( p );
			}
}


// distance ============================================================

