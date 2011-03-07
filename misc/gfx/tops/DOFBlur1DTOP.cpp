#include "DOFBlur1DTOP.h"

#include "../FBO.h"


DOFBlur1DTOP::DOFBlur1DTOP( Direction dir ) : TOP( 32, 32 ), m_direction(dir)
{
	shader = new Shader();
	pixelStep = new Attribute(1);

	if( m_direction == Horizontal )
	{
		shader->init(vs_nop, vs_nop_size, DOFBlur1DTOP_horizontal_ps, DOFBlur1DTOP_horizontal_ps_size);
		pixelStep->appendElement( 1.0f/m_xres ); // use x or y depending on direction of blur
	}else
	{
		shader->init(vs_nop, vs_nop_size, DOFBlur1DTOP_vertical_ps, DOFBlur1DTOP_vertical_ps_size);
		pixelStep->appendElement( 1.0f/m_yres ); // use x or y depending on direction of blur
	}

	shader->finalize();
	shader->setUniform( "pixelStep", pixelStep );
}

void DOFBlur1DTOP::setInputs( Texture *tex, Texture *coc )
{
	// if we dont have an output by now we create one ourselfs
	if( !out0 )
		setOutputs( tex->copy() );

	// adobt the size of the output
	TOP::setSize( out0->m_xres, out0->m_yres );

	// set pixelstep value in the shader which is used to work on pixels
	float tmp;

	if( m_direction == Horizontal )
		tmp = 1.0f/ out0->m_xres;  // use x or y depending on direction of blur
	else
		tmp = 1.0f/ out0->m_yres;  // use x or y depending on direction of blur

	pixelStep->setElement( 0, &tmp );


	// set input with the shader
	shader->setUniform( "input", tex->getUniform() );
	shader->setUniform( "coc", coc->getUniform() );
}


void DOFBlur1DTOP::render( float time )
{
	m_fbo->begin(false);
	glDisable( GL_DEPTH_TEST );
	g_screenQuad->render(shader);
	m_fbo->end();
}
