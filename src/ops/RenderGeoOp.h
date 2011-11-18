
#pragma once

#include <ops/Op.h>
#include <ops/Manager.h>
#include <gfx/Geometry.h>
#include <gfx/Shader.h>


//
// renders geometry
//
BASE_DECL_SMARTPTR(RenderGeoOp);
class RenderGeoOp : public base::ops::Op
{
public:
	RenderGeoOp() : base::ops::Op()
	{
	}

	virtual void execute()
	{
		// TODO: get shader
		// TODO: get geometry
		base::ops::Manager::context()->render( m_geo, m_shader );
	}

	static RenderGeoOpPtr create()
	{
		return RenderGeoOpPtr( new RenderGeoOp() );
	}

//private:
	base::GeometryPtr              m_geo;
	base::ShaderPtr             m_shader;
};
