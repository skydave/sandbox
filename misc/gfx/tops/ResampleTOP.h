//
//
//
#pragma once

#include "../TOP.h"



extern char ResampleTOP_ps[];
extern int ResampleTOP_ps_size;

//
//
//
struct ResampleTOP : TOP
{
	ResampleTOP();

	void setInputs( Texture *tex, int newXres, int newYres );

	void render( float time = 0.0f );

	Shader *shader;
};
