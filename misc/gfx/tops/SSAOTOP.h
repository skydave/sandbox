//
//
//
#pragma once

#include "../TOP.h"

#include "SSAOComputeTOP.h"
#include "SSAOBlurTOP.h"


//
//
//
struct SSAOTOP
{
	SSAOTOP();

	void setInputs( Texture *normal_depth );

	void render( float time = 0.0f );


	SSAOComputeTOP *ssaoCompute;
	SSAOBlurTOP *ssaoBlur;

	Shader *ssaoBlurShader;


	Texture *out0;
};
