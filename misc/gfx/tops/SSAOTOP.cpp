#include "SSAOTOP.h"

#include "../FBO.h"


SSAOTOP::SSAOTOP() : out0(0)
{
	// create setup
	ssaoCompute = new SSAOComputeTOP();
	ssaoBlur = new SSAOBlurTOP();
}

void SSAOTOP::setInputs( Texture *normal_depth )
{
	ssaoCompute->setInputs( normal_depth );
	ssaoBlur->setInputs( ssaoCompute->out0 );
	out0 = ssaoBlur->out0;
	//out0 = ssaoCompute->out0;
}


void SSAOTOP::render( float time )
{
	ssaoCompute->render();
	ssaoBlur->render();
}
