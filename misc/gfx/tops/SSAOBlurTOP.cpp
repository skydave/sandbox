#include "SSAOBlurTOP.h"

#include "../FBO.h"


SSAOBlurTOP::SSAOBlurTOP() : TOP( 32, 32 )
{
	ssaoBlurShader = new Shader();
	ssaoBlurShader->init(vs_nop, vs_nop_size, SSAOBlurTOP_ps, SSAOBlurTOP_ps_size);
	ssaoBlurShader->finalize();
}

void SSAOBlurTOP::setInputs( Texture *ao )
{
	// if we dont have an output by now we create one ourselfs
	if( !out0 )
		setOutputs( ao->copy() );

	// adobt the size of the input
	TOP::setSize( ao->m_xres, ao->m_yres );

	// set input with the shader
	ssaoBlurShader->setUniform( "input", ao->getUniform() );
}


void SSAOBlurTOP::render( float time )
{
	m_fbo->begin();
	g_screenQuad->render(ssaoBlurShader);
	m_fbo->end();
}
