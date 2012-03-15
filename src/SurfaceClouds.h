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


	SurfaceClouds();
	~SurfaceClouds();

	void                         setSunDir( math::Vec3f sunDir );
	void             setMaxVertexHeight( float maxVertexHeight );

	// (cached) parameters
	math::Vec3f                    m_sunDir;
	float                  m_maxVertexHeight;

	base::GeometryPtr                 m_geo;
	base::ShaderPtr                m_shader;
	base::FCurve            m_phaseFunction; // P_theta
	base::Texture2dPtr         m_parameters;
	float                 *m_parameters_tex; // we keep it to allow updating of parameters



};