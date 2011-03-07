//
//
//
#pragma once

#include "../TOP.h"

#include "Blur1DTOP.h"


//
//
//
struct BlurTOP
{
	BlurTOP();

	void setInputs( Texture *tex );

	void render( float time = 0.0f );

	Blur1DTOP *blur_horizontal;
	Blur1DTOP *blur_vertical;
	Texture *out0;
};
