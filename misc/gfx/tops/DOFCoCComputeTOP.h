//
//
//
#pragma once

#include "../TOP.h"


extern char DOFCoCComputeTOP_ps[];
extern int DOFCoCComputeTOP_ps_size;


//
//
//
struct DOFCoCComputeTOP : TOP
{

	DOFCoCComputeTOP();

	void setInputs( Texture *normal_depth, Attribute *focalLength, Attribute *fallOffStart, Attribute *fallOffEnd );

	void render( float time = 0.0f );

	Shader              *shader;
};
