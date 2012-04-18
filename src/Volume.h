#pragma once

#include <util/shared_ptr.h>
#include <gltools/gl.h>
#include <gltools/misc.h>
#include <util/StringManip.h>
#include <util/Path.h>
#include <gfx/Geometry.h>
#include <gfx/Shader.h>
#include <gfx/Texture.h>
#include <gfx/Image.h>
#include <gfx/Context.h>
#include <gfx/FCurve.h>
#include <gfx/glsl/common.h>
#include <gfx/FBO.h>



struct Light
{
	math::Vec3f color;
	float    exposure;
};

BASE_DECL_SMARTPTR_STRUCT(Volume);
struct Volume
{
	static VolumePtr                                                 create();
	static VolumePtr            create( const base::Path dataSourceGLSLPath ); // create volume with custom data source glsl module (file be be loaded from base::fs)

	void                                        render( base::CameraPtr cam );
	void                                                             reload(); // reloads the shaders (for development)
	void    setUniform( const std::string &name, base::AttributePtr uniform ); // to allow external code to communicate with glsldatasource module

	void                      setTotalCrossSection( float totalCrossSection );
	float                                        getTotalCrossSection( void );
	void                                            setAlbedo( float albedo );
	float                                                   getAlbedo( void );
	void                    setAbsorptionColor( math::Vec3f absorptionColor );
	math::Vec3f                                    getAbsorptionColor( void );
	void                    setScatteringColor( math::Vec3f scatteringColor );
	math::Vec3f                                    getScatteringColor( void );
	Light                                               getLight( int index );
	void                                   setLight( int index, Light light );


private:
	Volume();
	void              initialize( const base::Path _dataSourceGLSLPath = "" );
	void                                           updateCrossSectionValues();
	base::GeometryPtr                                   createProxyGeometry();


	// local (cached) variables
	float                  m_totalCrossSection;
	float                             m_albedo;
	math::Vec3f              m_absorptionColor;
	math::Vec3f              m_scatteringColor;

	base::Texture2dPtr              volumeBack;
	base::Texture2dPtr             volumeFront;
	base::FBOPtr                volumeFrontFBO;
	base::FBOPtr                 volumeBackFBO;

	base::ShaderPtr           volumeBackShader;
	base::ShaderPtr          volumeFrontShader;

	base::Texture2dPtr                   noise; // TODO: potentially static

	base::ShaderPtr               volumeShader;

	base::AttributePtr        voxelToWorldAttr;
	base::AttributePtr        worldToVoxelAttr;

	base::GeometryPtr                  m_proxy; // proxy geometry
	base::GeometryPtr              nearClipGeo;
	base::AttributePtr               nearClipP;
	base::AttributePtr             nearClipUVW;

	base::ShaderPtr                 dctCompute;

	// light0
	base::CameraPtr                     light0;
	base::FBOPtr                    dctMap0FBO;
	base::Texture2dArrayPtr            dctMap0;

	Light                         m_light0Info;

	// will be used in case volume has to provide its own voxelgrid data
	base::Texture3dPtr                 density;
};
