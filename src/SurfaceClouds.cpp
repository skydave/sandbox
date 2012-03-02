#include "SurfaceClouds.h"





SurfaceClouds::SurfaceClouds()
{
	m_geo = base::geo_grid( 250, 250 );
	base::apply_transform( m_geo, math::Matrix44f::ScaleMatrix( 30000.0f ) );
	base::apply_normals( m_geo );

	m_shader = base::Shader::load(base::Path( SRC_PATH ) + "/src/SurfaceClouds.vs.glsl", base::Path( SRC_PATH ) + "/src/SurfaceClouds.ps.glsl").attachPS( base::glsl::noiseSrc() ).attachVS( base::glsl::noiseSrc() );


	// initial setup
	setSunDir( math::Vec3f( 1.0f, 1.0f, 1.0f ) );
}



void SurfaceClouds::setSunDir( math::Vec3f sunDir )
{
	m_shader->setUniform( "sunDir", sunDir.normalized() );
	m_sunDir = sunDir;
}