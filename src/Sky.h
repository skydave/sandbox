#pragma once

#include <util/unordered_map.h>
#include <util/shared_ptr.h>
#include <gfx/Shader.h>
#include <gfx/Geometry.h>
#include <gfx/Texture.h>
#include <gfx/FBO.h>
#include <gfx/Camera.h>





BASE_DECL_SMARTPTR_STRUCT(Sky);
struct Sky
{
	Sky();

	static SkyPtr                          create();
	void              render( base::CameraPtr cam );



	base::GeometryPtr                m_domeGeometry;
	base::Texture2dPtr               m_starsTexture;
	base::ShaderPtr               m_starrySkyShader;


};
