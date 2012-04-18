
#include "Volume.h"
#include <util/fs.h>
#include <util/tuple.h>
#include <gfx/glsl/noise.h>
#include <iostream>
#include <fstream>
#include <sstream>


bool planeSegmentIntersection( const math::Vec3f& planeN, float planeDist,
                               const math::Vec3f& start, const math::Vec3f& end,
                               float& t )
{
    const math::Vec3f dir = end - start;
    const math::Vec3f pPoint = -planeN * planeDist;
	float denom = math::dotProduct( dir, planeN );
	if ( fabs(denom) < 1e-4f ) return false;
    t = math::dotProduct( planeN, pPoint - start ) / denom;
    return t >= 0.f && t <= 1.f;
}


inline void planePathIntersection( math::Vec3f &isect, size_t &nPoints, math::Vec3f planeN, float planeDist, const math::Vec3f &path0, const math::Vec3f &path1, const math::Vec3f &otherwise )
 {
	float t;
	if ( planeSegmentIntersection( planeN, planeDist, path0, path1, t ) )
	{
		isect = path0 + ( path1 - path0 ) * t;
		nPoints++;
	}
	else
	{
		isect = otherwise;
	}
 }

inline void planePathIntersection( math::Vec3f &isect, size_t &nPoints, math::Vec3f planeN, float planeDist, math::Vec3f path0, math::Vec3f path1, math::Vec3f path2, math::Vec3f path3 )
{
	float t;
	if ( planeSegmentIntersection( planeN, planeDist, path0, path1, t ) )
	{
		isect = path0 + ( path1 - path0 ) * t;
		nPoints++;
	} else if ( planeSegmentIntersection( planeN, planeDist, path1, path2, t ) )
	{
		isect = path1 + ( path2 - path1 ) * t;
		nPoints++;
	} else if ( planeSegmentIntersection( planeN, planeDist, path2, path3, t ) )
	{
		isect = path2 + ( path3 - path2 ) * t;
		nPoints++;
	};
}


// http://www.cg.informatik.uni-siegen.de/data/Publications/2005/rezksalamaVMV2005.pdf
size_t computeAABBPlaneIntersectionGeometry( const math::Vec3f& bbMin, const math::Vec3f& bbMax,
											 const math::Vec3f& planeN, float planeDist,
											 math::Vec3f *outVertices, math::Vec3f *outUVW )
{
    math::Vec3f bbPoints[ 8 ] = { math::Vec3f( bbMax.x, bbMax.y, bbMax.z ),
                        math::Vec3f( bbMax.x, bbMax.y, bbMin.z ),
                        math::Vec3f( bbMax.x, bbMin.y, bbMax.z ),
                        math::Vec3f( bbMin.x, bbMax.y, bbMax.z ),
                        math::Vec3f( bbMin.x, bbMax.y, bbMin.z ),
                        math::Vec3f( bbMax.x, bbMin.y, bbMin.z ),
						math::Vec3f( bbMin.x, bbMin.y, bbMax.z ),
                        math::Vec3f( bbMin.x, bbMin.y, bbMin.z ) };

    // precomputed vertex sorting by distance for each point being the closest
    static int vertexSequence[ 8 * 8 ] = { 0, 1, 2, 3, 4, 5, 6, 7,
                                           1, 4, 5, 0, 3, 7, 2, 6,
                                           2, 6, 0, 5, 7, 3, 1, 4,
                                           3, 0, 6, 4, 1, 2, 7, 5,
                                           4, 3, 7, 1, 0, 6, 5, 2,
                                           5, 2, 1, 7, 6, 0, 4, 3,
                                           6, 7, 3, 2, 5, 4, 0, 1,
                                           7, 5, 4, 6, 2, 1, 3, 0 };

	// find closest point
    int closestVert = 0;
	float closestDist = std::numeric_limits<float>::max();
	float farthestDist = -std::numeric_limits<float>::max();
    for( int i = 0; i < 8; i++ )
    {
        const math::Vec3f& p = bbPoints[ i ];
        float dist = p.x * planeN.x + p.y * planeN.y + p.z * planeN.z + planeDist;
        if ( dist < closestDist )
        {
            closestDist = dist;
            closestVert = i;
        }
        farthestDist = std::max( farthestDist, dist );
    }

    if ( closestDist > 0 || farthestDist < 0 )
    {
        // there's no intersection with the plane
        return false;
    }

    size_t nPoints = 0;

	// bring points into the right order by using LUT
    const math::Vec3f& p0 = bbPoints[ vertexSequence[ 8 * closestVert + 0 ] ];
    const math::Vec3f& p1 = bbPoints[ vertexSequence[ 8 * closestVert + 1 ] ];
    const math::Vec3f& p2 = bbPoints[ vertexSequence[ 8 * closestVert + 2 ] ];
    const math::Vec3f& p3 = bbPoints[ vertexSequence[ 8 * closestVert + 3 ] ];
    const math::Vec3f& p4 = bbPoints[ vertexSequence[ 8 * closestVert + 4 ] ];
    const math::Vec3f& p5 = bbPoints[ vertexSequence[ 8 * closestVert + 5 ] ];
    const math::Vec3f& p6 = bbPoints[ vertexSequence[ 8 * closestVert + 6 ] ];
    const math::Vec3f& p7 = bbPoints[ vertexSequence[ 8 * closestVert + 7 ] ];

    planePathIntersection(outVertices[0], nPoints, planeN, planeDist, p0, p1, p4, p7 );   // i0
    planePathIntersection(outVertices[2], nPoints, planeN, planeDist, p0, p2, p5, p7 );   // i2
    planePathIntersection(outVertices[4], nPoints, planeN, planeDist, p0, p3, p6, p7 );   // i4
    planePathIntersection(outVertices[1], nPoints, planeN, planeDist, p1, p5, outVertices[0] ); // i1
    planePathIntersection(outVertices[3], nPoints, planeN, planeDist, p2, p6, outVertices[2] ); // i3
    planePathIntersection(outVertices[5], nPoints, planeN, planeDist, p3, p4, outVertices[4] ); // i5

	// compute texture coords
    for( size_t i = 0; i < 6; i++ )
    {
		const math::Vec3f& p = outVertices[i];
		outUVW[i] = math::Vec3f(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f);
    }
    return nPoints;
}

