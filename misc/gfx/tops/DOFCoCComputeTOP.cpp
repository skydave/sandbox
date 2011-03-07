#include "DOFCoCComputeTOP.h"

#include "../FBO.h"


DOFCoCComputeTOP::DOFCoCComputeTOP() : TOP( 32, 32 )
{
	shader = new Shader();
	shader->init(vs_nop, vs_nop_size, DOFCoCComputeTOP_ps, DOFCoCComputeTOP_ps_size);
	shader->finalize();
}

void DOFCoCComputeTOP::setInputs( Texture *normal_depth, Attribute *focalLength, Attribute *fallOffStart, Attribute *fallOffEnd )
{
	// if we dont have an output by now we create one ourselfs
	if( !out0 )
		//setOutputs( normal_depth->copy() );
		setOutputs( Texture::createRGBA8( normal_depth->m_xres/4, normal_depth->m_yres/4 ) );

	// adobt the size of the input
	TOP::setSize( normal_depth->m_xres/4, normal_depth->m_yres/4 );


	// set input with the shader
	shader->setUniform( "normal_depth", normal_depth->getUniform() );
	shader->setUniform( "focalLength", focalLength );
	shader->setUniform( "fallOffStart", fallOffStart );
	shader->setUniform( "fallOffEnd", fallOffEnd );
}


void DOFCoCComputeTOP::render( float time )
{
	m_fbo->begin();
	g_screenQuad->render(shader);
	m_fbo->end();
}

