#include "DOFBlurTOP.h"

#include "../FBO.h"


DOFBlurTOP::DOFBlurTOP() : out0(0)
{
	// create setup
	blur_horizontal = new DOFBlur1DTOP(DOFBlur1DTOP::Horizontal);
	blur_vertical = new DOFBlur1DTOP(DOFBlur1DTOP::Vertical);
}

void DOFBlurTOP::setOutputs( Texture *out0 )
{
	blur_horizontal->setOutputs( out0->copy() );
	blur_vertical->setOutputs( out0 );

	//blur_horizontal->setOutputs( out0 );
}

void DOFBlurTOP::setInputs( Texture *tex, Texture *coc )
{
	blur_horizontal->setInputs( tex, coc );
	blur_vertical->setInputs( blur_horizontal->out0, coc );
	out0 = blur_vertical->out0;
	//out0 = blur_horizontal->out0;
}


void DOFBlurTOP::render( float time )
{
	blur_horizontal->render();
	blur_vertical->render();
}
