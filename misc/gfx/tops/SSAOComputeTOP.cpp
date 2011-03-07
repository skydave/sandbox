#include "SSAOComputeTOP.h"

#include "../FBO.h"


SSAOComputeTOP::SSAOComputeTOP() : TOP( 32, 32 )
{
	ssaoComputeShader = new Shader();
	ssaoComputeShader->init(SSAOComputeTOP_vs, SSAOComputeTOP_vs_size, SSAOComputeTOP_ps, SSAOComputeTOP_ps_size);
	ssaoComputeShader->finalize();


	int rndNormalsRes = 128;
	float *rndNormalsPix = (float *)mmalloc( sizeof(float) * 3 * rndNormalsRes * rndNormalsRes );
	for( int j=0; j<rndNormalsRes; ++j )
		for( int i=0; i<rndNormalsRes; ++i )
		{
			rndNormalsPix[j*rndNormalsRes*3 + i*3 + 0] = msys_frand()*2.0f - 1.0f;
			rndNormalsPix[j*rndNormalsRes*3 + i*3 + 1] = msys_frand()*2.0f - 1.0f;
			rndNormalsPix[j*rndNormalsRes*3 + i*3 + 2] = msys_frand()*2.0f - 1.0f;
		}
	Texture *rndNormals = new Texture( rndNormalsRes, rndNormalsRes, GL_RGB_FLOAT32_ATI, GL_RGB, GL_FLOAT, rndNormalsPix );

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	ssaoComputeShader->setUniform( "rndnorms", rndNormals->getUniform());
}

void SSAOComputeTOP::setInputs( Texture *normal_depth )
{
	// if we dont have an output by now we create one ourselfs
	if( !out0 )
		setOutputs( normal_depth->copy() );

	// adobt the size of the input
	TOP::setSize( normal_depth->m_xres, normal_depth->m_yres );

	// set input with the shader
	ssaoComputeShader->setUniform( "normal", normal_depth->getUniform() );
}


void SSAOComputeTOP::render( float time )
{
	m_fbo->begin();
	g_screenQuad->render(ssaoComputeShader);
	m_fbo->end();
}
