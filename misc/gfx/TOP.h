//
// basicly a more abstract FBO wrapper
//
#pragma once



#include "Geometry.h"
#include "Shader.h"
#include "Texture.h"


struct FBO;

extern Geometry *g_screenQuad;


extern char vs_nop[];
extern int vs_nop_size;

//
//
//
struct TOP
{
	TOP( int xres, int yres );



	void setOutputs( Texture *_out0, Texture *_out1 = 0, Texture *_out2 = 0, Texture *_out3 = 0 );
	void setSize( int newXres, int newYres ); // will change the size of the fbo and all the output textures to match given resolution




	FBO               *m_fbo;
	int       m_xres, m_yres;
	Texture *out0;
	Texture *out1;
	Texture *out2;
	Texture *out3;
};
