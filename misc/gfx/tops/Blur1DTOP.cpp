#include "Blur1DTOP.h"

#include "../FBO.h"


Blur1DTOP::Blur1DTOP( Direction dir ) : TOP( 32, 32 ), m_direction(dir)
{
	shader = new Shader();
	pixelStep = new Attribute(1);

	if( m_direction == Horizontal )
	{
		shader->init(vs_nop, vs_nop_size, Blur1DTOP_horizontal_ps, Blur1DTOP_horizontal_ps_size);
		pixelStep->appendElement( 1.0f/m_xres ); // use x or y depending on direction of blur
	}else
	{
		shader->init(vs_nop, vs_nop_size, Blur1DTOP_vertical_ps, Blur1DTOP_vertical_ps_size);
		pixelStep->appendElement( 1.0f/m_yres ); // use x or y depending on direction of blur
	}

	shader->finalize();
	shader->setUniform( "pixelStep", pixelStep );
}

void Blur1DTOP::setInputs( Texture *tex )
{
	// if we dont have an output by now we create one ourselfs
	if( !out0 )
		setOutputs( tex->copy() );

	// adobt the size of the input
	TOP::setSize( tex->m_xres, tex->m_yres );

	// set pixelstep value in the shader which is used to work on pixels
	float tmp;

	if( m_direction == Horizontal )
		tmp = 1.0f/ tex->m_xres;  // use x or y depending on direction of blur
	else
		tmp = 1.0f/ tex->m_yres;  // use x or y depending on direction of blur

	pixelStep->setElement( 0, &tmp );


	// set input with the shader
	shader->setUniform( "input", tex->getUniform() );
}


void Blur1DTOP::render( float time )
{
	m_fbo->begin();
	g_screenQuad->render(shader);
	m_fbo->end();
}
