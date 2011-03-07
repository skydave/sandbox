#include "ResampleTOP.h"

#include "../FBO.h"


ResampleTOP::ResampleTOP() : TOP( 32, 32 )
{
	shader = new Shader();
	shader->init(vs_nop, vs_nop_size, ResampleTOP_ps, ResampleTOP_ps_size);
	shader->finalize();
}

void ResampleTOP::setInputs( Texture *tex, int newXres, int newYres )
{
	// if we dont have an output by now we create one ourselfs
	if( !out0 )
		setOutputs( tex->copy() );

	// adobt the size of the input
	TOP::setSize( newXres, newYres );

	shader->setUniform( "input", tex->getUniform() );
}


void ResampleTOP::render( float time )
{
	m_fbo->begin();
	g_screenQuad->render(shader);
	m_fbo->end();
}
