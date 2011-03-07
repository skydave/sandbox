//
//
//
#pragma once

#include "../TOP.h"

#include "DOFBlur1DTOP.h"


//
//
//
struct DOFBlurTOP
{
	DOFBlurTOP();

	void setOutputs( Texture *out0 );
	void setInputs( Texture *tex, Texture *coc );

	void render( float time = 0.0f );

	DOFBlur1DTOP *blur_horizontal;
	DOFBlur1DTOP *blur_vertical;

	Texture *out0;
};
