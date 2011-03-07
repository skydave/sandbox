//
//
//
#pragma once

#include "../TOP.h"


extern char DOFCombineTOP_ps[];
extern int DOFCombineTOP_ps_size;


//
//
//
struct DOFCombineTOP : TOP
{

	DOFCombineTOP();

	void setInputs( Texture *source, Texture *normal_depth, Texture *blurMed, Texture *blurLarge_coc, Texture *blurredCoC, Texture *blurredDepth, Attribute *focalLength, Attribute *fallOffStart, Attribute *fallOffEnd );

	void render( float time = 0.0f );

	Shader           *shader;
	Attribute *pixelStepAttr;
};
