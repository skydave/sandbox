


#include "DOFCombineTOP.h"

#include "../FBO.h"


DOFCombineTOP::DOFCombineTOP() : TOP( 32, 32 )
{
	shader = new Shader();
	shader->init(vs_nop, vs_nop_size, DOFCombineTOP_ps, DOFCombineTOP_ps_size);
	shader->finalize();

	pixelStepAttr = new Attribute(2);
	pixelStepAttr->appendElement( 1.0f/32.0f, 1.0f/32.0f );
	shader->setUniform( "pixelStep", pixelStepAttr );
}

void DOFCombineTOP::setInputs( Texture *source, Texture *normal_depth, Texture *blurMed, Texture *blurLarge_coc, Texture *blurredCoC, Texture *blurredDepth, Attribute *focalLength, Attribute *fallOffStart, Attribute *fallOffEnd )
{
	// if we dont have an output by now we create one ourselfs
	if( !out0 )
		setOutputs( source->copy() );

	// adobt the size of the output
	TOP::setSize( out0->m_xres, out0->m_yres );

	shader->setUniform( "original", source->getUniform() );
	shader->setUniform( "normal_depth", normal_depth->getUniform() );
	shader->setUniform( "blurMed", blurMed->getUniform() );
	shader->setUniform( "blurLarge_coc", blurLarge_coc->getUniform() );
	shader->setUniform( "blurredCoC", blurredCoC->getUniform() );
	shader->setUniform( "blurredDepth", blurredDepth->getUniform() );
	shader->setUniform( "focalLength", focalLength );
	shader->setUniform( "fallOffStart", fallOffStart );
	shader->setUniform( "fallOffEnd", fallOffEnd );

	float pixelStep[2];
	pixelStep[0] = 1.0f/m_xres;
	pixelStep[1] = 1.0f/m_yres;
	pixelStepAttr->setElement( 0, pixelStep );
}


void DOFCombineTOP::render( float time )
{
	m_fbo->begin();
	g_screenQuad->render(shader);
	m_fbo->end();
}