Volume::Volume()
{
}

void Volume::initialize( const base::Path _dataSourceGLSLPath )
{
	bool usesInternalVoxelGrid = false;
	base::Path dataSourceGLSLPath = _dataSourceGLSLPath;
	if(!dataSourceGLSLPath.IsValid())
	{
		usesInternalVoxelGrid = true;
		dataSourceGLSLPath = base::Path( SRC_PATH ) + "src/Volume.voxelgrid.glsl";
	}

	// bound geometry --
	m_proxy = createProxyGeometry();


	// near clip geo --
	nearClipGeo = base::Geometry::createPolyGeometry();
	nearClipP = base::Attribute::createVec3f(6);
	nearClipUVW = base::Attribute::createVec3f(6);
	nearClipGeo->setAttr( "P", nearClipP );
	nearClipGeo->setAttr( "UVW", nearClipUVW );
	nearClipGeo->addPolygonVertex(5);
	nearClipGeo->addPolygonVertex(4);
	nearClipGeo->addPolygonVertex(3);
	nearClipGeo->addPolygonVertex(2);
	nearClipGeo->addPolygonVertex(1);
	nearClipGeo->addPolygonVertex(0);



	math::Matrix44f voxelToWorld = math::Matrix44f::TranslationMatrix( -0.5f, -0.5f, -0.5f ); // move from 0-1 space to -.5 to -.5 space
	voxelToWorldAttr = base::Attribute::createMat44();
	voxelToWorldAttr->appendElement( voxelToWorld );

	math::Matrix44f worldToVoxel = voxelToWorld.inverted();
	worldToVoxelAttr = base::Attribute::createMat44();
	worldToVoxelAttr->appendElement( worldToVoxel );

	volumeBack = base::Texture2d::createRGBAFloat16( 512, 512 );
	volumeFront = base::Texture2d::createRGBAFloat16( 512, 512 );

	// setup offscreen render pass --
	volumeFrontFBO = base::FBOPtr( new base::FBO( 512, 512 ) );
	volumeFrontFBO->setOutputs( volumeFront );
	volumeBackFBO = base::FBOPtr( new base::FBO( 512, 512 ) );
	volumeBackFBO->setOutputs( volumeBack );

	// volume density (we will fall back to this one if no data source module has been provided)
	if( usesInternalVoxelGrid )
	{
		usesInternalVoxelGrid = true;

		density = base::Texture3d::createFloat32(64, 64, 64	);

		math::PerlinNoise pn;
		pn.setFrequency( 1.5f );
		pn.setDepth(3);

		float *densityBuffer_f32 = (float*)malloc( density->m_xres*density->m_yres*density->m_zres*sizeof(float) );
		for( int k = 0; k<density->m_zres;++k )
			for( int j = 0; j<density->m_yres;++j )
				for( int i = 0; i<density->m_xres;++i )
				{
					//densityBuffer_f32[k*density->m_xres*density->m_yres + j*density->m_xres + i] = ((float)j/(float)density->m_yres);
					math::Vec3f p( ((float)i/(float)density->m_xres), ((float)j/(float)density->m_yres), ((float)k/(float)density->m_zres) );
					densityBuffer_f32[k*density->m_xres*density->m_yres + j*density->m_xres + i] = pn.perlinNoise_3D( p.x, p.y, p.z )*100.0f;
				}
		density->uploadFloat32( density->m_xres, density->m_yres, density->m_zres, densityBuffer_f32 );
		free(densityBuffer_f32);
	}


	// generate noise texture which is used in the final raymarcher to offset start positions. this actually improves the quality alot as it
	// reduces the ugly banding
	{
		int noiseRes = 512;
		int numNoiseValues = noiseRes * noiseRes;
		noise = base::Texture2d::createRGBAFloat16( noiseRes, noiseRes );

		float *noiseBuffer_rgbaf32 = (float*)malloc( numNoiseValues*4*sizeof(float) );
		for( int i = 0; i < numNoiseValues; ++i)
		{
			const float noise1 = (float)rand() / RAND_MAX;
			const float noise2 = (float)rand() / RAND_MAX;
			const float noise3 = (float)rand() / RAND_MAX;
			const float noise4 = (float)rand() / RAND_MAX;
			noiseBuffer_rgbaf32[ 4 * i + 0 ] = noise1;
			noiseBuffer_rgbaf32[ 4 * i + 1 ] = noise2;
			noiseBuffer_rgbaf32[ 4 * i + 2 ] = noise3;
			noiseBuffer_rgbaf32[ 4 * i + 3 ] = noise4;
		}

		noise->uploadRGBAFloat32( noise->m_xres, noise->m_yres, noiseBuffer_rgbaf32 );

		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

		free(noiseBuffer_rgbaf32);
	}

	volumeBackShader = base::Shader::load( base::Path( SRC_PATH ) + "src/Volume.back.vs.glsl", base::Path( SRC_PATH ) + "src/Volume.back.ps.glsl" );
	volumeFrontShader = base::Shader::load( base::Path( SRC_PATH ) + "src/Volume.front.vs.glsl", base::Path( SRC_PATH ) + "src/Volume.front.ps.glsl" );
	volumeShader = base::Shader::load( base::Path( SRC_PATH ) + "src/Volume.vs.glsl", base::Path( SRC_PATH ) + "src/Volume.ps.glsl" ).attachPS(dataSourceGLSLPath).attachPS(base::glsl::noiseSrc());
	volumeShader->setUniform( "volumeFront", volumeFront->getUniform() );
	volumeShader->setUniform( "volumeBack", volumeBack->getUniform() );
	volumeShader->setUniform( "voxelToWorld", voxelToWorldAttr );
	volumeShader->setUniform( "noiseTex", noise->getUniform() );

	dctCompute = base::Shader::load( base::Path( SRC_PATH ) + "src/Volume.dctcompute.vs.glsl", base::Path( SRC_PATH ) + "src/Volume.dctcompute.ps.glsl" ).attachPS(dataSourceGLSLPath).attachPS(base::glsl::noiseSrc());
	dctCompute->setUniform( "voxelToWorld", voxelToWorldAttr );
	dctCompute->setUniform( "worldToVoxel", worldToVoxelAttr );
	dctCompute->setUniform( "totalCrossSection", m_totalCrossSection );

	// setup light0
	light0 = base::CameraPtr( new base::Camera() );
	light0->m_aspectRatio = 1.0;
	//light0->m_transform = math::Matrix44f::RotationMatrixY( math::degToRad(-90.0f) )*math::Matrix44f::TranslationMatrix( 2.0, 0.0, 0.0f);
	light0->m_transform = math::Matrix44f::RotationMatrixX( math::degToRad(45.0f) )*math::Matrix44f::RotationMatrixY( math::degToRad(-90.0f) )*math::Matrix44f::TranslationMatrix( 1.2f, 1.2f, 0.0f);
	light0->update();

	dctMap0 = base::Texture2dArray::createRGBAFloat16( 512, 512, 2 );


	float *testBuffer_rgbaf32 = (float*)malloc( dctMap0->m_xres*dctMap0->m_yres*dctMap0->m_zres*sizeof(float)*4 );
	for( int k = 0; k<dctMap0->m_zres;++k )
		for( int j = 0; j<dctMap0->m_yres;++j )
			for( int i = 0; i<dctMap0->m_xres;++i )
			{
				math::Vec3f p( ((float)i/(float)dctMap0->m_xres), ((float)j/(float)dctMap0->m_yres), ((float)k/(float)dctMap0->m_zres) );
				testBuffer_rgbaf32[k*dctMap0->m_xres*dctMap0->m_yres*4 + j*dctMap0->m_xres*4 + i*4 + 0] = 0.0f;
				testBuffer_rgbaf32[k*dctMap0->m_xres*dctMap0->m_yres*4 + j*dctMap0->m_xres*4 + i*4 + 1] = 0.0f;
				testBuffer_rgbaf32[k*dctMap0->m_xres*dctMap0->m_yres*4 + j*dctMap0->m_xres*4 + i*4 + 2] = 0.0f;
				testBuffer_rgbaf32[k*dctMap0->m_xres*dctMap0->m_yres*4 + j*dctMap0->m_xres*4 + i*4 + 3] = 1.0f;
			}
	dctMap0->uploadRGBAFloat32( dctMap0->m_xres, dctMap0->m_yres, dctMap0->m_zres, testBuffer_rgbaf32 );




	dctMap0FBO = base::FBOPtr( new base::FBO( 512, 512 ) );
	dctMap0FBO->setOutputs( dctMap0 );
	dctMap0FBO->finalize();

	volumeShader->setUniform( "coeffTex", dctMap0->getUniform() );



	if( usesInternalVoxelGrid )
	{
		volumeShader->setUniform( "density", density->getUniform() );
		dctCompute->setUniform( "density", density->getUniform() );
	}




	// set defaults
	setTotalCrossSection( 20.0f );
	setAlbedo( 1.0f );
	setAbsorptionColor( math::Vec3f(0.5f,0.5f, 0.5f) );
	setScatteringColor(math::Vec3f(0.5f, 0.5f, 0.5f));

	Light l;
	l.color = math::Vec3f(1.0,1.0,1.0);
	l.exposure = 0.0f;
	setLight(0, l);
}

