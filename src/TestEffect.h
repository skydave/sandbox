//
// TestEffect renders plane and a mesh with phongshading and shadows
//
#pragma once

#include <gfx/Shader.h>
#include <gfx/Camera.h>
#include <gfx/Geometry.h>
#include <gfx/Texture.h>
#include <gfx/FBO.h>
#include <util/shared_ptr.h>



BASE_DECL_SMARTPTR_STRUCT(PointLight);
struct PointLight
{
	PointLight();
	void             setPos( const math::Vec3f &pos );
	void       setLookAt( const math::Vec3f &lookAt );
	void                                     update();

	base::FBOPtr                          m_shadowFBO; // fbo used to render depth map
	base::Texture2dPtr                    m_shadowMap; // the depth map
	base::CameraPtr                             m_cam; // light pov
	math::Vec3f                                 m_pos; // defines light cam
	math::Vec3f                              m_lookAt; // defines light cam
};


BASE_DECL_SMARTPTR_STRUCT(TestEffect);
// TestEffect renders plane and a mesh with phongshading and shadows
struct TestEffect
{
	void                           init(); // initializes the effect
	void     render(base::CameraPtr cam ); // renders the effect

	void                         reload(); // reloads shaders

	void                setKa( float ka );
	float                         getKa();
	void                setKd( float kd );
	float                         getKd();
	void                setKs( float ks );
	float                         getKs();
private:
	base::ShaderPtr      m_geometryShader; // used in primary path. shader uses light info
	base::ShaderPtr      m_depthMapShader; // used to render scene geometry in shadow pass
	PointLightPtr                  light0; // contains light info

	base::GeometryPtr             locator; // used to display light location


	// hardcoded scene geometry
	base::GeometryPtr          m_geometry; 
	base::GeometryPtr             m_plane;
};