#include "BlurTOP.h"

#include "../FBO.h"


BlurTOP::BlurTOP() : out0(0)
{
	// create setup
	blur_horizontal = new Blur1DTOP(Blur1DTOP::Horizontal);
	blur_vertical = new Blur1DTOP(Blur1DTOP::Vertical);
}

void BlurTOP::setInputs( Texture *tex )
{
	blur_horizontal->setInputs( tex );
	blur_vertical->setInputs( blur_horizontal->out0 );
	out0 = blur_vertical->out0;
}


void BlurTOP::render( float time )
{
	blur_horizontal->render();
	blur_vertical->render();
}
