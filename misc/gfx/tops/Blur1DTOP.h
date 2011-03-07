//
//
//
#pragma once

#include "../TOP.h"


extern char Blur1DTOP_horizontal_ps[];
extern int Blur1DTOP_horizontal_ps_size;

extern char Blur1DTOP_vertical_ps[];
extern int Blur1DTOP_vertical_ps_size;

//
//
//
struct Blur1DTOP : TOP
{
	enum Direction
	{
		Horizontal,
		Vertical
	};
	Blur1DTOP( Direction dir = Horizontal );

	void setInputs( Texture *tex );

	void render( float time = 0.0f );

	Shader        *shader;
	Attribute  *pixelStep;
	Direction m_direction;
};