base::GeometryPtr Volume::createProxyGeometry()
{
	base::GeometryPtr result = base::GeometryPtr(new base::Geometry(base::Geometry::QUAD));

	// unique vertex data
	std::vector<math::Vec3f> pos;
	pos.push_back( math::Vec3f(-.5f,-.5f,0.5f) );
	pos.push_back( math::Vec3f(-.5f,0.5f,0.5f) );
	pos.push_back( math::Vec3f(.5f,.5f,0.5f) );
	pos.push_back( math::Vec3f(.5f,-.5f,0.5f) );

	pos.push_back( math::Vec3f(-.5f,-.5f,-0.5f) );
	pos.push_back( math::Vec3f(-.5f,0.5f,-0.5f) );
	pos.push_back( math::Vec3f(.5f,.5f,-0.5f) );
	pos.push_back( math::Vec3f(.5f,-.5f,-0.5f) );

	std::vector<math::Vec3f> uvw;
	uvw.push_back( math::Vec3f(0.0f,0.0f,1.0f) );
	uvw.push_back( math::Vec3f(0.0f,1.0f,1.0f) );
	uvw.push_back( math::Vec3f(1.0f,1.0f,1.0f) );
	uvw.push_back( math::Vec3f(1.0f,0.0f,1.0f) );

	uvw.push_back( math::Vec3f(0.0f,0.0f,0.0f) );
	uvw.push_back( math::Vec3f(0.0f,1.0f,0.0f) );
	uvw.push_back( math::Vec3f(1.0f,1.0f,0.0f) );
	uvw.push_back( math::Vec3f(1.0f,0.0f,0.0f) );

	// quads
	std::vector< std::tuple<int, int, int, int> > quads;
	quads.push_back( std::make_tuple(3, 2, 1, 0) );
	quads.push_back( std::make_tuple(4, 5, 6, 7) );
	quads.push_back( std::make_tuple(7, 6, 2, 3) );
	quads.push_back( std::make_tuple(1, 5, 4, 0) );
	quads.push_back( std::make_tuple(6, 5, 1, 2) );
	quads.push_back( std::make_tuple(4, 7, 3, 0) );

	// split per face (because we have uv shells)
	base::AttributePtr positions = base::Attribute::createVec3f();
	base::AttributePtr uvwAttr = base::Attribute::createVec3f();
	base::AttributePtr uvAttr = base::Attribute::createVec2f();


	for( std::vector< std::tuple<int, int, int, int> >::iterator it = quads.begin(); it != quads.end(); ++it )
	{
		std::tuple<int, int, int, int> &quad = *it;
		int i0, i1, i2, i3;

		i0 = positions->appendElement( pos[std::get<0>(quad)] );
		uvwAttr->appendElement( uvw[std::get<0>(quad)] );
		uvAttr->appendElement( math::Vec2f(0.0f, 0.0f) );
		i1 = positions->appendElement( pos[std::get<1>(quad)] );
		uvwAttr->appendElement( uvw[std::get<1>(quad)] );
		uvAttr->appendElement( math::Vec2f(1.0f, 0.0f) );
		i2 = positions->appendElement( pos[std::get<2>(quad)] );
		uvwAttr->appendElement( uvw[std::get<2>(quad)] );
		uvAttr->appendElement( math::Vec2f(1.0f, 1.0f) );
		i3 = positions->appendElement( pos[std::get<3>(quad)] );
		uvwAttr->appendElement( uvw[std::get<3>(quad)] );
		uvAttr->appendElement( math::Vec2f(0.0f, 1.0f) );

		result->addQuad(i0, i1, i2, i3);			
	}

	result->setAttr( "P", positions);
	result->setAttr( "UVW", uvwAttr);
	result->setAttr( "UV", uvAttr);

	return result;
}


