#pragma once

#include <gfx/Shader.h>
#include <util/shared_ptr.h>


BASE_DECL_SMARTPTR_STRUCT(ColorGrading);
struct ColorGrading
{
	base::ShaderPtr shader;
	base::ShaderPtr lightMapShader;


};