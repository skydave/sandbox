//
//
//
#pragma once

#include "../TOP.h"


extern char SSAOBlurTOP_ps[];
extern int SSAOBlurTOP_ps_size;

//
//
//
struct SSAOBlurTOP : TOP
{
	SSAOBlurTOP();

	void setInputs( Texture *ao );

	void render( float time = 0.0f );

	Shader *ssaoBlurShader;
};
