//
//
//
#pragma once

#include "../TOP.h"


extern char SSAOComputeTOP_ps[];
extern int SSAOComputeTOP_ps_size;
extern char SSAOComputeTOP_vs[];
extern int SSAOComputeTOP_vs_size;

//
//
//
struct SSAOComputeTOP : TOP
{
	SSAOComputeTOP();

	void setInputs( Texture *normal_depth );

	void render( float time = 0.0f );

	Shader *ssaoComputeShader;
};
