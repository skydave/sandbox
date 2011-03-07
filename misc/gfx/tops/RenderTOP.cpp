#include "RenderTOP.h"

#include "../FBO.h"


RenderTOP::RenderTOP() : TOP( 32, 32 )
{
}

void RenderTOP::setInputs( int xres, int yres )
{
	// if we dont have an output by now we create one ourselfs
	if( !out0 )
		setOutputs( Texture::createRGBA8(xres, yres) );

	// adobt the size of the input
	TOP::setSize( xres, yres );

	// set rendercallback
	//renderCallback = _renderCallback;
}

/*
void RenderTOP::render( float time )
{
	m_fbo->begin();
	renderCallback->call(time);
	m_fbo->end();
}
*/

void RenderTOP::begin()
{
	m_fbo->begin();
}


void RenderTOP::end()
{
	m_fbo->end();
}