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
#include <gfx/glsl/noise.h>


BASE_DECL_SMARTPTR_STRUCT(SurfaceClouds);
struct SurfaceClouds
{


	SurfaceClouds()
	{
		m_geo = base::geo_grid( 250, 250 );
		base::apply_transform( m_geo, math::Matrix44f::ScaleMatrix( 30000.0f ) );
		base::apply_normals( m_geo );

		m_shader = base::Shader::load(base::Path( SRC_PATH ) + "/src/SurfaceClouds.vs.glsl", base::Path( SRC_PATH ) + "/src/SurfaceClouds.ps.glsl").attachPS( base::glsl::noiseSrc() ).attachVS( base::glsl::noiseSrc() );
	}



	base::GeometryPtr        m_geo;
	base::ShaderPtr       m_shader;



};