void Volume::render( base::CameraPtr cam )
{
	base::ContextPtr context = base::Context::current();

	glEnable( GL_CULL_FACE );

	// render shadowmap pass ======================================================
	context->setView( light0->m_viewMatrix, light0->m_transform, light0->m_projectionMatrix );

	dctCompute->setUniform( "wsLightPos", light0->m_transform.getTranslation() );
	dctMap0FBO->begin(0);
	context->render( m_proxy, dctCompute );
	dctMap0FBO->end();


	// render primary pass =========================================================
	context->setView( cam->m_viewMatrix, cam->m_transform, cam->m_projectionMatrix );

	// render back faces
	volumeBackFBO->begin();
	glFrontFace( GL_CW );
	context->render( m_proxy, volumeBackShader );
	volumeBackFBO->end();

	// render front faces
	volumeFrontFBO->begin();
	glFrontFace( GL_CCW );
	context->render( m_proxy, volumeFrontShader );

	// computer polygon from intersection of near clipping plane with bounding box
	// calculate the near clip plane in local space of the model
	const float clipEpsilon = 0.001f;
	math::Matrix44f modelViewInverse = context->getModelViewInverse();
    math::Vec3f nearClipPlaneN;
	float nearClipPlaneDist;
    {
        math::Vec3f vsOrigin( 0, 0, 0 );
		math::Vec3f vsClipPlanePoint( 0, 0, -(cam->m_znear + clipEpsilon) );
		math::Vec3f wsOrigin = math::transform(vsOrigin, modelViewInverse);
        math::Vec3f wsNearClipPlanePoint = math::transform(vsClipPlanePoint, modelViewInverse);

        nearClipPlaneN = ( wsNearClipPlanePoint - wsOrigin ).normalized();
        nearClipPlaneDist = -math::dotProduct( nearClipPlaneN, wsNearClipPlanePoint );
    }

	size_t npoints = computeAABBPlaneIntersectionGeometry(  math::Vec3f( -0.5f, -0.5f, -0.5f ), math::Vec3f( 0.5f,  0.5f,  0.5f ),
		nearClipPlaneN, nearClipPlaneDist, (math::Vec3f*)nearClipP->getRawPointer(), (math::Vec3f*)nearClipUVW->getRawPointer() );

	nearClipP->m_isDirty = true;
	nearClipUVW->m_isDirty = true;

	if( npoints )
		context->render( nearClipGeo, volumeFrontShader );


	volumeFrontFBO->end();


	// render volume ====================
	// setup volume shader
	volumeShader->setUniform( "wsLightPos", light0->m_transform.getTranslation() );

	math::Matrix44f worldLightProj = light0->m_viewMatrix * light0->m_projectionMatrix;
	volumeShader->setUniform( "worldToLightProj", worldLightProj );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	context->renderScreen( volumeShader );
	glDisable( GL_BLEND );
}

