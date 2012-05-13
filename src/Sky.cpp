#include "Sky.h"

#include <gfx/Context.h>





SkyPtr Sky::create()
{
	return SkyPtr( new Sky() );
}


Sky::Sky()
{
	// dome geometry
	m_domeGeometry = base::geo_sphere( 30, 30, 1.0f );
	m_domeGeometry->reverse();
	base::apply_normals( m_domeGeometry );

	// starry sky shader/texture
	m_starrySkyShader = base::Shader::load( base::Path( SRC_PATH ) + "src/Sky.cylindrical" );
	m_starsTexture = base::Texture2d::load( base::Path( SRC_PATH ) + "data/stars1.jpg" );
	m_starrySkyShader->setUniform( "texture", m_starsTexture->getUniform() );
}


void Sky::render( base::CameraPtr cam )
{
	base::ContextPtr context = base::Context::current();
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	context->setView( cam->m_viewMatrix.getOrientation(), cam->m_transform.getOrientation(), cam->m_projectionMatrix );
	glDisable( GL_DEPTH_TEST );
	glDepthMask(false);
	context->render( m_domeGeometry, m_starrySkyShader );
	glDepthMask(true);
	glEnable( GL_DEPTH_TEST );

	// TODO: introduce context::transformstate
	context->setCamera( cam );


}