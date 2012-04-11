#include "TestEffect.h"

#include <gfx/Context.h>


PointLight::PointLight()
{
	m_cam = base::CameraPtr( new base::Camera() );
	m_cam->m_znear = 1.0f;
	m_cam->m_zfar  = 10.0f;
	m_cam->m_aspectRatio = 1.0f;
	m_shadowMap = base::Texture2d::createRGBAFloat32();
	m_shadowFBO = base::FBO::create().width(512).height(512).attach(m_shadowMap);
}

void PointLight::setPos( const math::Vec3f &pos )
{
	m_pos = pos;
	update();
}

void PointLight::setLookAt( const math::Vec3f &lookAt )
{
	m_lookAt = lookAt;
	update();
}

void PointLight::update()
{
	m_cam->m_transform = math::createLookAtMatrix( m_pos, m_lookAt, math::Vec3f(0.0f,1.0f,0.0f), false );
	m_cam->update();
}



void TestEffect::init()
{
	locator = base::geo_sphere(10,10,0.1f);

	// setup light0 =============
	light0 = PointLightPtr( new PointLight() );
	light0->setPos(math::Vec3f( 0.0f, 6.0f, 3.0f ));
	light0->setLookAt(math::Vec3f( 0.0f, 0.0f, 0.0f ));
	light0->update();

	// setup shadow pass =============
	m_depthMapShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/TestEffect.simpleDepth" );

	// setup scene geometry ==========
	m_plane = base::geo_grid(30,30);
	base::apply_transform( m_plane, math::Matrix44f::ScaleMatrix(10.0f) );
	base::apply_normals(m_plane);
	m_geometry = base::Geometry::createReferenceMesh();

	m_geometryShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/TestEffect.simpleGeometry" );
	m_geometryShader->setUniform( "l", math::Vec3f( 1.0f, 1.0f, 1.0f ).normalized() );
	m_geometryShader->setUniform( "depthMap0", light0->m_shadowMap->getUniform() );


	// initial setup ==============
	setKa( 0.01f );
	setKd( 1.0f );
	setKs( 1.0f );
}

void TestEffect::render(base::CameraPtr cam )
{
	base::ContextPtr context = base::Context::current();

	glEnable( GL_DEPTH_TEST );

	// render shadowmap pass ==========
	light0->m_shadowFBO->begin();

	context->setCamera( light0->m_cam );

	//render scene geometry here
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
}



void TestEffect::reload()
{
	m_geometryShader->reload();
	m_depthMapShader->reload();
}

void TestEffect::setKa( float ka )
{
	m_geometryShader->setUniform( "ka", ka );
}

float TestEffect::getKa()
{
	return m_geometryShader->getUniform( "ka" )->get<float>(0);
}

void TestEffect::setKd( float kd )
{
	m_geometryShader->setUniform( "kd", kd );
}

float TestEffect::getKd()
{
	return m_geometryShader->getUniform( "kd" )->get<float>(0);
}


void TestEffect::setKs( float ks )
{
	m_geometryShader->setUniform( "ks", ks );
}

float TestEffect::getKs()
{
	return m_geometryShader->getUniform( "ks" )->get<float>(0);
}