VolumePtr Volume::create()
{
	VolumePtr v = VolumePtr( new Volume() );
	v->initialize();
	return v;
}

// create volume with custom data source glsl module (file be be loaded from base::fs)
VolumePtr Volume::create( const base::Path dataSourceGLSLPath )
{
	VolumePtr v = VolumePtr( new Volume() );
	v->initialize(dataSourceGLSLPath);
	return v;
}

// reloads the shaders (for development
void Volume::reload()
{
	volumeShader->reload();
	dctCompute->reload();
}

// to allow external code to communicate with glsldatasource module
void Volume::setUniform( const std::string &name, base::AttributePtr uniform )
{
	volumeShader->setUniform(name, uniform);
	dctCompute->setUniform(name, uniform);
}

void Volume::updateCrossSectionValues()
{
	// update absorption and scattering crosssections accordingly
	float scatteringCrossSection = m_albedo*m_totalCrossSection;
	float absorptionCrossSection = (1.0f-m_albedo)*m_totalCrossSection;
	math::Vec3f finalTotalCrossSection = scatteringCrossSection*(m_scatteringColor) + absorptionCrossSection*(1.0f - m_absorptionColor);
	volumeShader->setUniform( "totalCrossSection", finalTotalCrossSection );
	dctCompute->setUniform( "totalCrossSection", m_totalCrossSection );
	volumeShader->setUniform( "scatteringCrossSection", scatteringCrossSection*(m_scatteringColor) );
}

void Volume::setTotalCrossSection( float totalCrossSection )
{
	m_totalCrossSection = totalCrossSection;
	updateCrossSectionValues();
}

float Volume::getTotalCrossSection( void )
{
	return m_totalCrossSection;
}

void Volume::setAlbedo( float albedo )
{
	m_albedo = albedo;
	updateCrossSectionValues();
}
float Volume::getAlbedo( void )
{
	return m_albedo;
}

void Volume::setAbsorptionColor( math::Vec3f absorptionColor )
{
	m_absorptionColor = absorptionColor;
	updateCrossSectionValues();
}
math::Vec3f Volume::getAbsorptionColor( void )
{
	return m_absorptionColor;
}

void Volume::setScatteringColor( math::Vec3f scatteringColor )
{
	m_scatteringColor = scatteringColor;
	updateCrossSectionValues();
}
math::Vec3f Volume::getScatteringColor( void )
{
	return m_scatteringColor;
}

Light Volume::getLight( int index )
{
	return m_light0Info;
}

void Volume::setLight( int index, Light light )
{
	m_light0Info = light;
	volumeShader->setUniform( "lightColor", m_light0Info.color*pow(2.0f, m_light0Info.exposure) );
}
