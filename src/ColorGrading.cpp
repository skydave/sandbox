#include "ColorGrading.h"

#include <gfx/Context.h>




void ColorGrading::init()
{
	m_output = base::Texture2d::createRGBA8(512, 512);
	m_fbo = base::FBO::create().width(512).height(512).attach(m_output);
	m_shader = base::Shader::load( base::Path( SRC_PATH ) + "/src/ColorGrading" );
}

void ColorGrading::render(base::CameraPtr cam )
{
	base::ContextPtr context = base::Context::current();
	context->renderScreen( m_shader );
	/*

	glEnable( GL_DEPTH_TEST );

	//render pass
	context->render( m_geometry, m_depthMapShader );
	context->render( m_plane, m_depthMapShader );

	light0->m_shadowFBO->end();


	// render primary pass ============
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	context->setCamera( cam );

	math::Matrix44f viewToLight = context->getModelViewInverse() * light0->m_cam->m_viewMatrix * light0->m_cam->m_projectionMatrix;
	m_geometryShader->setUniform( "viewToLight0", viewToLight );
	m_geometryShader->setUniform( "lightPos0", context->worldToView(light0->m_cam->m_transform.getTranslation()) );

	//render scene geometry here
	context->render( m_geometry, m_geometryShader );
	context->render( m_plane, m_geometryShader );

	// render light locator (blue sphere)
	context->setModelMatrix( math::Matrix44f::TranslationMatrix( light0->m_cam->m_transform.getTranslation() ) );
	context->m_constantShader->setUniform( "color", math::Vec3f(0.0f,0.0f,1.0f) );
	context->render( locator, context->m_constantShader );
	context->setModelMatrix( math::Matrix44f::Identity() );
	*/
}

void ColorGrading::setInput( base::Texture2dPtr texture )
{
	m_shader->setUniform( "texture", texture->getUniform() );
}

// reloads shaders
void ColorGrading::reload()
{
	m_shader->reload();
}