//
// performs color grading on an input texture
//
#pragma once

#include <gfx/Shader.h>
#include <gfx/Camera.h>
#include <gfx/Geometry.h>
#include <gfx/Texture.h>
#include <gfx/FBO.h>
#include <util/shared_ptr.h>


BASE_DECL_SMARTPTR_STRUCT(ColorGrading);
struct ColorGrading
{
	void                                            init(); // initializes the effect
	void            setInput( base::Texture2dPtr texture );
	void                      render(base::CameraPtr cam ); // renders the effect

	void                                          reload(); // reloads shaders

private:
	base::FBOPtr                                     m_fbo;
	base::ShaderPtr                               m_shader;
	base::Texture2dPtr                            m_output;
};