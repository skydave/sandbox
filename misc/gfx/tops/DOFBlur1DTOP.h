//
//
//
#pragma once

#include "../TOP.h"


extern char DOFBlur1DTOP_horizontal_ps[];
extern int DOFBlur1DTOP_horizontal_ps_size;

extern char DOFBlur1DTOP_vertical_ps[];
extern int DOFBlur1DTOP_vertical_ps_size;

//
//
//
struct DOFBlur1DTOP : TOP
{
	enum Direction
	{
		Horizontal,
		Vertical
	};
	DOFBlur1DTOP( Direction dir = Horizontal );

	void setInputs( Texture *tex, Texture *coc );

	void render( float time = 0.0f );

	Shader        *shader;
	Attribute  *pixelStep;
	Direction m_direction;
};
